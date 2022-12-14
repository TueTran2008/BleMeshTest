
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
//#include "led_driver.h"
//#include "user_on_off_client.h"
/* Mesh */
#include "nrf_mesh.h"
#include "mesh_adv.h"
#include "ble_config.h"
#include "ble_softdevice_support.h"
#include "app_sef_provision.h"
#include "DataDefine.h"
#include "app_sef_provision.h"
#include "ble_uart_service_central.h"
#include "user_on_off_server.h"
#include "ble_advdata.h"

#define MESH_SOC_OBSERVER_PRIO 0

static bool m_is_device_privisioned = false;


#define APP_BLE_CONN_CFG_TAG    1                                       /**< Tag that refers to the BLE stack configuration set with @ref sd_ble_cfg_set. The default tag is @ref BLE_CONN_CFG_TAG_DEFAULT. */
#define APP_BLE_OBSERVER_PRIO   3                                       /**< BLE observer priority of the application. There is no need to modify this value. */

#define UART_TX_BUF_SIZE        256                                     /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE        256                                     /**< UART RX buffer size. */

#define NUS_SERVICE_UUID_TYPE   BLE_UUID_TYPE_VENDOR_BEGIN              /**< UUID type for the Nordic UART Service (vendor specific). */

#define ECHOBACK_BLE_UART_DATA  1                                       /**< Echo the UART data that is received over the Nordic UART Service (NUS) back to the sender. */


BLE_NUS_C_DEF(m_ble_nus_c);                                             /**< BLE Nordic UART Service (NUS) client instance. */
NRF_BLE_GATT_DEF(m_gatt);                                               /**< GATT module instance. */
BLE_DB_DISCOVERY_DEF(m_db_disc);                                        /**< Database discovery module instance. */
NRF_BLE_SCAN_DEF(m_scan);                                               /**< Scanning Module instance. */
NRF_BLE_GQ_DEF(m_ble_gatt_queue,                                        /**< BLE GATT Queue instance. */
               NRF_SDH_BLE_CENTRAL_LINK_COUNT,
               NRF_BLE_GQ_QUEUE_SIZE);

static uint16_t m_ble_nus_max_data_len = BLE_GATT_ATT_MTU_DEFAULT - OPCODE_LENGTH - HANDLE_LENGTH; /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */
/**@brief NUS UUID. */
static ble_uuid_t const m_nus_uuid =
{
    .uuid = BLE_UUID_NUS_SERVICE,
    .type = NUS_SERVICE_UUID_TYPE
};


/**@brief Function for handling asserts in the SoftDevice.
 *
 * @details This function is called in case of an assert in the SoftDevice.
 *
 * @warning This handler is only an example and is not meant for the final product. You need to analyze
 *          how your product is supposed to react in case of assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num     Line number of the failing assert call.
 * @param[in] p_file_name  File name of the failing assert call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(0xDEADBEEF, line_num, p_file_name);
}


/**@brief Function for handling the Nordic UART Service Client errors.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nus_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function to start scanning. */
void scan_start(void)
{
    ret_code_t ret;

    ret = nrf_ble_scan_start(&m_scan);
    APP_ERROR_CHECK(ret);
}

void scan_stop()
{
  nrf_ble_scan_stop();
}

extern adv_scan_data_t* nrf_ble_get_scan_plus_adv_buffer();
/**@brief Function for handling Scanning Module events.
 */
