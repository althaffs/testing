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
#include <curl/curl.h>
#include <openssl/ssl.h>
#include <artik_log.h>
#include <pthread.h>

#include <artik_module.h>
#include <artik_security.h>
#include <artik_http.h>
#include "os_http.h"

#define WAIT_CONNECT_POLLING_MS	500
#define FLAG_EVENT		(0x1 << 0)
#define MAX(a, b)		((a > b) ? a : b)
#define CB_CONTAINER		((artik_websocket_container *)\
				lws_get_protocol(wsi)->user)
#define CB_FDS			(((artik_websocket_user_data *)\
				CB_CONTAINER->fds)->fdset)
#define NUM_FDS			2
#define FD_CLOSE		0
#define FD_CONNECT		1
#define MAX_QUEUE_NAME		1024
#define MAX_QUEUE_SIZE		128
#define MAX_MESSAGE_SIZE	2048

typedef struct {
	char *cert;
	char *key;
} SSL_CTX_PARAMS;

typedef struct {
	artik_http_stream_callback callback;
	void *user_data;
} stream_callback_params;

static pthread_mutex_t lock;
static bool lock_initialized = false;

static void mutex_lock(void)
{
	if (!lock_initialized) {
		pthread_mutex_init(&lock, NULL);
		lock_initialized = true;
	}
	pthread_mutex_lock(&lock);
}

static void mutex_unlock(void)
{
	pthread_mutex_unlock(&lock);
}

static CURLcode ssl_ctx_callback(CURL *curl, void *sslctx, void *parm)
{
	CURLcode ret = CURLE_OK;
	artik_ssl_config *ssl_config = (artik_ssl_config *)parm;
	SSL_CTX *ctx = (SSL_CTX *)sslctx;
	BIO *b64 = NULL;
	X509 *x509_cert = NULL;
	EVP_PKEY *pk = NULL;

	log_dbg("");

	if (ssl_config->ca_cert.data && ssl_config->ca_cert.len &&
		ssl_config->verify_cert == ARTIK_SSL_VERIFY_REQUIRED) {
		X509_STORE *keystore = NULL;

		/* Convert CA certificate string into a BIO */
		b64 = BIO_new(BIO_s_mem());
		BIO_write(b64, ssl_config->ca_cert.data,
			ssl_config->ca_cert.len);

		/* Extrat X509 cert from the BIO */
		x509_cert = PEM_read_bio_X509(b64, NULL, NULL, NULL);
		if (!x509_cert) {
			log_err("Failed to extract cert from the bio");
			BIO_free(b64);
			ret = CURLE_SSL_CERTPROBLEM;
			goto exit;
		}

		log_dbg("");

		BIO_free(b64);

		keystore = SSL_CTX_get_cert_store(ctx);
		if (!keystore) {
			log_err("Failed to load keystore");
			BIO_free(b64);
			ret = CURLE_SSL_CERTPROBLEM;
			goto exit;
		}

		/* Set CA certificate to context */
		if (!X509_STORE_add_cert(keystore, x509_cert)) {
			log_err("Failed add certificate to the keystore");
			ret = CURLE_SSL_CERTPROBLEM;
			goto exit;
		}

		X509_free(x509_cert);
		x509_cert = NULL;
	}

	log_dbg("");

	if (ssl_config->client_cert.data && ssl_config->client_cert.len) {
		/* Convert certificate string into a BIO */
		b64 = BIO_new(BIO_s_mem());
		BIO_write(b64, ssl_config->client_cert.data,
			ssl_config->client_cert.len);

		/* Extract X509 cert from the BIO */
		x509_cert = PEM_read_bio_X509(b64, NULL, NULL, NULL);
		if (!x509_cert) {
			BIO_free(b64);
			ret = CURLE_SSL_CERTPROBLEM;
			goto exit;
		}

		log_dbg("");

		BIO_free(b64);

		/* Set certificate to context */
		if (!SSL_CTX_use_certificate(ctx, x509_cert)) {
			ret = CURLE_SSL_CERTPROBLEM;
			goto exit;
		}
	}

	log_dbg("");

	if (ssl_config->client_key.data && ssl_config->client_key.len) {
		/* Convert key string into a BIO */
		b64 = BIO_new(BIO_s_mem());
		if (!BIO_write(b64, ssl_config->client_key.data,
				ssl_config->client_key.len)) {
			BIO_free(b64);
			ret = CURLE_SSL_CERTPROBLEM;
			goto exit;
		}

		log_dbg("");

		/* Extract EVP key from the BIO */
		pk = PEM_read_bio_PrivateKey(b64, NULL, 0, NULL);
		if (!pk) {
			BIO_free(b64);
			ret = CURLE_SSL_CERTPROBLEM;
			goto exit;
		}

		BIO_free(b64);

		log_dbg("");

		/* Set private key to context */
		if (!SSL_CTX_use_PrivateKey(ctx, pk)) {
			ret = CURLE_SSL_CERTPROBLEM;
			goto exit;
		}

		log_dbg("");

		/* Check certificate/key pair validity */
		if (!SSL_CTX_check_private_key(ctx)) {
			ret = CURLE_SSL_CERTPROBLEM;
			goto exit;
		}
	}

exit:
	if (pk)
		EVP_PKEY_free(pk);

	if (x509_cert)
		X509_free(x509_cert);

	return ret;

}

