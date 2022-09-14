
#include <stdint.h>
#include <string.h>


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

/* Provisioning and configuration */
#include "mesh_provisionee.h"
#include "mesh_app_utils.h"

/* Models */
#include "generic_onoff_client.h"

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


/*User Model*/
#include "user_on_off_client.h"
#include "user_on_off_common.h"
/*Generic On off Client*/
#include "generic_onoff_client.h"
#include "generic_onoff_common.h"


#define CLIENT   0
#define SERVER  1
#if(SERVER + CLIENT >= 2)
#error"Fail BLE CONFIG"
#endif
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

/*****************************************************************************
 * Static function
 *****************************************************************************/
static void app_user_onoff_publish_interval_callback(access_model_handle_t model_handel, void *p_self)
{
   __LOG(LOG_SRC_APP, LOG_LEVEL_WARN, "Publish desired message here-Model Handler: %d\r\n", model_handel);
   // TODO: ADD PERIODIC Publish message here.
}

static void app_user_onoff_client_transaction_status_callback(access_model_handle_t model_handle, void *p_args, access_reliable_status_t status)
{
  switch(status)
  {
        case ACCESS_RELIABLE_TRANSFER_SUCCESS:
            __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Acknowledged transfer success.\n");
            break;

        case ACCESS_RELIABLE_TRANSFER_TIMEOUT:
            hal_led_blink_ms(HAL_LED_MASK, LED_BLINK_SHORT_INTERVAL_MS, LED_BLINK_CNT_NO_REPLY);
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
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "OnOff server: 0x%04x, Present OnOff: %d, Target OnOff: %d, Remaining Time: %d ms\n",
              p_meta->src.value, p_in->present_on_off, p_in->target_on_off, p_in->remaining_time);
    }
    else
    {
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "OnOff server: 0x%04x, Present OnOff: %d\n",
              p_meta->src.value, p_in->present_on_off);
    }
}




static void unicast_address_print(void)
{
   dsm_local_unicast_address_t node_address;
   dsm_local_unicast_addresses_get(&node_address);
   __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Node Address: 0x%04x \n", node_address.address_start);
}
static void device_identification_start_cb(uint8_t attention_duration_s)
{
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Initializing and adding models\n");
}

static void provisioning_complete_cb(void)
{
    __LOG(LOG_SRC_APP, LOG_LEVEL_ERROR, "Successfully provisioned\n");

#if MESH_FEATURE_GATT_ENABLED
    /* Restores the application parameters after switching from the Provisioning
     * service to the Proxy  */
    gap_params_init();
    conn_params_init();
#endif
    unicast_address_print();   
}

static void provisioning_aborted_cb(void)
{
    //hal_led_blink_stop();
    __LOG(LOG_SRC_APP, LOG_LEVEL_WARN, "Provisioning is aborted - Take a look\r\n");
}

static void start(void)
{
    //rtt_input_enable(rtt_input_handler, RTT_INPUT_POLL_PERIOD_MS);

    if (!m_device_provisioned)
    {
        static const uint8_t static_auth_data[NRF_MESH_KEY_SIZE] = STATIC_AUTH_DATA;
        mesh_provisionee_start_params_t prov_start_params =
        {
            .p_static_data    = static_auth_data,
            .prov_sd_ble_opt_set_cb = NULL,
    /**
     * Pointer to a function used to signal the completion of the device provisioning
     * procedure. Can be set to @c NULL if not used.
     *
     * @note Getting this callback means that a device is at minimum _provisioned_, however,
     *       it does not imply anthing about model configuration, added keys, etc. That may
     *       be altered by a Configuration Client at any point in time.
     *       See @ref CONFIG_SERVER_EVENTS.
     */
            .prov_complete_cb = provisioning_complete_cb,
            .prov_device_identification_start_cb = device_identification_start_cb,
            .prov_device_identification_stop_cb = NULL,
            .prov_abort_cb = provisioning_aborted_cb,
            .p_device_uri = EX_URI_LS_CLIENT
        };
        ERROR_CHECK(mesh_provisionee_prov_start(&prov_start_params));
    }
    else
    {
        unicast_address_print();
    }

    mesh_app_uuid_print(nrf_mesh_configure_device_uuid_get());

    ERROR_CHECK(mesh_stack_start());

    //__LOG(LOG_SRC_APP, LOG_LEVEL_INFO, m_usage_string);
    //hal_led_mask_set(HAL_LED_MASK, LED_MASK_STATE_OFF);
    //hal_led_blink_ms(HAL_LED_MASK, LED_BLINK_INTERVAL_MS, LED_BLINK_CNT_START);
}

static void node_reset(void)
{
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "----- Node reset  -----\n");
    //hal_led_blink_ms(HAL_LED_MASK, LED_BLINK_INTERVAL_MS, LED_BLINK_CNT_RESET);
    /* This function may return if there are ongoing flash operations. */
    mesh_stack_device_reset();
}
static void models_init_cb(void)
{
  m_client.setting.timeout = 0;
  m_client.setting.p_callbacks = &client_cbs;
  m_client.setting.force_segmented = false;
  m_client.setting.transmic_size = NRF_MESH_TRANSMIC_SIZE_SMALL;
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Initializing and adding models\n");
  user_on_off_client_init(&m_client, 1);
  
}

static void config_server_evt_cb(const config_server_evt_t * p_evt)
{
  if(p_evt->type == CONFIG_SERVER_EVT_NODE_RESET)
  {
    node_reset();
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
#warning "What does this UUID do"
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
}

void ble_stack_initialize(void)
{
  ble_stack_init();

  gap_params_init();

  conn_params_init();

  mesh_init();

  start();
}