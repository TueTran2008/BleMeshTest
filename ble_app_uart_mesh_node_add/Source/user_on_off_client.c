//#include "simple_on_off_client.h"
//#include "simple_on_off_common.h"



#include <stdint.h>
#include <stddef.h>
#include "model_common.h"

#include "user_on_off_client.h"
#include "user_on_off_common.h"
#include "access.h"
#include "access_config.h"
#include "access_reliable.h"
#include "device_state_manager.h"
#include "nrf_mesh.h"
#include "nrf_mesh_assert.h"
#include "log.h"
#include "nordic_common.h"
//#include "generic_onoff_client.h"


#define USER_ON_OFF_CLIENT_MODEL_ID       0xABCD






void opcode_handle_callback(access_model_handle_t model_handle, const access_message_rx_t * p_rx_message, void * p_args)
{
  user_on_off_client_t *p_client = (user_on_off_client_t*)p_args;
  user_on_off_status_msg_pkt_t data_temp = {0};
  if(p_rx_message->length == USER_ON_OFF_MESSAGE_MIN_PAYLOAD_LENGHT || 
  p_rx_message->length == USER_ON_OFF_MESSAGE_MAX_PAYLOAD_LENGHT )
  {
    user_on_off_status_msg_pkt_t *p_data = (user_on_off_status_msg_pkt_t *)p_rx_message->p_data;
    
    if(p_rx_message->length == USER_ON_OFF_MESSAGE_MIN_PAYLOAD_LENGHT)
    {
      data_temp.present_on_off = p_data->present_on_off;
      data_temp.remaining_time = /*p_data->remaining_time*/0;
      data_temp.target_on_off = p_data->target_on_off;
    }
    else
    {
      data_temp.present_on_off = p_data->present_on_off;
      data_temp.remaining_time = model_transition_time_decode(p_data->remaining_time);
      data_temp.target_on_off = p_data->target_on_off;
    }
    p_client->setting.p_callbacks->onoff_status_cb(p_client, &p_rx_message->meta_data, &data_temp);
  }
    
}


/*****************************************************************************
 * Static Variables
 *****************************************************************************/
//const static access_opcode_t opcode_id = {
//  .company_id = 0xFF,
//  .opcode = USER_ON_OFF_CLIENT_MODEL_ID
//};
//const access_opcode_handler_t m_opcode_handler[] =
//{
//  { opcode_id,
//    opcode_handle_callback
//  },
//};
static const access_opcode_handler_t m_opcode_handlers[] =
{
    {ACCESS_OPCODE_VENDOR(USER_ON_OFF_OPCODE_STATUS, USER_ON_OFF_COMPANY_ID), opcode_handle_callback},
};
/*****************************************************************************
 * Private Function
 *****************************************************************************/
/*
@   brief: Create Contend of Payload 
@   return: Payload size
*/
static uint8_t message_packet_create(user_on_off_msg_set_pkt_t *msg_pkt, user_on_off_msg_set_t *msg_set, const model_transition_t *p_transition)
{
  /*Most importance data when use light switch*/
  msg_pkt->on_off = msg_set->on_off;
  msg_pkt->tid = msg_set->tid;
  msg_pkt->pwm_period = msg_set->pwm_period;
  if(p_transition != NULL)
  {
    msg_pkt->transition_time = model_transition_time_encode(p_transition->transition_time_ms);
    msg_pkt->delay = model_delay_encode(p_transition->delay_ms);
    //msg_pkt->delay = model_trans
  }
  return USER_ON_OFF_MESSAGE_MIN_PAYLOAD_LENGHT;
}

/*
  Create Message to pass to publish API
*/
static void message_create(user_on_off_client_t *p_client, uint8_t* buffer, access_message_tx_t *p_msg, uint16_t msg_lenght, uint16_t tx_opcode, uint16_t company_opcode)
{
 // p_msg->
  //p_
  p_msg->force_segmented = p_client->setting.force_segmented;
  p_msg->access_token = nrf_mesh_unique_token_get();
  p_msg->length = msg_lenght;
  p_msg->opcode.opcode = tx_opcode;
  p_msg->opcode.company_id = company_opcode;
  p_msg->transmic_size = p_client->setting.transmic_size;
  p_msg->p_buffer = buffer;
  //p_msg->
}
/*
  @brief: Only use to send reliable message

*/
static void reliable_context_create(user_on_off_client_t *p_client, access_reliable_t *p_reliable, uint16_t reply_opcode, uint16_t company_opcode)
{
  /*Pass the value of access_message_tx_t*/
  //p_reliable->message = p_client->access_message.message;
  /*Pass the value of model Handle*/
  p_reliable->model_handle = p_client->model_handle;
  /**/
  p_reliable->reply_opcode.opcode = reply_opcode;
  p_reliable->reply_opcode.company_id = company_opcode;
  /**/
  p_reliable->timeout = p_client->setting.timeout;
  /**/
  p_reliable->status_cb = p_client->setting.p_callbacks->ack_transaction_status_cb;
}
/*****************************************************************************
 * Public API
 *****************************************************************************/