static void scan_evt_handler(scan_evt_t const * p_scan_evt)
{
    ret_code_t err_code;
    //scan_evt_debug = *p_scan_evt;
    
    switch(p_scan_evt->scan_evt_id)
    {
         case NRF_BLE_SCAN_EVT_CONNECTING_ERROR:
         {
              err_code = p_scan_evt->params.connecting_err.err_code;
              NRF_LOG_INFO("NRF_BLE_SCAN_EVT_CONNECTING_ERROR: 0x%02x\r\n", err_code);
              APP_ERROR_CHECK(err_code);
         } break;

         case NRF_BLE_SCAN_EVT_CONNECTED:
         {
              ble_gap_evt_connected_t const * p_connected =
                               p_scan_evt->params.connected.p_connected;
             // Scan is automatically stopped by the connection.
             __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Connecting to target %02x%02x%02x%02x%02x%02x",
                      p_connected->peer_addr.addr[0],
                      p_connected->peer_addr.addr[1],
                      p_connected->peer_addr.addr[2],
                      p_connected->peer_addr.addr[3],
                      p_connected->peer_addr.addr[4],
                      p_connected->peer_addr.addr[5]
                      );
         } break;

         case NRF_BLE_SCAN_EVT_SCAN_TIMEOUT:
         {
             __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Scan timed out.");
             scan_start();
         } break;
         //p_scan_evt->params.filter_match.p_adv_report
         
         case NRF_BLE_SCAN_EVT_FILTER_MATCH:
         {
            uint8_t *p_manufacture_data = NULL;
            adv_scan_data_t *p_adv_scan_data = nrf_ble_get_scan_plus_adv_buffer();
           

            //adv_debug = *(p_scan_evt->params.filter_match.p_adv_report);
            __LOG(LOG_SRC_APP, LOG_LEVEL_INFO,"Found Match Beacon\r\nRSSI%ddBm\r\nPeer Address:0x%08x\r\nData Direct Address:0x%08x\r\nData_ID:%d\r\n"
                                                                         , p_scan_evt->params.filter_match.p_adv_report->rssi
                                                                         , p_scan_evt->params.filter_match.p_adv_report->peer_addr
                                                                         , p_scan_evt->params.filter_match.p_adv_report->direct_addr
                                                                         , p_scan_evt->params.filter_match.p_adv_report->data_id
                                                                         );
             __LOG_XB(LOG_SRC_APP, LOG_LEVEL_INFO, "\r\nFilter Match Data Packet", p_adv_scan_data->data_buffer, p_adv_scan_data->index);
             /*Let's get beacon data :D*/
             uint16_t offset;
             uint16_t len;
             len = ble_advdata_search(p_adv_scan_data->data_buffer,  p_adv_scan_data->index, &offset,BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA);
             p_manufacture_data = &p_adv_scan_data->data_buffer[offset];
             __LOG_XB(LOG_SRC_APP, LOG_LEVEL_INFO, "\r\n Manufacture Specific Data", p_manufacture_data, len);
         }
         break;

         default:
             break;
    }
    //__LOG_XB(LOG_SRC_APP, LOG_LEVEL_INFO, "Scan data", p_scan_evt->params.p_whitelist_adv_report);
}


/**@brief Function for initializing the scanning and setting the filters.
 */
