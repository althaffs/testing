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
#include <dlfcn.h>
#include <pthread.h>

#include <artik_types.h>
#include <artik_log.h>
#include <artik_module.h>
#include <artik_platform.h>

#include "os_module.h"

#define MAX_STR_LEN 1024
#define PATH_STRING "libartik-sdk-%s.so.%d.%d.%d"
#define MODULE_STRING "%s_module"

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

static artik_list *requested_node = NULL;

typedef struct artik_module_info_t {
	char *module_name;
	void *dl_handle;
	void *dl_symbol;
} artik_module_info;

typedef struct artik_module_handle_t {
	artik_list node;
	artik_module_info info;
} artik_module_node;

static int artik_platform_id = -1;

/*
 * This table must follow exactly the same order as the
 * platform ID definition enum in artik_platform.h
 */
static const artik_api_module *artik_api_modules[] = {
	artik_api_generic_modules,
	artik_api_a520_modules,
	artik_api_a1020_modules,
	artik_api_a710_modules,
	artik_api_a530_modules
};

artik_error os_get_api_version(artik_api_version *version)
{
	if (!version)
		return E_BAD_ARGS;

	version->major = LIB_VERSION_MAJOR;
	version->minor = LIB_VERSION_MINOR;
	version->patch = LIB_VERSION_PATCH;
	snprintf(version->version, MAX_VERSION_STRING, "%d.%d.%d",
		 LIB_VERSION_MAJOR, LIB_VERSION_MINOR, LIB_VERSION_PATCH);

	return S_OK;
}

static void *artik_compare_module_name(void *node, void *target)
{
	if (strncmp(((artik_module_node *)node)->info.module_name,
				(char *)target, strlen((char *)target)) == 0) {
		/* if the modules are same */
		return node;
	}
	/* if not */
	return NULL;
}

static void *artik_compare_module_ops(void *node, artik_module_ops target)
{
	if (memcmp(((artik_module_node *)node)->info.dl_symbol, target,
							sizeof(void *)) == 0) {
		/* if the modules are same */
		return node;
	}
	/* if not */
	return NULL;
}

artik_module_ops os_request_api_module(const char *name)
{
	artik_list *node = NULL;
	void *dl_handle = NULL;
	void *dl_symbol = NULL;
	char str_buf[MAX_STR_LEN] = {0, };
	char *module_name = NULL;
	char *error_msg;
	unsigned int i = 0, len = 0;
	int platid = os_get_platform();

	if (!name)
		return INVALID_MODULE;

	while (artik_api_modules[platid][i].object != NULL) {
		if (!strncmp(artik_api_modules[platid][i].name, name,
							MAX_MODULE_NAME)) {

			mutex_lock();

			node = artik_list_get_by_check(requested_node,
				artik_compare_module_name, (void *)name);
			if (node) {
				module_name = ((artik_module_node *)
							node)->info.module_name;
				dl_symbol = ((artik_module_node *)
							node)->info.dl_symbol;
				dl_handle = ((artik_module_node *)
							node)->info.dl_handle;
			}

			node = artik_list_add(&requested_node, 0,
						sizeof(artik_module_node));
			if (node == NULL) {
				mutex_unlock();
				return INVALID_MODULE;
			} else if (dl_handle) {
				/* don't need to call DL again */
				goto exit;
			}

			memset(str_buf, 0, sizeof(str_buf));
			snprintf(str_buf, MAX_STR_LEN, PATH_STRING,
					artik_api_modules[platid][i].object,
					LIB_VERSION_MAJOR,
					LIB_VERSION_MINOR, LIB_VERSION_PATCH);
			dl_handle = (void *)dlopen(str_buf, RTLD_NOW);
			if (!dl_handle) {
				mutex_unlock();
				return INVALID_MODULE;
			}

			memset(str_buf, 0, sizeof(str_buf));
			snprintf(str_buf, MAX_STR_LEN, MODULE_STRING, name);
			dlerror();
			dl_symbol = dlsym(dl_handle, str_buf);
			error_msg = dlerror();
			if (error_msg != NULL) {
				mutex_unlock();
				return INVALID_MODULE;
			}

exit:
			if (module_name == NULL) {
				len = strlen(name) + 1;
				module_name = malloc(len);
				if (module_name == NULL) {
					mutex_unlock();
					return INVALID_MODULE;
				}
				memset(module_name, 0, len);
				strncpy(module_name, name, len);
			}

			((artik_module_node *)node)->info.module_name =
								module_name;
			((artik_module_node *)node)->info.dl_handle = dl_handle;
			((artik_module_node *)node)->info.dl_symbol = dl_symbol;

			mutex_unlock();

			break;
		}
		i++;
	}

	return (artik_module_ops)dl_symbol;
}

