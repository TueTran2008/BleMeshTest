
#include <stdint.h>
#include <string.h>

/*********************



BLE MESH CONFIG AND INITIALIZE




**********************/

/* HAL */
#include "boards.h"
#include "simple_hal.h"
#include "app_timer.h"
/* Core */
#include "nrf_mesh_config_core.h"
#include "nrf_mesh_gatt.h"
#include "nrf_mesh_configure.h"
#include "nrf_mesh.h"
#include "mesh_stack.h"
#include "device_state_manager.h"
#include "access_config.h"
#include "nrf_mesh_events.h"

/* Provisioning and configuration */
#include "mesh_provisionee.h"
#include "mesh_app_utils.h"
#include "ble_softdevice_support.h"
/* Models */


/* Logging and RTT */
#include "log.h"
#include "rtt_input.h"
#include "led_driver.h"

/* Example specific includes */
#include "app_config.h"
#include "nrf_mesh_config_examples.h"
#include "light_switch_example_common.h"
#include "example_common.h"
#include "ble_softdevice_support.h"
#include "user_on_off_client.h"

#include "user_on_off_common.h"

#include "ble_config.h"
#include "ble_softdevice_support.h"
#include "app_sef_provision.h"
#include "DataDefine.h"

#include "rssi_server.h"
#include "access.h"

#include "nrf_ble_scan.h"
#include "nrf_log.h"

#include "app_mesh_gateway_msg.h"
#include "app_mesh_message_queue.h"
#include "user_on_off_common.h"

#include "scanner.h"
#include "nrf_mesh.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "app_mesh_gateway_msg.h"
#include "app_mesh_check_duplicate.h"
#include "app_mesh_process_data.h"
#include "ble_uart_service.h"

#define CLIENT   0
#define SERVER  1
#if(SERVER + CLIENT >= 2)
#error"Fail BLE CONFIG"
#endif

#define SDK_COEXIST 1

#define KEY_LENGHT 16
/*****************************************************************************
 * Global variables
 *****************************************************************************/
user_on_off_client_t m_client;

/*****************************************************************************
 * Forward declaration
 *****************************************************************************/
static void app_user_onoff_publish_interval_callback(access_model_handle_t model_handel, void *p_self);

static void app_user_onoff_client_transaction_status_callback(access_model_handle_t model_handle, void *p_args, access_reliable_status_t status);

static void app_user_onoff_client_status_cb(const user_on_off_client_t * p_self, const access_message_rx_meta_t * p_meta, const user_on_off_status_msg_pkt_t * p_in);

static void app_user_onoff_client_beacon_cb(const user_on_off_client_t * p_self, const access_message_rx_meta_t * p_meta, const void * p_in);

static void app_user_onoff_client_other_device_cb(const user_on_off_client_t * p_self, const access_message_rx_t * p_meta);

static void ble_mesh_keys_generate(void);
/*Mesh Core Callback after things*/
static void app_mesh_core_event_cb(const nrf_mesh_evt_t * p_evt);

static void app_polling_mesh_event();
/*****************************************************************************
 * Static variables
 *****************************************************************************/
static bool m_device_provisioned;
static uint8_t m_tx_tid = 0;
const static user_on_off_callbacks_t client_cbs =
{
    .onoff_status_cb = app_user_onoff_client_status_cb,
    .ack_transaction_status_cb = app_user_onoff_client_transaction_status_callback,
    .periodic_publish_cb = app_user_onoff_publish_interval_callback,
    .beacon_callback = app_user_onoff_client_beacon_cb,
    .other_device_callback = app_user_onoff_client_other_device_cb
};

static nrf_mesh_evt_handler_t m_mesh_core_event_handler = { .evt_cb = app_mesh_core_event_cb };

/*****************************************************************************
 * Static function
 *****************************************************************************/



static void app_user_onoff_publish_interval_callback(access_model_handle_t model_handel, void *p_self)
{
  static uint8_t tick_500ms = 0;
  if(tick_500ms++ >= 5)
  {
    tick_500ms = 0;
    app_polling_mesh_event();
  }
  app_queue_process_polling_message();
}





static void unicast_address_print(void)
{
   dsm_local_unicast_address_t node_address;
   dsm_local_unicast_addresses_get(&node_address);
   __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Node Address: 0x%04x \n", node_address.address_start);
}

static void start(void)
{

    mesh_app_uuid_print(nrf_mesh_configure_device_uuid_get());

    ERROR_CHECK(mesh_stack_start());

}