static size_t response_callback(void *ptr, size_t size, size_t nmemb,
	void *userp)
{
	char **response = (char **)userp;
	int len = size * nmemb;
	const char *rx_data = (const char *)ptr;

	log_dbg("");

	if (!*response) {
		/* first call, allocate the response string */
		*response = strndup(rx_data, len);
		if (!*response)
			return 0;
	} else {
		/* not first call, need to realloc memory
		 * before concatenating
		 */
		*response = realloc(*response, strlen(*response) + len + 1);
		if (!*response)
			return 0;

		strncat(*response, rx_data, len);
	}

	return len;
}

static size_t stream_callback(void *ptr, size_t size, size_t nmemb,
	void *userp)
{
	stream_callback_params *cb_params = (stream_callback_params *)userp;
	int len = size * nmemb;
	char *data = (char *)ptr;

	log_dbg("");

	return (size_t)(cb_params->callback)(data, len, cb_params->user_data);
}

artik_error os_http_get_stream(const char *url, artik_http_headers *headers,
		int *status, artik_http_stream_callback callback,
		void *user_data, artik_ssl_config *ssl)
{
	CURL *curl;
	CURLcode res;
	artik_security_handle sec_handle = NULL;
	struct curl_slist *h_list = NULL;
	artik_error ret = S_OK;
	SSL_CTX_PARAMS params = { 0 };
	stream_callback_params cb_params = { 0 };
	int i;
	artik_security_module *security = NULL;

	log_dbg("");

	if (!url || !callback)
		return E_BAD_ARGS;

	mutex_lock();

	/* Initialize curl */
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	if (!curl) {
		log_err("Failed to initialize curl");
		curl_global_cleanup();
		mutex_unlock();
		return E_NOT_SUPPORTED;
	}

	/* Build request headers if any */
	if (headers && headers->num_fields) {
		for (i = 0; i < headers->num_fields; i++) {
			int hdrlen = strlen(headers->fields[i].name) + 2 +
					strlen(headers->fields[i].data) + 1;
			char *h = malloc(hdrlen);

			snprintf(h, hdrlen, "%s: %s", headers->fields[i].name,
						headers->fields[i].data);
			h_list = curl_slist_append(h_list, h);
			free(h);
		}
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, h_list);
	}

	cb_params.callback = callback;
	cb_params.user_data = user_data;

	/* Prepare curl parameters */
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, stream_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&cb_params);

	if (ssl && ssl->verify_cert == ARTIK_SSL_VERIFY_REQUIRED) {
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
		curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");
		curl_easy_setopt(curl, CURLOPT_CAPATH, NULL);
		curl_easy_setopt(curl, CURLOPT_CAINFO, NULL);
	} else {
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	}

	/* If we use the Secure Element, setup proper certificate/key pair */
	if (ssl && ssl->use_se) {
		security = (artik_security_module *)
					artik_request_api_module("security");

		if (security->request(&sec_handle) != S_OK) {
			log_err("Failed to request security module");
			ret = E_HTTP_ERROR;
			goto exit;
		}

		if (security->get_certificate(sec_handle, &params.cert) !=
									S_OK) {
			log_err("Failed to get certificate from the"\
				" security module");
			ret = E_HTTP_ERROR;
			goto exit;
		} else {
			if (ssl->client_cert.data) {
				free(ssl->client_cert.data);
				ssl->client_cert.data = NULL;
			}
			ssl->client_cert.data = strdup(params.cert);
			ssl->client_cert.len = strlen(params.cert);
		}

		if (security->get_key_from_cert(sec_handle, params.cert,
							&params.key) != S_OK) {
			log_err("Failed to get private key form the"\
				" security module");
			ret = E_HTTP_ERROR;
			goto exit;
		} else {
			if (ssl->client_key.data) {
				free(ssl->client_key.data);
				ssl->client_key.data = NULL;
			}
			ssl->client_key.data = strdup(params.key);
			ssl->client_key.len = strlen(params.key);
		}
	}

	if (ssl) {
		curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION,
							ssl_ctx_callback);
		curl_easy_setopt(curl, CURLOPT_SSL_CTX_DATA, ssl);
	}
	// curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

	/* Perform request */
	res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		ret = E_HTTP_ERROR;
		goto exit;
	}

