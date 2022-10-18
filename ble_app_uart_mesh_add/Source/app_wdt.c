#include "app_wdt.h"
#include "led_driver.h"
#include "nrf_gpiote.h"
#include "nrfx_gpiote.h"
#include "app_timer.h"


static nrfx_gpiote_pin_t wdt_pin = WDT_PIN;
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
    
   nrfx_gpiote_out_init(wdt_pin, &wdt_pin_cfg);
}
void app_wdt_feed()
{
  nrfx_gpiote_out_toggle(wdt_pin);
}