static void scan_init(void)
{
    ret_code_t          err_code;
    nrf_ble_scan_init_t init_scan;

    memset(&init_scan, 0, sizeof(init_scan));
    /*Already have scan request*/
    init_scan.connect_if_match = false;
    init_scan.conn_cfg_tag     = APP_BLE_CONN_CFG_TAG;
    //init_scan.p_scan_param->active = 0x01;
    

    err_code = nrf_ble_scan_init(&m_scan, &init_scan, scan_evt_handler);
    APP_ERROR_CHECK(err_code);
    uint8_t name_string[] = "TPMS4";
    err_code = nrf_ble_scan_filter_set(&m_scan, SCAN_NAME_FILTER, name_string);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_scan_filters_enable(&m_scan, NRF_BLE_SCAN_NAME_FILTER, false);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling database discovery events.
 *
 * @details This function is a callback function to handle events from the database discovery module.
 *          Depending on the UUIDs that are discovered, this function forwards the events
 *          to their respective services.
 *
 * @param[in] p_event  Pointer to the database discovery event.
 */
static void db_disc_handler(ble_db_discovery_evt_t * p_evt)
{
    ble_nus_c_on_db_disc_evt(&m_ble_nus_c, p_evt);
}





/**@brief   Function for handling app_uart events.
 *
 * @details This function receives a single character from the app_uart module and appends it to
 *          a string. The string is sent over BLE when the last character received is a
 *          'new line' '\n' (hex 0x0A) or if the string reaches the maximum data length.
 */
void uart_event_handle(app_uart_evt_t * p_event)
{
}

/*Things do after connect*/
void event_connected()
{
}
/*
  @brief: Check the packet send from Gateway is valid. If packet is valid => Ready to Self Provisioning
          Request gateway info again if fail
*/
static mesh_network_info_t network_info;

static void gateway_info_check_valid_packet(uint8_t *p_data, uint16_t lenght)
{
   uint32_t err_code = NRF_SUCCESS;
   if(lenght == sizeof(mesh_network_info_t) && p_data != NULL)
   {
      
      memcpy((void*)&network_info, (void*)p_data, sizeof(network_info));


      //__LOG(LOG_SRC_APP, LOG_LEVEL_INFO,"Receiving data.");
      __LOG_XB(LOG_SRC_APP, LOG_LEVEL_INFO,"NETWORK_KEY:\r\n",network_info.net_key, 16);
      __LOG_XB(LOG_SRC_APP, LOG_LEVEL_INFO,"APPLICATION_KEY:\r\n",network_info.app_key, 16);
      __LOG(LOG_SRC_APP, LOG_LEVEL_INFO,"Unicast address:0x%04x\r\n", network_info.unicast_address.address_start);
      app_self_provision(m_server.model_handle, network_info.app_key, network_info.net_key, network_info.unicast_address);
      
      scan_stop();
      m_is_device_privisioned = true;

      err_code = ble_nus_c_string_send(&m_ble_nus_c, "DIS", strlen("DIS"));
      if ( (err_code != NRF_ERROR_INVALID_STATE) && (err_code != NRF_ERROR_RESOURCES) )
      {
        APP_ERROR_CHECK(err_code);
      }
      err_code = sd_ble_gap_disconnect(m_ble_nus_c.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
      if (err_code != NRF_ERROR_INVALID_STATE)
      {
        APP_ERROR_CHECK(err_code);
      }
   }
   else
   {
      __LOG_XB(LOG_SRC_APP, LOG_LEVEL_INFO, "Mesh network info parse fail\r\n",p_data, lenght);
      /*Request again if fail to receive data*/
      err_code = ble_nus_c_string_send(&m_ble_nus_c, "REQ", strlen("REQ")); 
      if ( (err_code != NRF_ERROR_INVALID_STATE) && (err_code != NRF_ERROR_RESOURCES) )
      {
        APP_ERROR_CHECK(err_code);
      }
   }
}

/**@brief Callback handling Nordic UART Service (NUS) client events.
 *
 * @details This function is called to notify the application of NUS client events.
 *
 * @param[in]   p_ble_nus_c   NUS client handle. This identifies the NUS client.
 * @param[in]   p_ble_nus_evt Pointer to the NUS client event.
 */

/**@snippet [Handling events from the ble_nus_c module] */
static void ble_nus_c_evt_handler(ble_nus_c_t * p_ble_nus_c, ble_nus_c_evt_t const * p_ble_nus_evt)
{
    ret_code_t err_code;

    switch (p_ble_nus_evt->evt_type)
    {
        case BLE_NUS_C_EVT_DISCOVERY_COMPLETE:
            __LOG(LOG_SRC_APP, LOG_LEVEL_INFO,"Discovery complete\r\n");
            
            err_code = ble_nus_c_handles_assign(p_ble_nus_c, p_ble_nus_evt->conn_handle, &p_ble_nus_evt->handles);
            APP_ERROR_CHECK(err_code);
            err_code = ble_nus_c_string_send(&m_ble_nus_c, "REQ", strlen("REQ"));
            if ((err_code != NRF_SUCCESS) && (err_code != NRF_ERROR_BUSY))
            {
                __LOG(LOG_SRC_APP, LOG_LEVEL_INFO,"Failed sending NUS message. Error 0x%x. ", err_code);
                APP_ERROR_CHECK(err_code);
            }
            err_code = ble_nus_c_tx_notif_enable(p_ble_nus_c);
            APP_ERROR_CHECK(err_code);
            __LOG(LOG_SRC_APP, LOG_LEVEL_INFO,"Connected to device with Nordic UART Service.");
            break;

        case BLE_NUS_C_EVT_NUS_TX_EVT:
            //ble_nus_chars_received_uart_print(p_ble_nus_evt->p_data, p_ble_nus_evt->data_len);
            gateway_info_check_valid_packet(p_ble_nus_evt->p_data, p_ble_nus_evt->data_len);

            break;

        case BLE_NUS_C_EVT_DISCONNECTED:
            __LOG(LOG_SRC_APP, LOG_LEVEL_INFO,"Disconnected.");
            if(m_is_device_privisioned == false)
            {
              scan_start();
            }
            else
            {
              scan_stop();
            }
            break;
    }
}
/**@snippet [Handling events from the ble_nus_c module] */


/**
 * @brief Function for handling shutdown events.
 *
 * @param[in]   event       Shutdown type.
 */
static bool shutdown_handler(nrf_pwr_mgmt_evt_t event)
{
    ret_code_t err_code;

    err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    switch (event)
    {
        case NRF_PWR_MGMT_EVT_PREPARE_WAKEUP:
            // Prepare wakeup buttons.
            err_code = bsp_btn_ble_sleep_mode_prepare();
            APP_ERROR_CHECK(err_code);
            break;

        default:
            break;
    }

    return true;
}

//NRF_PWR_MGMT_HANDLER_REGISTER(shutdown_handler, APP_SHUTDOWN_HANDLER_PRIORITY);


/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    ret_code_t            err_code;
    ble_gap_evt_t const * p_gap_evt = &p_ble_evt->evt.gap_evt;
    /**/
    //nrf_ble_scan_on_ble_evt(p_ble_evt, &m_scan);

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            err_code = ble_nus_c_handles_assign(&m_ble_nus_c, p_ble_evt->evt.gap_evt.conn_handle, NULL);
            APP_ERROR_CHECK(err_code);
            __LOG(LOG_SRC_APP, LOG_LEVEL_INFO,"BLE Gap Event Connected\r\n");
            // start discovery of services. The NUS Client waits for a discovery result
            err_code = ble_db_discovery_start(&m_db_disc, p_ble_evt->evt.gap_evt.conn_handle);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_DISCONNECTED:

            __LOG(LOG_SRC_APP, LOG_LEVEL_INFO,"Disconnected. conn_handle: 0x%x, reason: 0x%x",
                         p_gap_evt->conn_handle,
                         p_gap_evt->params.disconnected.reason);
            break;

        case BLE_GAP_EVT_TIMEOUT:
            if (p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_CONN)
            {
                NRF_LOG_INFO("Connection Request timed out.");
            }
            break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported.
            err_code = sd_ble_gap_sec_params_reply(p_ble_evt->evt.gap_evt.conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
            // Accepting parameters requested by peer.
            err_code = sd_ble_gap_conn_param_update(p_gap_evt->conn_handle,
                                                    &p_gap_evt->params.conn_param_update_request.conn_params);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            __LOG(LOG_SRC_APP, LOG_LEVEL_INFO,"PHY update request.");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            NRF_LOG_DEBUG("GATT Client Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            __LOG(LOG_SRC_APP, LOG_LEVEL_INFO,"GATT Server Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;
        case BLE_GAP_EVT_ADV_REPORT:

            //adv_debug = (p_gap_evt->params.adv_report);
           // adv_packet = (p_gap_evt->params.adv_report);
            //__LOG(LOG_SRC_APP, LOG_LEVEL_INFO,"Receive beacon Packet:\r\nRSSI%ddBm\r\nPeer Address:0x%08x\r\nData Direct Address:0x%08x\r\nData_ID:%d"
                                                                         //, p_gap_evt->params.adv_report.rssi
                                                                         //, p_gap_evt->params.adv_report.peer_addr
                                                                         //, p_gap_evt->params.adv_report.direct_addr
                                                                         //, p_gap_evt->params.adv_report.data_id
                                                                         //);
            __LOG_XB(LOG_SRC_APP, LOG_LEVEL_INFO, "Data Packet", p_gap_evt->params.adv_report.data.p_data, p_gap_evt->params.adv_report.data.len);
            break;

        default:
            break;
    }
}

/**@brief Function for hand Mesh Events
 *
 */
static void mesh_soc_evt_handler(uint32_t evt_id, void * p_context)
{
    nrf_mesh_on_sd_evt(evt_id);
}
NRF_SDH_SOC_OBSERVER(m_mesh_soc_observer, MESH_SOC_OBSERVER_PRIO, mesh_soc_evt_handler, NULL);

/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_uart_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}


/**@brief Function for handling events from the GATT library. */
void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt)
{
    if (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED)
    {
        NRF_LOG_INFO("ATT MTU exchange completed.");

        m_ble_nus_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
        NRF_LOG_INFO("Ble NUS max data length set to 0x%X(%d)", m_ble_nus_max_data_len, m_ble_nus_max_data_len);
    }
}


