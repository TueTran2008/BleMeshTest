
#ifndef _USER_ON_OFF_SERVER_H
#define _USER_ON_OFF_SERVER_H
#include <stdint.h>
#include <stddef.h>
#include "model_common.h"

//#include "user_on_off_client.h"
#include "user_on_off_common.h"
#include "access.h"
#include "access_config.h"
#include "access_reliable.h"
#include "device_state_manager.h"
#include "nrf_mesh.h"
#include "nrf_mesh_assert.h"
#include "log.h"
#include "nordic_common.h"
#include "generic_onoff_server.h"
#include "model_common.h"



//static void set_handle_callback(access_model_handle_t model_handle, const access_message_rx_t *p_rx_message, void *p_args)
//{
//}

#define USER_ON_OFF_SERVER_MODE_ID      0x11AA


/*****************************************************************************
 * Forward Declartion
 *****************************************************************************/
typedef struct __user_on_off_server_t user_on_off_server_t;
/**
 * Callback type for Generic OnOff Set/Set Unacknowledged message.
 *
 * @param[in]     p_self                   Pointer to the model structure.
 * @param[in]     p_meta                   Access metadata for the received message.
 * @param[in]     p_in                     Pointer to the input parameters for the user application.
 * @param[in]     p_in_transition          Pointer to transition parameters, if present in the incoming message,
 *                                         otherwise set to null.
 * @param[out]    p_out                    Pointer to store the output parameters from the user application.
 *                                         If null, indicates that it is UNACKNOWLEDGED message and no
 *                                         output params are required.
 */
typedef void (*user_onoff_state_set_cb_t)(  const user_on_off_server_t * p_self,
                                             const access_message_rx_meta_t * p_meta,
                                             const user_on_off_msg_set_t * p_in,
                                             const model_transition_t * p_in_transition,
                                             user_on_off_status_msg_pkt_t * p_out);
 /**
 * Callback type for Generic OnOff Get message.
 *
 * @param[in]     p_self                   Pointer to the model structure.
 * @param[in]     p_meta                   Access metadata for the received message.
 * @param[out]    p_out                    Pointer to store the output parameters from the user application.
 */
typedef void (*user_onoff_state_get_cb_t)(const user_on_off_server_t * p_self,
                                          const access_message_rx_meta_t * p_meta,
                                          user_on_off_status_msg_pkt_t* p_out);


typedef void(*user_onoff_server_periodic_cb)(access_model_handle_t model_handle, void *p_args);


typedef struct
{
  user_onoff_state_set_cb_t set_cb;
  user_onoff_state_get_cb_t get_cb;
  user_onoff_server_periodic_cb period_cb;
}user_onoff_server_callbacks_t;

typedef struct
{
  //uint32_t timeout;

  bool force_segmented;
  /*Actually this is Transmic size - My mistake but just let it be because adjust it will waste a lot of time and i am so lazy to do so:D*/
  nrf_mesh_transmic_size_t transition_time;

  const user_onoff_server_callbacks_t *p_callbacks;

}user_on_off_server_params_and_callbacks_t;

struct __user_on_off_server_t
{
  access_model_handle_t model_handle;

  user_on_off_server_params_and_callbacks_t setting;

  tid_tracker_t tid_tracker;
  
  uint8_t state_handle;
};


uint32_t user_on_off_server_state_set(user_on_off_server_t * p_server, bool onoff);

uint32_t user_on_off_server_init(user_on_off_server_t * p_server, uint8_t element_index);

/*
  @brief: Publish the current status by sending to sender
  @param [in]: p_server. Pointer to server_type
  @param [in]: Message packet
*/
uint32_t user_on_off_server_status_publish(user_on_off_server_t * p_server, const user_on_off_status_msg_pkt_t * p_params);

#endif