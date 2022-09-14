#ifndef SIMPLE_ON_OFF_CLIENT_H__
#define SIMPLE_ON_OFF_CLIENT_H__

#include <stdint.h>
#include "access.h"
#include "user_on_off_common.h"
#include "access_reliable.h"
#include "model_common.h"


typedef enum
{
  USER_ON_OFF_STATUS_OFF = 0,
  USER_ON_OFF_STATUS_ON
}user_on_off_status_t;


/*Forward Declaration*/
typedef struct __user_on_off_client user_on_off_client_t;
typedef void (*user_on_off_status_cb_t)(const user_on_off_client_t * p_self,
                                                const access_message_rx_meta_t * p_meta,
                                                const user_on_off_status_msg_pkt_t * p_in);
//typedef void(*user_on_off_timeout_cb)(a)

typedef struct
{
    /** Client model response message callback. */
    user_on_off_status_cb_t onoff_status_cb;
    /** Callback to call after the acknowledged transaction has ended. */
    access_reliable_cb_t ack_transaction_status_cb;
    /** callback called at the end of the each period for the publishing */
    access_publish_timeout_cb_t periodic_publish_cb;
}user_on_off_callbacks_t;

typedef struct
{
    /*Reliable message timeout*/
    uint32_t timeout;

    nrf_mesh_transmic_size_t transmic_size; 

    bool force_segmented;
    
    user_on_off_callbacks_t *p_callbacks;
}user_on_off_params_and_callbacks_t;



struct __user_on_off_client
{
  /*Model Handle to pass in access_model_publish*/
  access_model_handle_t model_handle; 
  //access_model_handle_t access_handle;
  access_reliable_t access_message;
  /**/
  user_on_off_msg_set_pkt_t msg_pkt;
  /*callback and setting*/
  user_on_off_params_and_callbacks_t setting;
};



uint32_t user_on_off_client_init(user_on_off_client_t *p_client, uint16_t element_index);

uint32_t user_on_off_client_set(user_on_off_client_t *p_client, user_on_off_msg_set_t *p_params, model_transition_t *p_transition);

uint32_t user_on_off_client_set_unack(user_on_off_client_t *p_client, user_on_off_msg_set_t *p_params, model_transition_t *p_transition, uint16_t repeat_times);

uint32_t user_on_off_client_get(user_on_off_client_t *p_client);

#endif