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

#ifndef __TINYARA_SECURITY_H__
#define __TINYARA_SECURITY_H__

#include <artik_error.h>
#include <artik_list.h>

#include <tls/asn1.h>
#include <tls/x509_crt.h>

typedef struct {
	mbedtls_x509_time signing_time;
	mbedtls_asn1_buf digest;
	mbedtls_asn1_buf raw;
} authenticated_attributes;

typedef struct {
	mbedtls_x509_name issuer;
	mbedtls_x509_buf serial;
	mbedtls_md_type_t digest_alg_id;
	authenticated_attributes authenticated_attributes;
	mbedtls_asn1_buf encrypted_digest;
} signer_info;

typedef struct {
	artik_list node;
	mbedtls_md_type_t md_alg_id;
} digest_algo_id;

typedef struct {
	mbedtls_x509_crt *chain;
	signer_info signer;
} signed_data;

artik_error pkcs7_get_signed_data(mbedtls_asn1_buf *buf, mbedtls_x509_crt *rootCA, signed_data *sig_data);

#endif /* __TINYARA_SECURITY_H__ */
