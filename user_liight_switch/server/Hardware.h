/*Board LED*/
#ifndef __HARDWARE_H__
#define __HARDWARE_H__
#include "nrf_gpio.h"
#include "log.h"
#include "rtt_input.h"
#include "nrf_gpio.h"
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
#define BUTTON_USER_INDEX   1




#define LEDS_ARR {LED_PWR_R, LED_PWR_G, LED_GSM_R, LED_GSM_G, LED_ALARM_R,LED_ALARM_G}
#define BTN_ARR { BUTTON_ALARM, BUTTON_USER}

#define GPIO_Pin_Toggle(pin)   nrfx_gpiote_out_toggle(pin)
#define GPIO_Pin_Set(pin)      nrfx_gpiote_out_set(pin)
#define GPIO_Pin_Reset(pin)    nrfx_gpiote_out_clear(pin)


#define DEBUG_INFO(s, args...)      __LOG(LOG_SRC_APP, LOG_LEVEL_INFO,s, ##args);


#endif