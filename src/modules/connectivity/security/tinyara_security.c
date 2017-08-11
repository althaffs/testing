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


#include <tls/see_api.h>
#include <tls/x509_crt.h>
#include <tls/pk.h>
#include <tls/pem.h>
#include <tls/oid.h>

#include <artik_log.h>
#include <artik_security.h>
#include <artik_time.h>

#include "tinyara/tinyara_security.h"

typedef struct {
	artik_list node;
} security_node;

typedef struct {
	artik_list node;
	mbedtls_md_context_t md_ctx;
	mbedtls_asn1_buf data_digest;
	mbedtls_asn1_buf signed_digest;
	mbedtls_asn1_buf message_data;
	mbedtls_md_type_t digest_alg_id;
	mbedtls_x509_crt *chain;
} verify_node;

static artik_list *requested_nodes = NULL;
static artik_list *verify_nodes = NULL;

#define PEM_BEGIN_CRT	"-----BEGIN CERTIFICATE-----\n"
#define PEM_END_CRT	"-----END CERTIFICATE-----\n"

artik_error os_security_request(artik_security_handle *handle)
{
	security_node *node = (security_node *) artik_list_add(&requested_nodes,
						0, sizeof(security_node));

	if (!node)
		return E_NO_MEM;

	node->node.handle = (ARTIK_LIST_HANDLE)node;
	*handle = (artik_security_handle)node;

	if (artik_list_size(requested_nodes) == 1)
		see_init();

	return S_OK;
}

artik_error os_security_release(artik_security_handle handle)
{
	security_node *node = (security_node *)artik_list_get_by_handle(
				requested_nodes, (ARTIK_LIST_HANDLE)handle);

	if (!node)
		return E_BAD_ARGS;

	artik_list_delete_node(&requested_nodes, (artik_list *)node);

	if (artik_list_size(requested_nodes) == 0)
		see_free();

	return S_OK;
}

artik_error os_security_get_certificate(artik_security_handle handle,
					char **cert)
{
	security_node *node = (security_node *)artik_list_get_by_handle(
				requested_nodes, (ARTIK_LIST_HANDLE)handle);
	artik_error err = S_OK;
	int ret = 0;
	unsigned char *cert_data = NULL;
	unsigned int cert_len = SEE_MAX_BUF_SIZE;
	unsigned char *pem_data = NULL;
	size_t pem_len = 4096;
	unsigned char *dev_cert = NULL;
	int dev_cert_len = 0;
	unsigned char *p = NULL;
	size_t len = 0;

	if (!node || !cert || *cert)
		return E_BAD_ARGS;

	cert_data = zalloc(cert_len);
	if (!cert_data)
		return E_NO_MEM;

	pem_data = zalloc(pem_len);
	if (!pem_data) {
		err = E_NO_MEM;
		goto exit;
	}

	ret = see_get_certificate(cert_data, &cert_len, FACTORYKEY_ARTIK_CERT,
									0);
	if (ret != SEE_OK) {
		log_err("Failed to get certificate (err=%d)", ret);
		err = E_ACCESS_DENIED;
		goto exit;
	}

	/* Ignore the root CA and go to the device certificate chain */
	dev_cert = cert_data + ((cert_data[2] << 8) + cert_data[3] + 4);
	dev_cert_len = cert_len - (dev_cert - cert_data);

	/* Ignore the first certificate in the chain */
	p = dev_cert;
	ret = mbedtls_asn1_get_tag(&p, dev_cert + dev_cert_len, &len,
			(MBEDTLS_ASN1_CONSTRUCTED|MBEDTLS_ASN1_SEQUENCE));
	if (ret) {
		log_err("Failed to find first certificate in the chain\n"
			"(err=%d)", ret);
		err = E_ACCESS_DENIED;
		goto exit;
	}

	/* Only get the second certificate in the chain */
	p += len;
	dev_cert = p;
	ret = mbedtls_asn1_get_tag(&p, dev_cert + dev_cert_len, &len,
			(MBEDTLS_ASN1_CONSTRUCTED|MBEDTLS_ASN1_SEQUENCE));
	if (ret) {
		log_err("Failed to find second certificate in the chain\n"
			"(err=%d)", ret);
		err = E_ACCESS_DENIED;
		goto exit;
	}

	/* Certificate should be in DER format, just export it as PEM */
	ret = mbedtls_pem_write_buffer(PEM_BEGIN_CRT, PEM_END_CRT, dev_cert,
			len + 4, pem_data, pem_len, &pem_len);
	if (ret) {
		log_err("Failed to write PEM certificate (err=%d)", ret);
		err = E_ACCESS_DENIED;
		goto exit;
	}

	*cert = strndup((char *)pem_data, pem_len);
	if (*cert == NULL) {
		err = E_NO_MEM;
		goto exit;
	}

exit:
	if (cert_data)
		free(cert_data);
	if (pem_data)
		free(pem_data);

	return err;
}

