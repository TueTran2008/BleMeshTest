
/*****************************************************************************
 * Includies 
 *****************************************************************************/
 /*nRF5 SDK Include*/
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


/*user Includies */
#include "user_input.h"
/* Ble Mesh v5.0 Includies */
#include "nrf_mesh.h"
#include "mesh_adv.h"
#include "ble_config.h"
#include "ble_softdevice_support.h"
#include "app_sef_provision.h"
#include "DataDefine.h"

#include "nrf_mesh_config_app.h"

#include "ble_uart_service.h"
#include "nrf_ble_scan.h"
#include "ble_advdata.h"
#include "app_mesh_check_duplicate.h"
#include "flash_if.h"
System_t xSystem;

static void node_nus_data_handler(ble_nus_evt_t *p_evt);

/*****************************************************************************
 * MACROS
 *****************************************************************************/
#define MESH_SOC_OBSERVER_PRIO 0

#define APP_BLE_CONN_CFG_TAG            1                                           /**< A tag identifying the SoftDevice BLE configuration. */

#define DEVICE_NAME                     "SFUL PAIR DE0650C4486E"                        /**< Name of device. Will be included in the advertising data. */
#define NUS_SERVICE_UUID_TYPE           BLE_UUID_TYPE_VENDOR_BEGIN                  /**< UUID type for the Nordic UART Service (vendor specific). */

#define APP_BLE_OBSERVER_PRIO           3                                           /**< Application's BLE observer priority. You shouldn't need to modify this value. */

#define APP_ADV_INTERVAL                800                                          /**< The advertising interval (in units of 0.625 ms. This value corresponds to 40 ms). */

#define APP_ADV_DURATION                18000                                       /**< The advertising duration (180 seconds) in units of 10 milliseconds. */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(150, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (20 ms), Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(250, UNIT_1_25_MS)             /**< Maximum acceptable connection interval (75 ms), Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                       /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                      /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define UART_TX_BUF_SIZE                256                                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                256                                         /**< UART RX buffer size. */

/*IMPLEMENT nRF5 Macros*/
BLE_NUS_DEF(m_nus, NRF_SDH_BLE_TOTAL_LINK_COUNT);                                   /**< BLE NUS service instance. */
NRF_BLE_GATT_DEF(m_gatt);                                                           /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);                                                             /**< Context for the Queued Write module.*/
BLE_ADVERTISING_DEF(m_advertising);                                                 /**< Advertising module instance. */


//BLE_NUS_C_DEF(m_ble_nus_c);                                             /**< BLE Nordic UART Service (NUS) client instance. */
//NRF_BLE_GATT_DEF(m_gatt);                                               /**< GATT module instance. */
//BLE_DB_DISCOVERY_DEF(m_db_disc);                                        /**< Database discovery module instance. */
NRF_BLE_SCAN_DEF(m_scan);                                               /**< Scanning Module instance. */
/*NRF_BLE_GQ_DEF(m_ble_gatt_queue,                                        /**< BLE GATT Queue instance. */
               //NRF_SDH_BLE_CENTRAL_LINK_COUNT,
               //NRF_BLE_GQ_QUEUE_SIZE);*/


/*****************************************************************************
 * Static variables
 *****************************************************************************/
static app_beacon_data_t  m_beacon_list[APP_MAX_BEACON] = {0};
static uint16_t m_beacon_write = 0;
static uint16_t m_beacon_read = 0;
static bool read_from_flash = false;


static uint8_t    m_adv_handle           = BLE_GAP_ADV_SET_HANDLE_NOT_SET;          /**< Handle of adversting work*/
static uint16_t   m_conn_handle          = BLE_CONN_HANDLE_INVALID;                 /**< Handle of the current connection. */
static uint16_t   m_ble_nus_max_data_len = BLE_GATT_ATT_MTU_DEFAULT - 3;            /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */
static ble_uuid_t m_adv_uuids[]          =                                          /**< Universally unique service identifier. */
{
    {BLE_UUID_NUS_SERVICE, NUS_SERVICE_UUID_TYPE}
};
static bool m_is_node_request_data = false;

static beacon_data_callback_t beacon_cb;

static uint8_t m_device_mac[6] = {0};
/*****************************************************************************
 * Forward Declaration
 *****************************************************************************/
void advertising_stop();
void app_beacon_queue_read(app_beacon_data_t *p_beacon_data);