uint32_t user_on_off_client_init(user_on_off_client_t *p_client, uint16_t element_index)
{
  if(p_client->setting.p_callbacks == NULL ||
     p_client->setting.p_callbacks->onoff_status_cb == NULL||
     p_client->setting.p_callbacks->periodic_publish_cb == NULL ||
     p_client == NULL)
  {
    return NRF_ERROR_NULL;
  }
  if (p_client->setting.timeout == 0)
  {
    p_client->setting.timeout = MODEL_ACKNOWLEDGED_TRANSACTION_TIMEOUT;
  }
  access_model_add_params_t model_parameter = 
  {
    .element_index = element_index,
    .model_id = ACCESS_MODEL_VENDOR(USER_ON_OFF_CLIENT_MODEL_ID, USER_ON_OFF_COMPANY_ID),
    .p_opcode_handlers = &m_opcode_handlers[0],
    .opcode_count = 1,
    .p_args = p_client,
    .publish_timeout_cb = p_client->setting.p_callbacks->periodic_publish_cb 
  };


  uint32_t volatile status = access_model_add(&model_parameter, &p_client->model_handle);
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "ADD Model USER_ON_OFF_SWITCH With Handle: %d\r\n", p_client->model_handle);
  if (status == NRF_SUCCESS)
  {
    status = access_model_subscription_list_alloc(p_client->model_handle);
  }
  else
  {
     __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Fail to add Model with Error-code: 0x%02x", status);
  }
  return status;
}

uint32_t user_on_off_client_set(user_on_off_client_t *p_client, user_on_off_msg_set_t *p_params, model_transition_t *p_transition)
{
  uint8_t packet_size = 0;
  //user
  //if(p_client == NULL || p_params == NULL || p_transition == NULL)
  //{
  //  return NRF_ERROR_NULL;
  //}
  if(access_reliable_model_is_free(p_client->model_handle))
  {
    packet_size = message_packet_create(&p_client->msg_pkt, p_params, (const model_transition_t*)p_transition);
    message_create(p_client, (uint8_t*)&p_client->msg_pkt, &p_client->access_message.message, (uint16_t)packet_size, USER_ON_OFF_OPCODE_SET, USER_ON_OFF_COMPANY_ID);
    reliable_context_create(p_client, &p_client->access_message, USER_ON_OFF_OPCODE_STATUS, USER_ON_OFF_COMPANY_ID);
    return access_model_reliable_publish(&p_client->access_message); 
  }
  else
  {
    return NRF_ERROR_BUSY;
  }
}

uint32_t user_on_off_client_set_unack(user_on_off_client_t *p_client, user_on_off_msg_set_t *p_params, model_transition_t *p_transition, uint16_t repeat_times)
{
  uint8_t return_value = 1;
  uint8_t packet_size = 0;
  if(p_client == NULL || p_params == NULL || p_transition == NULL)
  {
    return NRF_ERROR_NULL;
  }    
   message_create(p_client, (uint8_t*)&p_client->msg_pkt, &p_client->access_message.message, (uint16_t)packet_size, USER_ON_OFF_OPCODE_SET, USER_ON_OFF_COMPANY_ID);
    //reliable_context_create(p_client, &p_client->access_message, USER_ON_OFF_OPCODE_STATUS, BYTECH_COMPANY_OPCODE);
  repeat_times++;
  while(repeat_times-- && return_value)
  {
    return_value = access_model_publish(p_client->model_handle, &p_client->access_message.message);
  }
  return return_value;
}

uint32_t user_on_off_client_get(user_on_off_client_t *p_client/*, user_on_off_msg_set_t *p_params, model_transition_t *p_transition*/)
{
  uint8_t packet_size = 0;
  if(p_client == NULL )
  {
    return NRF_ERROR_NULL;
  }
  if(access_reliable_model_is_free(p_client->model_handle))
  {
    message_create(p_client, NULL, &p_client->access_message.message, 0, USER_ON_OFF_OPCODE_SET, USER_ON_OFF_COMPANY_ID);
    reliable_context_create(p_client, &p_client->access_message, USER_ON_OFF_OPCODE_STATUS, USER_ON_OFF_COMPANY_ID);
    return access_model_reliable_publish(&p_client->access_message); 
  }
  else
  {
    return NRF_ERROR_BUSY;
  }
}