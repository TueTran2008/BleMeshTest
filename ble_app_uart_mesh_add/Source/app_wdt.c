#include "app_wdt.h"
#include "led_driver.h"
#include "nrf_gpiote.h"
#include "nrfx_gpiote.h"
#include "app_timer.h"
#include "app_wdt.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_drv_wdt.h"
#define WDT_ENABLE 1
static nrfx_gpiote_pin_t wdt_pin = WDT_PIN;
static nrf_drv_wdt_channel_id wdt_channel;
//static volatile uint32_t m_wdt_period = 0;
// Before WDT make a reset , it run this handler.WDT events handler.
static void wdt_event_handler(void);
static void software_watchdog_init(uint32_t ms);
 /*****************************************************************************
 * Forward Decleration 
 *****************************************************************************/
static void software_watchdog_init(uint32_t ms)
{
#if WDT_ENABLE
    uint32_t err_code;
    nrf_drv_wdt_config_t config = NRF_DRV_WDT_DEAFULT_CONFIG;
    config.reload_value = ms;
    err_code = nrf_drv_wdt_init(&config, wdt_event_handler);
    err_code = nrf_drv_wdt_channel_alloc(&wdt_channel);
    APP_ERROR_CHECK(err_code);
    nrf_drv_wdt_enable();
#endif
}
static void wdt_event_handler(void)
{
  NRF_LOG_ERROR("Watch Dog software trigger\r\n");
  NRF_LOG_FLUSH();
}
 /*****************************************************************************
 * Function Implementation 
 *****************************************************************************/
void app_wdt_init()
{
    if (!nrfx_gpiote_is_init())
    {
        APP_ERROR_CHECK(nrfx_gpiote_init());
    }
    nrfx_gpiote_out_config_t wdt_pin_cfg = {
      .init_state = NRF_GPIOTE_INITIAL_VALUE_LOW,
      .task_pin = false
    };
   software_watchdog_init(30000);
   nrfx_gpiote_out_init(wdt_pin, &wdt_pin_cfg);
}
void app_wdt_feed()
{
  /*Software Watchdog*/
  nrfx_gpiote_out_toggle(wdt_pin);
  #if WDT_ENABLE
  /*Hardware Watchdog*/
  nrf_drv_wdt_channel_feed(wdt_channel);
  #endif
}
