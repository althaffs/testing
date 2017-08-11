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


#include <string.h>
#include <openssl/engine.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/pkcs7.h>
#include <openssl/x509v3.h>

#include <artik_module.h>
#include <artik_security.h>
#include <artik_list.h>
#include <artik_log.h>
#include "os_security.h"

#define ARTIK_SE_ENGINE_NAME  "artiksee"
#define COOKIE_SECURITY       "SEC"
#define COOKIE_SIGVERIF       "SIG"

struct cert_params {
	const char *cert_id;
	X509 *cert;
};

typedef struct {
	artik_list node;
	char cookie[4];
	ENGINE *engine;
} security_node;

typedef struct {
	artik_list node;
	char cookie[4];
	PKCS7 *p7;
	X509 *signer_cert;
	PKCS7_SIGNER_INFO *signer;
	EVP_MD_CTX *md_ctx;
} verify_node;

static bool openssl_global_init = false;
static artik_list *requested_node = NULL;
static artik_list *verify_nodes = NULL;

static void free_all(EC_KEY *ec_key, BIGNUM *x, BIGNUM *y,
		     EC_POINT *ec_point, BN_CTX *ctx)
{
	if (ec_key != NULL)
		EC_KEY_free(ec_key);
	if (x != NULL)
		BN_free(x);
	if (y != NULL)
		BN_free(y);
	if (ec_point != NULL)
		EC_POINT_free(ec_point);
	if (ctx != NULL)
		BN_CTX_free(ctx);
}

static int get_curve_nid(X509 *c)
{
	int ret = 0;

	if (c->cert_info->key->algor->parameter) {
		ASN1_TYPE *p = c->cert_info->key->algor->parameter;

		if (p && p->type == V_ASN1_OBJECT)
			ret =
		OBJ_obj2nid(c->cert_info->key->algor->parameter->value.object);
	}

	return ret;
}

static EC_KEY *create_pub_key(const unsigned char *pub_bits, int pub_len,
			      unsigned int nid)
{
	EC_GROUP *grp = NULL;
	EC_KEY *ec_key = NULL;
	EC_POINT *ec_point = NULL;
	BN_CTX *ctx = NULL;
	BIGNUM *x = BN_new();
	BIGNUM *y = BN_new();

	pub_len--;

	BN_bin2bn(pub_bits + 1, pub_len / 2, x);
	BN_bin2bn(pub_bits + 1 + pub_len / 2, pub_len / 2, y);

	grp = EC_GROUP_new_by_curve_name(nid);
	if (grp == NULL)
		return NULL;

	ec_key = EC_KEY_new();
	if (!EC_KEY_set_group(ec_key, grp)) {
		free_all(ec_key, x, y, NULL, NULL);
		return NULL;
	}

	if (!EC_KEY_set_private_key(ec_key, x)) {
		free_all(ec_key, x, y, NULL, NULL);
		return NULL;
	}

	ec_point = EC_POINT_new(grp);
	ctx = BN_CTX_new();

	if (!EC_POINT_set_affine_coordinates_GFp(grp, ec_point, x, y, ctx) ||
	    !EC_KEY_set_public_key(ec_key, ec_point)) {

		free_all(ec_key, x, y, ec_point, ctx);
		return NULL;
	}

	free_all(NULL, x, y, ec_point, ctx);

	return ec_key;
}

static int asn1_parse_int(unsigned char **p, unsigned int n, unsigned int *res)
{
	*res = 0;

	for ( ; n > 0; --n) {
		if ((**p < '0') || (**p > '9'))
			return -1;
		*res *= 10;
		*res += (*(*p)++ - '0');
	}

	return 0;
}

