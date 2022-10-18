
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

extern void task_test_flash(void *p_args);

static void init_variables()
{
  //uint16_t sub_add = 0xC005;
  xSystem.status.is_device_adv = false;
  xSystem.led_driver = led_driver;
  /*Read From Flash Data :D*/
  //FlashParameter_t Read_from_flash;
  //xSystem.flash_parameter.config_parameter.AlarmConfig.Name.EnableSyncAlarm = 1;
  /*TODO Implement Read from Flash Saved Information*/
  //memcpy(&xSystem.flash_parameter.pair_information.topic_all, &sub_add, sizeof(sub_add));
  /**/
  mesh_network_info_t gateway_info;
  gateway_info.unicast_address.address_start = LOCAL_ADDRESS_START;
  gateway_info.unicast_address.count = ACCESS_ELEMENT_COUNT;
    /**<Copy Gateway Address>*/
  memcpy(&xSystem.network_info.unicast_address, &gateway_info.unicast_address, sizeof(gateway_info.unicast_address));
  
  NRF_LOG_INFO("Size of flash parameter: %d\r\n", sizeof(xSystem.flash_parameter));
}


extern void spi_main(void);
/**@brief Application main function.
 */
int main(void)
{ 
    init_variables();

    ble_uart_service_init();

    ble_mesh_stack_initialize();
    /*Include Mesh*/
    ble_mesh_start();

    app_wdt_init();

    rtt_input_init(NULL);
    /*Init Input Output Gpio*/
    //app_input_init();

    //xSystem.led_driver.Init();

    //RTC_Init();
    
    spi_main();
    for (;;)
    {
        NRF_LOG_FLUSH();
        (void)sd_app_evt_wait();
    }
}
/**
 * @}
 */
