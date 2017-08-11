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
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <artik_websocket.h>
#include <artik_security.h>
#include <artik_log.h>
#include <websocket.h>

#include "tls/see_api.h"
#include "os_websocket.h"

#define MBED_DEBUG_LEVEL	0

struct mbedtls_ctx {
	mbedtls_ssl_config *conf;
	mbedtls_x509_crt *cert;
	mbedtls_pk_context *pkey;
	mbedtls_entropy_context *entropy;
	mbedtls_ctr_drbg_context *ctr_drbg;
};

struct websocket_priv {
	websocket_t *cli;
	artik_websocket_callback rx_cb;
	void *rx_user_data;
	artik_websocket_callback conn_cb;
	void *conn_user_data;
	struct mbedtls_ctx tls_ctx;
};

static int websocket_parse_uri(const char *uri, char **host, char **path,
		int *port, bool *use_tls)
{
	char *tmp = NULL;
	int idx = 0;

	*host = NULL;

	if (!strncmp(uri, "ws://", strlen("ws://"))) {
		*port = 80;
		*use_tls = false;
		idx += strlen("ws://");
	} else if (!strncmp(uri, "wss://", strlen("wss://"))) {
		*port = 443;
		*use_tls = true;
		idx += strlen("wss://");
	} else {
		log_err("Malformed URI: should start with 'ws://' or 'wss://'");
		return -1;
	}

	tmp = strchr(uri + idx, ':');
	if (tmp) {
		char *end = NULL;
		unsigned long p = strtoul(tmp, &end, 10);

		if (p > 0)
			*port = (int)p;

		*host = strndup(uri + idx, tmp - (uri + idx));
		idx += end - tmp;
	}

	tmp = strchr(uri + idx, '/');
	if (!tmp) {
		log_err("Malformed URI: missing path after hostname");
		return -1;
	}

	if (*host == NULL)
		*host = strndup(uri + idx, tmp - (uri + idx));

	*path = strndup(tmp, strlen(tmp));

	return 0;
}

static ssize_t websocket_recv_cb(websocket_context_ptr ctx, uint8_t *buf,
			size_t len, int flags, void *user_data)
{
	struct websocket_info_t *info = user_data;
	int fd = info->data->fd;
	int retry_cnt = 3;
	ssize_t ret = 0;

	log_dbg("");

retry:
	if (info->data->tls_enabled) {
		ret = mbedtls_ssl_read(info->data->tls_ssl, buf, len);
		if (!ret) {
			websocket_set_error(info->data,
						WEBSOCKET_ERR_CALLBACK_FAILURE);
		} else if (ret < 0) {
			log_dbg("SSL read error %d", ret);
			if (!retry_cnt) {
				websocket_set_error(info->data,
						WEBSOCKET_ERR_CALLBACK_FAILURE);
				return ret;
			}
			retry_cnt--;
			goto retry;
		}
	} else {
		ret = recv(fd, buf, len, 0);
		if (!ret) {
			websocket_set_error(info->data,
						WEBSOCKET_ERR_CALLBACK_FAILURE);
		} else if (ret > 0) {
			if ((errno == EAGAIN) || (errno == EBUSY)) {
				if (!retry_cnt) {
					websocket_set_error(info->data,
						WEBSOCKET_ERR_CALLBACK_FAILURE);
					return ret;
				}
				retry_cnt--;
				goto retry;
			}
		}
	}

	return ret;
}

ssize_t websocket_send_cb(websocket_context_ptr ctx, const uint8_t *buf,
			size_t len, int flags, void *user_data)
{
	struct websocket_info_t *info = user_data;
	int fd = info->data->fd;
	int retry_cnt = 3;
	ssize_t ret = 0;

	log_dbg("");

retry:
	if (info->data->tls_enabled) {
		ret = mbedtls_ssl_write(info->data->tls_ssl, buf, len);
		if (!ret) {
			websocket_set_error(info->data,
						WEBSOCKET_ERR_CALLBACK_FAILURE);
		} else if (ret < 0) {
			log_dbg("SSL write error %d", ret);
			if (!retry_cnt) {
				websocket_set_error(info->data,
						WEBSOCKET_ERR_CALLBACK_FAILURE);
				return ret;
			}
			retry_cnt--;
			goto retry;
		}
	} else {
		ret = send(fd, buf, len, flags);
		if (ret < 0) {
			if ((errno == EAGAIN) || (errno == EBUSY)) {
				if (!retry_cnt) {
					websocket_set_error(info->data,
						WEBSOCKET_ERR_CALLBACK_FAILURE);
					return ret;
				}
				retry_cnt--;
				goto retry;
			}
		}
	}

	return ret;
}