static bool convert_asn1_time(ASN1_TYPE *in, artik_time *out)
{
	unsigned char *in_str;
	int len;

	if (!in || !out || (in->type != V_ASN1_UTCTIME))
		return false;

	in_str = in->value.asn1_string->data;
	len = strlen((const char *)in_str);
	memset(out, 0, sizeof(*out));

	/* Parse date */
	if (asn1_parse_int(&in_str, 2, &out->year))
		return false;
	if (asn1_parse_int(&in_str, 2, &out->month))
		return false;
	if (asn1_parse_int(&in_str, 2, &out->day))
		return false;
	if (asn1_parse_int(&in_str, 2, &out->hour))
		return false;
	if (asn1_parse_int(&in_str, 2, &out->minute))
		return false;

	/* Parse seconds if available */
	if (len > 10) {
		if (asn1_parse_int(&in_str, 2, &out->second))
			return false;
	}

	/* Check if we have 'Z' as expected */
	if ((len > 12) && (*in_str != 'Z'))
		return false;

	/* Adjust to full year */
	out->year += 100 * (out->year < 50);
	out->year += 1900;

	return true;
}

artik_error os_security_request(artik_security_handle *handle)
{
	ENGINE *engine = NULL;
	security_node *node = (security_node *) artik_list_add(&requested_node,
						0, sizeof(security_node));

	if (!node)
		return E_NO_MEM;
	node->node.handle = (ARTIK_LIST_HANDLE) node;
	*handle = (artik_security_handle)node;

	/* First try to load and init the OpenSSL SE engine */
	ENGINE_load_builtin_engines();
	engine = ENGINE_by_id(ARTIK_SE_ENGINE_NAME);
	if (!engine || !ENGINE_init(engine)) {
		if (engine)
			ENGINE_free(engine);
		artik_list_delete_node(&requested_node,
							(artik_list *)node);
		return E_ACCESS_DENIED;
	}
	if (!ENGINE_set_default(engine, ENGINE_METHOD_RAND |
					ENGINE_METHOD_ECDSA)) {
		ENGINE_finish(engine);
		ENGINE_free(engine);
		artik_list_delete_node(&requested_node, (artik_list *)node);
		return E_ACCESS_DENIED;
	}

	/* Create a new node in the requested list */
	node->engine = engine;
	strncpy(node->cookie, COOKIE_SECURITY, sizeof(node->cookie));

	return S_OK;
}

artik_error os_security_release(artik_security_handle handle)
{
	security_node *node = (security_node *)
		artik_list_get_by_handle(requested_node,
						(ARTIK_LIST_HANDLE) handle);

	if (!node || strncmp(node->cookie, COOKIE_SECURITY, sizeof(node->cookie)))
		return E_BAD_ARGS;

	/* Unload the engine and clean up */
	if (node->engine) {
		/* Code is not obvious, taken from Oleg's sdrclient lib */
		OBJ_cleanup();
		EVP_cleanup();
		ENGINE_unregister_ciphers(node->engine);
		ENGINE_unregister_digests(node->engine);
		ENGINE_unregister_ECDSA(node->engine);
		ENGINE_unregister_ECDH(node->engine);
		ENGINE_unregister_pkey_meths(node->engine);
		ENGINE_unregister_RAND(node->engine);
		ENGINE_remove(node->engine);
		ENGINE_cleanup();
		ENGINE_finish(node->engine);
		ENGINE_free(node->engine);
	}

	artik_list_delete_node(&requested_node, (artik_list *)node);

	return S_OK;
}

artik_error os_security_get_certificate(artik_security_handle handle,
					char **cert)
{
	security_node *node = (security_node *)
		artik_list_get_by_handle(requested_node,
						(ARTIK_LIST_HANDLE) handle);
	struct cert_params params;
	BIO *b64 = NULL;
	BUF_MEM *bptr = NULL;

	if (!node || !node->engine || !cert || *cert ||
			strncmp(node->cookie, COOKIE_SECURITY, sizeof(node->cookie)))
		return E_BAD_ARGS;

	memset(&params, 0, sizeof(params));
	params.cert_id = "ARTIK/0";

	/* Get the certificate from the SE */
	ENGINE_ctrl_cmd(node->engine, "LOAD_CERT_CTRL", 0, &params, NULL, 0);

	/* Convert the X509 cert into a string */
	b64 = BIO_new(BIO_s_mem());
	PEM_write_bio_X509(b64, params.cert);
	BIO_write(b64, "\0", 1);
	BIO_get_mem_ptr(b64, &bptr);

	/* Allocate memory for the certificate string */
	*cert = (char *)malloc(bptr->length);
	if (!(*cert)) {
		BIO_free(b64);
		return E_NO_MEM;
	}

	/* Copy the cert content into the string */
	BIO_read(b64, (void *)(*cert), bptr->length);
	BIO_free(b64);

	return S_OK;
}