artik_error os_security_get_key_from_cert(artik_security_handle handle,
					const char *cert, char **key)
{
	security_node *node = (security_node *)artik_list_get_by_handle(
				requested_nodes, (ARTIK_LIST_HANDLE)handle);
	unsigned char *pem_data = NULL;
	unsigned int pem_len = 4096;
	int ret = 0;
	artik_error err = S_OK;
	mbedtls_x509_crt x509_cert;
	mbedtls_ecp_keypair *eck = NULL;
	size_t olen = 0;

	if (!node || !cert || !key || *key)
		return E_BAD_ARGS;

	pem_data = zalloc(pem_len);
	if (!pem_data) {
		err = E_NO_MEM;
		goto exit;
	}

	mbedtls_x509_crt_init(&x509_cert);
	ret = mbedtls_x509_crt_parse(&x509_cert, (const unsigned char *)cert,
							strlen(cert) + 1);
	if (ret) {
		log_err("Failed to parse certificate (err=%d)", ret);
		err = E_BAD_ARGS;
		goto exit;
	}

	/* Extract point from public key */
	eck = mbedtls_pk_ec(x509_cert.pk);
	ret = mbedtls_ecp_point_write_binary(&eck->grp, &eck->Q,
			MBEDTLS_ECP_PF_UNCOMPRESSED,
			&olen, pem_data, pem_len);
	if (ret) {
		log_err("Failed to extract point (err=%d)", ret);
		err = E_BAD_ARGS;
		goto exit;
	}

	/* Use X coordinate as private key */
	ret =  mbedtls_mpi_read_binary(&eck->d, pem_data + 1, (olen - 1) / 2);
	if (ret) {
		log_err("Failed to import X MPI (err=%d)", ret);
		err = E_BAD_ARGS;
		goto exit;
	}

	/* Export key to PEM format */
	ret = mbedtls_pk_write_key_pem(&(x509_cert.pk), pem_data, pem_len);
	if (ret) {
		log_err("Failed to write key (err=%d)", ret);
		err = E_BAD_ARGS;
		goto exit;
}

	*key = strndup((char *)pem_data, strlen((char *)pem_data));

exit:
	mbedtls_x509_crt_free(&x509_cert);
	if (pem_data)
		free(pem_data);

	return err;
}

artik_error os_security_get_root_ca(artik_security_handle handle,
								char **root_ca)
{
	security_node *node = (security_node *)artik_list_get_by_handle(
				requested_nodes, (ARTIK_LIST_HANDLE)handle);
	artik_error err = S_OK;
	int ret = 0;
	unsigned char *cert_data = NULL;
	unsigned int cert_len = SEE_MAX_BUF_SIZE;
	unsigned char *pem_data = NULL;
	size_t pem_len = 4096;
	int root_ca_len = 0;

	if (!node || !root_ca || *root_ca)
		return E_BAD_ARGS;

	cert_data = zalloc(cert_len);
	if (!cert_data)
		return E_NO_MEM;

	pem_data = zalloc(pem_len);
	if (!pem_data) {
		err = E_NO_MEM;
		goto exit;
	}

	ret = see_get_certificate(cert_data, &cert_len, FACTORYKEY_ARTIK_CERT,
									0);
	if (ret != SEE_OK) {
		log_err("Failed to get root CA (err=%d)", ret);
		err = E_ACCESS_DENIED;
		goto exit;
	}

	/* Get the root CA length base on the sequence tag */
	root_ca_len = (cert_data[2] << 8) + cert_data[3] + 4;

	/* Certificate should be in DER format, just export it as PEM */
	ret = mbedtls_pem_write_buffer(PEM_BEGIN_CRT, PEM_END_CRT, cert_data,
			root_ca_len, pem_data, pem_len, &pem_len);
	if (ret) {
		log_err("Failed to write PEM root CA (err=%d)", ret);
		err = E_ACCESS_DENIED;
		goto exit;
	}

	*root_ca = strndup((char *)pem_data, pem_len);
	if (*root_ca == NULL) {
		err = E_NO_MEM;
		goto exit;
	}

exit:
	if (cert_data)
		free(cert_data);
	if (pem_data)
		free(pem_data);

	return err;
}

