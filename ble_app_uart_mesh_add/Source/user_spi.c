
#include "sdk_config.h"
#include "nrf_drv_spis.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "app_error.h"
#include <string.h>
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "log.h"
#include "ble_uart_service.h"
#include "DataDefine.h"
#include "app_sef_provision.h"

#include "app_mesh_gateway_msg.h"
#include "app_mesh_message_queue.h"
#include "app_mesh_process_data.h"
#include "net_state.h"

#include "flash_if.h"
#include "ble_config.h"
#include "led_driver.h"
#include "rtc.h"
#include "adc_button.h"
#include "Utilities.h"

/*****************************************************************************
 * Macro
 *****************************************************************************/
#define APP_SPI_ESP32_REQUEST_DATA        0x03  /*<ESP32 Request ESP32 Data Opcode>*/
#define APP_SPI_ESP32_REQUEST_MSG_PING    0x01  /*<ESP32 Request Ping message>*/
#define APP_SPI_ESP32_REQUEST_MSG_BEACON  0x02 /*<ESP32 Request Beacon message>*/
#define APP_SPI_ESP32_REQUEST_NEW_PAIR_BEACON 0x05
#define APP_SPI_ESP32_REQuEST_CONTROL_GPIO  0x04
#define APP_SPI_ESP32_REQUEST_RESPONSE_DATA 0x06

#define SPI_SLAVE_CS_PIN      28
#define SPI_SLAVE_MISO_PIN    25
#define SPI_SLAVE_MOSI_PIN    26
#define SPI_SLAVE_SCK_PIN     27

#define SPI_TOTAL_STATE       3
#define MAX_REQUEST           6
#define BUFFER_LENGHT 256
#define SPIS_INSTANCE 1 /**< SPIS instance index. */

#define TEST_STRING "Nodic"
#define APP_GD32_SPI_TOKEN      0x6699


/*****************************************************************************
 * Typedef Declaration
 *****************************************************************************/
typedef enum
{
  SPI_STATE_RX_REQUEST = 0x00,    /*Wait for GD32 Request data*/
  SPI_STATE_TX_ANSWER = 0x01
}trans_state_t;
typedef struct
{
  uint32_t timestamp;
  gp32_gpio_state_t gpio;
}app_gpio_msg;

/**<SPI Interface>*/
/****

  | 1 Byte First Token | + | 1 Byte Last Token| + | 1 Byte Message ID | + | 1 Byte Length| + |Payload|
****/

/*****************************************************************************
 * Typedef Declaration
 *****************************************************************************/
//typedef void(*spi_state_p_func)(void*args, trans_state_t *p_state);
/*****************************************************************************
 * Forward Declaration
 *****************************************************************************/
typedef void (*spi_ack_p_func)(uint8_t length);
static void spi_gd32_handle_ping(uint8_t length);
static void spi_gd32_handle_beacon(uint8_t length);
static void spi_gd32_handle_config(uint8_t length);
static void spi_gd32_handle_gpio(uint8_t length);
static void spi_gd32_handle_pair(uint8_t length);
static void spi_gd32_handle_reponse_pair_msg(uint8_t length);
/*****************************************************************************
 * Static Variables
 *****************************************************************************/
static queue_button_t m_last_adc_button_state = {0};
static const nrf_drv_spis_t spis = NRF_DRV_SPIS_INSTANCE(SPIS_INSTANCE);/**< SPIS instance. */
static uint8_t       m_spi_tx_buf[BUFFER_LENGHT+1] = {0};           /**< TX buffer. */
static uint8_t       m_spi_rx_buf[BUFFER_LENGHT+1] = {0};           /**< RX buffer. */
static bool m_xfer_done = false;
static volatile bool spis_xfer_done; /**< Flag used to indicate that SPIS instance completed the transfer. */
static trans_state_t spi_state = SPI_STATE_RX_REQUEST;
static const spi_ack_p_func m_spi_slave_handle[MAX_REQUEST] = 
{
  spi_gd32_handle_ping, 
  spi_gd32_handle_beacon, 
  spi_gd32_handle_config, 
  spi_gd32_handle_gpio, 
  spi_gd32_handle_pair, 
  spi_gd32_handle_reponse_pair_msg
};
/*****************************************************************************
 * Forward Declaration
 *****************************************************************************/
static bool spi_validate_handle(uint8_t msg_id);
static void spis_event_handler(nrf_drv_spis_event_t event);
static uint32_t ping_message_create(uint8_t *p_buffer);
/*****************************************************************************
 * Public API Declaration
 *****************************************************************************/
