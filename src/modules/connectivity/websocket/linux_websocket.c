/*
 *
 * Copyright 2017 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/eventfd.h>
#include <openssl/ssl.h>
#include <libwebsockets.h>
#include <errno.h>
#include <regex.h>

#include <artik_log.h>
#include <artik_module.h>
#include <artik_loop.h>
#include <artik_security.h>
#include <artik_websocket.h>
#include "os_websocket.h"

#define WAIT_CONNECT_POLLING_MS		500
#define FLAG_EVENT			(0x1 << 0)
#define MAX(a, b)			((a > b) ? a : b)
#define CB_CONTAINER			((os_websocket_container *)\
					lws_get_protocol(wsi)->user)
#define CB_FDS				(((os_websocket_fds *)\
					CB_CONTAINER->fds)->fdset)
#define NUM_FDS				4
#define FD_CLOSE			0
#define FD_CONNECT			1
#define FD_RECEIVE			2
#define FD_ERROR			3
#define MAX_QUEUE_NAME			1024
#define MAX_QUEUE_SIZE			128
#define MAX_MESSAGE_SIZE		2048
#define PROCESS_TIMEOUT_MS		10
#define ARTIK_WEBSOCKET_INTERFACE	((os_websocket_interface *)\
					config->private_data)
#define ARTIK_WEBSOCKET_PROTOCOL_NAME	"artik-websocket"
#define SSL_ALERT_FATAL			!strcmp(SSL_alert_type_string_long\
					(ret), "fatal")
#define UNKNOWN_CA			!strcmp(SSL_alert_desc_string_long\
					(ret), "unknown CA")
#define BAD_CERTIFICATE			!strcmp(SSL_alert_desc_string_long\
					(ret), "bad certificate")
#define HANDSHAKE_FAILURE		!strcmp(SSL_alert_desc_string_long\
					(ret), "handshake failure")

typedef struct {
	int fdset[NUM_FDS];
} os_websocket_fds;

typedef struct {
	char *send_message;
	int send_message_len;
	char *receive_message;
	os_websocket_fds *fds;
} os_websocket_container;

typedef struct {
	int watch_id;
	int fd;
	artik_websocket_callback callback;
	void *user_data;
	artik_loop_module *loop;
} os_websocket_data;

typedef struct os_websocket_security_data_t {
	artik_security_module *security;
	artik_security_handle sec_handle;
} os_websocket_security_data;

typedef struct {
	struct lws_context *context;
	struct lws *wsi;
	SSL_CTX *ssl_ctx;
	int loop_process_id;
	os_websocket_security_data *sec_data;
	os_websocket_container container;
	os_websocket_data data[NUM_FDS];
	bool error_connect;
} os_websocket_interface;

typedef struct {
	artik_list node;
	os_websocket_interface interface;
} websocket_node;

static artik_list *requested_node = NULL;

static const struct lws_extension exts[] = {
	{
		"permessage-deflate",
		lws_extension_callback_pm_deflate,
		"permessage-deflate; client_max_window_bits"
	},
	{
		"deflate-frame",
		lws_extension_callback_pm_deflate,
		"deflate_frame"
	},
	{ NULL, NULL, NULL /* terminator */ }
};

static int lws_callback(struct lws *wsi, enum lws_callback_reasons reason,
			void *user, void *in, size_t len);

static void ssl_ctx_info_callback(const SSL *ssl, int where, int ret);