artik_error os_get_random_bytes(artik_security_handle handle,
				unsigned char *rand, int len)
{
	artik_error err = S_OK;
	security_node *node = (security_node *)artik_list_get_by_handle(
				requested_nodes, (ARTIK_LIST_HANDLE)handle);
	int ret = 0;
	int total = 0;
	unsigned int *buf = NULL;

	if (!node || !rand || (len < 0))
		return E_BAD_ARGS;

	buf = malloc(SEE_MAX_RANDOM_SIZE);
	if (!buf)
		return E_NO_MEM;

	while (len >= SEE_MAX_RANDOM_SIZE) {
		ret = see_generate_random(buf, SEE_MAX_RANDOM_SIZE);
		if (ret != SEE_OK) {
			log_err("Failed to get random numbers (err=%d)\n", ret);
			err = E_ACCESS_DENIED;
			goto exit;
		}

		memcpy(rand + total, buf, SEE_MAX_RANDOM_SIZE);
		len -= SEE_MAX_RANDOM_SIZE;
		total += SEE_MAX_RANDOM_SIZE;
	}

	if (len) {
		ret = see_generate_random(buf, len);
		if (ret != SEE_OK) {
			log_err("Failed to get random numbers (err=%d)\n", ret);
			err = E_ACCESS_DENIED;
			goto exit;
		}

		memcpy(rand + total, buf, len);
	}

exit:
	free(buf);
	return err;
}

artik_error os_get_certificate_sn(artik_security_handle handle,
				unsigned char *sn, unsigned int *len)
{
	security_node *node = (security_node *)artik_list_get_by_handle(
				requested_nodes, (ARTIK_LIST_HANDLE)handle);
	int ret = 0;
	unsigned char *cert_data = NULL;
	unsigned int cert_len = SEE_MAX_BUF_SIZE;
	mbedtls_x509_crt cert;
	unsigned char *dev_cert = NULL;
	int dev_cert_len = 0;
	unsigned char *p = NULL;
	size_t plen = 0;

	if (!node || !sn || !len || (*len == 0))
		return E_BAD_ARGS;

	cert_data = zalloc(cert_len);
	if (!cert_data)
		return E_NO_MEM;

	ret = see_get_certificate(cert_data, &cert_len, FACTORYKEY_ARTIK_CERT,
									0);
	if (ret != SEE_OK) {
		log_err("Failed to get certificate (err=%d)", ret);
		free(cert_data);
		return E_ACCESS_DENIED;
	}

	/* Ignore the root CA and go to the device certificate chain */
	dev_cert = cert_data + ((cert_data[2] << 8) + cert_data[3] + 4);
	dev_cert_len = cert_len - (dev_cert - cert_data);

	/* Ignore the first certificate in the chain */
	p = dev_cert;
	ret = mbedtls_asn1_get_tag(&p, dev_cert + dev_cert_len, &plen,
		(MBEDTLS_ASN1_CONSTRUCTED|MBEDTLS_ASN1_SEQUENCE));
	if (ret) {
		log_err("Failed to find first certificate in the chain\n"
			"(err=%d)", ret);
		free(cert_data);
		return E_ACCESS_DENIED;
	}

	/* Only get the second certificate in the chain */
	p += plen;
	dev_cert = p;
	ret = mbedtls_asn1_get_tag(&p, dev_cert + dev_cert_len, &plen,
		(MBEDTLS_ASN1_CONSTRUCTED|MBEDTLS_ASN1_SEQUENCE));
	if (ret) {
		log_err("Failed to find second certificate in the chain\n"
			"(err=%d)", ret);
		free(cert_data);
		return E_ACCESS_DENIED;
	}

	mbedtls_x509_crt_init(&cert);
	ret = mbedtls_x509_crt_parse(&cert, dev_cert, plen + 4);
	if (ret) {
		log_err("Failed to parse certificate (err=%d)", ret);
		mbedtls_x509_crt_free(&cert);
		free(cert_data);
		return E_ACCESS_DENIED;
	}

	if (cert.serial.len > *len) {
		log_err("Buffer is too small");
		mbedtls_x509_crt_free(&cert);
		free(cert_data);
		return E_BAD_ARGS;
	}

	memcpy(sn, cert.serial.p, cert.serial.len);
	*len = cert.serial.len;

	mbedtls_x509_crt_free(&cert);
	free(cert_data);

	return S_OK;
}