void app_beacon_queue_write(app_beacon_data_t *p_beacon_data);
static uint8_t* app_filter_sensor_value(uint8_t *p_data, uint16_t length);
static inline bool app_check_valid_mac(uint8_t *mac);
static void app_handle_sensor_data(void *p_data);
static inline bool app_check_valid_token(uint16_t token);
static uint32_t mesh_pair_info_packet_create(uint8_t *p_out_buffer, uint32_t out_buffer_len, bool is_exchange_address, uint8_t msg_id);

/*****************************************************************************
 * Private function
 *****************************************************************************/
void app_ble_enter_pair_mode()
{
  xSystem.is_in_pair_mode = true;
  scan_stop();
  ble_service_advertising_start();
}
void app_ble_exit_pair_mode()
{
  xSystem.is_in_pair_mode = false;
  ble_service_advertising_stop();
  scan_start();
}


/**@brief Function for assert macro callback.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyse
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
    
}
/**@brief Function for initializing the timer module.
 */
static void timers_init(void)
{
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}
/**@brief Function for the GAP initialization.
 *
 * @details This function will set up all the necessary GAP (Generic Access Profile) parameters of
 *          the device. It also sets the permissions and appearance.
 */
static void gap_ble_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *) DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    __LOG(LOG_SRC_APP, LOG_LEVEL_DBG1, "Return code sd_ble_gap_device_name_set: 0x%02x\r\n", err_code);
    APP_ERROR_CHECK(err_code);
    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}
/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}
/**@brief Function for handling the data from the Nordic UART Service.
 *
 * @details This function will process the data received from the Nordic UART BLE Service and send
 *          it to the UART module.
 *
 * @param[in] p_evt       Nordic UART Service event.
 */
/**@snippet [Handling the data received over BLE] */
static void nus_data_handler(ble_nus_evt_t * p_evt)
{

    if (p_evt->type == BLE_NUS_EVT_RX_DATA)
    {
        uint32_t err_code;

        NRF_LOG_DEBUG("Received data from BLE NUS. Writing data on UART.");
        NRF_LOG_HEXDUMP_DEBUG(p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);

        for (uint32_t i = 0; i < p_evt->params.rx_data.length; i++)
        {
            do
            {
                err_code = app_uart_put(p_evt->params.rx_data.p_data[i]);
                if ((err_code != NRF_SUCCESS) && (err_code != NRF_ERROR_BUSY))
                {
                    NRF_LOG_ERROR("Failed receiving NUS message. Error 0x%x. ", err_code);
                    APP_ERROR_CHECK(err_code);
                }
            } while (err_code == NRF_ERROR_BUSY);
        }
        if (p_evt->params.rx_data.p_data[p_evt->params.rx_data.length - 1] == '\r')
        {
            while (app_uart_put('\n') == NRF_ERROR_BUSY);
        }
    }
}
/**@snippet [Handling the data received over BLE] */


/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    uint32_t           err_code;
    ble_nus_init_t     nus_init;
    nrf_ble_qwr_init_t qwr_init = {0};

    // Initialize Queued Write Module.
    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);

    // Initialize NUS.
    memset(&nus_init, 0, sizeof(nus_init));

    /*nus_init.data_handler = nus_data_handler;*/
    nus_init.data_handler = node_nus_data_handler;

    err_code = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);
}
/**@brief Function for handling an event from the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module
 *          which are passed to the application.
 *
 * @note All this function does is to disconnect. This could have been done by simply setting
 *       the disconnect_on_fail config parameter, but instead we use the event handler
 *       mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;
    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}
/**@brief Function for handling errors from the Connection Parameters module.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}
/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_ble_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;
    

    err_code = ble_conn_params_init(&cp_init);
    __LOG(LOG_SRC_APP, LOG_LEVEL_DBG1, "BLE CONN PARAMS INIT ErrorCode: 0x%02x\r\n", err_code);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
    uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break;
        case BLE_ADV_EVT_IDLE:
            sleep_mode_enter();
            break;
        default:
            break;
    }
}


/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    uint32_t err_code;
    ble_gap_evt_t const * p_gap_evt = &p_ble_evt->evt.gap_evt;
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            __LOG(LOG_SRC_APP, LOG_LEVEL_INFO,"BLE_Connected\r\n");
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            xSystem.is_in_pair_mode = true;
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            __LOG(LOG_SRC_APP, LOG_LEVEL_INFO,"BLE Disconnected\r\n");
            /**<Reset to state of adv beacuse of ble_adv_on_disconnect_disabled = true>*/
            xSystem.status.is_device_adv = false;
            xSystem.is_in_pair_mode = false;
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            // No system attributes have been stored.
            
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect 
            __LOG(LOG_SRC_APP, LOG_LEVEL_INFO,"BLE Disconnected on GATT Client timeout event.\r\n");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            __LOG(LOG_SRC_APP, LOG_LEVEL_INFO,"BLE Disconnected on GATT Server timeout event.\r\n");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;
        case BLE_GAP_EVT_ADV_REPORT:
        {
            //__LOG_XB(LOG_SRC_APP, LOG_LEVEL_INFO, "Data Packet", p_gap_evt->params.adv_report.data.p_data, p_gap_evt->params.adv_report.data.len);
            uint8_t *p_data = NULL;
            p_data = app_filter_sensor_value(p_gap_evt->params.adv_report.data.p_data, p_gap_evt->params.adv_report.data.len);
            if(p_data != NULL)
            {
              app_handle_sensor_data(p_data);
            }
        }
            break;
        default:
            // No implementation needed.
            break;
    }
}