void lws_cleanup(artik_websocket_config *config)
{
	if (config->private_data == NULL) {
		log_err("Cleaning unopened session");
		return;
	}
	void *protocol = (void *)lws_get_protocol(
						ARTIK_WEBSOCKET_INTERFACE->wsi);

	log_dbg("");

	/* Release security data and OpenSSL Engine */
	if (ARTIK_WEBSOCKET_INTERFACE->sec_data) {
		artik_security_module *security = (artik_security_module *)
					artik_request_api_module("security");
		security->release(
			ARTIK_WEBSOCKET_INTERFACE->sec_data->sec_handle);
		artik_release_api_module(security);

		free(ARTIK_WEBSOCKET_INTERFACE->sec_data);
	}

	artik_loop_module *loop = (artik_loop_module *)
					artik_request_api_module("loop");

	loop->remove_fd_watch(
			ARTIK_WEBSOCKET_INTERFACE->data[FD_CLOSE].watch_id);
	loop->remove_fd_watch(
			ARTIK_WEBSOCKET_INTERFACE->data[FD_CONNECT].watch_id);
	loop->remove_fd_watch(
			ARTIK_WEBSOCKET_INTERFACE->data[FD_RECEIVE].watch_id);
	loop->remove_fd_watch(
			ARTIK_WEBSOCKET_INTERFACE->data[FD_ERROR].watch_id);
	loop->remove_idle_callback(ARTIK_WEBSOCKET_INTERFACE->loop_process_id);
	artik_release_api_module(loop);

	/* Destroy context in libwebsockets API */
	lws_context_destroy(ARTIK_WEBSOCKET_INTERFACE->context);

	/* Free variables in ARTIK API */
	close(ARTIK_WEBSOCKET_INTERFACE->container.fds->fdset[FD_CLOSE]);
	close(ARTIK_WEBSOCKET_INTERFACE->container.fds->fdset[FD_CONNECT]);
	close(ARTIK_WEBSOCKET_INTERFACE->container.fds->fdset[FD_RECEIVE]);
	close(ARTIK_WEBSOCKET_INTERFACE->container.fds->fdset[FD_ERROR]);
	free(ARTIK_WEBSOCKET_INTERFACE->container.fds);
	free(protocol);

	/* Free OpenSSL context */
	SSL_CTX_free(ARTIK_WEBSOCKET_INTERFACE->ssl_ctx);

	/* Finalize freeing process */
	free(config->private_data);
	config->private_data = NULL;
}

int lws_callback(struct lws *wsi, enum lws_callback_reasons reason,
					void *user, void *in, size_t len)
{
	uint64_t event_setter = FLAG_EVENT;
	char *received = NULL;
	websocket_node *node = (websocket_node *)artik_list_get_by_handle(
					requested_node, (ARTIK_LIST_HANDLE)wsi);

	switch (reason) {

	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		log_dbg("LWS_CALLBACK_CLIENT_ESTABLISHED");
		if (write(CB_FDS[FD_CONNECT], &event_setter,
						sizeof(event_setter)) < 0)
			log_err("Failed to set connect event");
		break;

	case LWS_CALLBACK_CLIENT_WRITEABLE:
		log_dbg("LWS_CALLBACK_CLIENT_WRITEABLE");
		if (CB_CONTAINER->send_message) {
			lws_write(wsi, (unsigned char *)
				CB_CONTAINER->send_message,
				CB_CONTAINER->send_message_len, LWS_WRITE_TEXT);
			free(CB_CONTAINER->send_message -
						LWS_SEND_BUFFER_PRE_PADDING);
			CB_CONTAINER->send_message = NULL;
		}
		log_dbg("");
		break;

	case LWS_CALLBACK_CLIENT_RECEIVE:
		received = strndup((char *)in, strlen((char *)in) + 1);
		if (!received) {
			log_err("Failed to allocate memory");
			break;
		}
		CB_CONTAINER->receive_message = received;

		if (write(CB_FDS[FD_RECEIVE], &event_setter,
						sizeof(event_setter)) < 0)
			log_err("Failed to set connect event");
		break;

	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		log_dbg("LWS_CALLBACK_CLIENT_CONNECTION_ERROR");
		if (write(CB_FDS[FD_CLOSE], &event_setter,
						sizeof(event_setter)) < 0)
			log_err("Failed to set close event");
		break;

	case LWS_CALLBACK_CLOSED:
		log_dbg("LWS_CALLBACK_CLOSED");
		if (write(CB_FDS[FD_CLOSE], &event_setter,
						sizeof(event_setter)) < 0)
			log_err("Failed to set close event");
		break;

	case LWS_CALLBACK_WSI_DESTROY:
		log_dbg("LWS_CALLBACK_WSI_DESTROY");
		node->interface.error_connect = true;
		if (write(CB_FDS[FD_CLOSE], &event_setter,
						sizeof(event_setter)) < 0)
			log_err("Failed to set close event");
		break;

	case LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED:
		log_err("LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED: %s",
							(const char *)in);
		break;

	default:
		break;
	}

	return 0;
}

