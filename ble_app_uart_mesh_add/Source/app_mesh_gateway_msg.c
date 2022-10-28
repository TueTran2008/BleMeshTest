#include "nrf_queue.h"
#include "app_mesh_gateway_msg.h"
#include "nrf_error.h"
#include "nrf_assert.h"
#include "nrf_log.h"
#include "DataDefine.h"
#include "ble_uart_service.h"
#include "app_button.h"

NRF_QUEUE_DEF(gw_mesh_msq_t, app_mesh_msg_queue, MAX_MESSAGE_RECEIVE_FROM_MESH_NETWORK_IN_QUEUE, NRF_QUEUE_MODE_OVERFLOW);



/***********************
*
*
*
*
*   QUEUE WAIT FOR MESH MESSAGE AND SCAN BEACON DATA WAIT FOR PROCESSING
*
*
*
*
*
*

***********************/
//#warning "FAKE Battery Value :V"
static inline uint8_t GET_BATTERY()
{
  return app_adc_get_battery_percent();
}

void app_mesh_gw_create_message(uint8_t msg_type, uint8_t msg_id, uint8_t mesh_id, gw_mesh_msq_t *p_out)
{
  /*Parser Gateway mesage Type - Header*/
  app_alarm_message_structure_t gw_msg = {0};
  gw_msg.battery_value = GET_BATTERY();
  gw_msg.dev_type = APP_DEVICE_GW;
  gw_msg.msg_type = msg_type;
  gw_msg.fw_version = FIRMWARE_VERSION;

  gw_mesh_msq_t msg_arrive = {0};
  /*Size of MAC address is 6*/
  memcpy(&msg_arrive.mac, app_ble_get_mac(), 6);
  /*Look like publish all*/
  msg_arrive.mesh_id = mesh_id;
  memcpy(msg_arrive.data, (uint8_t*)& gw_msg, sizeof(app_alarm_message_structure_t));
  msg_arrive.len = sizeof(app_alarm_message_structure_t);
  msg_arrive.msg_id = msg_id;
}



uint32_t app_mesh_gw_queue_push(void *p_data)
{
  uint32_t retval = nrf_queue_push((nrf_queue_t*)&app_mesh_msg_queue, p_data);
  if(retval != NRF_SUCCESS)
  {
   // NRF_LOG_ERROR("Queue error:%d at func :%s", retval, __FUNCTION__);
  }
  return retval;
}

uint32_t app_mesh_gw_queue_pop(void* p_data)
{
  uint32_t retval = nrf_queue_pop((nrf_queue_t*)&app_mesh_msg_queue, p_data);
  if(retval != NRF_SUCCESS)
  {
    //NRF_LOG_ERROR("Queue error:%d at func :%s", retval, __FUNCTION__);
  }
  return retval;
}

bool app_mesh_gw_queue_is_empty()
{
  return nrf_queue_is_empty((nrf_queue_t*)&app_mesh_msg_queue);
}

bool app_mesh_gw_queue_is_full()
{
  return nrf_queue_is_full((nrf_queue_t*)&app_mesh_msg_queue);
}

static void mesh_process_non_sos_message()
{

}
static void mesh_process_sos_message()
{

}