static int websocket_genmask_cb(websocket_context_ptr ctx, uint8_t *buf,
		size_t len, void *user_data)
{
	memset(buf, rand(), len);

	return 0;
}

void websocket_print_on_msg_cb(websocket_context_ptr ctx,
		const websocket_on_msg_arg *arg, void *user_data)
{
	struct websocket_info_t *info = user_data;
	struct websocket_priv *priv = (struct websocket_priv *)
							info->data->user_data;

	if (!priv)
		return;

	if (WEBSOCKET_CHECK_NOT_CTRL_FRAME(arg->opcode)) {
		if (priv->rx_cb) {
			char *msg = strndup((const char *)arg->msg,
							arg->msg_length);
			if (msg)
				priv->rx_cb(priv->rx_user_data, (void *)msg);
		}
	} else if (WEBSOCKET_CHECK_CTRL_CLOSE(arg->opcode)) {
		log_dbg("MSG: Close\n");
		if (priv->conn_cb)
			priv->conn_cb(priv->conn_user_data,
						(void *)ARTIK_WEBSOCKET_CLOSED);
	} else if (WEBSOCKET_CHECK_CTRL_PING(arg->opcode)) {
		log_dbg("MSG: Ping\n");
	} else if (WEBSOCKET_CHECK_CTRL_PONG(arg->opcode)) {
		log_dbg("MSG: Pong\n");
	}
}

static websocket_cb_t callbacks = {
	websocket_recv_cb,		/* recv callback */
	websocket_send_cb,		/* send callback */
	websocket_genmask_cb,		/* gen mask callback */
	NULL,				/* recv frame start callback */
	NULL,				/* recv frame chunk callback */
	NULL,				/* recv frame end callback */
	websocket_print_on_msg_cb	/* recv message callback */
};

static void websocket_tls_debug(void *ctx, int level, const char *file,
								int line,
								const char *str)
{
	log_dbg("%s:%04d: %s", file, line, str);
}

static void ssl_cleanup(struct websocket_priv *priv)
{
	websocket_t *ws = priv->cli;
	struct mbedtls_ctx *ctx = &priv->tls_ctx;

	log_dbg("");

	if (ws->tls_ssl) {
		free(ws->tls_ssl);
		ws->tls_ssl = NULL;
	}

	if (ctx->ctr_drbg) {
		mbedtls_ctr_drbg_free(ctx->ctr_drbg);
		free(ctx->ctr_drbg);
	}

	if (ctx->entropy) {
		mbedtls_entropy_free(ctx->entropy);
		free(ctx->entropy);
	}

	if (ctx->pkey) {
		mbedtls_pk_free(ctx->pkey);
		free(ctx->pkey);
	}

	if (ctx->cert) {
		mbedtls_x509_crt_free(ctx->cert);
		free(ctx->cert);
	}

	if (ctx->conf) {
		mbedtls_ssl_config_free(ctx->conf);
		free(ctx->conf);
	}

	/* Nullify everything */
	memset(ctx, 0, sizeof(struct mbedtls_ctx));
}

static int see_generate_random_client(void *ctx, unsigned char *data,
								size_t len)
{
	artik_security_module *security = NULL;
	artik_security_handle handle;

	if (!data || !len)
		return -1;

	security = (artik_security_module *)
					artik_request_api_module("security");
	security->request(&handle);
	security->get_random_bytes(handle, data, len);
	security->release(handle);
	artik_release_api_module(security);

	return 0;
}

