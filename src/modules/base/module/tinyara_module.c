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

#include <artik_types.h>
#include <artik_module.h>
#include <artik_platform.h>
#include "os_module.h"

typedef struct {

	int id;

	const artik_api_module *modules;

} tinyara_platform;

static const tinyara_platform artik_api_modules[] = {
	{ ARTIK053, artik_api_a05x_modules },
	{ -1,		NULL}
};

artik_error os_get_api_version(artik_api_version *version)
{
	if (!version)
		return E_BAD_ARGS;

#ifdef LIB_VERSION_MAJOR
	version->major = LIB_VERSION_MAJOR;

#ifdef LIB_VERSION_MINOR
	version->minor = LIB_VERSION_MINOR;

#ifdef LIB_VERSION_PATCH
	version->patch = LIB_VERSION_PATCH;

	snprintf(version->version, MAX_VERSION_STRING, "%d.%d.%d",
		LIB_VERSION_MAJOR, LIB_VERSION_MINOR, LIB_VERSION_PATCH);
#endif
#endif
#endif

	return S_OK;
}

artik_module_ops os_request_api_module(const char *name)
{
	int i = 0, plat = 0;
	artik_module_ops ops = (artik_module_ops)NULL;

	if ((os_get_platform() == -1) || !name)
		return NULL;

	while (artik_api_modules[plat].modules) {
		if (artik_api_modules[plat].id == os_get_platform()) {
			artik_api_module *p_target = (artik_api_module *)
						artik_api_modules[plat].modules;

			for (; p_target->name; p_target++, i++) {
				if (strncmp(p_target->name, name,
							MAX_MODULE_NAME) == 0) {
					ops = (artik_module_ops)
							p_target->object;
					break;
				}
			}
		}
		plat++;
	}

	return ops;
}

artik_error os_release_api_module(const artik_module_ops module)
{
	int i = 0, plat = 0;
	artik_error ret = E_BAD_ARGS;

	if (os_get_platform() == -1)
		return E_NOT_SUPPORTED;

	while (artik_api_modules[plat].modules) {
		if (artik_api_modules[plat].id == os_get_platform()) {
			artik_api_module *p_target = (artik_api_module *)
						artik_api_modules[plat].modules;

			for (; p_target->name; p_target++, i++) {
				if (p_target->object == module) {
					/*
					 * Nothing to do here, may need to
					 * do some cleanup later
					 */
					ret = S_OK;
					break;
				}
			}
		}
		plat++;
	}

	return ret;
}

int os_get_platform(void)
{
	/*
	 * For each platform ID that may be returned here,
	 * appropriate entry in "artik_api_modules" must
	 * be filled up.
	 *
	 */
#ifdef CONFIG_ARCH_BOARD_ARTIK053
	return ARTIK053;
#else
	return -1;
#endif
}

artik_error os_get_platform_name(char *name)
{
	if (!name)
		return E_BAD_ARGS;

	if (os_get_platform() == -1)
		return E_NOT_SUPPORTED;

	strncpy(name, artik_platform_name[os_get_platform()],
							MAX_PLATFORM_NAME);

	return S_OK;
}

artik_error os_get_available_modules(artik_api_module **modules,
							int *num_modules)
{
	unsigned int i = 0;
	int plat = 0;

	if (!modules || !num_modules)
		return E_BAD_ARGS;

	if (os_get_platform() == -1)
		return E_NOT_SUPPORTED;

	while (artik_api_modules[plat].modules) {
		if (artik_api_modules[plat].id == os_get_platform()) {
			*modules = (artik_api_module *)
						artik_api_modules[plat].modules;

			/* Count number of entries in the modules array */
			while ((*modules)[i].name != NULL)
				i++;

			*num_modules = i;
		}
		plat++;
	}

	return S_OK;
}

bool os_is_module_available(artik_module_id_t id)
{
	int plat = 0;

	if (os_get_platform() == -1)
		return false;

	while (artik_api_modules[plat].modules) {
		if (artik_api_modules[plat].id == os_get_platform()) {
			artik_api_module *p_module = (artik_api_module *)
						artik_api_modules[plat].modules;

			for (; p_module->id >= 0; p_module++) {
				if (p_module->id == id)
					return true;
			}
		}
		plat++;
	}

	return false;
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
	char platform_info[] = "\t\"name\": \"%s\",\n";
	char modules_headher[] = "\t\"modules\":[\n";
	char modules_tail[] = "\t]\n}";
	char modules_info[] = "\t\t\"%s\",\n";
	int platid = os_get_platform();

	if (platid == -1)
		return NULL;

	artik_get_available_modules(&modules, &num_modules);

	max_plat_name_len = strlen(platform_info) + 10; /* Platform name */

	max_module_len = strlen(modules_info) + 10; /* module name */

	max_json_len = max_plat_name_len + (max_module_len * num_modules) +
				strlen(header) + strlen(modules_headher) +
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

	snprintf(entry, max_plat_name_len, platform_info,
						artik_platform_name[platid]);

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

	return json;
}