artik_error os_security_get_key_from_cert(artik_security_handle handle,
					  const char *cert, char **key)
{
	security_node *node = (security_node *)
		artik_list_get_by_handle(requested_node,
					(ARTIK_LIST_HANDLE) handle);
	artik_error ret = S_OK;
	X509 *x509_cert = NULL;
	BIO *b64 = NULL;
	EC_KEY *ec_key = NULL;
	int bits_len = 0;
	int nid = 0;
	BUF_MEM *bptr = NULL;
	unsigned char *ec_bits = NULL;

	if (!node || !node->engine || !cert || !key || *key ||
			strncmp(node->cookie, COOKIE_SECURITY, sizeof(node->cookie)))
		return E_BAD_ARGS;

	/* Convert certificate string into a BIO */
	b64 = BIO_new(BIO_s_mem());
	if (!b64)
		return E_NO_MEM;
	BIO_write(b64, cert, strlen(cert));

	/* Extract X509 cert from the BIO */
	x509_cert = PEM_read_bio_X509(b64, NULL, NULL, NULL);
	if (!x509_cert) {
		ret = E_BAD_ARGS;
		goto exit;
	}
	if (b64)
		BIO_free(b64);

	/* Get EC KEY out of the certificate */
	bits_len = x509_cert->cert_info->key->public_key->length;
	nid = get_curve_nid(x509_cert);
	ec_bits = x509_cert->cert_info->key->public_key->data;
	ec_key = create_pub_key(ec_bits, bits_len, nid);
	if (!ec_key) {
		ret = E_BAD_ARGS;
		goto exit;
	}

	/* Write result into a new BIO */
	b64 = BIO_new(BIO_s_mem());
	PEM_write_bio_ECPrivateKey(b64, ec_key, NULL, NULL, 0, NULL, NULL);
	BIO_write(b64, "\0", 1);
	BIO_get_mem_ptr(b64, &bptr);

	/* Allocate memory for the key string */
	*key = (char *)malloc(sizeof(**key) * bptr->length);
	if (!(*key)) {
		ret = E_NO_MEM;
		goto exit;
	}

	/* Copy the key content into the string */
	BIO_read(b64, (void *)(*key), bptr->length);

exit:
	if (x509_cert)
		X509_free(x509_cert);
	if (b64)
		BIO_free(b64);
	return ret;
}

artik_error os_security_get_root_ca(artik_security_handle handle,
					char **root_ca)
{
	return E_NOT_SUPPORTED;
}

artik_error os_get_random_bytes(artik_security_handle handle,
				unsigned char *rand, int len)
{
	security_node *node = (security_node *)
		artik_list_get_by_handle(requested_node,
						(ARTIK_LIST_HANDLE) handle);

	if (!node || !node->engine || !rand ||
			strncmp(node->cookie, COOKIE_SECURITY, sizeof(node->cookie)))
		return E_BAD_ARGS;

	if (!RAND_bytes(rand, len))
		return E_BAD_ARGS;

	return S_OK;
}

