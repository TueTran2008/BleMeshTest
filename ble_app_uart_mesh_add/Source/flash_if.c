



#include "nrf_mesh_config_core.h"
/* Provisioning and configuration */
#include "nrf_mesh_configure.h"

#include "mesh_config_entry.h"
#include "mesh_opt.h"


#include "log.h"
#include "DataDefine.h"
#include "DataDefine.h"
/* Gateway for the enocean record entry IDs */

#include <stddef.h>
#include <string.h>
#include "mesh_config.h"
#include "mesh_config_entry.h"
#include "mesh_config_backend.h"
#include "mesh_config_listener.h"
#include "mesh_opt.h"
#if PERSISTENT_STORAGE == 0
#include "bearer_event.h"
#endif
#include "utils.h"
#include "event.h"
#include "emergency_cache.h"

#include "nrf_section.h"
#include "nrf_error.h"

static app_flash_get_success_cb_t app_get_done = NULL;

#define GATEWAY_INFO_MAX_VALUE_STORE  1
#define GATEWAY_INFO_RECORD_START   0x0001
#define GATEWAY_INFO_RECORD_END     GATEWAY_INFO_RECORD_START + GATEWAY_INFO_MAX_VALUE_STORE
#define GATEWAY_INFO_FILE_ID (0x0011)


#define GATEWAY_INFO_ENTRY_ID     MESH_CONFIG_ENTRY_ID(GATEWAY_INFO_FILE_ID, GATEWAY_INFO_RECORD_START)

static mesh_network_info_t m_gateway_info = {0};
/* Live RAM representation of the value */
static uint32_t m_live_value = 5000;


/*****************************************************************************
 * Forward declaration
 *****************************************************************************/
static uint32_t gateway_info_setter(mesh_config_entry_id_t id, const void *p_entry);
static void gateway_info_gettter(mesh_config_entry_id_t id, void *p_entry);
static void gateway_info_deletter(mesh_config_entry_id_t id);
/* Declare a mesh config file with a unique file ID */
NRF_MESH_STATIC_ASSERT(MESH_OPT_FIRST_FREE_ID <= GATEWAY_INFO_FILE_ID);
MESH_CONFIG_FILE(m_gateway_info_file, GATEWAY_INFO_FILE_ID, MESH_CONFIG_STRATEGY_CONTINUOUS);
MESH_CONFIG_ENTRY(gateway_info,
                  GATEWAY_INFO_ENTRY_ID,
                  1,
                  sizeof(mesh_network_info_t),
                  gateway_info_setter,
                  gateway_info_gettter,
                  gateway_info_deletter,
                  false);

/* Gateway info mesh config data entry. */


/*****************************************************************************
 * Static Functions
 *****************************************************************************/
static uint32_t gateway_info_setter(mesh_config_entry_id_t id, const void *p_entry)
{
    if (!IS_IN_RANGE(id.record, GATEWAY_INFO_RECORD_START, GATEWAY_INFO_RECORD_END))
    {
        return NRF_ERROR_NOT_FOUND;
    }
    /*Get index record of entry*/
    uint16_t idx = GATEWAY_INFO_RECORD_START;
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Gateway info [%d] setter ...\n", idx);

    const mesh_network_info_t * p_gateway_info_flash = (const mesh_network_info_t *) p_entry;
    memcpy(&m_gateway_info, p_gateway_info_flash, sizeof(mesh_network_info_t));
    return NRF_SUCCESS;
}



static void gateway_info_gettter(mesh_config_entry_id_t id,void *p_entry)
{
    NRF_MESH_ASSERT_DEBUG(IS_IN_RANGE(id.record, GATEWAY_INFO_RECORD_START, GATEWAY_INFO_RECORD_END));

    uint16_t idx = id.record - GATEWAY_INFO_RECORD_START;
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Gateway info [%d] getter ...\n", idx);

    mesh_network_info_t * p_gateway_info_flash = (mesh_network_info_t *) p_entry;
    memcpy(p_gateway_info_flash, &m_gateway_info, sizeof(mesh_network_info_t));
    

}
static void gateway_info_deletter(mesh_config_entry_id_t id)
{
    NRF_MESH_ASSERT_DEBUG(IS_IN_RANGE(id.record, GATEWAY_INFO_RECORD_START, GATEWAY_INFO_RECORD_END));
    uint16_t idx = id.record - GATEWAY_INFO_RECORD_START;
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Gateway info [%d] deleter ...\n", idx);
    memset(&m_gateway_info, 0x00, sizeof(mesh_network_info_t));
}




void app_start_flash(app_flash_get_success_cb_t p_callback)
{
  if(p_callback)
  {
    app_get_done = p_callback; 
  }
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Starting application \n");
  mesh_config_entry_id_t idx = GATEWAY_INFO_ENTRY_ID;

  /**/
  for (uint8_t i = 0; i < GATEWAY_INFO_MAX_VALUE_STORE; i++)
  {
    mesh_network_info_t gateway_info;

    uint32_t status = mesh_config_entry_get(idx, (void*)&gateway_info);
    if (status == NRF_SUCCESS)
    {
        //__LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "gateway_info: %d \n", gateway_info);
        //app_get_done(&gateway_info);
        __LOG_XB(LOG_SRC_APP, LOG_LEVEL_INFO,"NETWORK_KEY:\r\n",m_gateway_info.net_key, 16);
        __LOG_XB(LOG_SRC_APP, LOG_LEVEL_INFO,"APPLICATION_KEY:\r\n",m_gateway_info.app_key, 16);
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO,"Unicast address:0x%04x\r\n", m_gateway_info.unicast_address.address_start + m_gateway_info.unicast_address.count);
    }
    else
    {
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Fail to read mesh network infomation => Please add this node to server to config");
    }
    idx.record++;
   }
}

void flash_read_gateway_info()
{
  mesh_config_entry_id_t idx = GATEWAY_INFO_ENTRY_ID;
  mesh_config_entry_get(idx, &m_gateway_info);
  __LOG_XB(LOG_SRC_APP, LOG_LEVEL_INFO,"NETWORK_KEY:\r\n",m_gateway_info.net_key, 16);
  __LOG_XB(LOG_SRC_APP, LOG_LEVEL_INFO,"APPLICATION_KEY:\r\n",m_gateway_info.app_key, 16);
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO,"Unicast address:0x%04x\r\n", m_gateway_info.unicast_address.address_start + m_gateway_info.unicast_address.count);

}
void flash_save_gateway_info(mesh_network_info_t *p_network)
{
  mesh_config_entry_id_t idx = GATEWAY_INFO_ENTRY_ID;
  __LOG_XB(LOG_SRC_APP, LOG_LEVEL_INFO,"SAVE NETWORK_KEY:\r\n",p_network->net_key, 16);
  __LOG_XB(LOG_SRC_APP, LOG_LEVEL_INFO,"SAVE APPLICATION_KEY:\r\n",p_network->app_key, 16);
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO,"SAVE Unicast address:0x%04x\r\n", p_network->unicast_address.address_start + p_network->unicast_address.count);
  mesh_config_entry_set(idx, p_network);
}

//static mesh_config_backend_iterate_action_t gateway_info_restore_callback(mesh_config_entry_id_t id, const uint8_t * p_entry, uint32_t entry_len)
//{
  
//}

//void mesh_flash_load_gatewayinfo()
//{
//  mesh_config_backend_read_all(mesh_config_backend_iterate_action_t);

//}