/**@brief Function for initializing the GATT library. */
void gatt_init(void)
{
    ret_code_t err_code;

    err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_gatt_att_mtu_central_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the UART. */
static void uart_init(void)
{
    ret_code_t err_code;

    app_uart_comm_params_t const comm_params =
    {
        .rx_pin_no    = RX_PIN_NUMBER,
        .tx_pin_no    = TX_PIN_NUMBER,
        .rts_pin_no   = RTS_PIN_NUMBER,
        .cts_pin_no   = CTS_PIN_NUMBER,
        .flow_control = APP_UART_FLOW_CONTROL_DISABLED,
        .use_parity   = false,
        .baud_rate    = UART_BAUDRATE_BAUDRATE_Baud115200
    };

    APP_UART_FIFO_INIT(&comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       uart_event_handle,
                       APP_IRQ_PRIORITY_LOWEST,
                       err_code);

    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the Nordic UART Service (NUS) client. */
static void nus_c_init(void)
{
    ret_code_t       err_code;
    ble_nus_c_init_t init;

    init.evt_handler   = ble_nus_c_evt_handler;
    init.error_handler = nus_error_handler;
    init.p_gatt_queue  = &m_ble_gatt_queue;

    err_code = ble_nus_c_init(&m_ble_nus_c, &init);
    APP_ERROR_CHECK(err_code);
}
/**@brief Function for initializing the timer. */
static void timer_nrf_init(void)
{
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}
/**@brief Function for initializing the nrf log module. */
static void log_nrf_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}


/** @brief Function for initializing the database discovery module. */
static void db_discovery_init(void)
{
    ble_db_discovery_init_t db_init;

    memset(&db_init, 0, sizeof(ble_db_discovery_init_t));

    db_init.evt_handler  = db_disc_handler;
    db_init.p_gatt_queue = &m_ble_gatt_queue;

    ret_code_t err_code = ble_db_discovery_init(&db_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the idle state (main loop).
 *
 * @details Handles any pending log operations, then sleeps until the next event occurs.
 */
static void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
}


void ble_uart_service_central_init()
{
    __LOG_INIT(LOG_SRC_APP | LOG_SRC_TRANSPORT, LOG_LEVEL_INFO | LOG_LEVEL_DBG1 | LOG_LEVEL_DBG3, LOG_CALLBACK_DEFAULT);
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "----- TUE MESH UART CENTRAL COMPOSITE-----\n");
    //__LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "----- TUE MESH UART CENTRAL COMPOSITE-----\n");
    timer_nrf_init();
    uart_init();
    db_discovery_init();
    ble_uart_stack_init();
    gatt_init();
    nus_c_init();
    scan_init();
}




static bool ble_advdata_name_part_find(uint8_t const * p_encoded_data,
                           uint16_t        data_len,
                           char    const * p_target_name)
{
    uint16_t        parsed_name_len;
    uint8_t const * p_parsed_name;
    uint16_t        data_offset          = 0;

    if (p_target_name == NULL)
    {
        return false;
    }


    parsed_name_len = ble_advdata_search(p_encoded_data,
                                         data_len,
                                         &data_offset,
                                         BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME);

    p_parsed_name = &p_encoded_data[data_offset];
    //__LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Parse name:%s\r\n", p_parsed_name);

    if (   (data_offset != 0)
        && (parsed_name_len != 0)
        /*&& (strlen(p_target_name) == parsed_name_len)*/
        /*&& (memcmp(p_target_name, p_parsed_name, parsed_name_len) == 0)*/
        && (memcmp(p_target_name, p_parsed_name, strlen(p_target_name)) == 0))
    {
        __LOG_XB(LOG_SRC_APP, LOG_LEVEL_INFO, "Complete Name", p_parsed_name, parsed_name_len);
        return true;
    }

    return false;
}

/** @brief Function for comparing the provided name with the advertised name.
 *
 * @param[in] p_adv_rp    Advertising data to parse.
 * @param[in] p_scan_ctx      Pointer to the Scanning Module instance.
 *
 * @retval True when the names match. False otherwise.
 */

bool advdata_part_name_find(ble_gap_evt_adv_report_t const * p_adv_rp,
                                   nrf_ble_scan_t     const * const p_scan_ctx)
{
    nrf_ble_scan_name_filter_t const * p_name_filter = &p_scan_ctx->scan_filters.name_filter;
    uint8_t                            counter       =
        p_scan_ctx->scan_filters.name_filter.name_cnt;
    uint8_t  index;
    uint16_t data_len;

    data_len = p_adv_rp->data.len;

    // Compare the name found with the name filter.
    for (index = 0; index < counter; index++)
    {
        if (ble_advdata_name_part_find(p_adv_rp->data.p_data,
                                  data_len,
                                  p_name_filter->target_name[index]))
        {
            return true;
        }
    }

    return false;
}