artik_error os_get_certificate_sn(artik_security_handle handle,
				unsigned char *sn, unsigned int *len)
{
	X509		*x509 = NULL;
	BIO		*ibio = NULL;
	ASN1_INTEGER	*serial = NULL;
	BIGNUM		*serialBN = NULL;
	char		*cert = NULL;
	artik_error	ret = S_OK;

	if (!sn || !len || (*len == 0))
		return E_BAD_ARGS;

	ret = os_security_get_certificate(handle, &cert);
	if (ret != S_OK)
		return ret;

	ibio = BIO_new(BIO_s_mem());
	if (BIO_puts(ibio, cert) < 0) {
		ret = E_INVALID_VALUE;
		goto exit;
	}

	x509 = PEM_read_bio_X509_AUX(ibio, NULL, NULL, NULL);
	if (!x509) {
		ret = E_BAD_ARGS;
		goto exit;
	}

	serial = X509_get_serialNumber(x509);
	if (!serial) {
		ret = E_INVALID_VALUE;
		goto exit;
	}

	serialBN = ASN1_INTEGER_to_BN(serial, NULL);
	if (BN_num_bytes(serialBN) > *len) {
		ret = E_BAD_ARGS;
		goto exit;
	}

	*len = BN_bn2bin(serialBN, sn);
exit:
	if (serialBN)
		BN_free(serialBN);
	if (cert)
		free(cert);
	if (x509)
		X509_free(x509);
	if (ibio)
		BIO_free(ibio);
	return ret;
}

