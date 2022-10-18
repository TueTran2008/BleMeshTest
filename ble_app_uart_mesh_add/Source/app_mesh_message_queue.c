#include "nrf_queue.h"
#include "app_mesh_message_queue.h"
#include "nrf_error.h"
#include "nrf_assert.h"
#include "nrf_log.h"
#include "DataDefine.h"


NRF_QUEUE_DEF(app_mesh_tx_evt_t, app_mesh_msg_q, MESH_EVENT_SIZE, NRF_QUEUE_MODE_OVERFLOW);


uint32_t app_mesh_tx_evt_message_queue_push(void *p_data)
{
  uint32_t retval = nrf_queue_push((nrf_queue_t*)&app_mesh_msg_q, p_data);
  if(retval != NRF_SUCCESS)
  {
    //NRF_LOG_ERROR("Queue error:%d at func :%s", retval, __FUNCTION__);
  }
  return retval;
}

uint32_t app_mesh_tx_evt_message_queue_pop(void* p_data)
{
  uint32_t retval = nrf_queue_pop((nrf_queue_t*)&app_mesh_msg_q, p_data);
  if(retval != NRF_SUCCESS)
  {
    //NRF_LOG_ERROR("Queue error:%d at func :%s", retval, __FUNCTION__);
  }
  return retval;
}

bool app_mesh_tx_evt_message_queue_is_empty()
{
  return nrf_queue_is_empty((nrf_queue_t*)&app_mesh_msg_q);
}

bool app_mesh_tx_evt_message_queue_is_full()
{
  return nrf_queue_is_full((nrf_queue_t*)&app_mesh_msg_q);
}