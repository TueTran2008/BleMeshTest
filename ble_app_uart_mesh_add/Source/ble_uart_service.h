#ifndef __BLE_UART_SERVICE_H_
#define __BLE_UART_SERVICE_H_



/**@brief Function for the BLE Service initialize to run parallel with MESH
 *
 */
#include "DataDefine.h"
#include "DataDefine.h"
#include "app_sef_provision.h"
void ble_uart_service_init();

void ble_service_advertising_start(void);

void ble_service_advertising_stop(void);

void scan_start(void);

void scan_stop(void);
/** @brief Make nRF52 Turn on UART service to pair beacon
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
void app_ble_enter_pair_mode();
/** @brief Make nRF52 Turn OFF UART
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
void app_ble_exit_pair_mode();

/** @brief  Get MAC address of device
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
uint8_t *app_ble_get_mac();


/** @brief  Load the global Variables
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
void app_ble_load_config_paramter();


//void app_beacon_queue_read(app_beacon_data_t *p_beacon_data);

//void app_beacon_queue_write(app_beacon_data_t *p_beacon_data);
#endif