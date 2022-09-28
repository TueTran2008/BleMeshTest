
/******************************************************************************
                                   INCLUDRIES				    			 
 ******************************************************************************/
#include "led_driver.h"
#include "nrf_gpiote.h"
#include "nrfx_gpiote.h"
#include "app_timer.h"

/******************************************************************************
                                   MACROS				    			 
 ******************************************************************************/

#define LED_ON    1
#define LED_OFF   0

#define TIMER_1MS 1000
/******************************************************************************
                                   PRIVATE VARIABLES				    			 
 ******************************************************************************/
static uint32_t led_ms_count;
static uint32_t led_ms_timeout;
//static led_blink_t m_led_blink;
static led_blink_t m_led[NUMBER_OF_LED];
uint8_t led_array[] = LEDS_ARR;

/******************************************************************************
                                   PRIVATES FUNCTIONS				    			 
 ******************************************************************************/
void led_timer_timeout(void *arg)
{
  for(uint8_t i = 0; i < NUMBER_OF_LED; i++)
  {
    if(m_led[i].led_delay == 0 || m_led[i].led_count == 0)
    {
      continue;
    }
    if(m_led[i].led_delay++ >= m_led[i].led_timeout)
    {
      if(m_led[i].led_count--)
      {
        GPIO_Pin_Toggle(led_array[i]);
      }
      else
      {
        m_led[i].led_delay = 0;
      }
    }
  }
}

/******************************************************************************
                                   PRIVATE VARIABLES				    			 
 ******************************************************************************/
APP_TIMER_DEF(m_led_timer_id);

/******************************************************************************
                                   GLOBAL FUNCTIONS				    			 
 ******************************************************************************/
static void led_init()
{
    /*Che*/
    
    if (!nrfx_gpiote_is_init())
    {
        APP_ERROR_CHECK(nrfx_gpiote_init());
    }
    nrfx_gpiote_out_config_t led_config = {
      .init_state = NRF_GPIOTE_INITIAL_VALUE_LOW,
      .task_pin = false
    };
    for (uint32_t i = 0; i < sizeof(led_array); i++)
    {
        APP_ERROR_CHECK(nrfx_gpiote_out_init(led_array[i], &led_config));
    }
    /*Init Timer for LED BLinking*/
    APP_ERROR_CHECK(app_timer_create(&m_led_timer_id, APP_TIMER_MODE_REPEATED, led_timer_timeout));
    APP_ERROR_CHECK(app_timer_start(m_led_timer_id, TIMER_1MS, NULL));
}

static void led_blink(led_id_t pin, uint32_t delay_ms, uint32_t blink_count)
{
  //m_led[1].led
  m_led[pin].led_allow = true;
  m_led[pin].led_allow = blink_count;
  m_led[pin].led_delay = delay_ms;
}
static void led_set(led_id_t pin, bool value)
{
  if(value)
  {
    GPIO_Pin_Set(led_array[pin]);
  }
  else
  {
    GPIO_Pin_Reset(led_array[pin]);
  }
}
static void led_toggle(led_id_t pin)
{
  GPIO_Pin_Toggle(led_array[pin]);
}


LED_DRVIER_T led_driver =
{
  .Init = led_init,
  .Blink = led_blink,
  .Set = led_set,
  .Toggle = led_toggle
};







