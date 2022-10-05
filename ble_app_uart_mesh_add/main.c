
#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "app_timer.h"
#include "ble_nus.h"
#include "app_uart.h"
#include "app_util_platform.h"
#include "bsp_btn_ble.h"
#include "nrf_pwr_mgmt.h"

#if defined (UART_PRESENT)
#include "nrf_uart.h"
#endif
#if defined (UARTE_PRESENT)
#include "nrf_uarte.h"
#include "log.h"
#endif

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include <string.h>
#include "DataDefine.h"
#include "ble_config.h"
#include "ble_uart_service.h"
#include "user_input.h"
#include "app_sef_provision.h"



extern void task_test_flash(void *p_args);

static void init_variables()
{
  xSystem.status.is_device_adv = false;
}


extern void spi_main(void);
/**@brief Application main function.
 */
int main(void)
{
    ble_uart_service_init();
    ble_mesh_stack_initialize();
    ble_mesh_start();
    rtt_input_init(NULL);
    app_self_provision(m_client.model_handle, xSystem.network_info.app_key, xSystem.network_info.net_key, xSystem.network_info.unicast_address);
    spi_main();
    for (;;)
    // Enter main loop.
    {
        (void)sd_app_evt_wait();
    }
}
//DSM_APP_MAX


/**
 * @}
 */