/*
 * Compare two X.509 Names (aka rdnSequence).
 *
 * See RFC 5280 section 7.1, though we don't implement the whole algorithm:
 * we sometimes return unequal when the full algorithm would return equal,
 * but never the other way. (In particular, we don't do Unicode normalisation
 * or space folding.)
 *
 * Return 0 if equal, -1 otherwise.
 */
static int x509_name_cmp(const mbedtls_x509_name *a, const mbedtls_x509_name *b)
{
	/* Avoid recursion, it might not be optimised by the compiler */
	while (a != NULL || b != NULL) {
		if (a == NULL || b == NULL)
			return -1;

		/* type */
		if (a->oid.tag != b->oid.tag
			|| a->oid.len != b->oid.len
			|| memcmp(a->oid.p, b->oid.p, b->oid.len) != 0)
			return -1;

		/* value */
		if (a->val.tag != b->val.tag
			|| a->val.len != b->val.len
			|| memcmp(a->val.p, b->val.p, b->val.len) != 0)
			return -1;

		/* structure of the list of sets */
		if (a->next_merged != b->next_merged)
			return -1;

		a = a->next;
		b = b->next;
	}

	/* a == NULL == b */
	return 0;
}

static artik_error check_pkcs7_validity(signed_data *sig_data)
{
	if (x509_name_cmp(&sig_data->signer.issuer, &sig_data->chain->issuer) != 0) {
		log_dbg("Issuer distinguished name does not match.");
		return E_SECURITY_INVALID_PKCS7;
	}

	if (sig_data->signer.serial.len == sig_data->chain->issuer_id.len
		&& memcmp(sig_data->signer.serial.p,
				  sig_data->chain->issuer_id.p, sig_data->chain->issuer_id.len) == 0) {
		log_dbg("Issuer serial number does not match.");
		return E_SECURITY_INVALID_PKCS7;
	}

	if ((sig_data->chain->ext_types & MBEDTLS_X509_EXT_EXTENDED_KEY_USAGE) == 0) {
		log_dbg("Extended key usage extension not found.");
		return E_SECURITY_INVALID_PKCS7;
	}

	if (mbedtls_x509_crt_check_extended_key_usage(
			sig_data->chain,
			MBEDTLS_OID_CODE_SIGNING,
			MBEDTLS_OID_SIZE(MBEDTLS_OID_CODE_SIGNING)) != 0) {
		log_dbg("Signer certificate verification failed: The purpose of the certificate is not digitalSignature.");
		return E_SECURITY_INVALID_PKCS7;
	}

	if (sig_data->signer.digest_alg_id != MBEDTLS_MD_SHA256) {
		log_dbg("Only verification with SHA256 is supported.");
		return E_NOT_SUPPORTED;
	}

	return S_OK;
}