artik_error os_verify_signature_init(artik_security_handle *handle,
		const char *signature_pem, const char *root_ca,
		const artik_time *signing_time_in, artik_time *signing_time_out)
{
	verify_node *node = NULL;
	BIO *sigbio = NULL;
	BIO *cabio = NULL;
	X509 *ca_cert = NULL;
	X509_STORE_CTX store_ctx;
	X509_STORE *store = NULL;
	artik_error ret = S_OK;

	STACK_OF(PKCS7_SIGNER_INFO) * sinfos = NULL;

	if (!handle || !signature_pem || !root_ca)
		return E_BAD_ARGS;

	node = (verify_node *)artik_list_add(&verify_nodes, 0, sizeof(verify_node));
	if (!node)
		return E_NO_MEM;

	/* Do OpenSSL one-time global initialization stuff */
	if (!openssl_global_init) {
		CRYPTO_malloc_init();
		OpenSSL_add_all_algorithms();
		openssl_global_init = true;
	}

	/* Parse root CA */
	cabio = BIO_new(BIO_s_mem());
	if (!cabio) {
		log_dbg("Failed to create bio for root CA");
		ret = E_NO_MEM;
		goto exit;
	}

	BIO_write(cabio, root_ca, strlen(root_ca));
	ca_cert = PEM_read_bio_X509_AUX(cabio, NULL, 0, NULL);
	if (!ca_cert) {
		log_dbg("Failed to parse bio for root CA certificate");
		ret = E_SECURITY_INVALID_X509;
		goto exit;
	}

	/* Parse PKCS7 signature */
	sigbio = BIO_new(BIO_s_mem());
	if (!sigbio) {
		log_dbg("Failed to create bio for PKCS7 signature");
		ret = E_NO_MEM;
		goto exit;
	}

	BIO_write(sigbio, signature_pem, strlen(signature_pem));
	node->p7 = PEM_read_bio_PKCS7(sigbio, NULL, 0, NULL);
	if (!node->p7) {
		log_dbg("Could not parse PKCS7 signature form PEM");
		ret = E_SECURITY_INVALID_PKCS7;
		goto exit;
	}

	/* Perform some checkng on the PKCS7 signature */
	if (!PKCS7_type_is_signed(node->p7)) {
		log_dbg("Wrong type of PKCS7, should be signed");
		ret = E_SECURITY_INVALID_PKCS7;
		goto exit;
	}

	sinfos = PKCS7_get_signer_info(node->p7);
	if (!sinfos || !sk_PKCS7_SIGNER_INFO_num(sinfos)) {
		log_dbg("No signers found in the PKCS7 structure");
		ret = E_SECURITY_INVALID_PKCS7;
		goto exit;
	}

	if (sk_PKCS7_SIGNER_INFO_num(sinfos) > 1) {
		log_err("Only verification for one signer is supported");
		ret = E_NOT_SUPPORTED;
		goto exit;
	}

	/* Verify signer certificate against ca chain */
	node->signer = sk_PKCS7_SIGNER_INFO_value(sinfos, 0);
	node->signer_cert = X509_find_by_issuer_and_serial(node->p7->d.sign->cert,
			node->signer->issuer_and_serial->issuer,
			node->signer->issuer_and_serial->serial);

	if (!node->signer_cert) {
		log_dbg("Failed to find the signer certificate in the PKCS7 structure");
		ret = E_SECURITY_INVALID_PKCS7;
		goto exit;
	}

	store = X509_STORE_new();
	X509_STORE_add_cert(store, ca_cert);
	if (!X509_STORE_CTX_init(&store_ctx, store, node->signer_cert,
				node->p7->d.sign->cert)) {
		log_dbg("Failed to initialize verification context");
		ret = E_SECURITY_CA_VERIF_FAILED;
		goto exit;
	}

	X509_STORE_CTX_set_purpose(&store_ctx, X509_PURPOSE_CRL_SIGN);
	if (X509_verify_cert(&store_ctx) <= 0) {
		log_dbg("Signer certificate verification failed (err=%d)",
				X509_STORE_CTX_get_error(&store_ctx));
		X509_STORE_CTX_cleanup(&store_ctx);
		X509_STORE_free(store);
		ret = E_SECURITY_CA_VERIF_FAILED;
		goto exit;
	}

	X509_STORE_CTX_cleanup(&store_ctx);
	X509_STORE_free(store);

	/* Verify signer attributes */
	if (!node->signer->auth_attr ||
			!sk_X509_ATTRIBUTE_num(node->signer->auth_attr)) {
		log_dbg("Signer does not have attributes");
		ret = E_SECURITY_INVALID_PKCS7;
		goto exit;
	}

	if (signing_time_in || signing_time_out) {
		artik_time pkcs7_signing_time;
		ASN1_TYPE *signing_time_attr = PKCS7_get_signed_attribute(node->signer,
				NID_pkcs9_signingTime);
		if (!signing_time_attr) {
			log_dbg("Signer does not have signing time");
			ret = E_SECURITY_INVALID_PKCS7;
			goto exit;
		}

		if (!convert_asn1_time(signing_time_attr, &pkcs7_signing_time)) {
			log_dbg("Could not parse signing time from PKCS7");
			ret = E_SECURITY_INVALID_PKCS7;
			goto exit;
		}

		log_info("SigningTime: %02d/%02d/%d %02d:%02d:%02d\n",
				pkcs7_signing_time.month, pkcs7_signing_time.day,
				pkcs7_signing_time.year, pkcs7_signing_time.hour,
				pkcs7_signing_time.minute, pkcs7_signing_time.second);

		if (signing_time_out)
			memcpy(signing_time_out, &pkcs7_signing_time, sizeof(artik_time));

		if (signing_time_in) {
			artik_time_module *time =
				(artik_time_module *)artik_request_api_module("time");

			if (time->compare_dates(&pkcs7_signing_time,
					signing_time_in) == -1) {
				log_dbg("Signing time happened before current signing time");
				ret = E_SECURITY_SIGNING_TIME_ROLLBACK;
				goto exit;
			}
		}
	}

	/* Prepare context for message digest computation */
	node->md_ctx = EVP_MD_CTX_create();
	if (!node->md_ctx) {
		log_dbg("Failed to create digest context");
		ret = E_NO_MEM;
		goto exit;
	}

	if (!EVP_DigestInit_ex(node->md_ctx,
			EVP_get_digestbynid(
				OBJ_obj2nid(node->signer->digest_alg->algorithm)),
			NULL)) {
		log_dbg("Failed to initialize digest context");
		EVP_MD_CTX_destroy(node->md_ctx);
		ret = E_BAD_ARGS;
		goto exit;
	}

	strncpy(node->cookie, COOKIE_SIGVERIF, sizeof(node->cookie));
	node->node.handle = (ARTIK_LIST_HANDLE) node;
	*handle = (artik_security_handle)node;

exit:
	if (cabio)
		BIO_free(cabio);
	if (sigbio)
		BIO_free(sigbio);
	if (ca_cert)
		X509_free(ca_cert);

	if (ret != S_OK)
		artik_list_delete_node(&verify_nodes, (artik_list *)node);

	return ret;
}