extern void app_beacon_queue_read(app_beacon_data_t *p_beacon_data);
static bool spi_validate_handle(uint8_t msg_id)
{
  if(msg_id >= MAX_REQUEST)
  {
    return false;
  }
  else
  {
    return true;
  }
}
/**
 * @brief SPIS user event handler.
 *
 * @param event
 */
static void spis_event_handler(nrf_drv_spis_event_t event)
{
    if (event.evt_type == NRF_DRV_SPIS_XFER_DONE /*&& event.rx_amount == 255*/)
    {
      /*Check for token*/
      if((m_spi_rx_buf[1] & APP_GD32_SPI_TOKEN >> 8) && (m_spi_rx_buf[0] & (APP_GD32_SPI_TOKEN & 0x00FF))
        && event.rx_amount == 255)
      {
          uint8_t msg_id = m_spi_rx_buf[2] - 1;
          if(spi_validate_handle(msg_id))
          {
          /*Handle the appropriate Message ID*/
            uint8_t payload_length = m_spi_rx_buf[3];
            m_spi_slave_handle[m_spi_rx_buf[2] - 1](payload_length);
          }
          else
          {
            APP_ERROR_CHECK(nrf_drv_spis_buffers_set(&spis, m_spi_tx_buf, 255, m_spi_rx_buf, 255));
          }
      }
      else
      {
        uint8_t *p_response = "FAIL";
        //m_spi_slave_handle[m_last_state_handle]()
        uint8_t buffer[255] = {0};
        uint16_t buffer_count = 0;
        uint16_t token = APP_GD32_SPI_TOKEN;
        memcpy(buffer + buffer_count, &token , sizeof(uint16_t));
        buffer_count += 2;
        buffer[buffer_count] = APP_SPI_ESP32_REQuEST_CONTROL_GPIO;
        buffer_count++;
        buffer[buffer_count] = strlen(p_response);
        buffer_count++;
        uint32_t check_sum = CalculateSum((uint8_t*)&p_response, strlen(p_response));
        memcpy(buffer + buffer_count, p_response, strlen(p_response));
        //memcpy()
        memcpy(m_spi_tx_buf, buffer, 255);
        APP_ERROR_CHECK(nrf_drv_spis_buffers_set(&spis, m_spi_tx_buf, 255, m_spi_rx_buf, 255));
      }
      m_xfer_done = true;
    }
     //spi_state_function[spi_state](&event, &spi_state);
} 
/** @brief      Create ping message send to esp32 ^^
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
void spi_reset_buff(void)
{
  if(m_xfer_done)
  {
    memset(m_spi_rx_buf, 0 ,255);
    //memset(m_spi_tx_buf, 0 ,255);
    m_xfer_done = false;
  }
}
/** @brief      Create ping message send to esp32 ^^
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
void spi_main(void)
{
    memset(m_spi_rx_buf, 0, sizeof(m_spi_rx_buf));
    nrf_drv_spis_config_t spis_config = NRF_DRV_SPIS_DEFAULT_CONFIG;
    spis_config.csn_pin               = SPI_SLAVE_CS_PIN;
    spis_config.miso_pin              = SPI_SLAVE_MISO_PIN;
    spis_config.mosi_pin              = SPI_SLAVE_MOSI_PIN;
    spis_config.sck_pin               = SPI_SLAVE_SCK_PIN;

    spis_config.csn_pullup = GPIO_PIN_CNF_PULL_Pullup;

    APP_ERROR_CHECK(nrf_drv_spis_init(&spis, &spis_config, spis_event_handler));
    /*Wait for master Header*/
    APP_ERROR_CHECK(nrf_drv_spis_buffers_set(&spis, m_spi_tx_buf, 255, m_spi_rx_buf, 255));
    //spi_state = SPI_STATE_RX_HEADER;
}

//ADD CHECK SUM IF NECESSARY
/** @brief      Create ping message send to esp32 ^^
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
static bool spi_verify_opcode(uint8_t esp32_opcode)
{
  if(esp32_opcode == APP_SPI_ESP32_REQUEST_MSG_PING ||
     esp32_opcode == APP_SPI_ESP32_REQUEST_MSG_BEACON)
  {
    return true;
  }
  else
  {
    return false;
  }
}
/** @brief      Create ping message send to esp32 ^^
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
static uint32_t prepare_data_send(uint8_t esp32_opcode)
{
  uint32_t length;
  return length;
}
/*
    Wait for ESP32 Data Request
*/

