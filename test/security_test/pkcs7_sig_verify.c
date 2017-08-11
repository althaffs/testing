#include <stdio.h>
#include <sys/stat.h>

#include <artik_module.h>
#include <artik_security.h>

#define INPUT_TIME_FORMAT    "MM/DD/YYYY HH:mm:SS"
#define JSON_RET_TPL         "{\"error\":%s,\"reason\":\"%s\",\"error_code\":%d}\n"
#define JSON_RET_TPL_TIME    "{\"error\":%s,\"reason\":\"%s\",\"error_code\":%d,\"signingTime\":\"%s\"}\n"
#define JSON_RET_MAX_LEN     256

static void usage(void)
{
	printf("Usage: pkcs7-sig-verify <signature> <root CA> <signed data> [signing date]\n\n");
	printf("signature - PKCS7 signature in PEM format\n");
	printf("root CA - X509 root CA certificate in PEM format\n");
	printf("signed data - file containing the signed data\n");
	printf("signing date (optional) - current signing date for rollback detction\n");
	printf("\tFormat is \"%s\"\n", INPUT_TIME_FORMAT);
	printf("\tIf not provided, rollback detection is not performed\n");
	printf("\nA JSON formatted string with verification result and error information is output on stdout\n");
	printf("Return value contains an error code among the following ones\n");
	printf("\t0: success\n");
	printf("\t-1: invalid parameters\n");
	printf("\t-2: invalid X509 certificate\n");
	printf("\t-3: invalid PKCS7 signature\n");
	printf("\t-4: CA verification failed\n");
	printf("\t-5: computed digest mismatch\n");
	printf("\t-6: signature verification failed\n");
	printf("\t-7: signing time rollback detected\n");
}

