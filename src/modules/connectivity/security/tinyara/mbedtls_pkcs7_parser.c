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

#include "tinyara_security.h"

#include <artik_log.h>

#include <tls/oid.h>

#define OID_PKCS7 MBEDTLS_OID_PKCS "\x07"
#define OID_PKCS7_DATA OID_PKCS7 "\x01"
#define OID_PKCS7_SIGNED_DATA OID_PKCS7 "\x02"
#define OID_PKCS9_CONTENT_TYPE MBEDTLS_OID_PKCS9 "\x03"
#define OID_PKCS9_SIGNING_TIME MBEDTLS_OID_PKCS9 "\x05"
#define OID_PKCS9_MESSAGE_DIGEST MBEDTLS_OID_PKCS9 "\x04"

/*
 * Parse AlgorithmIdentifier for hash algorithm
 * AlgorithmIdentifier  ::=  SEQUENCE  {
 *      algorithm               OBJECT IDENTIFIER,
 *      parameters              ANY DEFINED BY algorithm OPTIONAL  }
 */
static artik_error pkcs7_get_hash_alg(unsigned char **p, const unsigned char *end, mbedtls_md_type_t *md_alg)
{
	mbedtls_asn1_buf md_oid;
	mbedtls_asn1_buf md_params;

	if (mbedtls_asn1_get_alg(p, end, &md_oid, &md_params) != 0) {
		log_dbg("Unable to get OBJECT IDENTIFIER");
		return E_SECURITY_INVALID_PKCS7;
	}

	/* Make sure parameters is NULL */
	if (md_params.tag != MBEDTLS_ASN1_NULL) {
		log_dbg("parameters must be NULL");
		return E_SECURITY_INVALID_PKCS7;
	}

	if (mbedtls_oid_get_md_alg(&md_oid, md_alg) != 0) {
		log_dbg("Unable to convert OID to mbedtls_md_type_t.");
		return E_SECURITY_INVALID_PKCS7;
	}

	return S_OK;
}

/*
 * Parse AlgorithmIdentifier for encryption algorithm
 * AlgorithmIdentifier  ::=  SEQUENCE  {
 *      algorithm               OBJECT IDENTIFIER,
 *      parameters              ANY DEFINED BY algorithm OPTIONAL  }
 */
static artik_error pkcs7_get_encryption_alg(unsigned char **p, const unsigned char *end, mbedtls_pk_type_t *enc_alg)
{
	mbedtls_md_type_t md_alg;
	mbedtls_asn1_buf enc_oid;
	mbedtls_asn1_buf enc_params;

	enc_params.tag = 0;

	if (mbedtls_asn1_get_alg(p, end, &enc_oid, &enc_params) != 0) {
		log_dbg("Unable to get OBJECT IDENTIFIER");
		return E_SECURITY_INVALID_PKCS7;
	}

	/* Make sure parameters is NULL or absent */
	printf("params.tag = %d\n", enc_params.tag);
	if (enc_params.tag != MBEDTLS_ASN1_NULL && enc_params.tag != 0) {
		log_dbg("parameters must be NULL");
		return E_SECURITY_INVALID_PKCS7;
	}

	if (mbedtls_oid_get_sig_alg(&enc_oid, &md_alg, enc_alg) != 0) {
		log_dbg("Unable to convert OID to mbedtls_pk_type_t.");
		return E_SECURITY_INVALID_PKCS7;
	}

	return S_OK;
}


/*
 * Parse a DigestAlgorithmIdentifiers
 * DigestAlgorithmIdentifiers ::=
 *	SET OF DigestAlgorithmIdentifier
 *
 */
static artik_error pkcs7_get_digest_algorithm_identifiers(
	unsigned char **p,
	const unsigned char *end,
	artik_list **digest_algo_ids)
{
	artik_error err = S_OK;

	while (*p != end) {
		mbedtls_md_type_t md_alg_id;
		digest_algo_id *digest_id =
			(digest_algo_id *) artik_list_add(digest_algo_ids, NULL, sizeof(digest_algo_id));

		if (!digest_id) {
			log_dbg("Unable to allocate digest_algorithm_identifier");
			err = E_NO_MEM;
			goto cleanup;
		}

		err = pkcs7_get_hash_alg(p, end, &md_alg_id);
		if (err != S_OK)
			goto cleanup;

		digest_id->md_alg_id = md_alg_id;
	}

cleanup:
	if (err != S_OK)
		artik_list_delete_all(digest_algo_ids);

	return err;
}