static void node_reset(void)
{
    mesh_stack_device_reset();
}
static void models_init_cb(void)
{
  m_client.setting.timeout = 0;
  m_client.setting.p_callbacks = &client_cbs;
  m_client.setting.force_segmented = false;
  m_client.setting.transmic_size = NRF_MESH_TRANSMIC_SIZE_SMALL;
  //__LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Initializing and adding models\n");
  user_on_off_client_init(&m_client, 1);
  xSystem.client_model = m_client.model_handle;
}


static void config_server_evt_cb(const config_server_evt_t * p_evt)
{
  if(p_evt->type == CONFIG_SERVER_EVT_NODE_RESET)
  {
    node_reset();
  }
}

extern uint32_t app_start_flash(app_flash_get_success_cb_t p_callback);
static void app_mesh_core_event_cb(const nrf_mesh_evt_t * p_evt)
{
    /* USER_NOTE: User can insert mesh core event processing here */
    switch (p_evt->type)
    {
        /* Start user application specific functions only when stack is enabled */
        case NRF_MESH_EVT_ENABLED:
            __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Mesh evt: NRF_MESH_EVT_ENABLED \n");
            {
                static bool s_app_started;
                if (!s_app_started)
                {
                    uint32_t status = 0;
                    //app_start_flash(NULL);
                    s_app_started = true;
                    status = access_model_publish_period_set(m_client.model_handle, ACCESS_PUBLISH_RESOLUTION_100MS, 1);
                    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Publish Period: %d\n", status);
                    app_ble_load_config_paramter();
                    app_self_provision(m_client.model_handle, xSystem.network_info.app_key, xSystem.network_info.net_key, xSystem.network_info.unicast_address);
                }
            }
            break;

        default:
            break;
    }
}

static void mesh_init(void)
{
    mesh_stack_init_params_t init_params =
    {
        /*Why lowest*/
        .core.irq_priority       = NRF_MESH_IRQ_PRIORITY_LOWEST,
        /*LFC CLK : RC source*/
        .core.lfclksrc           = DEV_BOARD_LF_CLK_CFG,
//#warning "What does this UUID do"
        /*Auto Generate UUID */
        .core.p_uuid             = NULL,
        /*Regist Model in this function*/
        .models.models_init_cb   = models_init_cb,
        /*
         * Pointer to a function used to inform about events from the configuration server.
         * Can be set to @c NULL if not used.
         *
         * @warning If the device receives a @ref CONFIG_SERVER_EVT_NODE_RESET event, it will erase all
         * mesh data from the persistent storage and reset the device after forwarding the event to the
         * application. The application developer should take care to be in a defined state after reset.
         * This is considered a "factory reset" of the mesh device.
         */
        .models.config_server_cb = config_server_evt_cb
    };

    uint32_t status = mesh_stack_init(&init_params, &m_device_provisioned);
    switch (status)
    {
        case NRF_ERROR_INVALID_DATA:
            NRF_LOG_INFO("Data in the persistent memory was corrupted. Device starts as unprovisioned.\n");
            NRF_LOG_INFO("Reboot device before starting of the provisioning process.\n");
            break;
        case NRF_SUCCESS:
            NRF_LOG_INFO("NRF MESH STACK Init Successfully\r\n");
            break;
        default:
            ERROR_CHECK(status);
    }
    if(m_device_provisioned == true)
    {
      NRF_LOG_INFO("Device was provisioned\r\n");
    }
    else
    {
      NRF_LOG_ERROR("Device wasn't provisioned\r\n");
    }

    /* Register event handler to receive NRF_MESH_EVT_FLASH_STABLE. Application functionality will
    be started after this event */
    nrf_mesh_evt_handler_add(&m_mesh_core_event_handler);
    NRF_LOG_FLUSH();

}

void ble_mesh_stack_initialize(void)
{
#if(!SDK_COEXIST)
  gap_params_init();

  conn_params_init();
 #endif
  mesh_init();
}

void ble_mesh_start()
{
  start();
}

static void gateway_switch_info()
{
  mesh_gateway_transfer_t gateway_network;
}

void app_mesh_reprovisioning()
{

}