void ssl_ctx_info_callback(const SSL *ssl, int where, int ret)
{
	const char *str;
	int w;
	uint64_t event_setter = FLAG_EVENT;
	SSL_CTX *ssl_ctx = SSL_get_SSL_CTX(ssl);
	struct lws *wsi = (struct lws *)SSL_CTX_get_ex_data(ssl_ctx, 0);

	w = where & ~SSL_ST_MASK;

	if (w & SSL_ST_CONNECT)
		str = "SSL_connect";
	else if (w & SSL_ST_ACCEPT)
		str = "SSL_accept";
	else
		str = "undefined";

	if (where & SSL_CB_ALERT) {
		str = (where & SSL_CB_READ) ? "read" : "write";
		log_dbg("SSL Alert %s:%s:%s", str,
			SSL_alert_type_string_long(ret),
			SSL_alert_desc_string_long(ret));

		if (SSL_ALERT_FATAL && (UNKNOWN_CA || BAD_CERTIFICATE ||
							HANDSHAKE_FAILURE)) {
			if (write(CB_FDS[FD_ERROR], &event_setter,
						sizeof(event_setter)) < 0)
				log_err("Failed to set close event : %d",
					errno);
		}
	} else if (where & SSL_CB_EXIT) {
		if (ret == 0)
			log_err("%s:failed in %s",
				str, SSL_state_string_long(ssl));
	} else if (where & SSL_CB_HANDSHAKE_DONE)
		log_dbg("%s", SSL_state_string_long(ssl));
}