//static void mesh_soc_evt_handler(uint32_t evt_id, void * p_context)
//{
//    nrf_mesh_on_sd_evt(evt_id);
//}
static void mesh_soc_evt_handler(uint32_t evt_id, void * p_context)
{
    nrf_mesh_on_sd_evt(evt_id);
}
NRF_SDH_SOC_OBSERVER(m_mesh_soc_observer, MESH_SOC_OBSERVER_PRIO, mesh_soc_evt_handler, NULL);


/**@brief Function for the SoftDevice initialization.
 *
 * @details This function initializes the SoftDevice and the BLE event interrupt.
 */
//static void 

void ble_uart_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

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
    if ((m_conn_handle == p_evt->conn_handle) && (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED))
    {
        m_ble_nus_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Data len is set to 0x%X(%d)", m_ble_nus_max_data_len, m_ble_nus_max_data_len);
    }
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "ATT MTU exchange completed. central 0x%x peripheral 0x%x",
                  p_gatt->att_mtu_desired_central,
                  p_gatt->att_mtu_desired_periph);
}


/**@brief Function for initializing the GATT library. */
void gatt_init(void)
{
    ret_code_t err_code;

    err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
    APP_ERROR_CHECK(err_code);
}

static void node_nus_data_handler(ble_nus_evt_t *p_evt)
{
  uint32_t err_code = NRF_SUCCESS;
  if(p_evt->type == BLE_NUS_EVT_RX_DATA)
  {
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO,"Received data from BLE NUS. Writing data on UART\r\n");
    __LOG_XB(LOG_SRC_APP, LOG_LEVEL_INFO,"Data buffer:", p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);
    if(p_evt->params.rx_data.p_data && p_evt->params.rx_data.length)
    {
        pair_info_request_key_t *p_request_key = (pair_info_request_key_t*)p_evt->params.rx_data.p_data;
        if(p_request_key->token == BEACON_DEFAULT_TOKEN && p_request_key->msg_id == PAIR_ID_DEVICE_REQUEST_INFO)
        {
          /*Save the current*/
          __LOG_XB(LOG_SRC_APP, LOG_LEVEL_INFO,"Beacon MAC", p_request_key->mac , 6);/**<6 is the size of mac address*/
          m_is_node_request_data = 1;
          NRF_LOG_INFO("Node Notification request\r\n");
          if(m_is_node_request_data)
          {
            uint8_t ret_message[sizeof(pair_info_t) + 1];
            uint16_t length = sizeof(ret_message);
            mesh_pair_info_packet_create(ret_message, length, false, PAIR_ID_GATEWAY_ACCEPT_PAIR_REQUEST);  
            err_code = ble_nus_data_send(&m_nus, (uint8_t*)&ret_message, &length, m_conn_handle);
            NRF_LOG_INFO("Send gateway info through BLE CCCD\r\n");
            if ((err_code != NRF_ERROR_INVALID_STATE) &&
                (err_code != NRF_ERROR_RESOURCES) &&
                (err_code != NRF_ERROR_NOT_FOUND))
            {
                NRF_LOG_INFO("Send nus data error: 0x%02x\r\n", err_code);
                APP_ERROR_CHECK(err_code);
            }
            m_is_node_request_data = 0;
          }
        }
        else
        {
          NRF_LOG_ERROR("Wrong token or ID\r\n");
          err_code = sd_ble_gap_disconnect(p_evt->conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
          APP_ERROR_CHECK(err_code);
          /*Config AUTO Stop Adverting if disconnected =>*/
          xSystem.status.is_device_adv = false;
          xSystem.is_in_pair_mode = 0;
          scan_start();
        }

    }
  }
  else if(p_evt->type == BLE_NUS_EVT_COMM_STARTED)
  {
      NRF_LOG_INFO("NUS Central has ENABLE Notification\r\n");
  }
  else if(p_evt->type == BLE_NUS_EVT_COMM_STOPPED)
  {
      NRF_LOG_WARNING("NUS Central has DISABLE Notification\r\n");
  }
}