/** @brief    Send Alarm Control Message to Lamp
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
uint32_t app_mesh_publish_ping_msg(uint16_t address)
{
  if(xSystem.is_in_pair_mode == true)
  {
    NRF_LOG_WARNING("Device is paring => No publish data\r\n");
    return NRF_ERROR_BUSY;
  }
  app_mesh_gw_set_structure_t mesh_gateway_tx;
  mesh_gateway_tx.dev_type = APP_DEVICE_GW;
  if(address == 0xFFFF)
    mesh_gateway_tx.msg_type = APP_MSG_TYPE_PING_ALL;
  else
    mesh_gateway_tx.msg_type = APP_MSG_TYPE_PING_NODE;
  mesh_gateway_tx.value.name.src_addr = address;

  app_mesh_tx_evt_t tx_msg = {0};
  tx_msg.id = APP_MESH_ID_PUBLISH;
  memcpy(&tx_msg.data[0], &xSystem.flash_parameter.pair_information.topic_all, 2);
  memcpy(&tx_msg.data[2], &mesh_gateway_tx, sizeof(app_mesh_gw_set_structure_t));
  tx_msg.len = sizeof(app_mesh_gw_set_structure_t) + 2;
  if(app_mesh_tx_evt_message_queue_push(&tx_msg))
  {
      NRF_LOG_ERROR("Queue is full when write ping\r\n");
      return NRF_ERROR_NO_MEM;
  }
  return NRF_SUCCESS;
}
/** @brief    Send Alarm Control Message to Lamp
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
uint32_t app_mesh_publish_set_lamp(uint16_t source, bool alarm_state)
{
  if(xSystem.is_in_pair_mode == true)
  {
    NRF_LOG_WARNING("Device is paring => No publish data\r\n");
    return NRF_ERROR_BUSY;
  }
  app_mesh_gw_set_structure_t mesh_gateway_tx;
  mesh_gateway_tx.dev_type = APP_DEVICE_GW;
  mesh_gateway_tx.msg_type = APP_MSG_TYPE_SET_ALARM_SPEAKER;
  mesh_gateway_tx.value.name.alarm = alarm_state;
  mesh_gateway_tx.value.name.src_addr = source;

  app_mesh_tx_evt_t tx_msg = {0};
  tx_msg.id = APP_MESH_ID_PUBLISH;
  memcpy(&tx_msg.data[0], &xSystem.flash_parameter.pair_information.topic_all, 2);
  /**/
  
  memcpy(&tx_msg.data[2], &mesh_gateway_tx, sizeof(app_mesh_gw_set_structure_t));
  tx_msg.len = sizeof(app_mesh_gw_set_structure_t) + 2;
  if(app_mesh_tx_evt_message_queue_push(&tx_msg))
  {
      NRF_LOG_ERROR("Queue is full when set lamp\r\n");
      return NRF_ERROR_NO_MEM;
  }
  return NRF_SUCCESS;
}

/** @brief    Send Alarm Control Message to Lamp
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
uint32_t app_mesh_publish_set_sos_ack()
{
  if(xSystem.is_in_pair_mode == true)
  {
    NRF_LOG_WARNING("Device is paring => No publish data\r\n");
    return NRF_ERROR_BUSY;
  }
  app_mesh_gw_set_structure_t mesh_gateway_tx;
  mesh_gateway_tx.dev_type = APP_DEVICE_GW;
  mesh_gateway_tx.msg_type = APP_MSG_TYPE_ACK;

  app_mesh_tx_evt_t tx_msg = {0};
  tx_msg.id = APP_MESH_ID_PUBLISH;
  memcpy(&tx_msg.data[0], &xSystem.flash_parameter.pair_information.topic_all, 2);
  memcpy(&tx_msg.data[2], &mesh_gateway_tx, sizeof(app_mesh_gw_set_structure_t));
  tx_msg.len = sizeof(app_mesh_gw_set_structure_t) + 2;
  if(app_mesh_tx_evt_message_queue_push(&tx_msg))
  {
      NRF_LOG_ERROR("Queue is full when set ack \r\n");
      return NRF_ERROR_NO_MEM;
  }
  return NRF_SUCCESS;
}

/** @brief    Polling send mesh message from mesh queue
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
static void app_polling_mesh_event()
{
  uint32_t status = 0;
  app_mesh_tx_evt_t mesh_tx_evt = {0};
  if(!app_mesh_tx_evt_message_queue_pop(&mesh_tx_evt))
  {
    NRF_LOG_INFO("Get Queue message from publish Tx mesh\r\n");
    if(mesh_tx_evt.id == APP_MESH_ID_PUBLISH)
    {
       /*Determine Publish Topic*/
       uint16_t publish_add = 0;
       dsm_handle_t dsm_handle = 0;
       memcpy(&publish_add, &mesh_tx_evt.data[0], sizeof(publish_add));
       /*Change publish to appropriate topic*/
       app_mesh_add_new_publish_topic(true, publish_add, xSystem.client_model);
       /*Create message data*/
       app_mesh_gw_set_structure_t *tx_msg = (app_mesh_gw_set_structure_t*)&mesh_tx_evt.data[2];

       user_on_off_msg_set_t tx_msg_params = {0};
       tx_msg_params.tid = m_tx_tid++;
       tx_msg_params.len = sizeof(app_mesh_gw_set_structure_t);
       tx_msg_params.data = (uint8_t*)tx_msg;

       model_transition_t tx_transtion;
       status = user_on_off_client_set_unack(&m_client, &tx_msg_params,&tx_transtion, APP_UNACK_MSG_REPEAT_COUNT);
       NRF_LOG_INFO("Periodic publish mesh message to 0x%04x - error code: %d", publish_add, status);
    }
    else if(mesh_tx_evt.id == APP_MESH_ID_DISABLE)
    {
      uint32_t ret = 0;
      NRF_LOG_WARNING("Disable mesh network\r\n");
      scanner_disable();
      ret = nrf_mesh_disable();
    }
  }
}