static artik_error ssl_setup(struct websocket_priv *priv,
						artik_ssl_config *ssl_config)
{
	websocket_t *ws = priv->cli;
	struct mbedtls_ctx *ctx = &priv->tls_ctx;
	int ret = 0;

	log_dbg("");

	if (!ws->tls_enabled)
		return S_OK;

	ctx->conf = zalloc(sizeof(mbedtls_ssl_config));
	if (!ctx->conf) {
		ssl_cleanup(priv);
		return E_NO_MEM;
	}

	ws->tls_ssl = malloc(sizeof(mbedtls_ssl_context));
	if (!ws->tls_ssl) {
		ssl_cleanup(priv);
		return E_NO_MEM;
	}

	ctx->entropy = zalloc(sizeof(mbedtls_entropy_context));
	if (!ctx->entropy) {
		ssl_cleanup(priv);
		return E_NO_MEM;
	}

	ctx->ctr_drbg = zalloc(sizeof(mbedtls_ctr_drbg_context));
	if (!ctx->ctr_drbg) {
		ssl_cleanup(priv);
		return E_NO_MEM;
	}

	mbedtls_net_init(&(ws->tls_net));
	mbedtls_ssl_init(ws->tls_ssl);
	mbedtls_ssl_config_init(ctx->conf);
	mbedtls_entropy_init(ctx->entropy);
	mbedtls_ctr_drbg_init(ctx->ctr_drbg);
#ifdef MBEDTLS_DEBUG_C
	mbedtls_debug_set_threshold(MBED_DEBUG_LEVEL);
#endif

	/* Seed the Random Number Generator */
	ret = mbedtls_ctr_drbg_seed(ctx->ctr_drbg, mbedtls_entropy_func,
							ctx->entropy, NULL, 0);
	if (ret) {
		log_err("Failed to seed RNG (err=%d)", ret);
		ssl_cleanup(priv);
		return E_BAD_ARGS;
	}

	/* Setup default config */
	ret = mbedtls_ssl_config_defaults(ctx->conf, MBEDTLS_SSL_IS_CLIENT,
			MBEDTLS_SSL_TRANSPORT_STREAM,
			MBEDTLS_SSL_PRESET_DEFAULT);
	if (ret) {
		log_err("Failed to set configuration defaults (err=%d)", ret);
		ssl_cleanup(priv);
		return E_BAD_ARGS;
	}

	mbedtls_ssl_conf_dbg(ctx->conf, websocket_tls_debug, stdout);
	mbedtls_ssl_conf_rng(ctx->conf, mbedtls_ctr_drbg_random, ctx->ctr_drbg);

	if (ssl_config->use_se) {
		artik_security_module *security = (artik_security_module *)
					artik_request_api_module("security");
		artik_security_handle handle;
		artik_error err = S_OK;
		char *root_ca = NULL;
		char *dev_cert = NULL;
		const mbedtls_pk_info_t *pk_info;

		if (!security) {
			log_err("Failed to load security module");
			ssl_cleanup(priv);
			return E_NOT_SUPPORTED;
		}

		ctx->cert = zalloc(sizeof(mbedtls_x509_crt));
		if (!ctx->cert) {
			ssl_cleanup(priv);
			return E_NO_MEM;
		}

		ctx->pkey = zalloc(sizeof(mbedtls_pk_context));
		if (!ctx->pkey) {
			ssl_cleanup(priv);
			return E_NO_MEM;
		}

		err = security->request(&handle);
		if (ret != S_OK) {
			log_err("Failed to load security module (err=%d)", err);
			ssl_cleanup(priv);
			artik_release_api_module(handle);
			return ret;
		}

		err = security->get_certificate(handle, &dev_cert);
		if (ret != S_OK) {
			log_err("Failed to get device certificate (err=%d)",
									err);
			ssl_cleanup(priv);
			artik_release_api_module(handle);
			return ret;
		}

		err = security->get_root_ca(handle, &root_ca);
		if (ret != S_OK) {
			log_err("Failed to get root CA (err=%d)", err);
			ssl_cleanup(priv);
			artik_release_api_module(handle);
			free(dev_cert);
			return ret;
		}

		security->release(handle);
		artik_release_api_module(security);

		mbedtls_ssl_conf_rng(ctx->conf, see_generate_random_client,
								ctx->ctr_drbg);
		mbedtls_x509_crt_init(ctx->cert);
		mbedtls_pk_init(ctx->pkey);

		ret = mbedtls_x509_crt_parse(ctx->cert,
				(const unsigned char *)dev_cert,
				strlen(dev_cert) + 1);
		if (ret) {
			log_err("Failed to parse device certificate (err=%d)",
									ret);
			ssl_cleanup(priv);
			free(dev_cert);
			free(root_ca);
			return E_BAD_ARGS;
		}

		pk_info = mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY);
		if (!pk_info) {
			log_err("Failed to get private key info");
			ssl_cleanup(priv);
			free(dev_cert);
			free(root_ca);
			return E_BAD_ARGS;
		}

		ret = mbedtls_pk_setup(ctx->pkey, pk_info);
		if (ret) {
			log_err("Failed to setup private key info");
			ssl_cleanup(priv);
			free(dev_cert);
			free(root_ca);
			return E_BAD_ARGS;
		}

		((mbedtls_ecdsa_context *)(ctx->pkey->pk_ctx))->grp.id =
						MBEDTLS_ECP_DP_SECP256R1;
		((mbedtls_ecdsa_context *)(ctx->pkey->pk_ctx))->key_index =
						FACTORYKEY_ARTIK_DEVICE;

		ret = mbedtls_ssl_conf_own_cert(ctx->conf, ctx->cert,
								ctx->pkey);
		if (ret) {
			log_err("Failed to configure device cert/key (err=%d)",
									ret);
			ssl_cleanup(priv);
			free(dev_cert);
			free(root_ca);
			return E_BAD_ARGS;
		}

		if (root_ca) {
			ret = mbedtls_x509_crt_parse(ctx->cert,
					(const unsigned char *)root_ca,
					strlen(root_ca) + 1);
			if (ret) {
				log_err("Failed to parse root CA cert (err=%d)",
									ret);
				ssl_cleanup(priv);
				free(dev_cert);
				free(root_ca);
				return E_BAD_ARGS;
			}

			mbedtls_ssl_conf_ca_chain(ctx->conf, ctx->cert->next ?
					ctx->cert->next : ctx->cert, NULL);
		}

		ssl_config->ca_cert.data = NULL;
		ssl_config->verify_cert = ARTIK_SSL_VERIFY_NONE;

		free(dev_cert);
		free(root_ca);
	} else if (ssl_config->client_cert.data &&
					ssl_config->client_key.data) {
		ctx->cert = zalloc(sizeof(mbedtls_x509_crt));
		if (!ctx->cert) {
			ssl_cleanup(priv);
			return E_NO_MEM;
		}

		ctx->pkey = zalloc(sizeof(mbedtls_pk_context));
		if (!ctx->pkey) {
			ssl_cleanup(priv);
			return E_NO_MEM;
		}

		mbedtls_x509_crt_init(ctx->cert);
		mbedtls_pk_init(ctx->pkey);

		ret = mbedtls_x509_crt_parse(ctx->cert,
			(const unsigned char *)ssl_config->client_cert.data,
			ssl_config->client_cert.len);
		if (ret) {
			log_err("Failed to parse device certificate (err=%d)",
									ret);
			ssl_cleanup(priv);
			return E_BAD_ARGS;
		}

		ret = mbedtls_pk_parse_key(ctx->pkey,
			(const unsigned char *)ssl_config->client_key.data,
			ssl_config->client_key.len, NULL, 0);
		if (ret) {
			log_err("Failed to parse device key (err=%d)", ret);
			ssl_cleanup(priv);
			return E_BAD_ARGS;
		}

		ret = mbedtls_ssl_conf_own_cert(ctx->conf, ctx->cert,
								ctx->pkey);
		if (ret) {
			log_err("Failed to configure device cert/key (err=%d)",
									ret);
			ssl_cleanup(priv);
			return E_BAD_ARGS;
		}
	}

	/* Load root CA if provided */
	if (ssl_config->ca_cert.data) {
		if (!ctx->cert) {
			ctx->cert = zalloc(sizeof(mbedtls_x509_crt));
			if (!ctx->cert) {
				ssl_cleanup(priv);
				return E_NO_MEM;
			}
		}

		ret = mbedtls_x509_crt_parse(ctx->cert,
				(const unsigned char *)ssl_config->ca_cert.data,
				ssl_config->ca_cert.len);
		if (ret) {
			log_err("Failed to parse root CA certificate (err=%d)",
									ret);
			ssl_cleanup(priv);
			return E_BAD_ARGS;
		}

		mbedtls_ssl_conf_ca_chain(ctx->conf, ctx->cert->next ?
					ctx->cert->next : ctx->cert, NULL);
	}

	switch (ssl_config->verify_cert) {
	case ARTIK_SSL_VERIFY_NONE:
		ws->auth_mode = MBEDTLS_SSL_VERIFY_NONE;
		break;
	case ARTIK_SSL_VERIFY_OPTIONAL:
		ws->auth_mode = MBEDTLS_SSL_VERIFY_OPTIONAL;
		break;
	case ARTIK_SSL_VERIFY_REQUIRED:
		ws->auth_mode = MBEDTLS_SSL_VERIFY_REQUIRED;
		break;
	default:
		break;
	}

	ws->tls_conf = ctx->conf;

	return S_OK;
}

