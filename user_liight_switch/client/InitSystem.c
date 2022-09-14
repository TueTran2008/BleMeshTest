#include "Hardware.h"
#include "app_timer.h"
#include "InitSystem.h"
/* Logging and RTT */
#include "log.h"
#include "rtt_input.h"
#include "nrf_gpiote.h"
//#include "nrf_gpio.h"


#include "nrf.h"
#include "nrf_error.h"
#include "boards.h"
#include "nrf_delay.h"
#include "nrfx_gpiote.h"
#include "sdk_common.h"
#include "nrf_drv_clock.h"

#include "app_btn.h"

#include "simple_hal.h"
#if NRF_MODULE_ENABLED(NRF_CLOCK)

#endif
/******************************************************************************
                                   STATIC VARIABLES					    			 
 ******************************************************************************/

APP_TIMER_DEF(m_led_timer_id);
static uint8_t led_array[] = LEDS_ARR;

static uint32_t timer_ms = 0;






/******************************************************************************/

/******************************************************************************
                                   STATIC FUNCTION					    			 
 ******************************************************************************/


/******************************************************************************
                                   GLOBAL FUNCTION					    			 
 ******************************************************************************/
extern void button_event_handler(uint32_t button_number);

static void LOG_Init()
{    
    __LOG_INIT(LOG_SRC_APP | LOG_SRC_ACCESS, LOG_LEVEL_INFO|LOG_LEVEL_ERROR|LOG_LEVEL_WARN, LOG_CALLBACK_DEFAULT);
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "----- BLE MESH HELLO WORLD -----\n");
    DEBUG_INFO("Hello World: %d\r\n", 12);

}

static void GPIO_Init()
{
    if (!nrfx_gpiote_is_init())
    {
        APP_ERROR_CHECK(nrfx_gpiote_init());
    }
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "NRF GPIOTE INITED\r\n");
    nrfx_gpiote_out_config_t out_config = 
    {
      .init_state = NRF_GPIOTE_INITIAL_VALUE_HIGH,
      .task_pin = false
    };
    for (uint32_t i = 0; i < sizeof(led_array); i++)
    {
        APP_ERROR_CHECK(nrfx_gpiote_out_init(led_array[i], &out_config));
    }
}

static void timer_timeout(void *arg)
{
   timer_ms++;
  //GPIO_Pin_Toggle(LED_PWR_R);
  //GPIO_Pin_Toggle(LED_PWR_G);
  //GPIO_Pin_Toggle(LED_GSM_R);
  //GPIO_Pin_Toggle(LED_GSM_G);
  ////GPIO_Pin_Toggle(GPIO_Pin_Toggle);
  //GPIO_Pin_Toggle(LED_ALARM_G);
  //GPIO_Pin_Toggle(LED_ALARM_R);
  //DEBUG_INFO("GPIO TOGGLE\r\n");
}




static void lfclk_request(void)
{
    //ret_code_t err_code = nrf_drv_clock_init();
    //APP_ERROR_CHECK(err_code);
    //nrf_drv_clock_lfclk_request(NULL);
}
static void Timer_Init()
{
  //lfclk_request();
  //app_timer_init();
  APP_ERROR_CHECK(app_timer_create(&m_led_timer_id, APP_TIMER_MODE_REPEATED, timer_timeout));
  APP_ERROR_CHECK(app_timer_start(m_led_timer_id, 1000, NULL));
  timer_ms = 0;
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "App Timer Start\n");
}

void InitSystem()
{
  LOG_Init();
  GPIO_Init();
  Timer_Init();
  user_button_init(button_event_handler);
}
/******************************************************************************
                                   GLOBAL FUNCTION					    			 
 ******************************************************************************/
 uint32_t sys_get_ms()
 {
   return timer_ms;
 }