SSL_CTX *setup_ssl_ctx(os_websocket_security_data **security_data,
			artik_ssl_config *ssl_config, char *host)
{
	artik_error ret = S_OK;
	SSL_CTX *ssl_ctx = NULL;

	const SSL_METHOD *method;
	char *raw_cert = NULL;
	char *raw_key = NULL;
	BIO *bio = NULL;
	X509 *x509_cert = NULL;
	EVP_PKEY *pk = NULL;
	X509_VERIFY_PARAM *param = NULL;

	log_dbg("");

	/* Initialize OpenSSL library */
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();

	method = (SSL_METHOD *)SSLv23_client_method();
	if (method == NULL) {
		log_err("problem creating ssl method\n");
		return NULL;
	}

	/* Create an SSL Context */
	ssl_ctx = SSL_CTX_new(method);
	if (ssl_ctx == NULL) {
		log_err("problem creating ssl context\n");
		goto exit;
	}

	/* Set options for TLS */
	SSL_CTX_set_options(ssl_ctx, SSL_OP_NO_COMPRESSION);
	SSL_CTX_set_options(ssl_ctx, SSL_OP_CIPHER_SERVER_PREFERENCE);
	SSL_CTX_set_info_callback(ssl_ctx, ssl_ctx_info_callback);

	if (ssl_config->verify_cert == ARTIK_SSL_VERIFY_REQUIRED) {
		param = SSL_CTX_get0_param(ssl_ctx);
		X509_VERIFY_PARAM_set1_host(param, host, 0);
		SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER, NULL);
	} else{
		SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_NONE, NULL);
		SSL_CTX_set_default_verify_paths(ssl_ctx);
		SSL_CTX_load_verify_locations(ssl_ctx, "/etc/pki/tls/cert.pem",
									NULL);
	}

	if (ssl_config->ca_cert.data && ssl_config->ca_cert.len &&
			ssl_config->verify_cert == ARTIK_SSL_VERIFY_REQUIRED) {
		X509_STORE *keystore = NULL;

		/* Convert CA certificate string into a BIO */
		bio = BIO_new(BIO_s_mem());
		BIO_write(bio, ssl_config->ca_cert.data,
				ssl_config->ca_cert.len);

		/* Extract X509 cert from the BIO */
		x509_cert = PEM_read_bio_X509(bio, NULL, NULL, NULL);
		if (!x509_cert) {
			log_err("Failed to extract cert from the bio");
			BIO_free(bio);
			ret = E_WEBSOCKET_ERROR;
			goto exit;
		}

		log_dbg("");

		BIO_free(bio);

		keystore = X509_STORE_new();

		if (!keystore) {
			log_err("Failed to load keystore");
			BIO_free(bio);
			ret = E_WEBSOCKET_ERROR;
			goto exit;
		}

		SSL_CTX_set_cert_store(ssl_ctx, keystore);

		/* Set CA certificate to context */
		if (!X509_STORE_add_cert(keystore, x509_cert)) {
			log_err("Failed add certificate to the keystore");
			ret = E_WEBSOCKET_ERROR;
			goto exit;
		}

		X509_free(x509_cert);
		x509_cert = NULL;
	}

	if (ssl_config->use_se == false) {

		log_dbg("");

		if (ssl_config->client_cert.data &&
						ssl_config->client_cert.len) {
			/* Convert certificate string into a BIO */
			bio = BIO_new(BIO_s_mem());
			BIO_write(bio, ssl_config->client_cert.data,
					ssl_config->client_cert.len);

			/* Extract X509 cert from the BIO */
			x509_cert = PEM_read_bio_X509(bio, NULL, NULL, NULL);
			if (!x509_cert) {
				BIO_free(bio);
				ret = E_WEBSOCKET_ERROR;
				goto exit;
			}

			log_dbg("");

			BIO_free(bio);

			/* Set certificate to context */
			if (!SSL_CTX_use_certificate(ssl_ctx, x509_cert)) {
				ret = E_WEBSOCKET_ERROR;
				goto exit;
			}
		}

		log_dbg("");

		if (ssl_config->client_key.data && ssl_config->client_key.len) {
			/* Convert key string into a BIO */
			bio = BIO_new(BIO_s_mem());
			if (!BIO_write(bio, ssl_config->client_key.data,
					ssl_config->client_key.len)) {
				BIO_free(bio);
				ret = E_WEBSOCKET_ERROR;
				goto exit;
			}

			log_dbg("");

			/* Extract EVP key from the BIO */
			pk = PEM_read_bio_PrivateKey(bio, NULL, 0, NULL);
			if (!pk) {
				BIO_free(bio);
				ret = E_WEBSOCKET_ERROR;
				goto exit;
			}

			BIO_free(bio);

			log_dbg("");

			/* Set private key to context */
			if (!SSL_CTX_use_PrivateKey(ssl_ctx, pk)) {
				ret = E_WEBSOCKET_ERROR;
				goto exit;
			}

			log_dbg("");

			/* Check certificate/key pair validity */
			if (!SSL_CTX_check_private_key(ssl_ctx)) {
				ret = E_WEBSOCKET_ERROR;
				goto exit;
			}
		}

		return ssl_ctx;
	}

	*security_data = malloc(sizeof(os_websocket_security_data));
	if (*security_data == NULL) {
		log_err("Failed to allocate memory");
		ret = E_NO_MEM;
		goto exit;
	}
	/* Processes related with SE are below */
	security_data[0]->security = (artik_security_module *)
					artik_request_api_module("security");

	/* Initialize ARTIK Security module */
	ret = security_data[0]->security->request(
						&security_data[0]->sec_handle);
	if (ret != S_OK) {
		log_err("Failed to request security module (err=%d)\n", ret);
		goto exit;
	}

	/* Get a certificate in SE as a string */
	ret = security_data[0]->security->get_certificate(
				security_data[0]->sec_handle, &raw_cert);
	if (ret != S_OK) {
		log_err("Failed to get certificate (err=%d)\n", ret);
		goto exit;
	}

	/* Get a public key from cert as a string */
	ret = security_data[0]->security->get_key_from_cert(
			security_data[0]->sec_handle, raw_cert, &raw_key);
	if (ret != S_OK) {
		log_err("Failed to get key (err=%d)\n", ret);
		goto exit;
	}

	/* Convert a certificate into X509 format and put it on SSL Context */
	bio = BIO_new(BIO_s_mem());
	BIO_puts(bio, raw_cert);
	BIO_write(bio, raw_cert, strlen(raw_cert));
	x509_cert = PEM_read_bio_X509(bio, NULL, NULL, NULL);

	BIO_free(bio);

	SSL_CTX_use_certificate(ssl_ctx, x509_cert);

	/* Convert key string into a BIO */
	bio = BIO_new(BIO_s_mem());
	if (!BIO_write(bio, raw_key, strlen(raw_key))) {
		BIO_free(bio);
		ret = E_WEBSOCKET_ERROR;
		goto exit;
	}

	/* Extract EVP key from the BIO */
	pk = PEM_read_bio_PrivateKey(bio, NULL, 0, NULL);
	if (pk == NULL) {
		BIO_free(bio);
		ret = E_WEBSOCKET_ERROR;
		goto exit;
	}
	BIO_free(bio);

	/* Set private key to context */
	if (!SSL_CTX_use_PrivateKey(ssl_ctx, pk)) {
		ret = E_WEBSOCKET_ERROR;
		goto exit;
	}

	/* Check certificate/key pair validity */
	if (!SSL_CTX_check_private_key(ssl_ctx)) {
		log_err("certificate/key pair validation failed\n");
		ret = E_WEBSOCKET_ERROR;
		goto exit;
	}