static artik_error initialize_md_context(verify_node *node, signed_data *sig_data)
{
	const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(sig_data->signer.digest_alg_id);

	if (!md_info) {
		log_dbg("SHA256 is not supported by mbedtls.");
		return E_SECURITY_INVALID_PKCS7;
	}

	mbedtls_md_init(&node->md_ctx);
	if (mbedtls_md_setup(&node->md_ctx, md_info, 0) != 0) {
		log_dbg("Failed to initialize digest context.");
		return E_BAD_ARGS;
	}

	if (mbedtls_md_starts(&node->md_ctx) != 0) {
		log_dbg("Failed to prepare digest context.");
		mbedtls_md_free(&node->md_ctx);
		return E_BAD_ARGS;
	}

	return S_OK;
}

static artik_error copy_node_data(verify_node *node, signed_data *sig_data)
{
	size_t data_digest_len =  sig_data->signer.authenticated_attributes.digest.len;
	size_t signed_digest_len = sig_data->signer.encrypted_digest.len;
	size_t message_data_len = sig_data->signer.authenticated_attributes.raw.len;

	node->data_digest.p = malloc(sizeof(unsigned char) * data_digest_len);
	node->signed_digest.p = malloc(sizeof(unsigned char) * signed_digest_len);
	node->message_data.p = malloc(sizeof(unsigned char) * message_data_len);

	if (!node->data_digest.p || !node->signed_digest.p || !node->message_data.p) {
		if (node->data_digest.p)
			free(node->data_digest.p);

		if (node->signed_digest.p)
			free(node->signed_digest.p);

		if (node->message_data.p)
			free(node->message_data.p);

		return E_NO_MEM;
	}

	node->data_digest.len = data_digest_len;
	node->signed_digest.len = signed_digest_len;
	node->message_data.len = message_data_len;

	memcpy(node->data_digest.p, sig_data->signer.authenticated_attributes.digest.p, data_digest_len);
	memcpy(node->signed_digest.p, sig_data->signer.encrypted_digest.p, signed_digest_len);
	memcpy(node->message_data.p, sig_data->signer.authenticated_attributes.raw.p, message_data_len);
	*(node->message_data.p) = 0x31;

	return S_OK;
}

artik_error os_verify_signature_init(artik_security_handle *handle,
		const char *signature_pem, const char *root_ca,
		const artik_time *signing_time_in, artik_time *signing_time_out)
{
	artik_error err = S_OK;
	int ret;
	mbedtls_asn1_buf buf_pkcs7;
	signed_data sig_data;
	mbedtls_x509_crt x509_crt_root_ca;
	mbedtls_pem_context pem_ctx;
	artik_time signing_time;
	mbedtls_x509_time *x509_signing_time;

	if (!handle)
		return E_BAD_ARGS;

	verify_node *node = (verify_node *)artik_list_add(&verify_nodes, 0, sizeof(verify_node));

	if (!node)
		return E_NO_MEM;

	mbedtls_pem_init(&pem_ctx);

	if (mbedtls_pem_read_buffer(&pem_ctx,
								"-----BEGIN PKCS7-----",
								"-----END PKCS7-----",
								(unsigned char *)signature_pem,
								NULL,
								0,
								&buf_pkcs7.len) != 0) {
		log_dbg("failed to parse signature_pem.");
		err = E_SECURITY_INVALID_PKCS7;
		goto cleanup_node;
	}

	buf_pkcs7.p = pem_ctx.buf;
	buf_pkcs7.len = pem_ctx.buflen;

	mbedtls_x509_crt_init(&x509_crt_root_ca);
	ret = mbedtls_x509_crt_parse(&x509_crt_root_ca, (unsigned char *)root_ca, strlen(root_ca)+1);
	if (ret != 0) {
		log_dbg("Failed to parse root ca certificate (err %d).", ret);
		err = E_SECURITY_INVALID_X509;
		goto cleanup_pem;
	}

	err = pkcs7_get_signed_data(&buf_pkcs7, &x509_crt_root_ca, &sig_data);
	if (err != S_OK)
		goto cleanup;

	x509_signing_time = &sig_data.signer.authenticated_attributes.signing_time;
	signing_time.second = x509_signing_time->sec;
	signing_time.minute = x509_signing_time->min;
	signing_time.hour = x509_signing_time->hour;
	signing_time.day = x509_signing_time->day;
	signing_time.month = x509_signing_time->mon;
	signing_time.year = x509_signing_time->year;
	signing_time.day_of_week = -1;
	signing_time.msecond = 0;

	log_info("SigningTime: %02d/%02d/%d %02d:%02d:%02d\n",
			 signing_time.month, signing_time.day, signing_time.year,
			 signing_time.hour, signing_time.minute, signing_time.second);

	if (signing_time_out)
		memcpy(signing_time_out, &signing_time, sizeof(artik_time));


	if (signing_time_in) {
		artik_time_module *time = (artik_time_module *)artik_request_api_module("time");

		if (time->compare_dates(signing_time_in, &signing_time) > 0) {
			log_dbg("Signing time happened before current signing time");
			err = E_SECURITY_SIGNING_TIME_ROLLBACK;
			goto cleanup;
		}
	}

	err = check_pkcs7_validity(&sig_data);
	if (err != S_OK)
		goto cleanup;

	err = initialize_md_context(node, &sig_data);
	if (err != S_OK)
		goto cleanup;

	err = copy_node_data(node, &sig_data);
	if (err != S_OK)
		goto cleanup;

	node->chain = sig_data.chain;
	node->digest_alg_id = sig_data.signer.digest_alg_id;
	node->node.handle = (ARTIK_LIST_HANDLE)node;
	*handle = (artik_security_handle)node;

cleanup:
	mbedtls_x509_crt_free(&x509_crt_root_ca);
cleanup_pem:
	mbedtls_pem_free(&pem_ctx);
cleanup_node:
	if (err != S_OK)
		artik_list_delete_node(&verify_nodes, (artik_list *)node);

	return err;
}

