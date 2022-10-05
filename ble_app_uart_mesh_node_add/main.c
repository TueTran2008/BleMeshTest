
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "nordic_common.h"
#include "app_error.h"
#include "app_uart.h"
#include "ble_db_discovery.h"
#include "app_timer.h"
#include "app_util.h"
#include "bsp_btn_ble.h"
#include "ble.h"
#include "ble_gap.h"
#include "ble_hci.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"
#include "ble_nus_c.h"
#include "nrf_ble_gatt.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_ble_scan.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "user_input.h"

/*user Include */
#include "user_input.h"
/* Mesh */
#include "nrf_mesh.h"
#include "mesh_adv.h"
#include "ble_config.h"
#include "ble_softdevice_support.h"
#include "app_sef_provision.h"
#include "DataDefine.h"
#include "app_sef_provision.h"

#include "ble_uart_service_central.h"



extern void spi_main(void);
int main(void)
{
    ble_uart_service_central_init();
    ble_mesh_stack_initialize();
    // Start execution.
    NRF_LOG_INFO("BLE UART central example started.\r\n");
    scan_start();
    ble_mesh_start();
    rtt_input_init(NULL);
    ///spi_main();
    for (;;)
    {
        (void)sd_app_evt_wait();
    }
}
