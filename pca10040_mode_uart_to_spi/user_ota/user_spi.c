
#include "sdk_config.h"
#include "nrf_drv_spis.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "app_error.h"
#include <string.h>
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "ota_update.h"
#include "nrf_bootloader_app_start.h"
#include "flash_if.h"
#include "nrf_bootloader_dfu_timers.h"



/*****************************************************************************
 * Macro
 *****************************************************************************/
#define APP_SPI_ESP32_REQUEST_DATA        0x03  /*<ESP32 Request ESP32 Data Opcode>*/
#define APP_SPI_ESP32_REQUEST_MSG_PING    0x01  /*<ESP32 Request Ping message>*/
#define APP_SPI_ESP32_REQUEST_MSG_BEACON  0x02 /*<ESP32 Request Beacon message>*/
#define APP_SPI_ESP32_OTA_UPDATE_START    0x04  

#define APP_SPI_ESP32_OTA_UPDATE_TRANSFER 0x05
#define APP_SPI_ESP32_OTA_UPDATE_END      0x06
#define APP_SPI_ESP32_OTA_UPDATE_ACK      0x07
#define APP_SPI_ESP32_OTA_UPDATE_FAILED   0x08
#define APP_SPI_ESP32_OTA_UPDATE_ASK_NEW_FW 0x09



#define SPI_SLAVE_CS_PIN      28
#define SPI_SLAVE_MISO_PIN    25
#define SPI_SLAVE_MOSI_PIN    26
#define SPI_SLAVE_SCK_PIN     27

#define SPI_TOTAL_STATE       3

#define BUFFER_LENGHT 256
#define SPIS_INSTANCE 1 /**< SPIS instance index. */

#define TEST_STRING "Nodic"


#define APP_GD32_REQUEST_PING   0x00
#define APP_GD32_REQUEST_BEACON 0x01
#define APP_GD32_SEND_CONFIG    0x02
#define APP_GD32_SPI_TOKEN      0x6699
#define APP_GD32_FIRMWARE_VALID 0xABCD
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

typedef union __attribute((packed))
{ 
  struct __attribute((packed)){
    uint32_t crc32;
    uint32_t fw_size;
    uint8_t  is_fw_valid;
  }info;
  uint32_t value[3];
}firmware_valid_t;

/*****************************************************************************
 * Static Variables
 *****************************************************************************/
static const nrf_drv_spis_t spis = NRF_DRV_SPIS_INSTANCE(SPIS_INSTANCE);/**< SPIS instance. */
static uint8_t       m_spi_tx_buf[BUFFER_LENGHT] = {0};           /**< TX buffer. */
static uint8_t       m_spi_rx_buf[BUFFER_LENGHT] = {0};           /**< RX buffer. */
static volatile bool spis_xfer_done; /**< Flag used to indicate that SPIS instance completed the transfer. */
static trans_state_t spi_state = SPI_STATE_RX_REQUEST;
static firmware_valid_t firmware_valid = {0};
//static const spi_state_p_func[2] = {spi_state_rx_request, spi_state_tx_answer};
//static const spi_ack_p_func m_spi_slave_handle[3] = {spi_gd32_handle_ping, spi_gd32_handle_beacon, spi_gd32_handle_config};
/*****************************************************************************
 * Forward Declaration
 *****************************************************************************/

static void spis_event_handler(nrf_drv_spis_event_t event);
static uint32_t ping_message_create(uint8_t *p_buffer);
static uint32_t ack_message_create(uint8_t *p_buffer);
static uint32_t failed_message_create(uint8_t *p_buffer);
static uint32_t ask_message_create(uint8_t *p_buffer);
static void no_request_from_host();
static bool app_verify_new_firmware_valid(uint8_t *p_buffer);