artik_error os_websocket_open_stream(artik_websocket_config *config)
{
	struct websocket_priv *priv = NULL;
	websocket_return_t ret;
	char port_str[16];
	char *host = NULL;
	char *path = NULL;
	int port = 0;
	bool use_tls = false;

	log_dbg("");

	if (!config->uri) {
		log_err("Undefined uri");
		return E_WEBSOCKET_ERROR;
	}

	if (websocket_parse_uri(config->uri, &host, &path, &port, &use_tls)
									< 0) {
		log_err("Failed to parse uri");
		return E_WEBSOCKET_ERROR;
	}

	/* Allocate private data structure */
	priv = (struct websocket_priv *)zalloc(sizeof(struct websocket_priv));
	if (!priv) {
		log_err("Failed to allocate memory for private data");
		return E_NO_MEM;
	}

	priv->cli = (websocket_t *)zalloc(sizeof(websocket_t));
	if (!priv->cli) {
		log_err("Failed to allocate memory");
		free(priv);
		return E_NO_MEM;
	}

	/* Fill up the configuration structure */
	priv->cli->fd = -1;
	priv->cli->cb = &callbacks;
	priv->cli->tls_enabled = use_tls;
	priv->cli->user_data = (void *)priv;

	/* Setup TLS configuration if applicable */
	if (ssl_setup(priv, &(config->ssl_config)) != S_OK) {
		log_err("Failed to configure SSL");
		free(priv->cli);
		free(priv);
		return E_BAD_ARGS;
	}

	/* Convert port integer into a string */
	if (!itoa(port, port_str, 10)) {
		log_err("Invalid port parameter");
		free(priv->cli);
		free(priv);
		return E_BAD_ARGS;
	}

	/* Open the websocket client connection */
	ret = websocket_client_open(priv->cli, host, port_str, path);
	if (ret != WEBSOCKET_SUCCESS) {
		log_err("Failed to open websocket client (ret=%d)", ret);
		free(priv->cli);
		free(priv);
		return E_WEBSOCKET_ERROR;
	}

	config->private_data = (void *)priv;

	return S_OK;
}

