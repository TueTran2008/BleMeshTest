#include "log.h"
#include "rtt_input.h"
#include "user_input.h"
#include "app_timer.h"
#include "mesh_app_utils.h"
#include "string.h"
#include <stdlib.h>
#include "Utilities.h"
#include "user_on_off_client.h"
#include "user_on_off_common.h"
#include "model_common.h"
#define RTT_INPUT_POLL_PERIOD_MS    (1000)
APP_TIMER_DEF(m_rtt_timer);


extern user_on_off_client_t m_client;
small_buffer_t rtt_buffer;

static void rtt_input_handler(int key)
{
  rtt_buffer.buffer[rtt_buffer.write_index++] = key;
}
void read_buffer_timeout(void *p_args)
{ 
  model_transition_t transition_params;
  user_on_off_msg_set_t set_packet;
  static uint8_t tid = 0;
  static uint8_t m_allow_publish_count = 0;
  //uint8_t m_allow_publish_count = 0;
  uint8_t buffer_temp[64];
  uint8_t duty_cycle;
  uint8_t state;
  uint8_t *ptr_temp;
  bool m_is_send_unack = false; 
  if(rtt_buffer.write_index == 0)
  {
    return;
  }
  __LOG(LOG_SRC_APP, LOG_LEVEL_WARN, "Data Read from RTT:%s\r\n", rtt_buffer.buffer);
  rtt_buffer.write_index = 0;
  
  if(strstr(rtt_buffer.buffer, "SET"))
  {
    ptr_temp = strstr(rtt_buffer.buffer, "DUTY");
    if(ptr_temp)
    {
      /*Found the Set Command*/
      m_allow_publish_count++;
      /*Take Parameter inside bracket()*/
      CopyParameter(ptr_temp, buffer_temp,'(', ')');
      duty_cycle = atoi(buffer_temp);
      memset(buffer_temp, 0, sizeof(buffer_temp));
      __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "DutyCycle:%d\r\n", duty_cycle);
    }
    ptr_temp = strstr(rtt_buffer.buffer, "STATE");
    if(ptr_temp)
    {
      m_allow_publish_count++;
      CopyParameter(ptr_temp, buffer_temp,'(', ')');
      state = atoi(buffer_temp);
      memset(buffer_temp, 0, sizeof(buffer_temp));
      __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "STATE:%d\r\n", state);
    }
  }
  memset(rtt_buffer.buffer, 0, sizeof(rtt_buffer.buffer));
  if(m_allow_publish_count >=2)
  {
    //tid = 0;
    uint32_t status = NRF_SUCCESS;
    tid = 0;
    set_packet.on_off = state;
    set_packet.pwm_period = duty_cycle;
    set_packet.tid = tid;

    if(m_is_send_unack == false)
    {
      __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Start Send ACK MSG - Model_Handle:%d\r\n", m_client.model_handle);

     // (void)access_model_reliable_cancel(m_client.model_handle);
      status = user_on_off_client_set(&m_client, &set_packet, &transition_params);
    }
    else
    {
      status = user_on_off_client_set_unack(&m_client, &set_packet, &transition_params, 1);
    }
    if(status != NRF_SUCCESS)
    {
      __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Failed to send MESH message!!!Error code: 0x%02x\r\n", status);
    }
    else
    {
      __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, " MESH SENDED\r\n", status);
    }

    //return status;
  }
  else 
  {
    return;
  }
}

void rtt_input_init()
{
    ERROR_CHECK(app_timer_create(&m_rtt_timer, APP_TIMER_MODE_REPEATED, read_buffer_timeout));
    ERROR_CHECK(app_timer_start(m_rtt_timer, 10000, NULL));
    rtt_input_enable(rtt_input_handler, 100);
}



