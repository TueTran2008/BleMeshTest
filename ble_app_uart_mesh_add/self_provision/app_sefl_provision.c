/*****************************************************************************
 * Includes
 *****************************************************************************/
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
#include "DataDefine.h"
#include "nrf_nvic.h"
#include "nrf_mesh_config_core.h"
#include "mesh_opt_net_state.h"
#include "nrf_drv_rng.h"


extern void flash_read_gateway_info(mesh_network_info_t *p_network);
/*****************************************************************************
 * Private variables
 *****************************************************************************/
const static uint32_t NET_KEY_INDEX = 0;
const static uint8_t DEV_KEY_INDEX = 0;
/*APP KEY*/
const static uint32_t APP_KEY_INDEX = 0;
const static uint32_t m_iv_index = 1;
static uint8_t m_app_key[] = APP_KEY;   /**<Array of predefine keys => Use Random RNG to generates key in the future>*/
static uint8_t m_net_key[] = NET_KEY;
static uint8_t m_dev_key[] = DEV_KEY; 

nrf_mesh_network_secmat_t KEY;
//const static uint32_t 


/*DSM Handle*/
/**<Handle of each dsm object>*/
static dsm_handle_t m_subnet_handle = 0;
static dsm_handle_t m_appkey_handle = 0;
static dsm_handle_t m_devkey_handle = 0;
static dsm_handle_t m_group_1_addres_handle = 0;
static dsm_handle_t m_group_2_addres_handle = 0;
static dsm_handle_t m_group_3_addres_handle = 0;

/**<Static uniscast addresss for provisioning>*/
static dsm_local_unicast_address_t m_start_unicast_add = {
  .address_start = LOCAL_ADDRESS_START,
  .count = ACCESS_ELEMENT_COUNT
 };
static nrf_mesh_prov_provisioning_data_t provision_data = {
  .netkey = NET_KEY,
  .netkey_index = NET_KEY_INDEX,
  .iv_index = 0,
  .address = LOCAL_ADDRESS_START,
  .flags.iv_update = false,
  .flags.key_refresh = false
};
extern bool nrf_mesh_is_device_provisioned(void);
/*****************************************************************************
 * Function variables
 *****************************************************************************/
/*
 @brief: Set Unicast address
         devkey index must be equal to start unicast address
*/
static void self_provision(uint8_t *p_app_key, uint8_t *p_netkey, dsm_local_unicast_address_t unicast_add)
{
   ret_code_t err_code = NRF_SUCCESS;
  
  m_start_unicast_add = unicast_add;
  //__LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Last unicast address %d\r\n", last_unicast.address_start + last_unicast.count);
  memcpy(m_app_key, p_app_key, sizeof(m_app_key));
  memcpy(m_net_key, p_netkey, sizeof(m_net_key));
  err_code = dsm_local_unicast_addresses_set(&m_start_unicast_add);
    //__LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Local address set error: %d\r\n", err_code);
  APP_ERROR_CHECK(err_code);
  /*Add Mesh Keys*/
  err_code = dsm_subnet_add(0, m_net_key, &m_subnet_handle);
    //__LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Subnet add %d\r\n", err_code);
  APP_ERROR_CHECK(err_code);

  err_code = net_state_iv_index_set(0,0);
  APP_ERROR_CHECK(err_code);

  m_subnet_handle = dsm_net_key_index_to_subnet_handle(0);
  err_code = dsm_devkey_add(m_start_unicast_add.address_start + m_start_unicast_add.count, m_subnet_handle, m_dev_key, &m_devkey_handle);
  APP_ERROR_CHECK(err_code);
     /*Add App Key*/
  err_code = dsm_appkey_add(APP_KEY_INDEX, m_subnet_handle, p_app_key, &m_appkey_handle);
  //provision_data.netkey_index++;
  APP_ERROR_CHECK(err_code);
     /*Add Dev Key*/
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "SELF Provision self add key state\r\n");
}
/** @brief Function for getting vector of random numbers.
 *
 * @param[out] p_buff       Pointer to unit8_t buffer for storing the bytes.
 * @param[in]  length       Number of bytes to take from pool and place in p_buff.
 *
 * @retval     Number of bytes actually placed in p_buff.
 */
static uint8_t random_vector_generate(uint8_t * p_buff, uint8_t size)
{
    uint32_t err_code;
    uint8_t  available;

    nrf_drv_rng_bytes_available(&available);
    uint8_t length = MIN(size, available);

    err_code = nrf_drv_rng_rand(p_buff, length);
    APP_ERROR_CHECK(err_code);

    return length;
}
/*****************************************************************************
 * Public API
 *****************************************************************************/
 /** @brief
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
void subcribe_topic(access_model_handle_t model)
{
  ret_code_t err_code = NRF_SUCCESS;
  /*Add devkey handle to config server - PERSISTANCE STORAGE*/
  err_code = config_server_bind(m_devkey_handle);
  APP_ERROR_CHECK(err_code);
  /*Application Model Binds*/
  err_code = access_model_application_bind(model, m_appkey_handle);
  APP_ERROR_CHECK(err_code);
  /*Set the public address*/
  err_code = dsm_address_publish_add(MESH_TOPIC_CONTROL, &m_group_1_addres_handle);
  APP_ERROR_CHECK(err_code);

  err_code = access_model_publish_application_set(model, m_appkey_handle);
  APP_ERROR_CHECK(err_code);
  
  err_code = access_model_publish_address_set(model, m_group_1_addres_handle);
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Public address set %d\r\n", err_code);
  APP_ERROR_CHECK(err_code);
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Subribe Topic\r\n");