static artik_error pkcs7_get_chain_certs(mbedtls_x509_crt *chain, const unsigned char *buf, size_t buflen)
{
	int i = 0;

	while (i < buflen) {
		int ret = mbedtls_x509_crt_parse_der(chain, buf + i, buflen - i);

		if (ret != 0) {
			log_dbg("Unable to parse x509 certs. (err %d)", ret);
			return E_SECURITY_INVALID_PKCS7;
		}

		i += (((unsigned int)buf[i + 2] << 8) | (unsigned int)buf[i + 3]) + 4;
	}

	return S_OK;
}

static artik_error pkcs7_get_authenticated_attributes(
	unsigned char **p,
	const unsigned char *end,
	authenticated_attributes *attr) {
	bool hasContentType = false;
	bool hasSigningTime = false;
	bool hasMessageDigest = false;

	while (*p != end) {
		size_t len;
		unsigned char *end_seq = NULL;
		mbedtls_asn1_buf oid;

		if (mbedtls_asn1_get_tag(p, end, &len,  MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE) != 0) {
			log_dbg("Attribute: SEQUENCE not found");
			return E_SECURITY_INVALID_PKCS7;
		}

		end_seq = *p + len;

		if (mbedtls_asn1_get_tag(p, end_seq, &oid.len, MBEDTLS_ASN1_OID) != 0) {
			log_dbg("Attribute: OID not found");
			return E_SECURITY_INVALID_PKCS7;
		}

		oid.p = *p;
		*p += oid.len;

		if (MBEDTLS_OID_CMP(OID_PKCS9_CONTENT_TYPE, &oid) == 0) {
			mbedtls_asn1_buf content_oid;

			hasContentType = true;

			if (mbedtls_asn1_get_tag(p, end_seq, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SET) != 0) {
				log_dbg("ContentType: SET not found.");
				return E_SECURITY_INVALID_PKCS7;
			}

			if (mbedtls_asn1_get_tag(p, end_seq, &content_oid.len, MBEDTLS_ASN1_OID) != 0) {
				log_dbg("ContentType: OID not found.");
				return E_SECURITY_INVALID_PKCS7;
			}
			content_oid.p = *p;
			*p += content_oid.len;

			if (MBEDTLS_OID_CMP(OID_PKCS7_DATA, &content_oid) != 0) {
				log_dbg("ContentType: OID not equal to { pkcs-7 1 }.");
				return E_SECURITY_INVALID_PKCS7;
			}
		} else if (MBEDTLS_OID_CMP(OID_PKCS9_SIGNING_TIME, &oid) == 0) {
			hasSigningTime = true;
			size_t signing_time_len;

			if (mbedtls_asn1_get_tag(
					p, end_seq, &signing_time_len,
					MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SET) != 0) {
				log_dbg("Signing time: SEQUENCE not found.");
				return E_SECURITY_INVALID_PKCS7;
			}

			if (mbedtls_x509_get_time(p, *p + signing_time_len, &attr->signing_time) != 0) {
				log_dbg("Signing time: mbedtls_x509_get_time failed.");
				return E_SECURITY_INVALID_PKCS7;
			}
		} else if (MBEDTLS_OID_CMP(OID_PKCS9_MESSAGE_DIGEST, &oid) == 0) {
			size_t digest_message_len;

			hasMessageDigest = true;

			if (mbedtls_asn1_get_tag(
					p, end_seq, &digest_message_len,
					MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SET) != 0) {
				log_dbg("Digest message: SET not found.");
				return E_SECURITY_INVALID_PKCS7;
			}

			if (mbedtls_asn1_get_tag(p, end_seq, &attr->digest.len, MBEDTLS_ASN1_OCTET_STRING) != 0) {
				log_dbg("Digest message: OCTET_STRING not found.");
				return E_SECURITY_INVALID_PKCS7;
			}

			attr->digest.p = *p;
			*p = *p + attr->digest.len;
		} else {
			char str_oid[32];

			mbedtls_oid_get_numeric_string(str_oid, 32, &oid);
			log_dbg("OID %s is not supported.", str_oid);
			return E_SECURITY_INVALID_PKCS7;
		}
	}

	if (hasContentType && hasSigningTime && hasMessageDigest)
		return S_OK;


	log_dbg("ContentType or SigningTime or MessageDigest is not found");
	return E_SECURITY_INVALID_PKCS7;
}

