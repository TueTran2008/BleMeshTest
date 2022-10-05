

#include <stdint.h>
#include <string.h>
/*User Include*/

/* HAL */
#include "boards.h"
#include "simple_hal.h"
#include "app_timer.h"

/* Core */
#include "nrf_mesh_config_core.h"
#include "nrf_mesh_gatt.h"
#include "nrf_mesh_configure.h"
#include "nrf_mesh_events.h"
#include "nrf_mesh.h"
#include "mesh_stack.h"
#include "device_state_manager.h"
#include "access_config.h"
#include "proxy.h"

/* Provisioning and configuration */
#include "mesh_provisionee.h"
#include "mesh_app_utils.h"

/* Models */
//#include "generic_onoff_server.h"
#include "scene_setup_server.h"
#include "model_config_file.h"

/* Logging and RTT */
#include "log.h"
#include "rtt_input.h"

/* Example specific includes */
#include "app_config.h"
#include "example_common.h"
#include "nrf_mesh_config_examples.h"
#include "light_switch_example_common.h"
//#include "app_onoff.h"
#include "ble_softdevice_support.h"
#include "app_dtt.h"
#include "app_scene.h"
#include "nrf_gpiote.h"
/**/
#include "led_driver.h"
//#include "user_input.h"
#include "app_user_on_off_server.h"

#include "user_on_off_server.h"
#include "app_sef_provision.h"
#include "DataDefine.h"
#define SDK_COEXIST 1



/*****************************************************************************
 * Definitions
 *****************************************************************************/
#define APP_ONOFF_ELEMENT_INDEX     (0)
/* Controls if the model instance should force all mesh messages to be segmented messages. */
#define APP_FORCE_SEGMENTATION      (false)
/* Controls the MIC size used by the model instance for sending the mesh messages. */
#define APP_MIC_SIZE                (NRF_MESH_TRANSMIC_SIZE_SMALL)


//app_user_onoff_server_t a;


/*****************************************************************************
 * Forward declaration of static functions
 *****************************************************************************/
static void user_onoff_state_set_cb(const user_on_off_server_t * p_self,
                                       const access_message_rx_meta_t * p_meta,
                                       const user_on_off_msg_set_t * p_in,
                                       const model_transition_t * p_in_transition,
                                       user_on_off_status_msg_pkt_t * p_out);
static void user_onoff_state_get_cb(const user_on_off_server_t * p_self,
                                       const access_message_rx_meta_t * p_meta,
                                       user_on_off_status_msg_pkt_t * p_out);

static void mesh_events_handle(const nrf_mesh_evt_t * p_evt);
/*****************************************************************************
 * Static variables
 *****************************************************************************/
static bool m_device_provisioned;
static nrf_mesh_evt_handler_t m_event_handler =
{
    .evt_cb = mesh_events_handle,
};


user_on_off_server_t m_server;

const static user_onoff_server_callbacks_t server_cbs =
{
    .get_cb = user_onoff_state_get_cb,
    .set_cb = user_onoff_state_set_cb,
    .period_cb = NULL
};
/*****************************************************************************
 * Privates functions
 *****************************************************************************/
/* Callback for updating the hardware state */

static void user_onoff_state_set_cb(const user_on_off_server_t * p_self,
                                       const access_message_rx_meta_t * p_meta,
                                       const user_on_off_msg_set_t * p_in,
                                       const model_transition_t * p_in_transition,
                                       user_on_off_status_msg_pkt_t * p_out)
{
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "msg: SET: %d\n", p_in->on_off);
}


static void app_model_init(void)
{
    m_server.setting.force_segmented = false;
    m_server.setting.transition_time = APP_MIC_SIZE;
    m_server.setting.p_callbacks = &server_cbs;
    user_on_off_server_init(&m_server, 1);
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Server Model Handle: %d\n", m_server.model_handle);
   
}
static void user_onoff_state_get_cb(const user_on_off_server_t * p_self,
                                       const access_message_rx_meta_t * p_meta,
                                       user_on_off_status_msg_pkt_t * p_out)
{
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "msg: GET \n");
}



/*************************************************************************************************/

static void mesh_events_handle(const nrf_mesh_evt_t * p_evt)
{
    //mesh_network_info_t network_info = 0;
    if (p_evt->type == NRF_MESH_EVT_ENABLED)
    {
      if(m_device_provisioned == true)
      {
        subcribe_topic(m_server.model_handle);
      }
      else
      {
      }

    }
}

static void node_reset(void)
{
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "----- Node reset  -----\n");
    model_config_file_clear();

    mesh_stack_device_reset();
}

static void config_server_evt_cb(const config_server_evt_t * p_evt)
{
    if (p_evt->type == CONFIG_SERVER_EVT_NODE_RESET)
    {
        node_reset();
    }
}
static void unicast_address_print(void)
{
    dsm_local_unicast_address_t node_address;
    dsm_local_unicast_addresses_get(&node_address);
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Node Address: 0x%04x \n", node_address.address_start);
}



static void server_models_init_cb(void)
{
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Initializing and adding models\n");
    app_model_init();
}

static void mesh_init(void)
{
    /* Initialize the application storage for models */
    mesh_stack_init_params_t init_params =
    {
        .core.irq_priority       = NRF_MESH_IRQ_PRIORITY_LOWEST,
        .core.lfclksrc           = DEV_BOARD_LF_CLK_CFG,
        .core.p_uuid             = NULL,
        .models.models_init_cb   = server_models_init_cb,
        .models.config_server_cb = config_server_evt_cb
    };
    uint32_t status = mesh_stack_init(&init_params, &m_device_provisioned);
    if (status == NRF_SUCCESS)
    {
        /* Check if application stored data is valid, if not clear all data and use default values. */
        //status = model_config_file_config_apply();
    }
    switch (status)
    {
        case NRF_ERROR_INVALID_DATA:
            /* Clear model config file as loading failed */
            //model_config_file_clear();
            __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Data in the persistent memory was corrupted. Device starts as unprovisioned.\n");
            __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Reboot device before starting of the provisioning process.\n");
            break;
        case NRF_SUCCESS:
            __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "NRF MESH STACK Init Successfully - Read back Mesh Data from flash\r\n");
            unicast_address_print();
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
}

static void initialize(void)
{
    __LOG_INIT(LOG_SRC_APP | LOG_SRC_ACCESS | LOG_SRC_TRANSPORT, LOG_LEVEL_DBG1 | LOG_LEVEL_INFO | LOG_LEVEL_DBG2 | LOG_LEVEL_DBG3, LOG_CALLBACK_DEFAULT);
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "----- Tue Mesh Server TestBoard -----\n");
    ERROR_CHECK(app_timer_init());
    ble_stack_init();

#if MESH_FEATURE_GATT_ENABLED
    gap_params_init();
    conn_params_init();
#endif
    mesh_init();
}

static void start(void)
{
    mesh_app_uuid_print(nrf_mesh_configure_device_uuid_get());
    /* NRF_MESH_EVT_ENABLED is triggered in the mesh IRQ context after the stack is fully enabled.
     * This event is used to call Model APIs for establishing bindings and publish a model state information. */
    nrf_mesh_evt_handler_add(&m_event_handler);
    ERROR_CHECK(mesh_stack_start());
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