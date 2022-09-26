#ifndef _APP_SELF_PROVISION_H_
#define _APP_SELF_PROVISION_H_
#include "access.h"
#include "access_config.h"
#include "device_state_manager.h"

void app_self_provision(access_model_handle_t model_handle,
                        int8_t *p_app_key, uint8_t *p_netkey, 
                        dsm_local_unicast_address_t m_start_unicast_add);
#endif