exit:
	/* Clean up allocated memories and handle */
	if (pk)
		EVP_PKEY_free(pk);
	if (x509_cert)
		X509_free(x509_cert);
	if (raw_cert)
		free(raw_cert);
	if (raw_key)
		free(raw_key);

	if (ret != S_OK) {
		if (ssl_ctx != NULL)
			SSL_CTX_free(ssl_ctx);
		return NULL;
	}

	return ssl_ctx;
}

static int os_websocket_process_stream(void *user_data)
{
	os_websocket_interface *interface = (os_websocket_interface *)user_data;
	int n;

	n = lws_service(interface->context, PROCESS_TIMEOUT_MS);
	if (n < 0) {
		log_err("os_websocket_process_stream");
		return 0;
	}

	return 1;
}

static int websocket_parse_uri(const char *uri, char **host, char **path,
		int *port, bool *use_tls)
{
	char *protocol = NULL;
	char *_port = NULL;
	char *_path = NULL;
	char *_host = NULL;

	int err = 0;
	int ret = 0;

	regex_t preg;

	log_dbg("uri = %s", uri);

	const char *str_request = uri;
	const char *str_regex = "^(ws[s]?)://([0-9A-Za-z.-]+)"\
				"(:[0-9]+)?(/[0-9A-Za-z.-_~!$&'()*+,;=:@]*)?$";

	err = regcomp(&preg, str_regex, REG_EXTENDED);

	if (err == 0) {
		int match;
		size_t nmatch = 0;
		regmatch_t *pmatch = NULL;

		nmatch = preg.re_nsub + 1;

		pmatch = malloc(sizeof(*pmatch) * nmatch);

		if (pmatch && nmatch == 5) {
			match = regexec(&preg, str_request, nmatch, pmatch, 0);

			regfree(&preg);

			if (match == 0) {

				int start = pmatch[1].rm_so;
				int end = pmatch[1].rm_eo;
				size_t size = end - start;

				protocol = malloc(sizeof(*protocol)*(size+1));

				if (protocol) {
					strncpy(protocol, &str_request[start],
						size);
					protocol[size] = '\0';

					log_dbg("protocol = %s", protocol);

					if (!strcmp(protocol, "wss"))
						*use_tls = true;
					else
						*use_tls = false;

					log_dbg("use_tls = %d", *use_tls);
				}

				start = pmatch[2].rm_so;
				end = pmatch[2].rm_eo;
				size = end - start;

				_host = malloc(sizeof(*_host)*(size+1));

				if (_host) {
					strncpy(_host, &str_request[start],
						size);
					_host[size] = '\0';
					*host = _host;

					log_dbg("host = %s", *host);
				}

				start = pmatch[3].rm_so;
				end = pmatch[3].rm_eo;
				size = end - start;

				_port = malloc(sizeof(*_port)*(size+1));

				if (_port && size != 0) {
					char *p = NULL;

					strncpy(_port, &str_request[start],
						size);
					_port[size] = '\0';

					p = strtok(_port, ":");

					if (p)
						*port = atoi(p);
				} else {
					if (*use_tls == true)
						*port = 443;
					else
						*port = 80;
				}

				log_dbg("port = %d", *port);

				start = pmatch[4].rm_so;
				end = pmatch[4].rm_eo;
				size = end - start;

				_path = malloc(sizeof(*_path)*(size+1));

				if (_path) {
					strncpy(_path, &str_request[start],
						size);
					_path[size] = '\0';

					if (strcmp(_path, "") == 0)
						*path = "/";
					else
						*path = _path;

					log_dbg("path = %s", *path);
				}
			} else if (match == REG_NOMATCH) {
				log_err("%s is not a valid websocket uri",
					str_request);
				ret = -1;
			} else {
				char *text;
				size_t size;

				size = regerror(err, &preg, NULL, 0);
				text = malloc(sizeof(*text) * size);

				if (text) {
					regerror(err, &preg, text, size);
					log_err("%s", text);
					free(text);
				} else {
					log_err("No enough memory");
					ret = -2;
				}
			}
		} else {
			log_err("No enough memory");
			ret = -3;
		}
	}

	return ret;
}