/*
 * Parse a SignerInfo
 * SignerInfo ::= SEQUENCE {
 *	version Version,
 *	issuerAndSerialNumber IssuerAndSerialNumber,
 *	digestAlgorithm DigestAlgorithmIdentifier,
 *	authenticatedAttributes
 *		[0] IMPLICIT Attributes OPTIONAL,
 *	digestEncryptionAlgorithm
 *		DigestEncryptionAlgorithmIdentifier,
 *	encryptedDigest EncryptedDigest,
 *	unauthenticatedAttributes
 *		[1] IMPLICIT Attributes OPTIONAL }
 */
static artik_error pkcs7_get_signer_info(
	unsigned char **p,
	const unsigned char *end,
	signer_info *sig_info,
	artik_list *digest_algo_ids)
{
	artik_error err;
	mbedtls_pk_type_t enc_alg_id;
	size_t len;
	int ret;
	int ver;

	if (mbedtls_asn1_get_tag(p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE) != 0) {
		log_dbg("SignerInfo: SEQUENCE not found");
		return E_SECURITY_INVALID_PKCS7;
	}

	/* Version ::= INTEGER */
	if (mbedtls_asn1_get_int(p, end, &ver) != 0) {
		log_dbg("SignerInfo: Version not found.");
		return E_SECURITY_INVALID_PKCS7;
	}

	if (ver != 1) {
		log_dbg("SignerInfo: Version %d is not supported.", ver);
		return E_SECURITY_INVALID_PKCS7;
	}

	/* IssuerAndSerialNumber ::= SEQUENCE {
	 *	issuer Name
	 *	serialNumber CertificateSerialNumber }
	 */
	if (mbedtls_asn1_get_tag(p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE) != 0) {
		log_dbg("IssuerAndSerialNumber: SEQUENCE not found.");
		return E_SECURITY_INVALID_PKCS7;
	}

	/* Name ::= CHOICE { -- only one possibility for now --
	 *       rdnSequence  RDNSequence }
	 *
	 *  RDNSequence ::= SEQUENCE OF RelativeDistinguishedName
	 */
	if (mbedtls_asn1_get_tag(p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE) != 0) {
		log_dbg("RDNSequence: SEQUENCE not found.");
		return E_SECURITY_INVALID_PKCS7;
	}

	/*
	 * RelativeDistinguishedName
	 */
	memset(&sig_info->issuer, 0, sizeof(mbedtls_x509_name));
	ret = mbedtls_x509_get_name(p, *p + len, &sig_info->issuer);
	if (ret != 0) {
		log_dbg("Name: mbedtls_x509_get_name failed (err %d)", ret);
		return E_SECURITY_INVALID_PKCS7;
	}

	/*
	 * CertificateSerialNumber
	 */
	if (mbedtls_x509_get_serial(p, end, &sig_info->serial) != 0) {
		log_dbg("CertificateSerialNumber: mbedtls_x509_get_serial failed.");
		return E_SECURITY_INVALID_PKCS7;
	}

	/*
	 * DigestAlgorithmIdentifier
	 */
	err = pkcs7_get_hash_alg(p, end, &sig_info->digest_alg_id);
	if (err != S_OK) {
		log_dbg("digestAlgorithm not found.");
		return err;
	}

	digest_algo_id *id = (digest_algo_id *) artik_list_get_by_pos(digest_algo_ids, 0);

	if (id->md_alg_id != sig_info->digest_alg_id) {
		log_dbg("digestAlgorithm of SignerInfo does not match with digestAlgorithms of SignedData");
		return E_SECURITY_INVALID_PKCS7;
	}

	sig_info->authenticated_attributes.raw.p = *p;
	/*
	 * AuthenticatedAttributes ::= SET OF Attribute
	 */
	if (mbedtls_asn1_get_tag(p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_CONTEXT_SPECIFIC) != 0) {
		log_dbg("AuthenticatedAttributes not found.");
		return E_SECURITY_INVALID_PKCS7;
	}
	sig_info->authenticated_attributes.raw.len = len + *p -	sig_info->authenticated_attributes.raw.p;

	err = pkcs7_get_authenticated_attributes(p, *p + len, &sig_info->authenticated_attributes);
	if (err != S_OK)
		return err;

	/*
	 * DigestEncryptionAlgorithmIdentifier ::= AlgorithmIdentifier
	 */
	err = pkcs7_get_encryption_alg(p, end, &enc_alg_id);
	if (err != S_OK) {
		log_dbg("DigestEncryptionAlogirthmIdentifier not found.");
		return err;
	}

	if (enc_alg_id != MBEDTLS_PK_ECDSA) {
		log_dbg("Only RSA is supported");
		return E_SECURITY_INVALID_PKCS7;
	}

	/*
	 * EncryptedDigest ::= OCTET STRING
	 */
	if (mbedtls_asn1_get_tag(p, end, &sig_info->encrypted_digest.len, MBEDTLS_ASN1_OCTET_STRING) != 0) {
		log_dbg("Digest message: OCTET_STRING is not found.");
		return E_SECURITY_INVALID_PKCS7;
	}

	sig_info->encrypted_digest.p = *p;
	*p = *p + sig_info->encrypted_digest.len;

	return S_OK;
}