/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    uint32_t               err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    init.advdata.name_type          = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance = false;
    init.advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;

    init.srdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.srdata.uuids_complete.p_uuids  = m_adv_uuids;

    init.config.ble_adv_fast_enabled  = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = APP_ADV_DURATION;
    init.config.ble_adv_on_disconnect_disabled = true;
    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}

/**@brief Function for initializing the nrf log module.
 */
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


/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
    else
    {
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Faild at NRF_LOG_PROCESS\r\n");
     }
}


/**@brief Function for starting advertising.
 */
void ble_service_advertising_start(void)
{
    if(xSystem.status.is_device_adv == false)
    {
      uint32_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
      xSystem.status.is_device_adv = true;
      APP_ERROR_CHECK(err_code);
    }
    else
    {
      __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Device is already advertising\r\n");
    }

}
/**@brief Function for stop advertising.
 */
 void ble_service_advertising_stop(void)
{
    if(xSystem.status.is_device_adv == true)
    {
      uint32_t err_code = sd_ble_gap_adv_stop(m_advertising.adv_handle);
      xSystem.status.is_device_adv = false;
      APP_ERROR_CHECK(err_code);
    }
    else
    {
      __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Already stop ADV\r\n");
    }
}


/** @brief  Set Default mesh key
 *
 * @param[out]   none 
 * @param[in]    none
 *
 * @retval        none
 */
static uint32_t app_mesh_set_default_key(uint8_t *app_key, uint8_t *net_key)
{
  if(app_key == NULL || net_key == NULL)
  {
    return NRF_ERROR_NULL;
  }
  uint8_t default_netkey[16] = {0x88, 0x9B, 0xC2, 0x0F, 0xEE, 0xCD, 0x9B, 0xA3, 0xBD, 0xD4, 0x82, 0x67, 0x0F, 0x35, 0x37, 0x05};
  uint8_t default_appkey[16] = {0x1F, 0x63, 0xD2, 0xE5, 0xBD, 0x58, 0x05, 0x14, 0x02, 0x29, 0x92, 0x8F, 0xCF, 0x30, 0xFE, 0xF8};
  memcpy(app_key, default_appkey, NRF_MESH_KEY_SIZE);
  memcpy(net_key, default_netkey, NRF_MESH_KEY_SIZE);
  return NRF_SUCCESS;
}
/** @brief  GET Current Chip Device MAC Address
 *
 * @param[out]   none 
 * @param[in]    none
 *
 * @retval        none
 */
static void ble_load_mac_address()
{
    m_device_mac[0] = ((NRF_FICR->DEVICEADDR1 & 0x0000FF00) >> 8) | 0b11000000;
    m_device_mac[1] = NRF_FICR->DEVICEADDR1 & 0x000000FF;
    m_device_mac[2] =  ((NRF_FICR->DEVICEADDR0 & 0xFF000000) >> 24);
    m_device_mac[3] = ((NRF_FICR->DEVICEADDR0 & 0x00FF0000) >> 16);
    m_device_mac[4] = ((NRF_FICR->DEVICEADDR0 & 0x0000FF00) >> 8);
    m_device_mac[5] = ((NRF_FICR->DEVICEADDR0 & 0x000000FF) >> 0);
    __LOG_XB(LOG_SRC_APP, LOG_LEVEL_INFO, "MAC ADDRESS", m_device_mac, sizeof(m_device_mac));
}
/** @brief
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
uint8_t *app_ble_get_mac()
{
    if(read_from_flash)
    {
      return (uint8_t*)&xSystem.flash_parameter.pair_information.pair_mac[0];
    }
    else
    {
      return (uint8_t*)&m_device_mac[0];
    }
}
/** @brief  Transfer default mesh key to mesh key base on MAC address(Refer to BYTECH Standard:D)
 *        
 * @param[out] none    
 * @param[in]   
 *
 * @retval   NRF_SUCESS if successful transfer  
 */
