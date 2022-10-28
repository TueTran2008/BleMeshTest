#ifndef PA_LNA_H
#define PA_LNA_H

#include <stdint.h>

/*****************************************************************************/
/***
 * @brief	: Config gpio pin PA/LNA for mesh and BLE
 * @param	: gpio_pa_pin, gpio_lna_pin
 * @retval	:
 * @author	: Phinht
 * @created	: 05/09/2015
 * @version	:
 * @reviewer:	
 */
void pa_lna_gpio_config(uint8_t gpio_pa_pin, uint8_t gpio_lna_pin);

/*
  @brief: Call this after the mesh_init()
*/
void app_lna_pa_enable_module(void);

/*
  @brief: Call this before the mesh_init()
*/
void app_lna_pa_enable_in_ble_stack(void);

#endif /* PA_LNA_H */
