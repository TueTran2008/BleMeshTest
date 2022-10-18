
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

/*****************************************************************************
 * Macro
 *****************************************************************************/
#define APP_SPI_ESP32_REQUEST_DATA        0x03  /*<ESP32 Request ESP32 Data Opcode>*/
#define APP_SPI_ESP32_REQUEST_MSG_PING    0x01  /*<ESP32 Request Ping message>*/
#define APP_SPI_ESP32_REQUEST_MSG_BEACON  0x02 /*<ESP32 Request Beacon message>*/


#define SPI_SLAVE_CS_PIN      28
#define SPI_SLAVE_MISO_PIN    25
#define SPI_SLAVE_MOSI_PIN    26
#define SPI_SLAVE_SCK_PIN     27

#define SPI_TOTAL_STATE       3

#define BUFFER_LENGHT 256
#define SPIS_INSTANCE 1 /**< SPIS instance index. */

#define TEST_STRING "Nodic"


//#define APP_GD32_REQUEST_PING   0x00
//#define APP_GD32_REQUEST_BEACON 0x01
//#define APP_GD32_SEND_CONFIG    0x02
#define APP_GD32_SPI_TOKEN      0x6699
/**<SPI Interface>*/
/****

  | 1 Byte First Token | + | 1 Byte Last Token| + | 1 Byte Message ID | + | 1 Byte Length| + |Payload|
***/


/*****************************************************************************
 * Forward Declaration
 *****************************************************************************/
typedef enum
{
  SPI_STATE_RX_REQUEST = 0x00,    /*Wait for GD32 Request data*/
  SPI_STATE_TX_ANSWER = 0x01
}trans_state_t;

//typedef void(*spi_state_p_func)(void*args, trans_state_t *p_state);
typedef void (*spi_ack_p_func)(uint8_t length);
//static void spi_state_rx_request(void*args, trans_state_t *p_state);
//static void spi_state_tx_answer(void*args, trans_state_t *p_state);

static void spi_gd32_handle_ping(uint8_t length);
static void spi_gd32_handle_beacon(uint8_t length);
static void spi_gd32_handle_config(uint8_t length);
/*****************************************************************************
 * Static Variables
 *****************************************************************************/
static const nrf_drv_spis_t spis = NRF_DRV_SPIS_INSTANCE(SPIS_INSTANCE);/**< SPIS instance. */
static uint8_t       m_spi_tx_buf[BUFFER_LENGHT+1] = {0};           /**< TX buffer. */
static uint8_t       m_spi_rx_buf[BUFFER_LENGHT+1] = {0};           /**< RX buffer. */
static volatile bool spis_xfer_done; /**< Flag used to indicate that SPIS instance completed the transfer. */
static trans_state_t spi_state = SPI_STATE_RX_REQUEST;
//static const spi_state_p_func[2] = {spi_state_rx_request, spi_state_tx_answer};
static const spi_ack_p_func m_spi_slave_handle[3] = {spi_gd32_handle_ping, spi_gd32_handle_beacon, spi_gd32_handle_config};
/*****************************************************************************
 * Forward Declaration
 *****************************************************************************/

static void spis_event_handler(nrf_drv_spis_event_t event);
static uint32_t ping_message_create(uint8_t *p_buffer);
/*****************************************************************************
 * Public API Declaration
 *****************************************************************************/
 extern void app_beacon_queue_read(app_beacon_data_t *p_beacon_data);
/**
 * @brief SPIS user event handler.
 *
 * @param event
 */



static void spis_event_handler(nrf_drv_spis_event_t event)
{
    
    if (event.evt_type == NRF_DRV_SPIS_XFER_DONE)
    {
      /*Check for token*/
      if((m_spi_rx_buf[1] & APP_GD32_SPI_TOKEN >> 8) && (m_spi_rx_buf[0] & (APP_GD32_SPI_TOKEN & 0x00FF)))
      {
          /*Handle the appropriate Message ID*/
          uint8_t payload_length = m_spi_rx_buf[3];
          m_spi_slave_handle[m_spi_rx_buf[2] - 1](payload_length);
      }
    }
     //spi_state_function[spi_state](&event, &spi_state);
}

