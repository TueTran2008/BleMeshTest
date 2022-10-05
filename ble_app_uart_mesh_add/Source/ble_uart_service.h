#ifndef __BLE_UART_SERVICE_H_
#define __BLE_UART_SERVICE_H_



/**@brief Function for the BLE Service initialize to run parallel with MESH
 *
 */
void ble_uart_service_init();

void ble_service_advertising_start(void);

void ble_service_advertising_stop(void);

void scan_start(void);

void scan_stop(void);
#endif