artik_error os_verify_signature_update(artik_security_handle handle,
		unsigned char *data, unsigned int data_len)
{
	verify_node *node = (verify_node *)artik_list_get_by_handle(verify_nodes,
		(ARTIK_LIST_HANDLE)handle);

	if (!node || !data || !data_len ||
			strncmp(node->cookie, COOKIE_SIGVERIF, sizeof(node->cookie)))
		return E_BAD_ARGS;

	if (!EVP_DigestUpdate(node->md_ctx, data, data_len)) {
		log_dbg("Failed to update data for digest computation");
		return E_BAD_ARGS;
	}

	return S_OK;
}

artik_error os_verify_signature_final(artik_security_handle handle)
{
	artik_error ret = S_OK;
	ASN1_OCTET_STRING *data_digest = NULL;
	EVP_PKEY *pkey = NULL;
	unsigned char md_dat[EVP_MAX_MD_SIZE], *abuf = NULL;
	unsigned int md_len = 0;
	int alen = 0;
	verify_node *node = (verify_node *)artik_list_get_by_handle(verify_nodes,
		(ARTIK_LIST_HANDLE)handle);

	if (!node || strncmp(node->cookie, COOKIE_SIGVERIF, sizeof(node->cookie)))
		return E_BAD_ARGS;

	if (!EVP_DigestFinal_ex(node->md_ctx, md_dat, &md_len)) {
		log_dbg("Failed to finalize digest computation");
		ret = E_BAD_ARGS;
		goto exit;
	}

	/* Extract digest from the signer info */
	data_digest = PKCS7_digest_from_attributes(node->signer->auth_attr);
	if (!data_digest) {
		log_dbg("Failed to get digest from signer's attributes");
		ret = E_SECURITY_INVALID_PKCS7;
		goto exit;
	}

	/* Compare with the signature digest */
	if ((data_digest->length != (int)md_len) ||
			memcmp(data_digest->data, md_dat, md_len)) {
		log_dbg("Computed data digest mismatch");
		ret = E_SECURITY_DIGEST_MISMATCH;
		goto exit;
	}

	/* Verify Signature */
	if (!EVP_VerifyInit_ex(node->md_ctx,
			EVP_get_digestbynid(
					OBJ_obj2nid(node->signer->digest_alg->algorithm)),
			NULL)) {
		log_dbg("Failed to initialize signature verification");
		ret = E_SECURITY_INVALID_PKCS7;
		goto exit;
	}

	alen = ASN1_item_i2d((ASN1_VALUE *)node->signer->auth_attr, &abuf,
			ASN1_ITEM_rptr(PKCS7_ATTR_VERIFY));
	if (alen <= 0) {
		log_dbg("Failed to get signature from signer info");
		ret = E_SECURITY_INVALID_PKCS7;
		goto exit;
	}

	if (!EVP_VerifyUpdate(node->md_ctx, abuf, alen)) {
		log_dbg("Failed to feed signature for verification");
		ret = E_BAD_ARGS;
		goto exit;
	}

	/* Extract public key from signer certificate */
	pkey = X509_get_pubkey(node->signer_cert);
	if (!pkey) {
		log_dbg("Failed to get public key from signer certificate");
		ret = E_SECURITY_INVALID_X509;
		goto exit;
	}

	/* Perform the final verification */
	if (EVP_VerifyFinal(node->md_ctx, node->signer->enc_digest->data,
			node->signer->enc_digest->length, pkey) <= 0) {
		log_dbg("Signature verification failed (err=%lu)", ERR_get_error());
		ret = E_SECURITY_SIGNATURE_MISMATCH;
		goto exit;
	}

exit:
	if (abuf)
		OPENSSL_free(abuf);
	if (pkey)
		EVP_PKEY_free(pkey);
	PKCS7_free(node->p7);
	EVP_MD_CTX_destroy(node->md_ctx);
	artik_list_delete_node(&verify_nodes, (artik_list *)node);

	return ret;
}