exit:
	if (status)
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, status);

	curl_easy_cleanup(curl);
	curl_global_cleanup();

	mutex_unlock();

	if (h_list)
		curl_slist_free_all(h_list);

	if (params.cert)
		free(params.cert);

	if (params.key)
		free(params.key);

	if (sec_handle)
		security->release(sec_handle);

	if (security)
		artik_release_api_module(security);

	return ret;
}

artik_error os_http_get(const char *url, artik_http_headers *headers,
	char **response, int *status, artik_ssl_config *ssl)
{
	CURL *curl;
	CURLcode res;
	artik_security_handle sec_handle = NULL;
	struct curl_slist *h_list = NULL;
	artik_error ret = S_OK;
	SSL_CTX_PARAMS params = { 0 };
	int i;
	artik_security_module *security = NULL;

	log_dbg("");

	if (!url || !response)
		return E_BAD_ARGS;

	mutex_lock();

	/* Initialize curl */
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	if (!curl) {
		log_err("Failed to initialize curl");
		curl_global_cleanup();
		mutex_unlock();
		return E_NOT_SUPPORTED;
	}

	/* Build request headers if any */
	if (headers && headers->num_fields) {
		for (i = 0; i < headers->num_fields; i++) {
			int hdrlen = strlen(headers->fields[i].name) + 2 +
					strlen(headers->fields[i].data) + 1;
			char *h = malloc(hdrlen);

			snprintf(h, hdrlen, "%s: %s", headers->fields[i].name,
						headers->fields[i].data);
			h_list = curl_slist_append(h_list, h);
			free(h);
		}
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, h_list);
	}

	/* Initialize response */
	*response = NULL;

	/* Prepare curl parameters */
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, response_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)response);

	if (ssl && ssl->verify_cert == ARTIK_SSL_VERIFY_REQUIRED) {
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
		curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");
		curl_easy_setopt(curl, CURLOPT_CAPATH, NULL);
		curl_easy_setopt(curl, CURLOPT_CAINFO, NULL);
	} else {
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	}

	/* If we use the Secure Element, setup proper certificate/key pair */
	if (ssl && ssl->use_se) {
		security = (artik_security_module *)
					artik_request_api_module("security");

		if (security->request(&sec_handle) != S_OK) {
			log_err("Failed to request security module");
			ret = E_HTTP_ERROR;
			goto exit;
		}

		if (security->get_certificate(sec_handle, &params.cert)
								!= S_OK) {
			log_err("Failed to get certificate");
			log_err("from the security module");
			ret = E_HTTP_ERROR;
			goto exit;
		} else {
			if (ssl->client_cert.data) {
				free(ssl->client_cert.data);
				ssl->client_cert.data = NULL;
			}
			ssl->client_cert.data = strdup(params.cert);
			ssl->client_cert.len = strlen(params.cert);
		}

		if (security->get_key_from_cert(sec_handle, params.cert,
							&params.key) != S_OK) {
			log_err("Failed to get private");
			log_err("key form the security module");
			ret = E_HTTP_ERROR;
			goto exit;
		} else {
			if (ssl->client_key.data) {
				free(ssl->client_key.data);
				ssl->client_key.data = NULL;
			}
			ssl->client_key.data = strdup(params.key);
			ssl->client_key.len = strlen(params.key);
		}
	}

	if (ssl) {
		curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION,
							ssl_ctx_callback);
		curl_easy_setopt(curl, CURLOPT_SSL_CTX_DATA, ssl);
	}
	// curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

	/* Perform request */
	res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		ret = E_HTTP_ERROR;
		goto exit;
	}

