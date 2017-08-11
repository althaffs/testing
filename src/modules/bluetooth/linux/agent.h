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

#ifndef __ARTIK_BT_AGENT_H
#define __ARTIK_BT_AGENT_H

#ifdef __cplusplus
extern "C" {
#endif

artik_error bt_agent_register_capability(artik_bt_agent_capability e);
artik_error bt_agent_set_default(void);
artik_error bt_agent_unregister(void);
artik_error bt_agent_set_callback(artik_bt_agent_callbacks *agent_callback);
artik_error bt_agent_send_pincode(artik_bt_agent_request_handle handle, char *pincode);
artik_error bt_agent_send_passkey(artik_bt_agent_request_handle handle, unsigned int passkey);
artik_error bt_agent_send_error(artik_bt_agent_request_handle handle, artik_bt_agent_request_error e,
		const char *err_msg);
artik_error bt_agent_send_empty_response(artik_bt_agent_request_handle handle);

#ifdef __cplusplus
}
#endif

#endif /* __ARTIK_BT_AGENT_H */