uint32_t app_mesh_transfer_to_mac_key(uint8_t* app_key, uint8_t *net_key, uint8_t *device_mac)
{
  if(app_key == NULL || net_key == NULL || device_mac ==NULL)
  {
    return NRF_ERROR_NULL;
  }
  memcpy(app_key, device_mac, 6);
  memcpy(net_key, device_mac, 6);
  return NRF_SUCCESS;
}
/** @brief
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
uint32_t app_mesh_set_unicast_address(dsm_local_unicast_address_t *unicast_address, uint16_t start_address, uint16_t count)
{
    if(unicast_address == NULL)
    {
      return NRF_ERROR_NULL;
    }
    unicast_address->address_start = start_address;
    unicast_address->count = count;
    return NRF_SUCCESS;
}
/** @brief    Prepare packet message return to slave node request
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
static uint32_t mesh_pair_info_packet_create(uint8_t *p_out_buffer, uint32_t out_buffer_len, bool is_exchange_address, uint8_t msg_id)
{
  /**<Return request message = MSG_ID + pair_info_t>*/
  uint8_t message_buffer[sizeof(pair_info_t) + 1] = {0};
  pair_info_t pair_info;
  if(p_out_buffer == NULL)
  {
    return NRF_ERROR_NULL;
  }
  if(out_buffer_len < sizeof(message_buffer))
  {
    return NRF_ERROR_NO_MEM;
  }
  memcpy(pair_info.pair_mac, app_ble_get_mac(), sizeof(pair_info.pair_mac));

  pair_info.next_unicast_addr = xSystem.flash_parameter.pair_information.next_unicast_addr;
  pair_info.exchange_pair_addr = xSystem.flash_parameter.pair_information.exchange_pair_addr;
  
  pair_info.topic_all = xSystem.flash_parameter.pair_information.topic_all;
  pair_info.topic_control = xSystem.flash_parameter.pair_information.topic_control;
  pair_info.topic_warning = xSystem.flash_parameter.pair_information.topic_warning;
  pair_info.topic_report = xSystem.flash_parameter.pair_information.topic_report;
  pair_info.topic_ctrol_ack = xSystem.flash_parameter.pair_information.topic_ctrol_ack;
  pair_info.topic_warning_ack = xSystem.flash_parameter.pair_information.topic_warning_ack;

  memcpy(pair_info.key.appkey, xSystem.flash_parameter.pair_information.key.appkey, NRF_MESH_KEY_SIZE);
  memcpy(pair_info.key.netkey, xSystem.flash_parameter.pair_information.key.netkey, NRF_MESH_KEY_SIZE);
  pair_info.state = PAIR_INFO_STATE_PROVISIONING;
  /*Fill message ID*/
  message_buffer[0] = msg_id;
  memcpy(&message_buffer[1], &pair_info, sizeof(pair_info));
  
  /*Copy data to out buffer*/
  memcpy(p_out_buffer, message_buffer, sizeof(message_buffer));
  /*Save save next unicast when provision other*/
  xSystem.flash_parameter.pair_information.next_unicast_addr = pair_info.next_unicast_addr + 
                                                              + xSystem.network_info.unicast_address.count;
  //xSystem.flash_parameter.pair_information.exchange_pair_addr = 
  xSystem.flash_parameter.pair_information.exchange_pair_addr =  pair_info.exchange_pair_addr + 2;
  NRF_LOG_INFO("Next provsion Address:%d", xSystem.flash_parameter.pair_information.exchange_pair_addr);
  xSystem.flash_parameter.total_provisioned_node++;
  app_flash_save_config_parameter();
  //xSystem.flash_parameter.node_provision_add.count = 
  return NRF_SUCCESS;
}
/** @brief    Save Next Node provision Information
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
static uint32_t mesh_pair_save_next_provision_node(dsm_local_unicast_address_t save_address)
{
  
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
 }


 /**/