void spi_main(void)
{
    memset(m_spi_rx_buf, 0, sizeof(m_spi_rx_buf));
    nrf_drv_spis_config_t spis_config = NRF_DRV_SPIS_DEFAULT_CONFIG;
    spis_config.csn_pin               = SPI_SLAVE_CS_PIN;
    spis_config.miso_pin              = SPI_SLAVE_MISO_PIN;
    spis_config.mosi_pin              = SPI_SLAVE_MOSI_PIN;
    spis_config.sck_pin               = SPI_SLAVE_SCK_PIN;

    APP_ERROR_CHECK(nrf_drv_spis_init(&spis, &spis_config, spis_event_handler));
    /*Wait for master Header*/
    APP_ERROR_CHECK(nrf_drv_spis_buffers_set(&spis, m_spi_tx_buf, 255, m_spi_rx_buf, 255));
    //spi_state = SPI_STATE_RX_HEADER;
}
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
  memcpy(ping_msg.gateway_mac, app_ble_get_mac, 6);
  ping_msg.in_pair_mode = xSystem.is_in_pair_mode;
  ping_msg.sensor_count = xSystem.sensor_count;
  //ping_msg.
  app_get_sequence_number_and_iv_index(&temp_iv_index, &temp_sq_number);
  ping_msg.iv_index = temp_iv_index;
  ping_msg.sequence_number = temp_sq_number;

  memcpy(ping_msg.mesh_key.appkey, xSystem.network_info.app_key, NRF_MESH_KEY_SIZE);
  memcpy(ping_msg.mesh_key.netkey, xSystem.network_info.net_key, NRF_MESH_KEY_SIZE);
  /*Copy payload to out buffer*/
  memcpy(p_buffer + buffer_count, &ping_msg, sizeof(app_beacon_ping_msg_t));
  buffer_count += sizeof(app_beacon_ping_msg_t);
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
  buffer_count += sizeof(app_beacon_data_t);
  memcpy(p_buffer + buffer_count, &beacon_data, sizeof(app_beacon_data_t));
  return NRF_SUCCESS;
}

static void spi_gd32_handle_ping(uint8_t length)
{
  /*unused variable*/
  (void)length;
  uint8_t temp_buffer[BUFFER_LENGHT];
  ping_message_create(temp_buffer);
  memcpy(m_spi_tx_buf, temp_buffer, BUFFER_LENGHT);
  NRF_LOG_INFO("Send Ping message\r\n");
  APP_ERROR_CHECK(nrf_drv_spis_buffers_set(&spis, m_spi_tx_buf, 255, m_spi_rx_buf, 255));
}
static void spi_gd32_handle_beacon(uint8_t length)
{
  /*unused variable*/
  app_beacon_data_t *p_beacon;
  (void)length;
  uint8_t temp_buffer[BUFFER_LENGHT];
  if(app_queue_mesh_read_node(p_beacon) == 0xFF)
  {
    // NO BEACON DATA
    beacon_message_create(temp_buffer, NULL);
  }
  else
  {
    NRF_LOG_WARNING("Sending Beacon SPI Through BEACON MSG ID\r\n");
    beacon_message_create(temp_buffer, p_beacon);
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

    //TODO : Reconfig sequence number;
  }
  if(p_config->config_netkey)
  {
    const uint8_t length = 16;
    NRF_LOG_INFO("Config NetKey");
    NRF_LOG_HEXDUMP_INFO(p_config->netkey, length);
    memcpy(xSystem.flash_parameter.pair_information.key.netkey, p_config->netkey, 16);
    p_temp_netkey = p_config->netkey;
    reconfig_mesh = true;
    //TODO : Reconfig sequence number;
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
    app_get_sequence_number_and_iv_index(&current_iv_index,&current_sequence_number);
    uint32_t sequen_block = (p_config->sequence_number / 8192) + 1;
    NRF_LOG_INFO("Config IV Index:%d - Sequence Number Block: %d\r\n", p_config->iv_index, sequen_block);
    net_state_iv_index_and_seqnum_block_set(p_config->iv_index, true, sequen_block);
    //reconfig_mesh = true;
  }
  app_flash_save_config_parameter();
  if(reconfig_mesh)
  {
    app_mesh_reprovision_from_server(p_temp_appkey, p_temp_netkey);
  }

  /*Config Mesh Network need to reset the device*/
}



