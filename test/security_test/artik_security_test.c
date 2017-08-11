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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <artik_module.h>
#include <artik_security.h>

static artik_error test_security_get_serial_number(void)
{
	artik_error ret = S_OK;
	artik_security_module *security = (artik_security_module *)
					artik_request_api_module("security");
	artik_security_handle handle;
	unsigned char serial_number[ARTIK_CERT_SN_MAXLEN];
	unsigned int lenSN = ARTIK_CERT_SN_MAXLEN;
	int i = 0;

	fprintf(stdout, "TEST: %s starting\n", __func__);

	ret = security->request(&handle);
	if (ret != S_OK) {
		fprintf(stderr, "Failed to request security module (err=%d)\n",
									ret);
		return ret;
	}
	ret = security->get_certificate_sn(handle, serial_number, &lenSN);
	if (ret != S_OK) {
		fprintf(stderr, "Failed to get serial number from certificate"\
			" (err=%d)\n", ret);
		goto exit;
	}
	fprintf(stdout, "Serial Number :\n");
	while (i < lenSN) {

		fprintf(stdout, "%02x", serial_number[i]);
		++i;
	}
	fprintf(stdout, "\n");

exit:

	security->release(handle);

	fprintf(stdout, "TEST: %s %s\n", __func__, (ret == S_OK) ? "succeeded" :
								"failed");

	artik_release_api_module(security);

	return ret;
}

static artik_error test_security_get_certificate_and_key(void)
{
	artik_error ret = S_OK;
	artik_security_module *security = (artik_security_module *)
					artik_request_api_module("security");
	artik_security_handle handle;
	char *cert = NULL;
	char *key = NULL;

	fprintf(stdout, "TEST: %s starting\n", __func__);

	ret = security->request(&handle);
	if (ret != S_OK) {
		fprintf(stderr, "Failed to request security module (err=%d)\n",
									ret);
		return ret;
	}

	ret = security->get_certificate(handle, &cert);
	if (ret != S_OK) {
		fprintf(stderr, "Failed to get certificate (err=%d)\n",
									ret);
		goto exit;
	}

	fprintf(stdout, "Certificate:\n%s\n", cert);

	ret = security->get_key_from_cert(handle, cert, &key);
	if (ret != S_OK) {
		fprintf(stderr, "Failed to get key (err=%d)\n", ret);
		goto exit;
	}

	fprintf(stdout, "Key:\n%s\n", key);

exit:

	security->release(handle);

	if (cert)
		free(cert);
	if (key)
		free(key);
	fprintf(stdout, "TEST: %s %s\n", __func__, (ret == S_OK) ? "succeeded" :
								"failed");

	artik_release_api_module(security);

	return ret;
}

static artik_error test_security_random_bytes(void)
{
	artik_error ret = S_OK;
	artik_security_module *security = (artik_security_module *)
					artik_request_api_module("security");
	artik_security_handle handle;
	unsigned char randbytes[32];
	int i = 0;

	fprintf(stdout, "TEST: %s starting\n", __func__);

	ret = security->request(&handle);
	if (ret != S_OK) {
		fprintf(stderr, "Failed to request security module (err=%d)\n",
									ret);
		return ret;
	}

	ret = security->get_random_bytes(handle, randbytes, 32);
	if (ret != S_OK) {
		fprintf(stderr, "Failed to get random bytes (err=%d)\n", ret);
		goto exit;
	}

	fprintf(stdout, "Random bytes: ");

	for (i = 0; i < 32; i++)
		fprintf(stdout, "0x%02x ", randbytes[i]);

	fprintf(stdout, "\n");

exit:
	security->release(handle);

	fprintf(stdout, "TEST: %s %s\n", __func__, (ret == S_OK) ? "succeeded" :
								"failed");

	artik_release_api_module(security);

	return ret;
}

int main(void)
{
	artik_error ret = S_OK;

	fprintf(stdout, "artik_security_test:\n");

	ret = test_security_get_serial_number();
	if (ret != S_OK)
		goto exit;

	ret = test_security_get_certificate_and_key();
	if (ret != S_OK)
		goto exit;

	ret = test_security_random_bytes();
exit:
	return (ret == S_OK) ? 0 : 1;
}