artik_error os_release_api_module(const artik_module_ops module)
{
	artik_list *node = NULL;
	artik_error ret = S_OK;
	void *dl_handle = NULL;
	char *module_name = NULL;

	mutex_lock();

	node = artik_list_get_by_check(requested_node, artik_compare_module_ops,
									module);
	if (node == NULL) {
		log_err("releasing invalid module");
		ret = E_BAD_ARGS;
		goto exit;
	}

	dl_handle = ((artik_module_node *)node)->info.dl_handle;
	module_name = ((artik_module_node *)node)->info.module_name;

	ret = artik_list_delete_check(&requested_node, artik_compare_module_ops,
									module);
	if (ret != S_OK) {
		log_err("failed to delete module");
		goto exit;
	}

	node = artik_list_get_by_check(requested_node, artik_compare_module_ops,
									module);
	if (node == NULL) {
		free(module_name);
		dlclose(dl_handle);
	}

exit:
	mutex_unlock();

	return ret;
}

int os_get_platform(void)
{
	FILE *f = NULL;
	char line[256];

	if (artik_platform_id >= GENERIC)
		goto exit;

	f = fopen("/proc/device-tree/model", "re");
	if (f == NULL)
		return -1;

	if (fgets(line, sizeof(line), f) != NULL) {
		if (strstr(line, "ARTIK5"))
			artik_platform_id = ARTIK520;
		else if (strstr(line, "ARTIK10"))
			artik_platform_id = ARTIK1020;
		else if (strstr(line, "artik710"))
			artik_platform_id = ARTIK710;
		else if (strstr(line, "artik530"))
			artik_platform_id = ARTIK530;
		else
			artik_platform_id = GENERIC;
	}
	fclose(f);

exit:
	return artik_platform_id;
}

artik_error os_get_platform_name(char *name)
{
	int platid = artik_get_platform();

	if (!name)
		return E_BAD_ARGS;

	strncpy(name, artik_platform_name[platid], MAX_PLATFORM_NAME);

	return S_OK;
}

artik_error os_get_available_modules(artik_api_module **modules, int
								*num_modules)
{
	unsigned int i = 0;
	int platid = artik_get_platform();

	if (!modules || !num_modules)
		return E_BAD_ARGS;

	/* Count number of entries in the modules array */
	while (artik_api_modules[platid][i].object != NULL)
		i++;

	*modules = (artik_api_module *)artik_api_modules[platid];
	*num_modules = i;

	return S_OK;
}

bool os_is_module_available(artik_module_id_t id)
{
	unsigned int i = 0;
	bool found = false;
	int platid = artik_get_platform();

	while (artik_api_modules[platid][i].object != NULL) {
		if (artik_api_modules[platid][i].id == id) {
			found = true;
			break;
		}
		i++;
	}

	return found;
}

char *os_get_device_info(void)
{
	char *entry = NULL;
	char *json = NULL;
	int max_plat_name_len = 0, max_module_len = 0, max_json_len = 0;

	artik_api_module *modules = NULL;
	int num_modules = 0;
	int i = 0;

	char header[] = "{\n";
	char tail[] = "\n}";

	char platform_info[] = "\t\"name\": \"%s\",\n";
	char modules_headher[] = "\t\"modules\":[\n";
	char modules_tail[] = "\t]\n";
	char modules_info[] = "\t\"%s\",\n";

	int platid = artik_get_platform();

	artik_get_available_modules(&modules, &num_modules);

	max_plat_name_len = strlen(platform_info) + 10; /* Platform name */

	max_module_len = strlen(modules_info) + 10; /* module name */

	max_json_len = max_plat_name_len + (max_module_len * num_modules) +
					strlen(header) + strlen(tail) +
					strlen(modules_headher) +
					strlen(modules_tail) + 1;
	json = (char *)malloc(max_json_len);
	if (!json)
		return json;

	/* Start building the JSON string */
	memset(json, 0, max_json_len);
	strncat(json, header, strlen(header));

	entry = (char *)malloc(max_plat_name_len);
	if (!entry) {
		free(json);
		return NULL;
	}

	if (platid == ARTIK520)
		snprintf(entry, max_plat_name_len, platform_info, "ARTIK520");
	else if (platid == ARTIK1020)
		snprintf(entry, max_plat_name_len, platform_info, "ARTIK1020");
	else if (platid == ARTIK710)
		snprintf(entry, max_plat_name_len, platform_info, "ARTIK710");
	else if (platid == ARTIK530)
		snprintf(entry, max_plat_name_len, platform_info, "ARTIK530");
	else
		snprintf(entry, max_plat_name_len, platform_info, "GENERIC");

	/* Copy platform info */
	strncat(json, entry, max_plat_name_len);
	free(entry);

	/* Copy available modules */
	strncat(json, modules_headher, strlen(modules_headher));

	for (i = 0; i < num_modules; i++) {
		entry = (char *)malloc(max_module_len);
		if (!entry) {
			free(json);
			return NULL;
		}
		snprintf(entry, max_module_len, modules_info, modules[i].name);
		strncat(json, entry, max_module_len);
		free(entry);
	}

	/* Remove last comma */
	json[strlen(json) - 2] = '\n';
	json[strlen(json) - 1] = '\0';

	strncat(json, modules_tail, strlen(modules_tail));
	strncat(json, tail, strlen(tail));

	return json;
}