artik_error os_websocket_write_stream(artik_websocket_config *config,
							char *message, int len)
{
	struct websocket_priv *priv = (struct websocket_priv *)
							config->private_data;
	websocket_frame_t frame;
	websocket_return_t ret;

	log_dbg("");

	if (!priv)
		return E_NOT_INITIALIZED;

	frame.opcode = WEBSOCKET_TEXT_FRAME;
	frame.msg = (const uint8_t *)message;
	frame.msg_length = len;

	ret = websocket_queue_msg(priv->cli, &frame);
	if (ret != WEBSOCKET_SUCCESS) {
		log_err("Failed to send message (ret=%d)", ret);
		return E_WEBSOCKET_ERROR;
	}

	return S_OK;
}

artik_error os_websocket_set_connection_callback(artik_websocket_config *config,
		artik_websocket_callback callback, void *user_data)
{
	struct websocket_priv *priv = (struct websocket_priv *)
							config->private_data;

	log_dbg("");

	if (!priv)
		return E_NOT_INITIALIZED;

	priv->conn_cb = callback;
	priv->conn_user_data = user_data;

	/* If we are already connected, trigger the connected callback */
	if (priv->conn_cb && (priv->cli->state == WEBSOCKET_RUNNING))
		priv->conn_cb(priv->conn_user_data,
					(void *)ARTIK_WEBSOCKET_CONNECTED);

	return S_OK;
}

artik_error os_websocket_set_receive_callback(artik_websocket_config *config,
		artik_websocket_callback callback, void *user_data)
{
	struct websocket_priv *priv = (struct websocket_priv *)
							config->private_data;

	log_dbg("");

	if (!priv)
		return E_NOT_INITIALIZED;

	priv->rx_cb = callback;
	priv->rx_user_data = user_data;

	return S_OK;
}

artik_error os_websocket_close_stream(artik_websocket_config *config)
{
	struct websocket_priv *priv = (struct websocket_priv *)
							config->private_data;

	log_dbg("");

	if (!priv)
		return E_NOT_INITIALIZED;

	websocket_queue_close(priv->cli, NULL);
	ssl_cleanup(priv);
	free(priv->cli);
	free(priv);

	return S_OK;
}