artik_error os_websocket_open_stream(artik_websocket_config *config)
{
	artik_error ret = S_OK;
	os_websocket_fds *fds;
	os_websocket_interface *interface;
	struct lws_context *context;
	struct lws_context_creation_info info;
	struct lws *wsi;
	struct lws_protocols *protocols;
	os_websocket_security_data *sec_data = NULL;
	artik_loop_module *loop = (artik_loop_module *)
					artik_request_api_module("loop");
	websocket_node *node;

	char *host = NULL;
	char *path = NULL;
	int port = 0;
	int len;
	bool use_tls = false;

	if (!config->uri) {
		log_err("Undefined uri");
		ret = E_WEBSOCKET_ERROR;
		goto exit;
	}

	if (websocket_parse_uri(config->uri, &host, &path, &port,
						&use_tls) < 0) {
		log_err("Failed to parse uri");
		ret = E_WEBSOCKET_ERROR;
		goto exit;
	}

	log_dbg("");

	interface = malloc(sizeof(os_websocket_interface));
	if (interface == NULL) {
		log_err("Failed to allocate memory");
		ret = E_NO_MEM;
		goto exit;
	}

	protocols = malloc(2 * sizeof(struct lws_protocols));
	if (protocols == NULL) {
		log_err("Failed to allocate memory");
		ret = E_NO_MEM;
		goto exit;
	}
	memset(protocols, 0, 2 * sizeof(struct lws_protocols));

	protocols[0].name = ARTIK_WEBSOCKET_PROTOCOL_NAME;
	protocols[0].callback = lws_callback;
	protocols[0].per_session_data_size = 0;
	protocols[0].rx_buffer_size = 4096;
	protocols[0].id = (uintptr_t)config;
	protocols[0].user = (void *)&interface->container;

	memset(&info, 0, sizeof(struct lws_context_creation_info));
	info.port = CONTEXT_PORT_NO_LISTEN;
	info.iface = NULL;
	info.protocols = protocols;
	info.gid = -1;
	info.uid = -1;

	info.provided_client_ssl_ctx = setup_ssl_ctx(&sec_data,
						&config->ssl_config, host);
	if (info.provided_client_ssl_ctx == NULL) {
		ret = E_WEBSOCKET_ERROR;
		goto exit;
	}

	lws_set_log_level(0, NULL);

	context = lws_create_context(&info);
	if (context == NULL) {
		log_err("Creating libwebsocket context failed");
		ret = E_WEBSOCKET_ERROR;
		goto exit;
	}

	/* Check if there is an enabled proxy */
	char *http_proxy = getenv("http_proxy");
	char *https_proxy = getenv("https_proxy");

	if (http_proxy || https_proxy) {

		char *proxy = NULL;
		char *str_regex = NULL;
		char *lws_proxy = NULL;

		regex_t preg;

		if (use_tls && https_proxy) {
			proxy = https_proxy;
			str_regex = strdup(
					"^https://([0-9A-Za-z.-]+):([0-9]+)$");
		} else if (!use_tls && http_proxy) {
			proxy = http_proxy;
			str_regex = strdup(
					"^http://([0-9A-Za-z.-]+):([0-9]+)$");
		}

		if (proxy && str_regex &&
			regcomp(&preg, str_regex, REG_EXTENDED) == 0) {

			int match;
			size_t nmatch = 0;
			regmatch_t *pmatch = NULL;

			nmatch = preg.re_nsub + 1;

			pmatch = malloc(sizeof(*pmatch) * nmatch);

			if (pmatch && nmatch == 3) {
				match = regexec(&preg, proxy, nmatch, pmatch,
									0);

				regfree(&preg);

				if (match == 0) {
					char *address = NULL;
					char *port = NULL;

					int start = pmatch[1].rm_so;
					int end = pmatch[1].rm_eo;
					size_t size = end - start;

					address = malloc(sizeof(*address)*
						(size+1));

					if (address) {
						strncpy(address, &proxy[start],
							size);
						address[size] = '\0';
					}

					start = pmatch[2].rm_so;
					end = pmatch[2].rm_eo;
					size = end - start;

					port = malloc(sizeof(*port)*(size+1));

					if (port) {
						strncpy(port,
							&proxy[start],
							size);
						port[size] = '\0';
					}

					if (address && port) {
						len  =
							strlen(address) + 1 +
							strlen(port) + 1;

						lws_proxy = (char *)malloc(len);

					}

					if (lws_proxy)
						snprintf(
						lws_proxy,
						len, "%s:%s",
						address, port);

					if (lws_proxy && lws_set_proxy(context,
						lws_proxy) != 0) {
						ret = E_WEBSOCKET_ERROR;
							goto exit;
					}

					if (lws_proxy)
						free(lws_proxy);

					if (address)
						free(address);

					if (port)
						free(port);

				} else {
					log_err("Wrong websocket proxy");
					ret = E_WEBSOCKET_ERROR;
					goto exit;
				}
			}
		}
	}

	struct lws_client_connect_info conn_info;

	memset(&conn_info, 0, sizeof(conn_info));

	char *hostport = NULL;

	if (host) {
		hostport = malloc(strlen(host) + 5 + 1);
		sprintf(hostport, "%.*s:%d", 256, host, port);
	}

	conn_info.context = context;
	conn_info.address = host ? host : "";
	conn_info.port = port;
	conn_info.path = path ? path : "";
	conn_info.host = hostport ? hostport : "";
	conn_info.origin = host ? host : "";
	conn_info.protocol = ARTIK_WEBSOCKET_PROTOCOL_NAME;
	conn_info.ietf_version_or_minus_one = -1;
	conn_info.client_exts = exts;

	if (use_tls) {
		switch (config->ssl_config.verify_cert) {
		case ARTIK_SSL_VERIFY_NONE:
		case ARTIK_SSL_VERIFY_OPTIONAL:
			conn_info.ssl_connection = 2;
			break;
		default:
			conn_info.ssl_connection = 1;
			break;
		}
	} else {
		conn_info.ssl_connection = 0;
	}

	wsi = lws_client_connect_via_info(&conn_info);
	if (wsi == NULL) {
		log_err("Connecting websocket failed");
		ret = E_WEBSOCKET_ERROR;
		goto exit;
	}

	free(hostport);

	fds = malloc(sizeof(*fds));
	if (fds == NULL) {
		log_err("Failed to allocate memory");
		ret = E_NO_MEM;
		goto exit;
	}
	memset(fds, 0, sizeof(*fds));

	fds->fdset[FD_CLOSE] = eventfd(0, 0);
	fds->fdset[FD_CONNECT] = eventfd(0, 0);
	fds->fdset[FD_RECEIVE] = eventfd(0, 0);
	fds->fdset[FD_ERROR] = eventfd(0, 0);


	interface->context = (void *)context;
	interface->wsi = (void *)wsi;
	interface->container.fds = (void *)fds;
	interface->ssl_ctx = info.provided_client_ssl_ctx;
	interface->sec_data = sec_data;
	interface->error_connect = false;

	node = (websocket_node *)artik_list_add(&requested_node,
				(ARTIK_LIST_HANDLE)wsi, sizeof(websocket_node));
	if (!node)
		return E_NO_MEM;

	memcpy(&node->interface, interface, sizeof(node->interface));

	SSL_CTX_set_ex_data(interface->ssl_ctx, 0, (void *)wsi);

	loop->add_idle_callback(&interface->loop_process_id,
				os_websocket_process_stream, (void *)interface);

	config->private_data = (void *)interface;
exit:
	return ret;
}