static int convert_err_code(artik_error err)
{
	int ret = 0;

	switch (err) {
	case S_OK:
		ret = 0;
		break;
	case E_SECURITY_INVALID_X509:
		ret = -2;
		break;
	case E_SECURITY_INVALID_PKCS7:
		ret = -3;
		break;
	case E_SECURITY_CA_VERIF_FAILED:
		ret = -4;
		break;
	case E_SECURITY_DIGEST_MISMATCH:
		ret = -5;
		break;
	case E_SECURITY_SIGNATURE_MISMATCH:
		ret = -6;
		break;
	case E_SECURITY_SIGNING_TIME_ROLLBACK:
		ret = -7;
		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}

void convert_time_to_str(artik_time *time, char *str, int len)
{
	snprintf(str, len, "%02d/%02d/%04d %02d:%02d:%02d",	time->month, time->day,
			time->year, time->hour, time->minute, time->second);
}

static bool read_pem_file(const char *filename, char **out)
{
	FILE *fp = NULL;
	struct stat st;
	int read_bytes = 0;

	if (!filename || !out)
		return false;

	fp = fopen(filename, "r");
	if (!fp)
		return false;

	fstat(fileno(fp), &st);
	*out = malloc(st.st_size + 1);
	if (!out) {
		fclose(fp);
		return false;
	}

	while (read_bytes < st.st_size) {
		int remaining = st.st_size - read_bytes;

		read_bytes += fread(*out + read_bytes, 1, remaining, fp);
		if (read_bytes <= 0) {
			if (ferror(fp)) {
				fclose(fp);
				free(*out);
				return false;
			}
			break;
		}
	}

	(*out)[st.st_size] = '\0';
	fclose(fp);

	return true;
}

static int parse_int(unsigned char **p, unsigned int n, unsigned int *res)
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

int main(int argc, char **argv)
{
	int ret = -1;
	artik_error err = S_OK;
	artik_security_module *security = NULL;
	artik_security_handle handle;
	FILE *data_fp = NULL;
	char *ca_pem = NULL;
	char *sig_pem = NULL;
	artik_time *current_signing_time = NULL;
	artik_time pkcs7_signing_time;
	char json_ret[JSON_RET_MAX_LEN];
	char signing_time_str[20];

	memset(json_ret, 0, sizeof(json_ret));
	memset(signing_time_str, 0, sizeof(signing_time_str));

	if (argc < 4) {
		usage();
		return -1;
	}

	/* Parse signing time if provided */
	if (argc > 4) {
		unsigned char *date = (unsigned char *)argv[4];

		if (strlen((const char *)date) < strlen(INPUT_TIME_FORMAT)) {
			fprintf(stderr, "Invalid signing time\n");
			usage();
			return -1;
		}

		current_signing_time = malloc(sizeof(artik_time));
		if (!current_signing_time) {
			snprintf(json_ret, JSON_RET_MAX_LEN, JSON_RET_TPL, "true",
					"Failed to allocate memory", E_NO_MEM);
			fprintf(stdout, "%s", json_ret);
			return -1;
		}

		if (parse_int(&date, 2, &current_signing_time->month)) {
			snprintf(json_ret, JSON_RET_MAX_LEN, JSON_RET_TPL, "true",
					"Failed to parse month", E_BAD_ARGS);
			fprintf(stdout, "%s", json_ret);
			free(current_signing_time);
			return -1;
		}

		date++;

		if (parse_int(&date, 2, &current_signing_time->day)) {
			snprintf(json_ret, JSON_RET_MAX_LEN, JSON_RET_TPL, "true",
					"Failed to parse day", E_BAD_ARGS);
			fprintf(stdout, "%s", json_ret);
			free(current_signing_time);
			return -1;
		}

		date++;

		if (parse_int(&date, 4, &current_signing_time->year)) {
			snprintf(json_ret, JSON_RET_MAX_LEN, JSON_RET_TPL, "true",
					"Failed to parse year", E_BAD_ARGS);
			fprintf(stdout, "%s", json_ret);
			free(current_signing_time);
			return -1;
		}

		date++;

		if (parse_int(&date, 2, &current_signing_time->hour)) {
			snprintf(json_ret, JSON_RET_MAX_LEN, JSON_RET_TPL, "true",
					"Failed to parse hour", E_BAD_ARGS);
			fprintf(stdout, "%s", json_ret);
			free(current_signing_time);
			return -1;
		}

		date++;

		if (parse_int(&date, 2, &current_signing_time->minute)) {
			snprintf(json_ret, JSON_RET_MAX_LEN, JSON_RET_TPL, "true",
					"Failed to parse minutes", E_BAD_ARGS);
			fprintf(stdout, "%s", json_ret);
			free(current_signing_time);
			return -1;
		}

		date++;

		if (parse_int(&date, 2, &current_signing_time->second)) {
			snprintf(json_ret, JSON_RET_MAX_LEN, JSON_RET_TPL, "true",
					"Failed to parse seconds", E_BAD_ARGS);
			fprintf(stdout, "%s", json_ret);
			free(current_signing_time);
			return -1;
		}
	}

	if (!read_pem_file(argv[1], &sig_pem)) {
		snprintf(json_ret, JSON_RET_MAX_LEN, JSON_RET_TPL, "true",
				"Cannot read PKCS7 signature file", E_BAD_ARGS);
		usage();
		ret = convert_err_code(E_BAD_ARGS);
		goto exit;
	}

	if (!read_pem_file(argv[2], &ca_pem)) {
		snprintf(json_ret, JSON_RET_MAX_LEN, JSON_RET_TPL, "true",
				"Cannot read root CA file", E_BAD_ARGS);
		usage();
		ret = convert_err_code(E_BAD_ARGS);
		goto exit;
	}

	data_fp = fopen(argv[3], "r");
	if (!data_fp) {
		snprintf(json_ret, JSON_RET_MAX_LEN, JSON_RET_TPL, "true",
				"Cannot read signed data file", E_BAD_ARGS);
		usage();
		ret = convert_err_code(E_BAD_ARGS);
		goto exit;
	}

	security = (artik_security_module *)artik_request_api_module("security");
	if (!security) {
		snprintf(json_ret, JSON_RET_MAX_LEN, JSON_RET_TPL, "true",
				"Security module is not available", E_NOT_SUPPORTED);
		ret = convert_err_code(E_NOT_SUPPORTED);
		goto exit;
	}

	err = security->verify_signature_init(&handle, sig_pem, ca_pem,
			current_signing_time, &pkcs7_signing_time);

	convert_time_to_str(&pkcs7_signing_time, signing_time_str,
			sizeof(signing_time_str));

	if (err != S_OK) {
		if (err == E_SECURITY_SIGNING_TIME_ROLLBACK)
			snprintf(json_ret, JSON_RET_MAX_LEN, JSON_RET_TPL_TIME, "true",
					"Rollback signature error detected", convert_err_code(err),
					signing_time_str);
		else
			snprintf(json_ret, JSON_RET_MAX_LEN, JSON_RET_TPL, "true",
					"Failed to initialize signature verification",
					convert_err_code(err));
		ret = convert_err_code(err);
		goto exit;
	}

	while (1) {
		unsigned char buf[512];
		int len = 0;

		len = fread(buf, 1, sizeof(buf), data_fp);
		if (len <= 0) {
			if (ferror(data_fp)) {
				snprintf(json_ret, JSON_RET_MAX_LEN, JSON_RET_TPL_TIME, "true",
						"Failed to read data from file", E_ACCESS_DENIED,
						signing_time_str);
				ret = convert_err_code(E_ACCESS_DENIED);
				goto exit;
			} else {
				break;
			}
		}

		err = security->verify_signature_update(handle, buf, len);
		if (err != S_OK) {
			snprintf(json_ret, JSON_RET_MAX_LEN, JSON_RET_TPL_TIME, "true",
					"Failed to initialize signature verification",
					convert_err_code(err), signing_time_str);
			ret = convert_err_code(err);
			goto exit;
		}
	}

	err = security->verify_signature_final(handle);
	if (err != S_OK)  {
		snprintf(json_ret, JSON_RET_MAX_LEN, JSON_RET_TPL_TIME, "true",
				"Verification failed", convert_err_code(err), signing_time_str);
		ret = convert_err_code(err);
		goto exit;
	}

	snprintf(json_ret, JSON_RET_MAX_LEN, JSON_RET_TPL_TIME, "false",
			"Verification successful", convert_err_code(err), signing_time_str);
	ret = 0;

exit:
	fprintf(stdout, "%s", json_ret);

	if (current_signing_time)
		free(current_signing_time);
	if (ca_pem)
		free(ca_pem);
	if (sig_pem)
		free(sig_pem);
	if (data_fp)
		fclose(data_fp);
	artik_release_api_module(security);

	return ret;
}
