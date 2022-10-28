



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
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "flash_if.h"
static app_flash_get_success_cb_t app_get_done = NULL;

#define GATEWAY_INFO_MAX_VALUE_STORE  1
#define GATEWAY_INFO_RECORD_START   0x0001
#define GATEWAY_INFO_RECORD_END     GATEWAY_INFO_RECORD_START + GATEWAY_INFO_MAX_VALUE_STORE
#define GATEWAY_INFO_FILE_ID (0x0011)


#define GATEWAY_INFO_ENTRY_ID     MESH_CONFIG_ENTRY_ID(GATEWAY_INFO_FILE_ID, GATEWAY_INFO_RECORD_START)

static FlashParameter_t m_gateway_info = {0};
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
                  /*sizeof(xSystem.flash_parameter)*/64,
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

    const FlashParameter_t * p_gateway_info_flash = (const FlashParameter_t *) p_entry;
    memcpy(&m_gateway_info, p_gateway_info_flash, sizeof(FlashParameter_t));
    return NRF_SUCCESS;
}



static void gateway_info_gettter(mesh_config_entry_id_t id,void *p_entry)
{
    NRF_MESH_ASSERT_DEBUG(IS_IN_RANGE(id.record, GATEWAY_INFO_RECORD_START, GATEWAY_INFO_RECORD_END));

    uint16_t idx = id.record - GATEWAY_INFO_RECORD_START;
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Gateway info [%d] getter ...\n", idx);
    FlashParameter_t *p_gateway_info_flash = (FlashParameter_t*)p_entry;
    memcpy(p_gateway_info_flash, &m_gateway_info, sizeof(FlashParameter_t));
}
static void gateway_info_deletter(mesh_config_entry_id_t id)
{
    NRF_MESH_ASSERT_DEBUG(IS_IN_RANGE(id.record, GATEWAY_INFO_RECORD_START, GATEWAY_INFO_RECORD_END));
    uint16_t idx = id.record - GATEWAY_INFO_RECORD_START;
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Gateway info [%d] deleter ...\n", idx);
    memset(&m_gateway_info, 0x00, sizeof(mesh_network_info_t));
}

uint32_t app_start_flash(app_flash_get_success_cb_t p_callback)
{
}

void flash_read_gateway_info(mesh_network_info_t *p_network)
{

}
void flash_save_gateway_info(mesh_network_info_t *p_network)
{

}

uint32_t app_flash_save_config_parameter()
{
  mesh_config_entry_id_t idx = GATEWAY_INFO_ENTRY_ID;
  uint32_t status = mesh_config_entry_set(idx, (void*)&xSystem.flash_parameter);
  if (status == NRF_SUCCESS)
  {
       NRF_LOG_INFO("Save Flash Parameter: %s - Line:%s - Error:%d\r\n", __FUNCTION__, __LINE__, status);
  }
  else
  {
       __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Fail to write mesh network infomation => Please add this node to server to config");
  }
  return status;
}
uint32_t app_flash_get_config_parameter()
{
  uint32_t ret_code = 0;
  mesh_config_entry_id_t idx = GATEWAY_INFO_ENTRY_ID;
  //FlashParameter_t flash_read;
  ret_code = mesh_config_entry_get(idx, &xSystem.flash_parameter);
  //APP_ERROR_CHECK(ret_code);
  NRF_LOG_INFO("Get Flash Parameter: %s - Line:%s- Error code:%d\r\n", __FUNCTION__, __LINE__, ret_code);
  return ret_code;
}