artik_error os_verify_signature_update(artik_security_handle handle,
		unsigned char *data, unsigned int data_len)
{
	verify_node *node = (verify_node *)artik_list_get_by_handle(verify_nodes, (ARTIK_LIST_HANDLE)handle);

	if (!node || !data || !data_len)
		return E_BAD_ARGS;

	if (mbedtls_md_update(&node->md_ctx, data, data_len) != 0) {
		log_dbg("Failed to update data for digest computation.");
		return E_BAD_ARGS;
	}

	return S_OK;
}

artik_error os_verify_signature_final(artik_security_handle handle)
{
	artik_error err = S_OK;
	int ret = 0;
	unsigned char md_dat[MBEDTLS_MD_MAX_SIZE];
	unsigned char hash[MBEDTLS_MD_MAX_SIZE];
	unsigned char md_len;
	verify_node *node = (verify_node *)artik_list_get_by_handle(verify_nodes, (ARTIK_LIST_HANDLE)handle);

	if (!node)
		return E_BAD_ARGS;

	/* Compute final hash */
	if (mbedtls_md_finish(&node->md_ctx, md_dat) != 0) {
		log_dbg("Failed to finalize digest computation");
		err = E_BAD_ARGS;
		goto cleanup;
	}

	md_len = mbedtls_md_get_size(node->md_ctx.md_info);

	/* Compare with the signer info digest */
	if (md_len != node->data_digest.len || memcmp(node->data_digest.p, md_dat, md_len)) {
		log_dbg("Computed digest mismatch.");
		err = E_SECURITY_DIGEST_MISMATCH;
		goto cleanup;
	}

	/* Compute hash of the message */
	if (mbedtls_md(node->md_ctx.md_info, node->message_data.p, node->message_data.len, hash) != 0) {
		log_dbg("Failed to compute hash of AuthenticatedAttribute.");
		err = E_SECURITY_SIGNATURE_MISMATCH;
		goto cleanup;
	}

	/* Verify signature of the message*/
	ret = mbedtls_pk_verify(
		&node->chain->pk,
		node->digest_alg_id, hash,
		0,
		node->signed_digest.p,
		node->signed_digest.len);
	if (ret != 0) {
		log_dbg("Signature verification failed. (err %d)", ret);
		err = E_SECURITY_SIGNATURE_MISMATCH;
		goto cleanup;
	}

cleanup:
	mbedtls_md_free(&node->md_ctx);
	free(node->data_digest.p);
	free(node->signed_digest.p);
	free(node->message_data.p);

	mbedtls_x509_crt_free(node->chain);
	artik_list_delete_node(&verify_nodes, (artik_list *)node);

	return err;
}