void app_ble_load_config_paramter()
{
  if(/**/!app_flash_get_config_parameter())
  {

    NRF_LOG_INFO("Get Flash Config Paramter\r\n");
    memcpy(xSystem.network_info.app_key, xSystem.flash_parameter.pair_information.key.appkey, 16);
    memcpy(xSystem.network_info.net_key, xSystem.flash_parameter.pair_information.key.netkey, 16);
    read_from_flash = true;
  }
  else
  {
    /**/
    NRF_LOG_INFO("No Saved Data Config => Self Init Config Parameter\r\n");
    app_mesh_set_default_key(xSystem.network_info.app_key, xSystem.network_info.net_key);
    /*<load the mac address of current chip>*/
    ble_load_mac_address();
    /**<set Bytech standard key>*/
    app_mesh_transfer_to_mac_key(xSystem.network_info.app_key, xSystem.network_info.net_key, app_ble_get_mac());

    xSystem.flash_parameter.pair_information.topic_all = MESH_TOPIC_ALL;
    xSystem.flash_parameter.pair_information.topic_control = MESH_TOPIC_CONTROL;
    xSystem.flash_parameter.pair_information.topic_warning = MESH_TOPIC_WARNING;
    xSystem.flash_parameter.pair_information.topic_report = MESH_TOPIC_REPORT;
    xSystem.flash_parameter.pair_information.topic_ctrol_ack = MESH_TOPIC_CONTROL_ACK;
    xSystem.flash_parameter.pair_information.topic_warning_ack = MESH_TOPIC_WARNING_ACK;

    /*Copy Key*/
    memcpy(xSystem.flash_parameter.pair_information.key.appkey, xSystem.network_info.app_key, 16);
    memcpy(xSystem.flash_parameter.pair_information.key.netkey, xSystem.network_info.net_key, 16);
  
    /**/
    memcpy(&xSystem.flash_parameter.pair_information.pair_mac, app_ble_get_mac(), 6);
    /**<Set standard MAC address>*/
    mesh_network_info_t gateway_info;
    gateway_info.unicast_address.address_start = LOCAL_ADDRESS_START;
    gateway_info.unicast_address.count = ACCESS_ELEMENT_COUNT;
    /**<Copy Gateway Address>*/
    memcpy(&xSystem.network_info.unicast_address, &gateway_info.unicast_address, sizeof(gateway_info.unicast_address));
    xSystem.flash_parameter.pair_information.next_unicast_addr = xSystem.network_info.unicast_address.address_start;
    xSystem.flash_parameter.pair_information.exchange_pair_addr = xSystem.network_info.unicast_address.address_start + 2;


    xSystem.flash_parameter.config_parameter.SpeakerVolume = 50;
    xSystem.flash_parameter.config_parameter.AlarmConfig.Name.EnableSyncAlarm = 1;
    xSystem.flash_parameter.config_parameter.AlarmConfig.Name.EnableBuzzer = 1;
    app_flash_save_config_parameter();
    read_from_flash = false;
  }
}

void ble_uart_service_init()
{
    log_nrf_init();
    NRF_LOG_INFO("NRF LOG INIT\r\n");
    __LOG_INIT(LOG_SRC_APP | LOG_SRC_TRANSPORT | LOG_SRC_ACCESS, LOG_LEVEL_INFO | LOG_LEVEL_DBG3 | LOG_LEVEL_DBG2 | LOG_LEVEL_DBG1, LOG_CALLBACK_DEFAULT);
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "----- BLE UART GATEWAY SERVICE Initialize-----\n");
    /*<set default mesh key for static variables>*/
    

    /*nRF5 SDK Init*/
    //uart_init();

    timers_init();
    ble_uart_stack_init();
    gap_ble_params_init();
    gatt_init();
    services_init();
    advertising_init();
    conn_ble_params_init();
    scan_init();
    scan_start();
}


static bool ble_advdata_name_part_find(uint8_t const * p_encoded_data, uint16_t data_len, char const * p_target_name)
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

/** @brief Filter sensor belong to fire, temperature, or somekind base on manufacturer value
 
 * @param[in] p_data: Received Advertising Data
 * @param[in] length: Lenght of received Advertising Data
 *
 * @retval Pointer point to Beacon message
 */