/** @brief    Handle Message from Opcode BEACON_OPCODE
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
static void app_user_onoff_client_beacon_cb(const user_on_off_client_t * p_self, 
                                            const access_message_rx_meta_t * p_meta,
                                            const void * p_in)
{
  mesh_beacon_parse_data_t *p_beacon_parse = (mesh_beacon_parse_data_t*)p_in;
  app_alarm_message_structure_t alarm_struct_sos;
  memset(&alarm_struct_sos, 0, sizeof(app_alarm_message_structure_t));
  NRF_LOG_INFO("New mesh beacon tid  Unicast address %d\r\n", MESH_APP_COMMON_INVALID_UNICAST_ADDR);

  app_transaction_t tid_arrive;
  tid_arrive.tid = p_beacon_parse->beacon_tid.info.tid;
  tid_arrive.unicast_addr = MESH_APP_COMMON_INVALID_UNICAST_ADDR; // beacon has no unicast address
  if (app_mesh_tid_is_duplicate(&tid_arrive))
  {
    NRF_LOG_INFO("Duplicate beacon\r\n");
    return;
  }
  app_mesh_insert_tid(&tid_arrive);  
  alarm_struct_sos.battery_value = p_beacon_parse->battery;
  alarm_struct_sos.fw_version = p_beacon_parse->beacon_tid.info.version; //hardcode fw version, dueto payload len limited
  alarm_struct_sos.dev_type = p_beacon_parse->device_type;
  alarm_struct_sos.msg_type = p_beacon_parse->msg_type;

  gw_mesh_msq_t msg_arrive;
  memset(&msg_arrive, 0, sizeof(gw_mesh_msq_t));
  memcpy(msg_arrive.mac, p_beacon_parse->mac, 6); // Mark as invalid
  msg_arrive.mesh_id = MESH_APP_COMMON_INVALID_UNICAST_ADDR;
  memcpy(msg_arrive.data, (uint8_t *) &alarm_struct_sos, sizeof(app_alarm_message_structure_t));
  msg_arrive.len = sizeof(app_alarm_message_structure_t);
  msg_arrive.msg_id = GW_MESH_MSQ_ID_ALARM_SYSTEM;
  if (app_mesh_gw_queue_push(&msg_arrive))
  {
    NRF_LOG_INFO("Push message mesh to queue fail\r\n");
    gw_mesh_msq_t msg;
    app_mesh_gw_queue_pop(&msg);
    app_mesh_gw_queue_push(&msg_arrive);
  }
  return;
}
/** @brief    Handle Message from Opcode OTHER_DEVICE_OPCODE
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
static void app_user_onoff_client_other_device_cb(const user_on_off_client_t * p_self,
                                                  const access_message_rx_t * p_meta)
{
  NRF_LOG_INFO("Other Device Callback Opcode - Rx Length:%u\r\n",  p_meta->length);

  const uint8_t *p_data = &p_meta->p_data[1];
  app_transaction_t tid_arrive;

  tid_arrive.tid = p_meta->p_data[0];
  tid_arrive.unicast_addr = p_meta->meta_data.src.value;
  if (app_mesh_tid_is_duplicate(&tid_arrive))
  {
    NRF_LOG_WARNING("Duplicate tid\r\n");
    return;
  }
  app_mesh_insert_tid(&tid_arrive);

  gw_mesh_msq_t rx_gw_msg;
  rx_gw_msg.len = p_meta->length - 1;
  memcpy(rx_gw_msg.data, p_data, rx_gw_msg.len);
  /*Get mac Address of sent node*/
  memcpy(rx_gw_msg.mac, p_meta->meta_data.p_core_metadata->params.scanner.adv_addr.addr, 6);
  rx_gw_msg.mesh_id = MESH_APP_COMMON_INVALID_UNICAST_ADDR;
  rx_gw_msg.msg_id = GW_MESH_MSG_ID_UNKNOWN_SYSTEM;

  if (app_mesh_gw_queue_push(&rx_gw_msg))
  {
    NRF_LOG_ERROR("Push message mesh to queue fail\r\n");
    gw_mesh_msq_t msg;
    app_mesh_gw_queue_pop(&msg);
    app_mesh_gw_queue_push(&rx_gw_msg);
  }
}
/** @brief    Handle Message from Opcode OTHER_DEVICE_OPCODE
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
static void app_user_onoff_client_transaction_status_callback(access_model_handle_t model_handle, void *p_args, access_reliable_status_t status)
{

}
/* IU OnOff client model interface: Process the received status message in this callback */
static void app_user_onoff_client_status_cb(const user_on_off_client_t * p_self,
                                               const access_message_rx_meta_t * p_meta,
                                               const user_on_off_status_msg_pkt_t * p_in)
{
  user_on_off_status_msg_pkt_t *p_status =  (user_on_off_status_msg_pkt_t*)p_in;
  app_transaction_t tid_arrive;
  tid_arrive.tid = p_status->tid;
  tid_arrive.unicast_addr = p_meta->src.value;
  app_alarm_message_structure_t m_alarm_sos;
  
  if (!app_mesh_tid_is_duplicate(&tid_arrive))
  {
    gw_mesh_msq_t rx_gw_msg;
    app_mesh_alarm_message_structure_t *alarm = (app_mesh_alarm_message_structure_t*)p_status->data;
    
    rx_gw_msg.len = sizeof(app_alarm_message_structure_t);
    memcpy(rx_gw_msg.mac, alarm->mac, 6);
    memcpy(rx_gw_msg.data, &alarm->alarm, sizeof(app_alarm_message_structure_t));
    rx_gw_msg.msg_id = GW_MESH_MSQ_ID_ALARM_SYSTEM;
    rx_gw_msg.mesh_id = p_meta->src.value;

    rx_gw_msg.len = p_in->len;
    if (app_mesh_gw_queue_push(&rx_gw_msg))
    {
      NRF_LOG_ERROR("Push message mesh to queue fail\r\n");
      gw_mesh_msq_t msg;
      app_mesh_gw_queue_pop(&msg);
      app_mesh_gw_queue_push(&rx_gw_msg);
    }
    else
    {
      NRF_LOG_INFO("Push message mesh to queue success\r\n");
    }
  }
  else
  {
    NRF_LOG_WARNING("Duplicate Data\r\n");
  }
}


