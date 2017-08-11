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

#include <artik_module.h>
#include "os_module.h"

EXPORT_API artik_error artik_get_api_version(artik_api_version * version)
{
	return os_get_api_version(version);
}

EXPORT_API artik_module_ops artik_request_api_module(const char *name)
{
	return os_request_api_module(name);
}

EXPORT_API artik_error artik_release_api_module(const artik_module_ops module)
{
	return os_release_api_module(module);
}

EXPORT_API int artik_get_platform(void)
{
	return os_get_platform();
}

EXPORT_API artik_error artik_get_platform_name(char *name)
{
	return os_get_platform_name(name);
}

EXPORT_API artik_error artik_get_available_modules(artik_api_module**modules,
							int *num_modules)
{
	return os_get_available_modules(modules, num_modules);
}

EXPORT_API bool artik_is_module_available(artik_module_id_t id)
{
	return os_is_module_available(id);
}

EXPORT_API char *artik_get_device_info(void)
{
	return os_get_device_info();
}
