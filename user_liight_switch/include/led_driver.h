#ifndef _LED_DRIVER_J_
#define _LED_DRIVER_J_

#include "nrf_gpio.h"
#include "log.h"
#include "rtt_input.h"
#include "nrf_gpio.h"
#include "nrfx_gpiote.h"
#include "nrf_gpiote.h"
//#include "nrf_gpio.h"


#include "nrf.h"
#include "nrf_error.h"


/*LED DEFINE*/
#define LED_PWR_R   NRF_GPIO_PIN_MAP(0, 22)
#define LED_PWR_G   NRF_GPIO_PIN_MAP(0, 23)

#define LED_GSM_R   NRF_GPIO_PIN_MAP(0, 28)
#define LED_GSM_G   NRF_GPIO_PIN_MAP(0, 29)

#define LED_ALARM_R NRF_GPIO_PIN_MAP(0, 12)
#define LED_ALARM_G NRF_GPIO_PIN_MAP(0, 13)

#define GSM_PWR_KEY NRF_GPIO_PIN_MAP(0, 4)
#define GSM_STATUS  NRF_GPIO_PIN_MAP(0, 5)

#define BUTTON_ALARM NRF_GPIO_PIN_MAP(0, 26)
#define BUTTON_USER  NRF_GPIO_PIN_MAP(0, 16)
#define BUTTON_ARARM_INDEX 0
#define BUTTON_USER_INDEX  1


#define NUMBER_OF_LED 6

#define LEDS_ARR {LED_PWR_R, LED_PWR_G, LED_GSM_R, LED_GSM_G, LED_ALARM_R,LED_ALARM_G}
#define BTN_ARR { BUTTON_ALARM, BUTTON_USER}

#define GPIO_Pin_Toggle(pin)   nrfx_gpiote_out_toggle(pin)
#define GPIO_Pin_Set(pin)      nrfx_gpiote_out_set(pin)
#define GPIO_Pin_Reset(pin)    nrfx_gpiote_out_clear(pin)





typedef enum
{
  LED_PWR_RED   = 0,
  LED_PWR_GREEN,
  LED_GSM_RED,
  LED_GSM_GREEN,
  LED_ALARM_RED,
  LED_ALARM_GREEN
}led_id_t;


typedef struct __LED_DRIVER
{
 void(*Init)(void);
 void(*Blink)(led_id_t pin, uint32_t delay_ms, uint32_t blink_count);
 void(*Set)(led_id_t pin, bool value);
 void(*Toggle)(led_id_t pin);
}LED_DRVIER_T;

typedef struct
{
  /*struct 
  {
    uint8_t m_led_pwr_r : 1;
    uint8_t m_led_pwr_g : 1;
    uint8_t m_led_gsm_r : 1;
    uint8_t m_led_gsm_g : 1;
    uint8_t m_led_alarm_r : 1;
    uint8_t m_led_alarm_g : 1;
    uint8_t reserved : 2;
  }led_blink_allow;
  struct
  {
    uint32_t m_led_pwr_r;
    uint32_t m_led_pwr_g;
    uint32_t m_led_gsm_r;
    uint32_t m_led_gsm_g;
    uint32_t m_led_alarm_r;
    uint32_t m_led_alarm_g;
  }led_blink_time_delay;
  struct
  {
    uint8_t m_led_pwr_r;
    uint8_t m_led_pwr_g;
    uint8_t m_led_gsm_r;
    uint8_t m_led_gsm_g;
    uint8_t m_led_alarm_r;
    uint8_t m_led_alarm_g;
  }led_blink_count;*/
  uint32_t led_count;
  uint32_t led_timeout;
  uint8_t led_delay;
  uint8_t led_allow : 1;
  uint8_t led_state : 1;
}led_blink_t;

extern LED_DRVIER_T led_driver;


#endif