exit:
	if (status)
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, status);

	curl_easy_cleanup(curl);
	curl_global_cleanup();

	mutex_unlock();

	if (h_list)
		curl_slist_free_all(h_list);

	if (params.cert)
		free(params.cert);

	if (params.key)
		free(params.key);

	if (sec_handle)
		security->release(sec_handle);

	if (security)
		artik_release_api_module(security);

	return ret;
}

artik_error os_http_post(const char *url, artik_http_headers *headers,
	const char *body, char **response, int *status, artik_ssl_config *ssl)
{
	CURL *curl;
	CURLcode res;
	struct curl_slist *h_list = NULL;
	artik_error ret = S_OK;
	int i;
	SSL_CTX_PARAMS params = { 0 };
	artik_security_handle sec_handle = NULL;
	artik_security_module *security = NULL;

	log_dbg("");

	if (!url || !response) {
		log_err("Bad arguments");
		return E_BAD_ARGS;
	}

	mutex_lock();

	/* Initialize curl */
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	if (!curl) {
		log_err("Failed to initialize curl");
		curl_global_cleanup();
		mutex_unlock();
		return E_NOT_SUPPORTED;
	}

	/* Build request headers if any */
	if (headers && headers->num_fields) {
		for (i = 0; i < headers->num_fields; i++) {
			int hdrlen = strlen(headers->fields[i].name) + 2 +
					strlen(headers->fields[i].data) + 1;
			char *h = malloc(hdrlen);

			snprintf(h, hdrlen, "%s: %s", headers->fields[i].name,
						headers->fields[i].data);
			h_list = curl_slist_append(h_list, h);
			free(h);
		}
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, h_list);
	}

	/* Initialize response */
	*response = NULL;

	/* Prepare curl parameters */
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, response_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)response);

	if (ssl && ssl->verify_cert == ARTIK_SSL_VERIFY_REQUIRED) {
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
		curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");
		curl_easy_setopt(curl, CURLOPT_CAPATH, NULL);
		curl_easy_setopt(curl, CURLOPT_CAINFO, NULL);
	} else {
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	}

	/* If we use the Secure Element, setup proper certificate/key pair */
	if (ssl && ssl->use_se) {
		security = (artik_security_module *)
					artik_request_api_module("security");

		if (security->request(&sec_handle) != S_OK) {
			log_err("Failed to request security module");
			ret = E_HTTP_ERROR;
			goto exit;
		}

		if (security->get_certificate(sec_handle, &params.cert)
								!= S_OK) {
			log_err("Failed to get certificate");
			log_err("from the security module");
			ret = E_HTTP_ERROR;
			goto exit;
		} else {
			if (ssl->client_cert.data) {
				free(ssl->client_cert.data);
				ssl->client_cert.data = NULL;
			}
			ssl->client_cert.data = strdup(params.cert);
			ssl->client_cert.len = strlen(params.cert);
		}

		if (security->get_key_from_cert(sec_handle, params.cert,
							&params.key) != S_OK) {
			log_err("Failed to get private");
			log_err("key form the security module");
			ret = E_HTTP_ERROR;
			goto exit;
		} else {
			if (ssl->client_key.data) {
				free(ssl->client_key.data);
				ssl->client_key.data = NULL;
			}
			ssl->client_key.data = strdup(params.key);
			ssl->client_key.len = strlen(params.key);
		}
	}

	if (ssl) {
		curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION,
							ssl_ctx_callback);
		curl_easy_setopt(curl, CURLOPT_SSL_CTX_DATA, ssl);
	}

	if (body)
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (void *)body);

	// curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

	/* Perform request */
	res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		log_err("curl request failed (curl err=%d)", res);
		ret = E_HTTP_ERROR;
		goto exit;
	}