artik_error os_websocket_write_stream(artik_websocket_config *config,
							char *message, int len)
{
	artik_error ret = S_OK;
	char *websocket_buf = NULL;

	log_dbg("");

	websocket_node *node = (websocket_node *)artik_list_get_by_handle(
		requested_node, (ARTIK_LIST_HANDLE)
		ARTIK_WEBSOCKET_INTERFACE->wsi);

	if (node->interface.error_connect) {
		log_err("Impossible to write, no connection");
		ret = E_WEBSOCKET_ERROR;
		goto exit;
	}

	websocket_buf = malloc(LWS_SEND_BUFFER_PRE_PADDING + len +
						LWS_SEND_BUFFER_POST_PADDING);
	if (websocket_buf == NULL) {
		log_err("Failed to allocate memory");
		ret = E_NO_MEM;
		goto exit;
	}

	memcpy(websocket_buf + LWS_SEND_BUFFER_PRE_PADDING, message, len);
	ARTIK_WEBSOCKET_INTERFACE->container.send_message = websocket_buf +
						LWS_SEND_BUFFER_PRE_PADDING;
	ARTIK_WEBSOCKET_INTERFACE->container.send_message_len = len;
	lws_callback_on_writable(ARTIK_WEBSOCKET_INTERFACE->wsi);

exit:
	return ret;
}

int os_websocket_close_callback(int fd, enum watch_io io, void *user_data)
{
	uint64_t n = 0;
	os_websocket_data *data = (os_websocket_data *)user_data;

	log_dbg("");

	if (read(fd, &n, sizeof(uint64_t)) < 0) {
		log_err("close callback error");
		return 0;
	}

	data[FD_CLOSE].callback(data->user_data, (void *)
							ARTIK_WEBSOCKET_CLOSED);

	return 1;
}