/*
 * Parse a SignedData
 * SignedData ::= SEQUENCE {
 *	version Version,
 *	digestAlgorithms DigestAlgorithmIdentifiers,
 *	contentInfo contentInfo,
 *	certificates
 *		[0] IMPLICIT ExtendedCertificatesAndCertificates OPTIONAL,
 *	crls
 *		[1] IMPLICIT CertificateRevocationLists OPTIONAL,
 *	signerInfos SignerInfos }
 *
 * The field crls is just ignored.
 */
artik_error pkcs7_get_signed_data(mbedtls_asn1_buf *buf, mbedtls_x509_crt *rootCA, signed_data *sig_data)
{
	unsigned int flags;
	mbedtls_asn1_buf sig_data_oid;
	mbedtls_asn1_buf ct_oid;
	artik_error err = S_OK;
	unsigned char **p = &buf->p;
	const unsigned char *const end = buf->p + buf->len;
	size_t len;
	int ver;
	int ret;
	artik_list *digest_algo_ids = NULL;
	mbedtls_x509_crt *x509_current = NULL;
	mbedtls_x509_crt *x509_next = NULL;
	mbedtls_x509_crt *x509_prev = NULL;

	memset(sig_data, 0, sizeof(signed_data));

	if (mbedtls_asn1_get_tag(p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE)) {
		log_dbg("ContentInfo: SEQUENCE not found.");
		return E_SECURITY_INVALID_PKCS7;
	}

	if (mbedtls_asn1_get_tag(p, end, &sig_data_oid.len, MBEDTLS_ASN1_OID)) {
		log_dbg("ContentInfo: OID not found.");
		return E_SECURITY_INVALID_PKCS7;
	}

	sig_data_oid.p = *p;
	*p += sig_data_oid.len;

	if (MBEDTLS_OID_CMP(OID_PKCS7_SIGNED_DATA, &sig_data_oid) != 0) {
		log_dbg("ContentInfo: Bad OID.");
		return E_SECURITY_INVALID_PKCS7;
	}

	if (mbedtls_asn1_get_tag(p, end, &len, MBEDTLS_ASN1_CONTEXT_SPECIFIC | MBEDTLS_ASN1_CONSTRUCTED) != 0) {
		log_dbg("ContentInfo: content not found.");
		return E_SECURITY_INVALID_PKCS7;
	}

	if (*p + len != end) {
		log_dbg("ContentInfo: size does not match.");
		return E_SECURITY_INVALID_PKCS7;
	}

	if (mbedtls_asn1_get_tag(p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE)) {
		log_dbg("SignedData: SEQUENCE not found");
		return E_SECURITY_INVALID_PKCS7;
	}

	/* Version ::= INTEGER */
	if (mbedtls_asn1_get_int(p, end, &ver) != 0) {
		log_dbg("SignedData: Unable to get version of SignedData.");
		return E_SECURITY_INVALID_PKCS7;
	}

	if (ver != 1) {
		log_dbg("SignedData: Only version 1 is supported.");
		return E_SECURITY_INVALID_PKCS7;
	}

	/*
	 * DigestAlgorithmIdentifiers ::=
	 *	SET OF DigestAlgorithmIdentifier
	 */
	if (mbedtls_asn1_get_tag(p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SET) != 0) {
		log_dbg("SignedData: DigestAlgorithmIdentifiers not found.");
		return E_SECURITY_INVALID_PKCS7;
	}

	if (pkcs7_get_digest_algorithm_identifiers(p, *p + len, &digest_algo_ids) != S_OK)
		return E_SECURITY_INVALID_PKCS7;

	if (artik_list_size(digest_algo_ids) != 1) {
		log_dbg("Only verification for one digest algorithm identifer is supported");
		return E_SECURITY_INVALID_PKCS7;
	}

	/*
	 * ContentInfo ::= SEQUENCE {
	 *	contentType ContentType,
	 *	content
	 *		[0] EXPLICIT ANY DEFINED BY contentType OPTIONAL
	 * }
	 *
	 * Only detached format is supported, so content must be NULL or absent.
	 */
	if (mbedtls_asn1_get_tag(p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE) != 0) {
		log_dbg("SignedData: contentInfo not found.");
		return E_SECURITY_INVALID_PKCS7;
	}

	ct_oid.tag = **p;
	if (mbedtls_asn1_get_tag(p, *p + len, &ct_oid.len, MBEDTLS_ASN1_OID) != 0) {
		log_dbg("SignedData: contentInfo is not valid: contentType not found.");
		return E_SECURITY_INVALID_PKCS7;
	}
	ct_oid.p = *p;
	*p += ct_oid.len;
	if (ct_oid.p + ct_oid.len != *p) {
		log_dbg("SignedData: Only detached format is supported.");
		return E_SECURITY_INVALID_PKCS7;
	}

	if (MBEDTLS_OID_CMP(OID_PKCS7_DATA, &ct_oid) != 0) {
		char str_oid[32];

		mbedtls_oid_get_numeric_string(str_oid, 32, &ct_oid);
		log_dbg("SignedData: OID is %s but expected OID is 1.2.840.113549.1.7.1", str_oid);
		return E_SECURITY_INVALID_PKCS7;
	}

	/*
	 * ExtendedCertificatesAndCertificates ::= SET OF ExtendedCertificateOrCertificate
	 * ExtendedCertificateOrCertificate ::= CHOICE { certificate Certificate, -- X.509 }
	 * In this case only X509 certificate is supported.
	 */
	sig_data->chain = (mbedtls_x509_crt *) calloc(1, sizeof(mbedtls_x509_crt));
	if (!sig_data->chain) {
		free(sig_data->chain);
		sig_data->chain = NULL;
		log_dbg("Failed to allocate mbedtls_x509_crt.");
		err = E_NO_MEM;
		goto cleanup;
	}

	mbedtls_x509_crt_init(sig_data->chain);

	if (mbedtls_asn1_get_tag(p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_CONTEXT_SPECIFIC) != 0) {
		log_dbg("SignedData: Unable to find ExtendedCertifcatesAndCertifcates");
		err = E_SECURITY_INVALID_PKCS7;
		goto cleanup;
	}

	err = pkcs7_get_chain_certs(sig_data->chain, *p, len);
	if (err != S_OK)
		goto cleanup;

	*p += len;

	/*
	 * Reverse chain we assume that the certifcate chain is in the following order
	 * Intermediate certificate -> end certificate
	 */
	x509_current = sig_data->chain;
	while (x509_current != NULL) {
		x509_next = x509_current->next;
		x509_current->next = x509_prev;
		x509_prev = x509_current;
		x509_current = x509_next;
	}
	sig_data->chain = x509_prev;

	ret = mbedtls_x509_crt_verify(sig_data->chain, rootCA, NULL, NULL, &flags, NULL, NULL);
	if (ret != 0) {
		log_dbg("Failed to verify certificate chain. (ret %d, flags %d)", ret, flags);
		err = E_SECURITY_CA_VERIF_FAILED;
		goto cleanup;
	}

	/*
	 * SignerInfos ::= SET OF SignerInfo
	 *
	 * We assume that only one element is present in SignerInfos.
	 */
	mbedtls_asn1_get_tag(p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SET);
	if (*p + len != end) {
		log_dbg("SignerInfos MUST be the last element.");
		err = E_SECURITY_INVALID_PKCS7;
		goto cleanup;
	}

	err = pkcs7_get_signer_info(p, end, &sig_data->signer, digest_algo_ids);

cleanup:
	if (err != S_OK) {
		if (sig_data->chain) {
			mbedtls_x509_crt_free(sig_data->chain);
			free(sig_data->chain);
		}
	}

	return err;
}