static uint8_t* app_filter_sensor_value(uint8_t *p_data, uint16_t length)
{
  if(p_data == NULL || length == 0)
  {
    return NULL;
  }
  uint8_t *p_manufacture_data = NULL;
  uint16_t offset = 0;
  uint16_t len = 0;
  len = ble_advdata_search(p_data, length, &offset, BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA);
  /*Move the pointer to the Manufacture data really in*/
  if(len == 0 || offset == 0)
  {
    return NULL;
  }
  p_manufacture_data = p_data + offset;
  
  if(p_manufacture_data[0] != (APP_BEACON_COMPANY_ID >> 8) || 
     p_manufacture_data[1] != (APP_BEACON_COMPANY_ID & 0x00FF))
  {
    return NULL;
  }
  else
  {
    /*if get the correct company => return to the correct data type*/
    /*<2 is length of manufacture data >*/
    p_manufacture_data = p_manufacture_data + 2;
    //__LOG_XB(LOG_SRC_APP, LOG_LEVEL_INFO, "\r\n Manufacture Specific Data", p_manufacture_data, len - 2);
    return p_manufacture_data;
  }
}

/** @brief: Verify if beacon data belong to this network by checking mac address
 *
 * @param[in]: p_ma: Pointer to MAC adddress
 * @param[in] 
 *
 * @retval: true if same mac address
 */
static inline bool app_check_valid_mac(uint8_t *p_mac)
{
  if(memcmp(p_mac, app_ble_get_mac(), 6) == 0)
  {
    
    return true;
  }
  else
  {
    NRF_LOG_ERROR("Scanned Beacon Device doesn't belong to this network => reprovision beacon\r\n");
    return false;
  }
}

/** @brief: Verify if beacon data belong to this network by Token
 *
 * @param[in]: p_ma: Pointer to MAC adddress
 * @param[in] 
 *
 * @retval: true if same mac address
 */
static inline bool app_check_valid_token(uint16_t token)
{
  uint16_t reference_toke = APP_TOKEN;
  if(memcmp(&token, &reference_toke, sizeof(reference_toke)) == 0)
  {
    return true;
  }
  else
  {
    return false;
  }
}

/** @brief: Write sensor value to appropriate place
 *
 * @param[in]: p_ma: Pointer to beacon data type
 * @param[in] 
 *
 * @retval: true if same mac address
 */
 void app_beacon_queue_write(app_beacon_data_t *p_beacon_data)
 {

    //static app_beacon_data_t m_last_beacon_data;
    for(uint16_t index = 0; index <= m_beacon_write; index++)
    {
      /*Insert the same data at same position*/
       if(memcmp(&m_beacon_list[index].device_mac, p_beacon_data->device_mac, 6) == 0 )
          /*&& m_beacon_list[index].is_data_valid)*/
       {
          if(1/*m_beacon_list[index].beacon_tid.info.tid < p_beacon_data->beacon_tid.info.tid*/)
          { 
            /*New data :D*/
            
            memcpy(&m_beacon_list[index], p_beacon_data, sizeof(app_beacon_data_t));
            m_beacon_list[index].is_data_valid = 1; // need to read this data
            return;
          }
          else
          {
            return;
          }
       }
    }
    /*Insert Data if it's from new beacon :D*/
    m_beacon_list[m_beacon_write].is_data_valid = 1; // Data available to read;
    memcpy(&m_beacon_list[m_beacon_write], p_beacon_data, 6);
    if(m_beacon_write == APP_MAX_BEACON)
    {
      __LOG(LOG_SRC_APP, LOG_LEVEL_ERROR, "Beacon queue full\r\n");
    }
    m_beacon_write++;
    NRF_LOG_INFO("Insert new Beacon to the list: %d\r\n", m_beacon_write);
    xSystem.sensor_count = m_beacon_write;
 }
 /** @brief   Circle Read m_beacon_list to send data
 *
 * @param[in]
 * @param[in] 
 *
 * @retval 
 */
void app_beacon_queue_read(app_beacon_data_t *p_beacon_data)
{
   /*Gui du lieu cam bien moi di*/
   for(uint16_t index = m_beacon_read; index <= m_beacon_write; index++)
   {
      if(m_beacon_list[index].is_data_valid == 1)
      {
          __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Read index:%u\r\n", index);
          *p_beacon_data = m_beacon_list[index];
          /*Clear new data is available*/
           m_beacon_list[index].is_data_valid = 0;
          /*Handle the index in array => Carelessly Handle causes Memory leak:D*/
          if(index < m_beacon_write)
          {
            m_beacon_read = index + 1;
          }
          else if(index == m_beacon_write)
          {
            m_beacon_read = 0;
          }
          return;
      }
   }
   m_beacon_read = 0xFF;
   p_beacon_data = NULL;
   
}
/** @brief 
 *
 * @param[in]
 * @param[in] 
 *
 * @retval 
 */