exit:
	if (status)
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, status);

	curl_easy_cleanup(curl);
	curl_global_cleanup();

	mutex_unlock();

	if (h_list)
		curl_slist_free_all(h_list);

	if (params.cert)
		free(params.cert);

	if (params.key)
		free(params.key);

	if (sec_handle)
		security->release(sec_handle);

	if (security)
		artik_release_api_module(security);

	return ret;
}

artik_error os_http_put(const char *url, artik_http_headers *headers,
	const char *body, char **response, int *status, artik_ssl_config *ssl)
{
	CURL *curl;
	CURLcode res;
	struct curl_slist *h_list = NULL;
	artik_error ret = S_OK;
	int i;
	SSL_CTX_PARAMS params = { 0 };
	artik_security_handle sec_handle = NULL;
	artik_security_module *security = NULL;

	log_dbg("");

	if (!url || !response)
		return E_BAD_ARGS;

	mutex_lock();

	/* Initialize curl */
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	if (!curl) {
		log_err("Failed to initialize curl");
		curl_global_cleanup();
		mutex_unlock();
		return E_NOT_SUPPORTED;
	}

	/* Build request headers if any */
	if (headers && headers->num_fields) {
		for (i = 0; i < headers->num_fields; i++) {
			int hdrlen = strlen(headers->fields[i].name) + 2 +
					strlen(headers->fields[i].data) + 1;
			char *h = malloc(hdrlen);

			snprintf(h, hdrlen, "%s: %s", headers->fields[i].name,
						headers->fields[i].data);
			h_list = curl_slist_append(h_list, h);
			free(h);
		}
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, h_list);
	}

	/* Initialize response */
	*response = NULL;

	/* Prepare curl parameters */
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, response_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)response);

	if (ssl && ssl->verify_cert == ARTIK_SSL_VERIFY_REQUIRED) {
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
		curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");
		curl_easy_setopt(curl, CURLOPT_CAPATH, NULL);
		curl_easy_setopt(curl, CURLOPT_CAINFO, NULL);
	} else {
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	}

	/* If we use the Secure Element, setup proper certificate/key pair */
	if (ssl && ssl->use_se) {
		security = (artik_security_module *)
					artik_request_api_module("security");

		if (security->request(&sec_handle) != S_OK) {
			log_err("Failed to request security module");
			ret = E_HTTP_ERROR;
			goto exit;
		}

		if (security->get_certificate(sec_handle, &params.cert)
								!= S_OK) {
			log_err("Failed to get certificate");
			log_err("from the security module");
			ret = E_HTTP_ERROR;
			goto exit;
		} else {
			if (ssl->client_cert.data) {
				free(ssl->client_cert.data);
				ssl->client_cert.data = NULL;
			}
			ssl->client_cert.data = strdup(params.cert);
			ssl->client_cert.len = strlen(params.cert);
		}

		if (security->get_key_from_cert(sec_handle, params.cert,
							&params.key) != S_OK) {
			log_err("Failed to get private");
			log_err("key form the security module");
			ret = E_HTTP_ERROR;
			goto exit;
		} else {
			if (ssl->client_key.data) {
				free(ssl->client_key.data);
				ssl->client_key.data = NULL;
			}
			ssl->client_key.data = strdup(params.key);
			ssl->client_key.len = strlen(params.key);
		}
	}

	if (ssl) {
		curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION,
							ssl_ctx_callback);
		curl_easy_setopt(curl, CURLOPT_SSL_CTX_DATA, ssl);
	}

	if (body)
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (void *)body);
	/* curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); */

	/* Perform request */
	res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		ret = E_HTTP_ERROR;
		goto exit;
	}