/*****************************************************************************
 * Public API Declaration
 *****************************************************************************/
 //extern void app_beacon_queue_read(app_beacon_data_t *p_beacon_data);
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
      if((m_spi_rx_buf[0] & APP_GD32_SPI_TOKEN >> 8) && (m_spi_rx_buf[1] & (APP_GD32_SPI_TOKEN & 0x00FF)))
      {
          nrf_bootloader_dfu_inactivity_timer_restart(NRF_BOOTLOADER_MS_TO_TICKS(3000), no_request_from_host);
          /*Handle the appropriate Message ID*/
          uint8_t payload_length = m_spi_rx_buf[3];
          uint8_t *p_payload = &m_spi_rx_buf[4];
          //m_spi_slave_handle[m_spi_rx_buf[2]]( );
          switch(m_spi_rx_buf[2])
          {
            case APP_SPI_ESP32_OTA_UPDATE_START:
            {
              uint32_t retval = 0;
              //uint32_t firmware_size = p_payload[3] << 24 | p_payload[2] << 16 | p_payload[1] << 8 | p_payload[0];
              //retval = ota_update_start(firmware_size);

              /*Reply to GD32. Create Tx data*/

            }break;
            case APP_SPI_ESP32_OTA_UPDATE_TRANSFER:
            {
              uint8_t payload_length = m_spi_rx_buf[3];
              uint8_t *p_payload = &m_spi_rx_buf[4];
              uint32_t retval = 0;
              retval = ota_update_write_next(p_payload, payload_length);
              if(retval)
              {
                if (app_ota_is_all_data_received())
                {
                    NRF_LOG_INFO("On OTA End\r\n");
                    ack_message_create(m_spi_tx_buf);
                    /*Prepare packet to send*/
                    APP_ERROR_CHECK(nrf_drv_spis_buffers_set(&spis, m_spi_tx_buf, 256, m_spi_rx_buf, 256));
                    NVIC_SystemReset();
                    NRF_LOG_INFO("Never reach here, unless YOU are DOOM\r\n");
                    while (1);
                }
              }
              else
              {
                    failed_message_create(m_spi_tx_buf);
                    /*Prepare packet to send*/
                    APP_ERROR_CHECK(nrf_drv_spis_buffers_set(&spis, m_spi_tx_buf, 256, m_spi_rx_buf, 256));
              }
            }break;

            case APP_SPI_ESP32_OTA_UPDATE_END:
            {
                    NRF_LOG_INFO("OTA DONE => Reset the system\r\n");
                    ack_message_create(m_spi_tx_buf);
                    /*Prepare packet to send*/
                    APP_ERROR_CHECK(nrf_drv_spis_buffers_set(&spis, m_spi_tx_buf, 256, m_spi_rx_buf, 256));
                    NVIC_SystemReset();
            }break;
            case APP_SPI_ESP32_OTA_UPDATE_ASK_NEW_FW:
            {
                    uint8_t payload_length = m_spi_rx_buf[3];
                    uint8_t *p_payload = &m_spi_rx_buf[4];
                    if(app_verify_new_firmware_valid(p_payload))
                    {
                      ota_update_start(firmware_valid.info.fw_size);
                      memset(m_spi_tx_buf, 0, sizeof(m_spi_tx_buf));
                      ack_message_create(m_spi_tx_buf);
                      /*Prepare packet to send*/
                      APP_ERROR_CHECK(nrf_drv_spis_buffers_set(&spis, m_spi_tx_buf, 256, m_spi_rx_buf, 256));
                    }
                    else
                    {
                      /*JUMP TO APPLICATION AREA*/
                      nrf_bootloader_app_start();
                    }
            }break;

      
          }
      }
    }
     //spi_state_function[spi_state](&event, &spi_state);
}
static bool is_allow_boot = false;
static void no_request_from_host()
{
  NRF_LOG_INFO("No Request from host --> Jump to SoftDevice\r\n");
  //nrf_bootloader_app_start();
  is_allow_boot = true;
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
    ask_message_create(m_spi_tx_buf);
    //ack_message_create(m_spi_tx_buf);
    APP_ERROR_CHECK(nrf_drv_spis_buffers_set(&spis, m_spi_tx_buf, 255, m_spi_rx_buf, 255));
    NRF_LOG_INFO("SPI Main Initialize\r\n");
    nrf_bootloader_dfu_inactivity_timer_restart(
                        NRF_BOOTLOADER_MS_TO_TICKS(3000),
                        no_request_from_host);
    while(1)
    {
     if(is_allow_boot == true)
     {
      nrf_bootloader_app_start();
     }
     NRF_LOG_FLUSH();
    }
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

static uint32_t ack_message_create(uint8_t *p_buffer)
{
  if(p_buffer != NULL)
  {
    return NRF_ERROR_NULL;
  }

  /*Set the protocol*/
  uint16_t buffer_count = 0;
  uint16_t token = APP_GD32_SPI_TOKEN;

  memcpy(p_buffer + buffer_count, &token , sizeof(uint16_t));
  buffer_count += 2;
  p_buffer[buffer_count] = APP_SPI_ESP32_OTA_UPDATE_ACK;
  buffer_count++;
  /*Length*/
  p_buffer[buffer_count] = 0;
  buffer_count++;
  /*Create payload*/

  /*Copy payload to out buffer*/
  return NRF_SUCCESS;
}

static uint32_t failed_message_create(uint8_t *p_buffer)
{
  if(p_buffer != NULL)
  {
    return NRF_ERROR_NULL;
  }

  /*Set the protocol*/
  uint16_t buffer_count = 0;
  uint16_t token = APP_GD32_SPI_TOKEN;

  memcpy(p_buffer + buffer_count, &token , sizeof(uint16_t));
  buffer_count += 2;
  p_buffer[buffer_count] = APP_SPI_ESP32_OTA_UPDATE_FAILED;
  buffer_count++;
  /*Length*/
  p_buffer[buffer_count] = 0;
  buffer_count++;
  /*Create payload*/

  /*Copy payload to out buffer*/
  return NRF_SUCCESS;
}


static uint32_t ask_message_create(uint8_t *p_buffer)
{
  if(p_buffer != NULL)
  {
    return NRF_ERROR_NULL;
  }

  /*Set the protocol*/
  uint16_t buffer_count = 0;
  uint16_t token = APP_GD32_SPI_TOKEN;

  memcpy(p_buffer + buffer_count, &token , sizeof(uint16_t));
  buffer_count += 2;
  p_buffer[buffer_count] = APP_SPI_ESP32_OTA_UPDATE_ASK_NEW_FW;
  buffer_count++;
  /*Length*/
  p_buffer[buffer_count] = 0;
  buffer_count++;
  /*Copy payload to out buffer*/
  return NRF_SUCCESS;
}

static bool app_verify_new_firmware_valid(uint8_t *p_buffer)
{
  if(p_buffer == NULL)
  {
    return false;
  }
  firmware_valid_t *p_firmware_valid = (firmware_valid_t*)p_buffer;
  if(p_firmware_valid->info.is_fw_valid == true)
  {
    firmware_valid.info.fw_size = p_firmware_valid->info.is_fw_valid;
    firmware_valid.info.crc32 = p_firmware_valid->info.crc32;
    return true;
  }
  else
  {
    //nrf_bootloader_app_start_final(OTA_UPDATE_APPLICATION_START_ADDR);
    return false;
  }
}
/*
    Wait for ESP32 Data Request
*/



