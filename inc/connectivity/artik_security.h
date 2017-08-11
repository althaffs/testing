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

#ifndef	__ARTIK_SECURITY_H__
#define	__ARTIK_SECURITY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "artik_time.h"
#include "artik_error.h"
#include "artik_types.h"
#include "artik_time.h"

/*! \file artik_security.h
 *
 *  \brief Security module definition
 *
 *  Definitions and functions for accessing
 *  the Security module and make use of the
 *  hardware Secure Element for cryptographic
 *  and signing features.
 *
 * \example security_test/artik_security_test.c
 */

/*!
 *  \brief Maximum length for the serial number strings
 *
 *  Maximum length allowed for string
 *  containing an serial number
 *  from a certificate of artik secure element
 */
#define ARTIK_CERT_SN_MAXLEN 20

/*!
 *  \brief Security handle type
 *
 *  Handle type used to carry instance specific
 *  information for a security object
 */
typedef void *artik_security_handle;

/*! \struct artik_security_module
 *
 *  \brief Security module operations
 *
 *  Structure containing all the exposed operations exposed
 *  by the Security module
 */
typedef struct {
	/*!
	 *  \brief Request a security instance
	 *
	 *  \param[out] handle Handle tied to the requested security
	 *              instance returned by the function.
	 *
	 *  \return S_OK on success, error code otherwise
	 */
	artik_error(*request) (artik_security_handle * handle);
	/*!
	 *  \brief Release a security instance
	 *
	 *  \param[in] handle Handle tied to the requested security
	 *             instance to be released.
	 *             This handle is returned by the request
	 *             function.
	 *
	 *  \return S_OK on success, error code otherwise
	 */
	artik_error(*release) (artik_security_handle handle);
	/*!
	 *  \brief Get the certificate stored in the SE
	 *
	 *  \param[in] handle Handle tied to a requested security
	 *             instance.
	 *             This handle is returned by the request
	 *             function.
	 *  \param[out] cert Pointer to a string that will be
	 *              allocated by the function
	 *              and filled with the content of the
	 *              certificate. This string must
	 *              be freed by the calling function.
	 *
	 *  \return S_OK on success, error code otherwise
	 */
	artik_error(*get_certificate) (artik_security_handle handle,
				       char **cert);
	/*!
	 *  \brief Get private key from the certificate stored in the
	 *         SE
	 *
	 *  \param[in] handle Handle tied to a requested security
	 *             instance.
	 *             This handle is returned by the request function.
	 *  \param[in] cert Pointer to a string containing the
	 *             certificate to generate
	 *             the private key from
	 *  \param[out] key Pointer to a string that will be allocated
	 *              by the function
	 *              and filled with the content of the key. This
	 *              string must
	 *              be freed by the calling function.
	 *
	 *  \return S_OK on success, error code otherwise
	 */
	artik_error(*get_key_from_cert) (artik_security_handle handle,
					 const char *cert, char **key);

	/*!
	 *  \brief Get root CA stored in the SE
	 *
	 *  \param[in] handle Handle tied to a requested security
	 *             instance.
	 *             This handle is returned by the request function.
	 *  \param[out] root_ca Pointer to a string that will be
	 *              allocated by the function
	 *              and filled with the content of the CA. This
	 *              string must
	 *              be freed by the calling function.
	 *
	 *  \return S_OK on success, error code otherwise
	 */
	artik_error(*get_root_ca) (artik_security_handle handle,
						char **root_ca);

	/*!
	 *  \brief Generate true random bytes
	 *
	 *  \param[in] handle Handle tied to a requested security
	 *             instance.
	 *             This handle is returned by the request function.
	 *  \param[out] rand Pointer to a preallocated array that will
	 *              be filled with the
	 *              generated random bytes
	 *  \param[in] len Number of random bytes to generate
	 *
	 *  \return S_OK on success, error code otherwise
	 */
	artik_error(*get_random_bytes) (artik_security_handle handle,
					unsigned char *rand, int len);

	/*!
	 *  \brief Get the serial number from the certificate
	 *
	 *  \param[in] handle Handle tied to a requested security
	 *             instance.
	 *             This handle is returned by the request function.
	 *  \param[out] sn preallocated array provided by the user
	 *  \param[in,out] len size of the pointer preallocated and
	 *                 set after the pointer was filled.
	 *
	 *  \return S_OK on success, error code otherwise
	 */
	artik_error(*get_certificate_sn) (artik_security_handle handle,
				unsigned char *sn, unsigned int *len);

	/*!
	 *  \brief Initialize verification of PKCS7 signature against a signed
	 *         binary.
	 *
	 *  \param[out] handle signing handle returned by the API. It must be passed
	 *                     to subsequent calls to \ref verify_signature_update
	 *                     and \ref verify_signature_final.
	 *  \param[in] signature_pem PKCS7 signature in a PEM encoded string.
	 *  \param[in] root_ca X509 certificate of the root CA against which to
	 *                     verify the signer certificate in a PEM encoded
	 *                     string.
	 *  \param[in] signing_time_in If provided, the verification function fails
	 *                             if the date occured before the signing time
	 *                             specified in the PKCS7 signature.
	 *  \param[out] signing_time_out If provided, this ate is filled up with the
	 *                               signing time extracted from the PKCS7 data.
	 *
	 *  \return S_OK on success, error code otherwise
	 */
	artik_error(*verify_signature_init) (artik_security_handle * handle,
			const char *signature_pem, const char *root_ca,
			const artik_time * signing_time_in, artik_time * signing_time_out);

	/*!
	 *  \brief Feed data of the signed binary to the verification process.
	 *
	 *  Subsequent calls to this same function specifiying various data
	 *  lengths can be made to feed big amount of data to the verification
	 *  process.
	 *
	 *  \param[in] handle Handle returned by \ref verify_signature_init after
	 *                    initialization of the verification process.
	 *  \param[in] data Pointer to a buffer containing a portion of the data to
	 *                  feed.
	 *  \param[in] data_len Length of the buffer passed in the \ref data
	 *                      parameter.
	 *
	 *  \return S_OK on success, error code otherwise
	 */
	artik_error(*verify_signature_update) (artik_security_handle handle,
			unsigned char *data, unsigned int data_len);

	/*!
	 *  \brief Finalize signature verification process and return final result.
	 *
	 *  Calls to this function must be made after proper initialization of the
	 *  signature verification process (\ref verify_signature_init) and after
	 *  signed data has been fed using the \ref verify_signature_update
	 *  function.
	 *
	 *  \param[in] handle Handle returned by \ref verify_signature_init after
	 *                    initialization of the verification process.
	 *
	 *  \return S_OK on signature verification success, error code otherwise
	 *
	 *  Signature verification related errors are listed below:
	 *      E_SECURITY_ERROR                  (-7000)
	 *      E_SECURITY_INVALID_X509           (-7001)
	 *      E_SECURITY_INVALID_PKCS7          (-7002)
	 *      E_SECURITY_CA_VERIF_FAILED        (-7003)
	 *      E_SECURITY_DIGEST_MISMATCH        (-7004)
	 *      E_SECURITY_SIGNATURE_MISMATCH     (-7005)
	 *      E_SECURITY_SIGNING_TIME_ROLLBACK  (-7006)
	 */
	artik_error(*verify_signature_final) (artik_security_handle handle);

} artik_security_module;

extern const artik_security_module security_module;

#ifdef __cplusplus
}
#endif
#endif				/* __ARTIK_SECURITY_H__ */