/***********************************************************************/
/*
  MESH RESET STACK IMPLIMENTATION

*/
APP_TIMER_DEF(app_timer_leave_mesh_network);
static void do_reset()
{
    static bool device_reset = false;
#if MESH_FEATURE_GATT_PROXY_ENABLED
    (void) proxy_stop();
#endif
    if (device_reset == false)
    {
        device_reset = true;
        mesh_stack_config_clear();
        mesh_stack_device_reset();
    }
    else
    {
        hal_device_reset(0);
        while(1);
    }
}

void application_leave_mesh_network()
{
//    if (!mesh_stack_is_device_provisioned())
//        return;

    uint32_t error_code;

    error_code = app_timer_create(&app_timer_leave_mesh_network,
                                APP_TIMER_MODE_REPEATED,
                                do_reset);

    APP_ERROR_CHECK(error_code);

    error_code = app_timer_start(app_timer_leave_mesh_network, APP_TIMER_TICKS(1000), NULL);
    APP_ERROR_CHECK(error_code);
}
/*TUETD CLEAR*/
/*
 * @brief Clear mesh network configuration
 */
void mesh_core_clear(void *arg)
{
  NRF_LOG_INFO("----- Mesh reset  -----\n");
	/* This function may return if there are ongoing flash operations. */
	//    mesh_stack_device_reset();
  application_leave_mesh_network();
}

