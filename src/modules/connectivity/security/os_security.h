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

#ifndef	__OS_SECURITY_H__
#define	__OS_SECURITY_H__

#include "artik_error.h"

artik_error os_security_request(artik_security_handle *handle);
artik_error os_security_release(artik_security_handle handle);
artik_error os_security_get_certificate(artik_security_handle handle,
					char **cert);
artik_error os_security_get_key_from_cert(artik_security_handle handle,
					const char *cert, char **key);
artik_error os_security_get_root_ca(artik_security_handle handle,
					char **root_ca);
artik_error os_get_random_bytes(artik_security_handle handle,
					unsigned char *rand, int len);
artik_error os_get_certificate_sn(artik_security_handle handle,
					unsigned char *sn, unsigned int *len);
artik_error os_verify_signature_init(artik_security_handle *handle,
		const char *signature_pem, const char *root_ca,
		const artik_time *signing_time_in, artik_time *signing_time_out);
artik_error os_verify_signature_update(artik_security_handle handle,
		unsigned char *data, unsigned int data_len);
artik_error os_verify_signature_final(artik_security_handle handle);

#endif  /* __OS_SECURITY_H__ */
