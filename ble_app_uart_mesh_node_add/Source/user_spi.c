
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


#define BUFFER_LENGHT 32
#define SPIS_INSTANCE 1 /**< SPIS instance index. */
static const nrf_drv_spis_t spis = NRF_DRV_SPIS_INSTANCE(SPIS_INSTANCE);/**< SPIS instance. */
#define TEST_STRING "Nordic"

static uint8_t       m_tx_buf[BUFFER_LENGHT] = {0};           /**< TX buffer. */
static uint8_t       m_rx_buf[BUFFER_LENGHT] = {0};           /**< RX buffer. */
//static const uint8_t m_length = sizeof(m_tx_buf);        /**< Transfer length. */\

static volatile bool spis_xfer_done; /**< Flag used to indicate that SPIS instance completed the transfer. */

static uint32_t spi_state = 0;


/*If slave want to request send data to master => need to use one more pin to indicate*/
typedef enum
{
  SPI_STATE_RX_HEADER = 0x01,
  SPI_STATE_RX_PAYLOAD,
  SPI_STATE_TX_HEADER,
  SPI_STATE_PAYLOAD
}trans_state_t;

static void spi_slave_decode_header(uint8_t *p_data, uint16_t len)
{
  
}

/**
 * @brief SPIS user event handler.
 *
 * @param event
 */

void spis_event_handler(nrf_drv_spis_event_t event)
{
    switch (spi_state)
    {
      case SPI_STATE_RX_HEADER:
      {
        if (event.evt_type == NRF_DRV_SPIS_XFER_DONE)
        {
            //__LOG_XB(LOG_SRC_APP, LOG_LEVEL_INFO, "Received packet header", m_rx_buf, event.rx_amount);
        }
      }
      break;
    }
}

void spi_main(void)
{
    nrf_drv_spis_config_t spis_config = NRF_DRV_SPIS_DEFAULT_CONFIG;
    spis_config.csn_pin               = 1;
    spis_config.miso_pin              = 2;
    spis_config.mosi_pin              = 3;
    spis_config.sck_pin               = 4;

    //APP_ERROR_CHECK(nrf_drv_spis_init(&spis, &spis_config, spis_event_handler));
   // APP_ERROR_CHECK(nrf_drv_spis_buffers_set(&spis, m_tx_buf, m_length, m_rx_buf, m_length));

}