int os_websocket_connection_callback(int fd, enum watch_io io, void *user_data)
{
	uint64_t n = 0;
	os_websocket_data *data = (os_websocket_data *)user_data;

	log_dbg("");

	if (read(fd, &n, sizeof(uint64_t)) < 0) {
		log_err("connection callback error");
		return 0;
	}

	data[FD_CONNECT].callback(data->user_data, (void *)
						ARTIK_WEBSOCKET_CONNECTED);

	return 1;
}

int os_websocket_error_callback(int fd, enum watch_io io, void *user_data)
{
	uint64_t n = 0;
	os_websocket_data *data = (os_websocket_data *)user_data;

	log_dbg("");

	if (read(fd, &n, sizeof(uint64_t)) < 0) {
		log_err("error callback error");
		return 0;
	}

	data[FD_ERROR].callback(data->user_data, (void *)
					ARTIK_WEBSOCKET_HANDSHAKE_ERROR);

	return 1;
}

artik_error os_websocket_set_connection_callback(artik_websocket_config *config,
			artik_websocket_callback callback, void *user_data)
{
	artik_error ret = S_OK;
	os_websocket_fds *fds = ARTIK_WEBSOCKET_INTERFACE->container.fds;
	artik_loop_module *loop = (artik_loop_module *)
					artik_request_api_module("loop");
	os_websocket_data *data = ((os_websocket_interface *)
						config->private_data)->data;

	log_dbg("");

	data[FD_CLOSE].callback = callback;
	data[FD_CLOSE].user_data = user_data;
	data[FD_CONNECT].callback = callback;
	data[FD_CONNECT].user_data = user_data;
	data[FD_ERROR].callback = callback;
	data[FD_ERROR].user_data = user_data;

	ret = loop->add_fd_watch(fds->fdset[FD_CLOSE], WATCH_IO_IN,
			os_websocket_close_callback, (void *)data,
			&data[FD_CLOSE].watch_id);

	if (ret != S_OK) {
		log_err("Failed to set fd watch close callback");
		goto exit;
	}

	ret = loop->add_fd_watch(fds->fdset[FD_CONNECT], WATCH_IO_IN,
			os_websocket_connection_callback, (void *)data,
			&data[FD_CONNECT].watch_id);

	if (ret != S_OK) {
		log_err("Failed to set fd watch connection callback");
		goto exit;
	}

	ret = loop->add_fd_watch(fds->fdset[FD_ERROR], WATCH_IO_IN,
			os_websocket_error_callback, (void *)data,
			&data[FD_ERROR].watch_id);

	if (ret != S_OK) {
		log_err("Failed to set fd watch error callback");
		goto exit;
	}

exit:
	return ret;
}

int os_websocket_receive_callback(int fd, enum watch_io io, void *user_data)
{
	uint64_t n = 0;
	artik_websocket_config *config = (artik_websocket_config *)user_data;
	os_websocket_data *data = ((os_websocket_interface *)
						config->private_data)->data;

	log_dbg("");

	if (read(fd, &n, sizeof(uint64_t)) < 0) {
		log_err("receive callback error");
		return 0;
	}

	if (ARTIK_WEBSOCKET_INTERFACE->container.receive_message == NULL) {
		log_err("Websocket: receive message failed");
		return 0;
	}

	data[FD_RECEIVE].callback(data[FD_RECEIVE].user_data, (void *)
		ARTIK_WEBSOCKET_INTERFACE->container.receive_message);

	return 1;
}

artik_error os_websocket_set_receive_callback(artik_websocket_config *config,
			artik_websocket_callback callback, void *user_data)
{
	artik_error ret = S_OK;
	os_websocket_fds *fds = ARTIK_WEBSOCKET_INTERFACE->container.fds;
	os_websocket_data *data = ((os_websocket_interface *)
						config->private_data)->data;
	artik_loop_module *loop = (artik_loop_module *)
					artik_request_api_module("loop");

	log_dbg("");

	data[FD_RECEIVE].callback = callback;
	data[FD_RECEIVE].user_data = user_data;

	ret = loop->add_fd_watch(fds->fdset[FD_RECEIVE], WATCH_IO_IN,
			os_websocket_receive_callback, (void *)config,
			&data[FD_RECEIVE].watch_id);
	if (ret != S_OK) {
		log_err("Failed to set fd watch close callback");
		goto exit;
	}

exit:
	return ret;
}

artik_error os_websocket_close_stream(artik_websocket_config *config)
{
	artik_error ret = S_OK;

	log_dbg("");

	lws_cleanup(config);

	return ret;
}