/** @brief      Create ping message send to esp32 ^^
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
static uint32_t ping_message_create(uint8_t *p_buffer)
{
  if(p_buffer == NULL)
  {
    return NRF_ERROR_NULL;
  }
  uint32_t temp_iv_index = 0;
  uint32_t temp_sq_number = 0;
  /*Set the protocol*/
  uint16_t buffer_count = 0;
  uint16_t token = APP_GD32_SPI_TOKEN;  // TOKEN
  memcpy(p_buffer + buffer_count, &token , sizeof(uint16_t));
  buffer_count += 2;
  p_buffer[buffer_count] = APP_SPI_ESP32_REQUEST_MSG_PING; // ID
  buffer_count++;
  p_buffer[buffer_count] = sizeof(app_beacon_ping_msg_t);// Length
  buffer_count++;
  /*Create payload*/
  app_beacon_ping_msg_t ping_msg;
  ping_msg.alarm_value = xSystem.alarm_value;
  //NRF_LOG_WARNING("Alarm value:%d", (uint16_t)(ping_msg.alarm_value));
  memcpy(ping_msg.gateway_mac, app_ble_get_mac, 6);
  ping_msg.in_pair_mode = xSystem.is_in_pair_mode;
  ping_msg.sensor_count = xSystem.sensor_count;
  app_get_sequence_number_and_iv_index(&temp_iv_index, &temp_sq_number);
  ping_msg.iv_index = temp_iv_index;
  ping_msg.sequence_number = temp_sq_number;
  /*Get the button state*/
  queue_button_t local_button;
  if(app_button_get_state(&local_button))
  {
    ping_msg.button_state = local_button;
    m_last_adc_button_state = local_button;
  }
  else
  {
    ping_msg.button_state = m_last_adc_button_state;
  }

  memcpy(ping_msg.mesh_key.appkey, xSystem.network_info.app_key, NRF_MESH_KEY_SIZE);
  memcpy(ping_msg.mesh_key.netkey, xSystem.network_info.net_key, NRF_MESH_KEY_SIZE);
  /*Copy payload to out buffer*/
  uint32_t check_sum = CalculateSum((uint8_t*)&ping_msg, sizeof(app_beacon_ping_msg_t));
  memcpy(p_buffer + buffer_count, &ping_msg, sizeof(app_beacon_ping_msg_t));
  buffer_count += sizeof(app_beacon_ping_msg_t);
  //NRF_LOG_HEXDUMP_WARNING(p_buffer, buffer_count);
  return NRF_SUCCESS;
}
/** @brief      Create ping message send to esp32 ^^
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
static uint32_t beacon_message_create(uint8_t *p_buffer, app_beacon_data_t *p_beacon_in)
{
  if(p_buffer == NULL)
  {
    return NRF_ERROR_NULL;
  }
  uint16_t buffer_count = 0;
  uint16_t token = APP_GD32_SPI_TOKEN;
  memcpy(p_buffer + buffer_count, &token , sizeof(uint16_t));
  buffer_count += 2;
  p_buffer[buffer_count] = APP_SPI_ESP32_REQUEST_MSG_BEACON;
  buffer_count++;
  p_buffer[buffer_count] = sizeof(app_beacon_msg_t);
  buffer_count++;
  //app_beacon_msg_t beacon_message;
  app_beacon_data_t beacon_data;
  if(p_beacon_in == NULL)
  {
    memset(&beacon_data, 0, sizeof(app_beacon_data_t));
  }
  else
  {
    memcpy(&beacon_data, &p_beacon_in, sizeof(app_beacon_data_t));
  }
  memcpy(p_buffer + buffer_count, &beacon_data, sizeof(app_beacon_data_t));
  uint32_t check_sum = CalculateSum((uint8_t*)&beacon_data, sizeof(app_beacon_data_t));
  buffer_count += sizeof(app_beacon_data_t);
  //NRF_LOG_HEXDUMP_WARNING(p_buffer, buffer_count);
  return NRF_SUCCESS;
}
/** @brief      Handle correstponse to Ping message from gd32
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
static void spi_gd32_handle_ping(uint8_t length)
{
  /*unused variable*/
  (void)length;
  uint8_t temp_buffer[BUFFER_LENGHT];
  ping_message_create(temp_buffer);
  memcpy(m_spi_tx_buf, temp_buffer, BUFFER_LENGHT);
  NRF_LOG_WARNING("Send Ping message\r\n");
  APP_ERROR_CHECK(nrf_drv_spis_buffers_set(&spis, m_spi_tx_buf, 255, m_spi_rx_buf, 255));
}
/** @brief      Create ping message send to esp32 ^^
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
static void spi_gd32_handle_gpio(uint8_t length)
{
  app_gpio_msg *p_gpio_msg = (app_gpio_msg*)&m_spi_rx_buf[4];
  gp32_gpio_state_t gpio_state = p_gpio_msg->gpio;
  /*Handle GPIO MESSAGE*/
  app_gd32_gpio_msg_handle(gpio_state);
  NRF_LOG_INFO("Timestamp: %u\r\n", p_gpio_msg->timestamp);
  /**/
  RTC_UpdateTimeFromServer(p_gpio_msg->timestamp);
  //TODO IMPLEMENT RTC SET VALUE :D

}
static void spi_gd32_handle_beacon(uint8_t length)
{
  /*unused variable*/
  static app_beacon_data_t m_last_beacon = {0};
  app_beacon_data_t p_beacon;
  (void)length;
  uint8_t temp_buffer[BUFFER_LENGHT];
  if(app_queue_mesh_read_node(&p_beacon) == 0xFF)
  {
    // NO BEACON DATA
    beacon_message_create(temp_buffer, &m_last_beacon);
  }
  else
  {
    memcpy(&m_last_beacon, &p_beacon, sizeof(app_beacon_data_t));
    NRF_LOG_WARNING("Sending Beacon SPI Through BEACON MSG ID\r\n");
    beacon_message_create(temp_buffer, &p_beacon);
  }

  memcpy(m_spi_tx_buf, temp_buffer, BUFFER_LENGHT);
  APP_ERROR_CHECK(nrf_drv_spis_buffers_set(&spis, m_spi_tx_buf, 255, m_spi_rx_buf, 255));
}
/** @brief      Create ping message send to esp32 ^^
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
static void spi_gd32_handle_config(uint8_t length)
{
  NRF_LOG_WARNING("Handle config message");
  bool reconfig_mesh = 0;
  uint32_t current_sequence_number = 0;
  uint32_t current_iv_index = 0;
  uint8_t *p_temp_appkey = NULL;
  uint8_t *p_temp_netkey = NULL;
  GD32Config_t *p_config = (GD32Config_t*)&m_spi_rx_buf[4];
  if(p_config->config_alarm)
  {
    NRF_LOG_INFO("Config Alarm\r\n");
    xSystem.flash_parameter.config_parameter.AlarmConfig = p_config->alarm;
  }
  if(p_config->config_appkey)
  {
    NRF_LOG_INFO("Config AppKey");
    const uint8_t length = 16;
    NRF_LOG_HEXDUMP_INFO(p_config->appkey, length);
    memcpy(xSystem.flash_parameter.pair_information.key.appkey, p_config->appkey, 16);
    p_temp_appkey = p_config->appkey;
    reconfig_mesh = true;
  }
  if(p_config->config_netkey)
  {
    const uint8_t length = 16;
    NRF_LOG_INFO("Config NetKey");
    NRF_LOG_HEXDUMP_INFO(p_config->netkey, length);
    memcpy(xSystem.flash_parameter.pair_information.key.netkey, p_config->netkey, 16);
    p_temp_netkey = p_config->netkey;
    reconfig_mesh = true;
  }
  /*Handle Speaker Config*/
  if(p_config->config_speaker)
  {
    NRF_LOG_INFO("Config Speaker:%d\r\n", p_config->speaker);
    if(p_config->speaker >= 100)
    {
      p_config->speaker = 100;
    }
    else if(p_config->speaker <= 0)
    {
      p_config->speaker = 0;
    }
      xSystem.flash_parameter.config_parameter.SpeakerVolume = p_config->speaker;
  }
  /*Handle Speaker Config*/
  if(p_config->config_IVindex && p_config->config_seqnumber)
  {
    //uint32_t ret_code = 0;
    uint32_t timeout = 10;
    app_get_sequence_number_and_iv_index(&current_iv_index, &current_sequence_number);
    uint32_t sequen_block = (p_config->sequence_number / 8192) + 1;
    NRF_LOG_INFO("Config IV Index:%d - Sequence Number Block: %d\r\n", p_config->iv_index, sequen_block);
    while(net_state_iv_index_and_seqnum_block_set(p_config->iv_index, false, sequen_block) && timeout--)
    {
      if(timeout == 1)
      {
        timeout = 0;
        net_state_iv_index_set(p_config->iv_index + 1, 0);
      }
    }
    mesh_core_clear(NULL);
  }
  if(p_config->config_mac)
  {
    NRF_LOG_INFO("Config MAC Address\r\n");
    memcpy(xSystem.flash_parameter.pair_information.pair_mac, p_config->mac, 6);
  }
  app_flash_save_config_parameter();
  if(reconfig_mesh)
  {
    app_mesh_reprovision_from_server(p_temp_appkey, p_temp_netkey);
  }
  uint8_t *p_response = "OK";
  uint8_t buffer[255] = {0};
  uint16_t buffer_count = 0;
  uint16_t token = APP_GD32_SPI_TOKEN;
  memcpy(buffer + buffer_count, &token , sizeof(uint16_t));
  buffer_count += 2;
  buffer[buffer_count] = APP_SPI_ESP32_REQuEST_CONTROL_GPIO;
  buffer_count++;
  buffer[buffer_count] = strlen(p_response);
  buffer_count++;
  memcpy(buffer + buffer_count, p_response, strlen(p_response));
  uint32_t check_sum = CalculateSum((uint8_t*)&p_response, strlen(p_response));
  memcpy(m_spi_tx_buf, buffer, 255);
  APP_ERROR_CHECK(nrf_drv_spis_buffers_set(&spis, m_spi_tx_buf, 255, m_spi_rx_buf, 255));
}
/*
  @ Send MAC Address and Device types to ESP32. Therefore, ESP32
  checks if node has already existed => To gave the appropriate Unicast Adddress 
*/
static void spi_gd32_handle_pair(uint8_t length)
{
  beacon_pair_info_t pair_msg;
  uint8_t buffer[255] = {0};
  memcpy(pair_msg.device_mac, xSystem.status.beacon_pair_info.device_mac, 6);
  pair_msg.pair_success = xSystem.status.beacon_pair_info.pair_success;
  pair_msg.device_type = xSystem.status.beacon_pair_info.device_type;
  /*Prepare packet*/
  uint16_t buffer_count = 0;
  uint16_t token = APP_GD32_SPI_TOKEN;
  memcpy(buffer + buffer_count, &token , sizeof(uint16_t));
  buffer_count += 2;
  buffer[buffer_count] = APP_SPI_ESP32_REQUEST_NEW_PAIR_BEACON;
  buffer_count++;
  buffer[buffer_count] = sizeof(beacon_pair_info_t);
  buffer_count += sizeof(beacon_pair_info_t);
  buffer_count++;
  memcpy(buffer + buffer_count, &pair_msg, sizeof(beacon_pair_info_t));
  uint32_t check_sum = CalculateSum((uint8_t*)&pair_msg, sizeof(beacon_pair_info_t));
  buffer_count = buffer_count + sizeof(beacon_pair_info_t);
 // NRF_LOG_HEXDUMP_DEBUG(buffer, buffer_count);
  memcpy(m_spi_tx_buf, buffer , 255);
  APP_ERROR_CHECK(nrf_drv_spis_buffers_set(&spis, m_spi_tx_buf, 255, m_spi_rx_buf, 255));
}
/*
  @ Send MAC Address and Device types to ESP32. Therefore, ESP32
  
*/
static void spi_gd32_handle_reponse_pair_msg(uint8_t length)
{
  //**TODO Create link does the response to the node
  //**
  uint8_t *p_response = NULL;
  pair_response_info_t *p_unicast = (pair_response_info_t*)&m_spi_rx_buf[4];
  uint16_t node_next_add = 0;
  memcpy(&node_next_add, p_unicast, 2);
  if(node_next_add >= APP_MESH_MIN_VALID_UNICAST_ADD
     && node_next_add <= APP_MESH_MAX_VALID_UNICAST_ADD)
  {
    //xSystem.
    memcpy(&xSystem.flash_parameter.pair_information.next_unicast_addr, p_unicast, 2);
    app_node_send_nus_data();
    p_response = "OK";
  }
  else
  {
    p_response = "FAIL";
    NRF_LOG_ERROR("Invalid unicast address");
  }
  uint8_t buffer[255] = {0};
  uint16_t buffer_count = 0;
  uint16_t token = APP_GD32_SPI_TOKEN;
  memcpy(buffer + buffer_count, &token , sizeof(uint16_t));
  buffer_count += 2;
  buffer[buffer_count] = APP_SPI_ESP32_REQUEST_RESPONSE_DATA;
  buffer_count++;
  buffer[buffer_count] = strlen(p_response);
  buffer_count++;
  memcpy(buffer + buffer_count, p_response, strlen(p_response));
  //memcpy()
  uint32_t check_sum = CalculateSum((uint8_t*)&p_response, strlen(p_response));
  memcpy(m_spi_tx_buf, buffer, 255);
  APP_ERROR_CHECK(nrf_drv_spis_buffers_set(&spis, m_spi_tx_buf, 255, m_spi_rx_buf, 255));
}
