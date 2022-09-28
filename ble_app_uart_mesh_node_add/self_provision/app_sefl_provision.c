#include "access.h"
#include "device_state_manager.h"
#include "app_error.h"
#include "nrf_mesh_prov_types.h"
#include "user_on_off_client.h"
#include "log.h"
#include "config_server.h"
#include "ble_config.h"
#include "access_config.h"
#include "app_sef_provision.h"
#include "net_state.h"

#define LOCAL_ADDRESS_START 0x0060
#define LOCAL_COUNT 1
#define GROUP_ADDRESS 0xC001
/*Netkey*/
//#define APP_NETKEY {0x8C, 0x5C, 0x10, 0x23, 0xC9, 0x77, 0x8D, 0xFA, 0xC3, 0x69, 0x9F, 0x1C, 0x9E, 0x4A, 0x25, 0x28}
//#define APP_NETKEY_INDEX 0
///*Device Key*/
//#define APP_DEVKEY  {}
//#define APP_DEVKEY_INDEX 

#define NET_KEY {0x8C, 0x5C, 0x10, 0x23, 0xC9, 0x77, 0x8D, 0xFA, 0xC3, 0x69, 0x9F, 0x1C, 0x9E, 0x4A, 0x25, 0x28}
#define DEV_KEY {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}
#define APP_KEY {0x5C, 0x44, 0xD9, 0x7D, 0xDA, 0x11, 0x53, 0x57, 0x49, 0x7D, 0x23, 0xBF, 0x5B, 0xCF, 0xAD, 0xD9}
const static uint32_t NET_KEY_INDEX = 0;


const static uint8_t DEV_KEY_INDEX = 0;

/*APP KEY*/

const static uint32_t APP_KEY_INDEX = 0;
const static uint32_t m_iv_index = 1;
static uint8_t m_app_key[] = APP_KEY;
static uint8_t m_net_key[] = NET_KEY;

 
#warning"/*TODO: IS m_dev_key is KEY.privacy :D :D*/"
static uint8_t m_dev_key[] = DEV_KEY;

nrf_mesh_network_secmat_t KEY;
//const static uint32_t 


/*DSM Handle*/
static dsm_handle_t m_subnet_handle = DSM_HANDLE_INVALID;
static dsm_handle_t m_appkey_handle = DSM_HANDLE_INVALID;
static dsm_handle_t m_devkey_handle = DSM_HANDLE_INVALID;
static dsm_handle_t m_group_addres_handle = 0;

//extern user_on_off_client_t user_client;








static dsm_local_unicast_address_t m_start_unicast_add = {
  .address_start = LOCAL_ADDRESS_START,
  .count = ACCESS_ELEMENT_COUNT
 };
static nrf_mesh_prov_provisioning_data_t provision_data = {
  .netkey = NET_KEY,
  .netkey_index = NET_KEY_INDEX,
  .iv_index = 0,
  .address = LOCAL_ADDRESS_START + LOCAL_COUNT,
  .flags.iv_update = false,
  .flags.key_refresh = false
};
extern bool nrf_mesh_is_device_provisioned(void);
/*
 @brief: Set Unicast address
         devkey index must be equal to start unicast address
*/
static void self_provision(uint8_t *p_app_key, uint8_t *p_netkey, dsm_local_unicast_address_t unicast_add)
{
   ret_code_t err_code = NRF_SUCCESS;
   //memcpy(&provision_data.netkey, p_netkey, 16);
   //m_start_unicast_add = unicast_add;
   //m_start_unicast_add = unicast_add;
   //provision_data.address = (uint16_t)(m_start_unicast_add.address_start + m_start_unicast_add.count);
  /*Set the Unicast Address of Gateway*/
  //bool is_device_provisioned = nrf_mesh_is_device_provisioned();
  //dsm_local_unicast_address_t last_unicast = {0};
  //dsm_local_unicast_addresses_get(&last_unicast);
  //APP_ERROR_CHECK(err_code);
  m_start_unicast_add.address_start = unicast_add.address_start;
  m_start_unicast_add.count = ACCESS_ELEMENT_COUNT;
  provision_data.address = unicast_add.address_start;
  //__LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Last unicast address %d\r\n", last_unicast.address_start + last_unicast.count);
 
  err_code = dsm_local_unicast_addresses_set(&m_start_unicast_add);
    //__LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Local address set error: %d\r\n", err_code);
  APP_ERROR_CHECK(err_code);
  /*Add Mesh Keys*/
  err_code = dsm_subnet_add(0, m_net_key, &m_subnet_handle);
    //__LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Subnet add %d\r\n", err_code);
  APP_ERROR_CHECK(err_code);


  APP_ERROR_CHECK(err_code);
  err_code = net_state_iv_index_set(0,0);
     //
  m_subnet_handle = dsm_net_key_index_to_subnet_handle(0);
  err_code = dsm_devkey_add(m_start_unicast_add.address_start + m_start_unicast_add.count, m_subnet_handle, m_dev_key, &m_devkey_handle);
  APP_ERROR_CHECK(err_code);
     /*Add App Key*/
  err_code = dsm_appkey_add(APP_KEY_INDEX, m_subnet_handle, p_app_key, &m_appkey_handle);
  provision_data.netkey_index++;
  APP_ERROR_CHECK(err_code);
     /*Add Dev Key*/
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "SELF Provision self add key state\r\n");
}
/*
*/
static void subcribe_topic(access_model_handle_t model)
{
  ret_code_t err_code = NRF_SUCCESS;
  /*Add devkey handle to config server - PERSISTANCE STORAGE*/
  err_code = config_server_bind(m_devkey_handle);
  APP_ERROR_CHECK(err_code);
  err_code = access_model_application_bind(model, m_appkey_handle);
  APP_ERROR_CHECK(err_code);

  err_code = access_model_publish_application_set(model, m_appkey_handle);
  APP_ERROR_CHECK(err_code);
  
  err_code = access_model_publish_address_set(model, m_group_addres_handle);
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Public address set %d\r\n", err_code);
  APP_ERROR_CHECK(err_code);
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Subribe Topic\r\n");

// Add the address to the DSM as a subscription address:
  dsm_address_subscription_add(GROUP_ADDRESS, &m_group_addres_handle);
  // Add the subscription to the model:
  access_model_subscription_add(model, m_group_addres_handle);

 // mesh_stack_device_reset(); 

}

/*
  
*/
void app_self_provision(access_model_handle_t model_handle,
                        int8_t *p_app_key, uint8_t *p_netkey, 
                        dsm_local_unicast_address_t m_start_unicast_add)
{
  bool is_device_provisioned = nrf_mesh_is_device_provisioned();
  if(is_device_provisioned == false)
  {
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Begin Self Provision Process\r\n");
    self_provision(p_app_key, p_netkey, m_start_unicast_add);
    subcribe_topic(model_handle);
  }
  else
  {
    dsm_local_unicast_address_t last_unicast = {0};
    dsm_local_unicast_addresses_get(&last_unicast); 
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Last unicast address %d\r\n", last_unicast.address_start);
  }
}