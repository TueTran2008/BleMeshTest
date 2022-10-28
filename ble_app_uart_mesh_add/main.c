
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
#include "app_wdt.h"
#include "led_driver.h"
#include "rtc.h"
#include "adc_button.h"
#include "app_btn.h"
#include "nrfx_ppi.h"
#include "led_driver.h"
extern void task_test_flash(void *p_args);

static void init_variables()
{
  //uint16_t sub_add = 0xC005;
  xSystem.status.is_device_adv = false;
  xSystem.led_driver = led_driver;
  mesh_network_info_t gateway_info;
  gateway_info.unicast_address.address_start = LOCAL_ADDRESS_START;
  gateway_info.unicast_address.count = ACCESS_ELEMENT_COUNT;
    /**<Copy Gateway Address>*/
  xSystem.status.init_done = false;
  memcpy(&xSystem.network_info.unicast_address, &gateway_info.unicast_address, sizeof(gateway_info.unicast_address));
}
extern void spi_main(void);
extern void spi_reset_buff(void);
/**@brief Application main function.
*/
int main(void)
{ 
    ret_code_t err_code;
    //err_code = nrf_drv_ppi_init();
    //APP_ERROR_CHECK(err_code);
    init_variables();
    //ble_load_mac_address();

    ble_uart_service_init();
    
    ble_mesh_stack_initialize();
     /*Start the Mesh Stack*/
    ble_mesh_start();
     /*Initialize Peripherals*/
    app_wdt_init();

    rtt_input_init(NULL);

    app_input_init();

    xSystem.led_driver.Init();

    RTC_Init();
    
    spi_main();
/*Comment when using dev kit to debug*/
#if(1)
    adc_init();
#endif
    /*<Supply 3V3 for smart module>*/
    xSystem.led_driver.Set(POWER_3V3_INDEX, false);
    /*<Supply on 3V8 for smart module>*/
    xSystem.led_driver.Set(POWER_3V8_INDEX, true);
    /**/
    for (;;)
    {
        /*Code no Blocking in background-loop*/
        NRF_LOG_FLUSH();
        (void)sd_app_evt_wait();
        spi_reset_buff();
        app_wdt_feed();
        /*Scanning int while 1 - Maybe Button Callback must create timer interrupt to handle */
        app_btn_scan(NULL);
    }
}
/**
 * @}
 */
