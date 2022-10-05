
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
static void ble_mesh_keys_generate(void);
/*Mesh Core Callback after things*/
static void app_mesh_core_event_cb(const nrf_mesh_evt_t * p_evt);
/*****************************************************************************
 * Static variables
 *****************************************************************************/
static bool m_device_provisioned;
const static user_on_off_callbacks_t client_cbs =
{
    .onoff_status_cb = app_user_onoff_client_status_cb,
    .ack_transaction_status_cb = app_user_onoff_client_transaction_status_callback,
    .periodic_publish_cb = app_user_onoff_publish_interval_callback
};

static nrf_mesh_evt_handler_t m_mesh_core_event_handler = { .evt_cb = app_mesh_core_event_cb };

/*****************************************************************************
 * Static function
 *****************************************************************************/

static void ble_mesh_keys_generate()
{
}

static void app_user_onoff_publish_interval_callback(access_model_handle_t model_handel, void *p_self)
{
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Client Periodic Interval Callback\r\n");
  uint32_t status = NRF_SUCCESS;
  user_on_off_msg_set_t set_packet;
  //tid = 0;
  model_transition_t transition_params;
  set_packet.on_off = 1;
  set_packet.pwm_period = 20;
  set_packet.tid = 0;
  status = user_on_off_client_set(&m_client, &set_packet, &transition_params);
}

static void app_user_onoff_client_transaction_status_callback(access_model_handle_t model_handle, void *p_args, access_reliable_status_t status)
{
  switch(status)
  {
        case ACCESS_RELIABLE_TRANSFER_SUCCESS:
            __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Acknowledged transfer success.\n");
            break;

        case ACCESS_RELIABLE_TRANSFER_TIMEOUT:
            __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Acknowledged transfer timeout.\n");
            break;

        case ACCESS_RELIABLE_TRANSFER_CANCELLED:
            __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Acknowledged transfer cancelled.\n");
            break;

        default:
            __LOG(LOG_SRC_APP, LOG_LEVEL_ERROR, "Error Internal Failed.\n");
            ERROR_CHECK(NRF_ERROR_INTERNAL);
            break;
    
  }
}



/* IU OnOff client model interface: Process the received status message in this callback */
static void app_user_onoff_client_status_cb(const user_on_off_client_t * p_self,
                                               const access_message_rx_meta_t * p_meta,
                                               const user_on_off_status_msg_pkt_t * p_in)
{
    //user_on_off_status_msg_pkt_t *data = (user_on_off_status_msg_pkt_t *)p_in; 
    if (p_in->remaining_time > 0)
    {

    }
    else
    {

    }
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
                    //app_start_flash(NULL);
                    s_app_started = true;
                    /*
                    if(m_device_provisioned)
                    {
                       unicast_address_print();
                    }
                    else
                    {
                       mesh_network_info_t mesh_gateway = {0};
                       app_generate_random_keys(mesh_gateway.app_key, mesh_gateway.net_key);
                       mesh_gateway.unicast_address.address_start = LOCAL_ADDRESS_START;
                       mesh_gateway.unicast_address.count = ACCESS_ELEMENT_COUNT;
                       app_self_provision(m_client.model_handle, app_key, net_key, m_start_unicast_add);
                    }
                    uint32_t status = 0;
                    status = access_model_publish_period_set(m_client.model_handle, ACCESS_PUBLISH_RESOLUTION_10S, 1);
                    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Publish Period Settingg: %d\n", status);
                    */
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
            __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Data in the persistent memory was corrupted. Device starts as unprovisioned.\n");
            __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Reboot device before starting of the provisioning process.\n");
            break;
        case NRF_SUCCESS:
            __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "NRF MESH STACK Init Successfully\r\n");
            break;
        default:
            ERROR_CHECK(status);
    }
    if(m_device_provisioned == true)
    {
      __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Device was provisioned\r\n");
    }
    else
    {
      __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Device wasn't provisioned\r\n");
    }
    status = access_model_publish_period_set(m_client.model_handle, ACCESS_PUBLISH_RESOLUTION_10S, 1);
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Publish Period: %d\n", status);
    /* Register event handler to receive NRF_MESH_EVT_FLASH_STABLE. Application functionality will
    be started after this event */
    nrf_mesh_evt_handler_add(&m_mesh_core_event_handler);

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

void log_error_code(ret_code_t ret_code)
{
  //if(ret_code != NRF_SUCCESS)
  //{
  //   __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Error code: 0x%02x at function %s line: %s\r\n", ret_code, __FUNCTION__, __LINE__);
  //}
}

static void gateway_switch_info()
{
  mesh_gateway_transfer_t gateway_network;
}