static void app_handle_sensor_data(void *p_data)
{
   app_beacon_data_t this_beacon;
   app_beacon_t *p_beacon_msg = (app_beacon_t*)p_data;
   if(!app_check_valid_mac(p_beacon_msg->uuid.detail.gw_mac))
   {
      return;
   }
   if(!app_check_valid_token(p_beacon_msg->token))
   {  
      /**/
      return;
   }
   app_transaction_t tid_arrive;
   tid_arrive.tid = p_beacon_msg->uuid.detail.beacon_tid.info.tid;
   tid_arrive.unicast_addr = p_beacon_msg->uuid.detail.unicast_addr;

  /*Check xem TID có trùng không :D*/
  if (app_mesh_tid_is_duplicate(&tid_arrive))
  {
    static uint32_t count = 0;
    if (count++ == 10)
    {
      NRF_LOG_INFO("Duplicate beacon\r\n");
      count = 0;
    }
    return;
  }
	/*Filter Duplicate data*/
   app_mesh_insert_tid(&tid_arrive);
   /*alarm structure parser*/
   app_alarm_message_structure_t alarm_struct_sos = {0};
   alarm_struct_sos.battery_value = p_beacon_msg->battery;
   alarm_struct_sos.dev_type = p_beacon_msg->device_type;
   alarm_struct_sos.fw_version = p_beacon_msg->uuid.detail.beacon_tid.info.version;
   alarm_struct_sos.msg_type = p_beacon_msg->msg_type;
   alarm_struct_sos.reserve.raw = p_beacon_msg->uuid.detail.temperature.value;
  
   /*Gateway structure*/
   gw_mesh_msq_t msg_arrive;
   memset(&msg_arrive, 0, sizeof(gw_mesh_msq_t));

   msg_arrive.msg_id = GW_MESH_MSQ_ID_ALARM_SYSTEM;
   msg_arrive.mesh_id = p_beacon_msg->uuid.detail.unicast_addr;
   msg_arrive.len = sizeof(app_alarm_message_structure_t);

   memcpy(msg_arrive.mac, p_beacon_msg->uuid.detail.device_mac, 6);
   memcpy(msg_arrive.data, &alarm_struct_sos, sizeof(alarm_struct_sos));
 
   if(app_mesh_gw_queue_push(&msg_arrive))
   {
      NRF_LOG_ERROR("Push mesasage mesh tp queue failed");
      gw_mesh_msq_t rx_msg;
      app_mesh_gw_queue_pop(&rx_msg);
      app_mesh_gw_queue_push(&msg_arrive);
   }
   else
   {
      
   }
   if(p_beacon_msg->msg_type == APP_MSG_TYPE_NO_ALARM)
   {

   }
   else{

   }
}


void app_beacon_add_receive_data_cb(beacon_data_callback_t *p_cb)
{
}


void app_mesh_polling_handle()
{
  gw_mesh_msq_t rx_message = {0};
  if(app_mesh_gw_queue_pop(&rx_message))
  {
    NRF_LOG_ERROR("Cannot get message from queue\r\n");
  }
  else
  {
    NRF_LOG_INFO("");
  }
}


/*Fake data function to test */

#if(DEV_TEST)
static uint8_t fake_count = 0;
void fake_insert_new_data()
{
   app_beacon_data_t m_fake;
   static uint8_t mac_fake[6] = {11, 22, 33 , 44 ,55 ,66};
   m_fake.is_data_valid = 1;
   m_fake.beacon_tid.value = 0;
   //m_fake.device_type = 1;
   memcpy(m_fake.device_mac, mac_fake, 6);
   app_beacon_queue_write(&m_fake);
   mac_fake[5]++;
}
void fake_insert_duplicate_data()
{
   app_beacon_data_t m_fake;
   uint8_t mac_fake[6] = {11, 22, 33 , 44 ,55 ,66};
   m_fake.is_data_valid = 1;
   m_fake.beacon_tid.value = 0;
   //m_fake.device_type = 1;
   memcpy(m_fake.device_mac, mac_fake, 6);
   app_beacon_queue_write(&m_fake);
}
void fake_send_data()
{
  app_beacon_data_t m_fake;
  app_beacon_queue_read(&m_fake);
}



#endif