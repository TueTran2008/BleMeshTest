#ifndef __BUTTON_ADC_H__
#define __BUTTON_ADC_H__

typedef enum
{
  BUTTON_IDLE = 0,      /*No pressing*/
  BUTTON_PRESSED = 1,
  BUTTON_HOLD = 3,
}custom_buttom_state_t;

typedef union
{
  uint8_t button_state[6];
}queue_button_t;
/**
 * @brief Initailize all adc
 */
void adc_init();
/**
 * @brief Monitoring state of all Button to send to the host
 * @return Buffer state of ADC Button
 */
bool app_button_get_state(queue_button_t *p_btn_state);

uint8_t app_adc_get_battery_percent();
#endif