// Add the address to the DSM as a subscription address:
  //dsm_address_subscription_add(GROUP_ADDRESS_1, &m_group_1_addres_handle);
  dsm_address_subscription_add(MESH_TOPIC_ALL, &m_group_2_addres_handle);
  dsm_address_subscription_add(MESH_TOPIC_REPORT, &m_group_3_addres_handle);
  // Add the subscription to the model:
  //access_model_subscription_add(model, m_group_1_addres_handle);
  access_model_subscription_add(model, m_group_2_addres_handle);
  access_model_subscription_add(model, m_group_3_addres_handle);
 // mesh_stack_device_reset(); 
}

/** @brief
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
void app_self_provision(access_model_handle_t model_handle,
                        int8_t *p_app_key, uint8_t *p_netkey, 
                        dsm_local_unicast_address_t m_start_unicast_add)
{
  bool is_device_provisioned = nrf_mesh_is_device_provisioned();
  if(is_device_provisioned == false)
  {
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Begin Self Provision\r\n");
    self_provision(p_app_key, p_netkey, m_start_unicast_add);
  }
  else
  {
    dsm_local_unicast_address_t last_unicast = {0};
    dsm_local_unicast_addresses_get(&last_unicast); 
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Last unicast address %d\r\n", last_unicast.address_start);
  }
  subcribe_topic(model_handle);
}
/** @brief
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
//void app_gateway_get_current_info()
//{
//  dsm_local_unicast_address_t local_addr;
//  uint32_t ret_code = 0;
//  mesh_gateway_transfer_t gateway_info;
//  nrf_mesh_secmat_t secmat;

//  dsm_handle_t get_app_key_handle = dsm_appkey_index_to_appkey_handle(0);
//  dsm_handle_t get_net_key_handle = dsm_net_key_index_to_subnet_handle(0);
//  ret_code = dsm_tx_secmat_get(get_net_key_handle, get_app_key_handle, &secmat);
//  /**<Get all current subnet keys>*/
//  //dsm_subnet_key_get(gateway_info.net_key.n_value, &gateway_info.net_key.net_count);
//  //ret_code = dsm_subnet_key_get(m_subnet_handle, gateway_info.net_key.n_value);
//  //APP_ERROR_CHECK(ret_code);
//  //__LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Get current network security materials\r\n");
//  //__LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Netkey secmatr:\r\n:");
//  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "***) Network identifier:%d\r\n", secmat.p_net->nid);
//  __LOG_XB(LOG_SRC_APP, LOG_LEVEL_INFO, "***) Network Encyption Key", secmat.p_net->encryption_key, NRF_MESH_KEY_SIZE);
//  __LOG_XB(LOG_SRC_APP, LOG_LEVEL_INFO, "\r\n***) Network Private Key", secmat.p_net->privacy_key, NRF_MESH_KEY_SIZE);

//  //__LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "\r\n\r\nGet current Applcation security materials\r\n");
//  //__LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Appke secmatr:\r\n:");
//  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "***) Application  ID:%d\r\n", secmat.p_app->aid);
//  __LOG_XB(LOG_SRC_APP, LOG_LEVEL_INFO, "***) Application Encyption Key=", secmat.p_app->key, NRF_MESH_KEY_SIZE);
//  //__LOG_XB(LOG_SRC_APP, LOG_LEVEL_INFO, "\r\n***) Network Private Key", secmat.p_net->privacy_key);
//  /*Get sequence number and vector index*/
//}

/** @brief
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
void app_get_sequence_number_and_iv_index(uint32_t *iv_index, uint32_t *sequence_number)
{
  uint32_t ret_code;
  mesh_opt_seqnum_persist_data_t seqnum_data = {0};
  *iv_index = net_state_tx_iv_index_get();
  ret_code = mesh_config_entry_get(MESH_OPT_NET_STATE_SEQ_NUM_BLOCK_EID, &seqnum_data);
   __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Get sequence number data from flash%d\r\n", ret_code);
   APP_ERROR_CHECK(ret_code);
  *sequence_number = seqnum_data.next_block;
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "***) IV index :%d\r\n", *iv_index);
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "***) Sequence Number:%u\r\n", *sequence_number);
}
/** @brief
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
void app_generate_random_keys(uint8_t *app_key, uint8_t *net_key)
{
    uint32_t err_code;
     
    err_code = nrf_drv_rng_init(NULL);
    APP_ERROR_CHECK(err_code);

    uint8_t p_buff[NRF_MESH_KEY_SIZE];
    /*Generate application key*/
    uint8_t length = random_vector_generate(p_buff, NRF_MESH_KEY_SIZE);
    memcpy(net_key, p_buff, sizeof(p_buff));
    /*Generate network key*/
    length = random_vector_generate(p_buff, NRF_MESH_KEY_SIZE);
    memcpy(app_key, p_buff, sizeof(p_buff));

    __LOG_XB(LOG_SRC_APP, LOG_LEVEL_INFO, "***) Generate Application Key", app_key, NRF_MESH_KEY_SIZE);
    __LOG_XB(LOG_SRC_APP, LOG_LEVEL_INFO, "\r\n***) Generate Network Key", net_key, NRF_MESH_KEY_SIZE);
}
/** @brief
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
void app_tx_parse_and_send()
{
    mesh_network_info_t mesh_keys = {0};
    flash_read_gateway_info(&mesh_keys);
    uint32_t iv_index = 0;
    uint32_t sequence_number = 0;
    app_get_sequence_number_and_iv_index(&iv_index, &sequence_number);
    /*TODO:"Sending function*/
}
