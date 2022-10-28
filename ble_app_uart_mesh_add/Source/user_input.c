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
#include "DataDefine.h"
#include "ble_uart_service.h"
#include "app_sef_provision.h"
#include "app_wdt.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "ble_config.h"
#include "flash_if.h"
#include "app_mesh_check_duplicate.h"
#include "app_mesh_gateway_msg.h"
#include "app_mesh_message_queue.h"
#include "app_mesh_process_data.h"
#include "rtc.h"
#include "nrf_log_ctrl.h"



#define RTT_INPUT_POLL_PERIOD_MS    (1000)
APP_TIMER_DEF(m_rtt_timer);

#if(DEV_TEST)
extern mesh_network_info_t gateway_info;
extern user_on_off_client_t m_client;
extern void flash_save_gateway_info(mesh_network_info_t *p_network);
extern void fake_insert_new_data();
extern void fake_send_data();
extern void fake_insert_duplicate_data();
extern uint32_t getcounter(void);
//extern void advertising_start(void);
//extern void advertising_stop(void);
#endif


small_buffer_t rtt_buffer;

static rtt_uart_service_handle_t uart_handle;

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
  if(strstr(rtt_buffer.buffer, "IV"))
  {
    uint32_t iv_index = 0;
    uint32_t sequence_number = 0;
    app_get_sequence_number_and_iv_index(&iv_index, &sequence_number);
  }
  if(strstr(rtt_buffer.buffer, "MESH_RESET"))
  {
    mesh_core_clear(NULL);
  }
  if(strstr(rtt_buffer.buffer, "INSERT_NEW"))
  {
    fake_insert_new_data();
  }
  if(strstr(rtt_buffer.buffer, "BEACON_READ"))
  {
    app_beacon_data_t beacon;
    app_queue_mesh_read_node(&beacon);
  }
  if(strstr(rtt_buffer.buffer, "INSERT_DUPLI"))
  {
    fake_insert_duplicate_data();
  }
  if(strstr(rtt_buffer.buffer, "READ"))
  {
    fake_send_data();
  }
  if(strstr(rtt_buffer.buffer, "GENERATE"))
  {
    uint8_t app_key[16];
    uint8_t net_key[16];
    app_generate_random_keys(app_key, net_key);
    //app_gateway_get_current_info();
  }
  if(strstr(rtt_buffer.buffer,"ENTER_PAIR"))
  {
    NRF_LOG_INFO("User start advertising\r\n");
    NRF_LOG_FLUSH();
    app_ble_enter_pair_mode();
    
  }
  if(strstr(rtt_buffer.buffer,"EXIT_PAIR"))
  {
    NRF_LOG_INFO("User stop advertising\r\n");
    NRF_LOG_FLUSH();
    app_ble_exit_pair_mode();
  }

  if(strstr(rtt_buffer.buffer,"BLETX"))
  {
    CopyParameter(ptr_temp, buffer_temp,'(', ')');  
    if(uart_handle)
    {
      uart_handle((void*)buffer_temp);
    }
  }
  if(strstr(rtt_buffer.buffer, "SAVE_FLASH"))
  {
    app_flash_save_config_parameter();
  }
  if(strstr(rtt_buffer.buffer, "GET_FLASH"))
  {
    app_flash_get_config_parameter();
  }
  if(strstr(rtt_buffer.buffer, "SET_LAMP"))
  {
    static bool set_lamp_value = false;
    if(set_lamp_value)
    {
      set_lamp_value = false;
    }
    else
    {
      set_lamp_value = true;
    }
    app_mesh_publish_set_lamp(0xC005, set_lamp_value);
  }
  if(strstr(rtt_buffer.buffer, "MESH_RECONFIG"))
  {
    uint8_t appkey[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    uint8_t netkey[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    app_mesh_reprovision_from_server(appkey, netkey);
  }
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
  if(strstr(rtt_buffer.buffer, "BUZZER_TOGGLE"))
  {
    xSystem.led_driver.Toggle(BUZZER_PIN_INDEX);
  }
  if(strstr(rtt_buffer.buffer, "LED_TOGGLE"))
  {
    xSystem.led_driver.Toggle(LED_ALARM_INDEX);
  }
  if(strstr(rtt_buffer.buffer, "RTC_SET"))
  {
    uint8_t ptr_temp[128] = {0};
    CopyParameter(rtt_buffer.buffer, ptr_temp,'(', ')');
    uint32_t timestamp = atoi(ptr_temp);
    NRF_LOG_WARNING("Set time stamp: %u\r\n", timestamp);
    RTC_UpdateTimeFromServer(timestamp);
  }
  if(strstr(rtt_buffer.buffer, "GET_MAC"))
  {
    NRF_LOG_INFO("Manually get MAC address");
    NRF_LOG_HEXDUMP_INFO(app_ble_get_mac(), 6);
  }
  if(strstr(rtt_buffer.buffer, "REPROVISION_TRUE"))
  {
    app_unit_test_reprosivion(true);
  }
  if(strstr(rtt_buffer.buffer, "REPROVISION_FALSE"))
  {
    app_unit_test_reprosivion(false);
  }
  if(strstr(rtt_buffer.buffer, "MODEL_TEST"))
  {
        uint32_t status = 0;
       user_on_off_msg_set_t tx_msg_params = {0};
       tx_msg_params.tid = 1;
       tx_msg_params.len = 10;
       tx_msg_params.data = "Hello";

       model_transition_t tx_transtion;
       status = unit_test_model_layer(&m_client, &tx_msg_params,&tx_transtion, 2);
       if(status)
       {
         NRF_LOG_INFO("Periodic publish mesh message - error code: %d", status);
       }
  }
  if(strstr(rtt_buffer.buffer, "RTC_GET"))
  {
    //uint32_t timestamp = getcounter();
    //NRF_LOG_INFO("Timestamp:%u", timestamp);
    DateTime_t get_datetime;
    get_datetime = RTC_GetDateTime();
    NRF_LOG_ERROR("Get datetime\n%d/%d/%d\n%d:%d:%u",
                  get_datetime.Day,
                  get_datetime.Month,
                  get_datetime.Year,
                  get_datetime.Hour,
                  get_datetime.Minute,
                  get_datetime.Second);

  }
  memset(rtt_buffer.buffer, 0, sizeof(rtt_buffer.buffer));
  if(m_allow_publish_count >=2)
  {
    //tid = 0;
    uint32_t status = NRF_SUCCESS;
    tid = 0;
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

void rtt_input_init(rtt_uart_service_handle_t p_callback)
{
    uart_handle = p_callback;
    ERROR_CHECK(app_timer_create(&m_rtt_timer, APP_TIMER_MODE_REPEATED, read_buffer_timeout));
    ERROR_CHECK(app_timer_start(m_rtt_timer, 5000, NULL));
    rtt_input_enable(rtt_input_handler, 10);
}