exit:
	if (status)
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, status);

	curl_easy_cleanup(curl);
	curl_global_cleanup();

	mutex_unlock();

	if (h_list)
		curl_slist_free_all(h_list);

	if (params.cert)
		free(params.cert);

	if (params.key)
		free(params.key);

	if (sec_handle)
		security->release(sec_handle);

	if (security)
		artik_release_api_module(security);

	return ret;
}

artik_error os_http_delete(const char *url, artik_http_headers *headers,
	char **response, int *status, artik_ssl_config *ssl)
{
	CURL *curl;
	CURLcode res;
	struct curl_slist *h_list = NULL;
	artik_error ret = S_OK;
	int i;
	SSL_CTX_PARAMS params = { 0 };
	artik_security_handle sec_handle = NULL;
	artik_security_module *security = NULL;

	log_dbg("");

	if (!url || !response)
		return E_BAD_ARGS;

	mutex_lock();

	/* Initialize curl */
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	if (!curl) {
		log_err("Failed to initialize curl");
		curl_global_cleanup();
		mutex_unlock();
		return E_NOT_SUPPORTED;
	}

	/* Build request headers if any */
	if (headers && headers->num_fields) {
		for (i = 0; i < headers->num_fields; i++) {
			int hdrlen = strlen(headers->fields[i].name) + 2 +
					strlen(headers->fields[i].data) + 1;
			char *h = malloc(hdrlen);

			snprintf(h, hdrlen, "%s: %s", headers->fields[i].name,
						headers->fields[i].data);
			h_list = curl_slist_append(h_list, h);
			free(h);
		}
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, h_list);
	}

	/* Initialize response */
	*response = NULL;

	/* Prepare curl parameters */
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, response_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)response);

	if (ssl && ssl->verify_cert == ARTIK_SSL_VERIFY_REQUIRED) {
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
		curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");
		curl_easy_setopt(curl, CURLOPT_CAPATH, NULL);
		curl_easy_setopt(curl, CURLOPT_CAINFO, NULL);
	} else {
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	}

	/* If we use the Secure Element, setup proper certificate/key pair */
	if (ssl && ssl->use_se) {
		security = (artik_security_module *)
					artik_request_api_module("security");

		if (security->request(&sec_handle) != S_OK) {
			log_err("Failed to request security module");
			ret = E_HTTP_ERROR;
			goto exit;
		}

		if (security->get_certificate(sec_handle, &params.cert)
								!= S_OK) {
			log_err("Failed to get certificate");
			log_err("from the security module");
			ret = E_HTTP_ERROR;
			goto exit;
		} else {
			if (ssl->client_cert.data) {
				free(ssl->client_cert.data);
				ssl->client_cert.data = NULL;
			}
			ssl->client_cert.data = strdup(params.cert);
			ssl->client_cert.len = strlen(params.cert);
		}

		if (security->get_key_from_cert(sec_handle, params.cert,
							&params.key) != S_OK) {
			log_err("Failed to get private");
			log_err("key form the security module");
			ret = E_HTTP_ERROR;
			goto exit;
		} else {
			if (ssl->client_key.data) {
				free(ssl->client_key.data);
				ssl->client_key.data = NULL;
			}
			ssl->client_key.data = strdup(params.key);
			ssl->client_key.len = strlen(params.key);
		}
	}

	if (ssl) {
		curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION,
							ssl_ctx_callback);
		curl_easy_setopt(curl, CURLOPT_SSL_CTX_DATA, ssl);
	}
	/* curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); */

	/* Perform request */
	res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		ret = E_HTTP_ERROR;
		goto exit;
	}

exit:
	if (status)
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, status);

	curl_easy_cleanup(curl);
	curl_global_cleanup();

	mutex_unlock();

	if (h_list)
		curl_slist_free_all(h_list);

	if (params.cert)
		free(params.cert);

	if (params.key)
		free(params.key);

	if (sec_handle)
		security->release(sec_handle);

	if (security)
		artik_release_api_module(security);

	return ret;
}
