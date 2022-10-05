/*
#include "generic_onoff_server.h"
#include "generic_onoff_common.h"
#include "generic_onoff_messages.h"
*/
#include "user_on_off_client.h"
#include "user_on_off_common.h"
#include "user_on_off_server.h"

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "access.h"
#include "access_config.h"
#include "nrf_mesh_assert.h"
#include "nrf_mesh_utils.h"
#include "nordic_common.h"
#include "log.h"

static uint32_t status_send(user_on_off_server_t * p_server,
                            const access_message_rx_t * p_message,
                            const user_on_off_status_msg_pkt_t * p_params)
{
    /*Declare Variables for storing data in this function*/
    user_on_off_status_msg_pkt_t p_msg_pkt;

    if (p_params->present_on_off > 1 ||
        p_params->target_on_off  > 1 ||
        p_params->remaining_time > TRANSITION_TIME_STEP_10M_MAX)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    /*Story state*/
    p_msg_pkt.present_on_off = p_params->present_on_off;
    /*
      if the Remaining Transition time is greater than zero => This ACK Messgage.
      => Decode the Message Transition time 
    */
    if (p_params->remaining_time > 0)
    {
        p_msg_pkt.target_on_off = p_params->target_on_off;
        p_msg_pkt.remaining_time = model_transition_time_encode(p_params->remaining_time);
    }
    /*Assigned value for message*/
    
    access_message_tx_t reply =
    {
        .opcode = ACCESS_OPCODE_VENDOR(USER_ON_OFF_OPCODE_STATUS, USER_ON_OFF_COMPANY_ID),
        .p_buffer = (const uint8_t *) &p_msg_pkt,
        .length = p_params->remaining_time > 0 ? USER_ON_OFF_MESSAGE_MIN_PAYLOAD_LENGHT : USER_ON_OFF_MESSAGE_MAX_PAYLOAD_LENGHT,
        .force_segmented = p_server->setting.force_segmented,
        .transmic_size = p_server->setting.transition_time
    };
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Reply Message\r\n+Length:%d\r\n+Force_Segmented:%d\r\n+Transmic_Size:%d\r\n", reply.length, reply.force_segmented, reply.transmic_size);
    
    if (p_message == NULL)
    {
        return access_model_publish(p_server->model_handle, &reply);
    }
    else
    {
        return access_model_reply(p_server->model_handle, p_message, &reply);
    }
}

static void periodic_publish_cb(access_model_handle_t handle, void * p_args)
{
    user_on_off_server_t *p_server = (user_on_off_server_t*)p_args;
    user_on_off_status_msg_pkt_t v_outdata;
    //p_server->setting.p_callbacks->get_cb(p_server, NULL, &v_outdata);
    p_server->setting.p_callbacks->get_cb(p_server, NULL, NULL);
    (void) status_send(p_server, NULL, &v_outdata);
}

/** Opcode Handlers */
/*
*   @brief: Check if the message is ACK and UNACK 
*/

static inline bool set_params_validate(const access_message_rx_t * p_rx_msg, const user_on_off_msg_set_pkt_t * p_params)
{
    return (
            (p_rx_msg->length == USER_ON_OFF_MESSAGE_MIN_PAYLOAD_LENGHT || p_rx_msg->length == USER_ON_OFF_MESSAGE_MAX_PAYLOAD_LENGHT) &&
            (p_params->on_off <= 1)
           );
}

/*Handle Set Opcode*/
static void handle_set(access_model_handle_t model_handle, const access_message_rx_t * p_rx_msg, void * p_args)
{
    user_on_off_server_t * p_server = (user_on_off_server_t *) p_args;
    /*Set Parameters*/
    user_on_off_msg_set_t in_data = {0};
    model_transition_t in_data_transition = {0};
    /*Status Parameters*/
    user_on_off_status_msg_pkt_t status_out_data = {0};
    /**/
    user_on_off_msg_set_pkt_t * p_msg_params_packed = (user_on_off_msg_set_pkt_t *) p_rx_msg->p_data;


    if (set_params_validate(p_rx_msg, p_msg_params_packed))
    {
        in_data.on_off = p_msg_params_packed->on_off;
        in_data.tid = p_msg_params_packed->tid;
        in_data.pwm_period = p_msg_params_packed->pwm_period;
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "get valid set-message\r\n+OnOffState:%d\r\n+TID:%d\r\n+pwm_period:%d\r\n", in_data.on_off, in_data.tid, in_data.pwm_period);
        /*This is a applicaition TID not in the stack :D */
        if (model_tid_validate(&p_server->tid_tracker, &p_rx_msg->meta_data, USER_ON_OFF_OPCODE_SET, in_data.tid))
        {
            if (p_rx_msg->length == USER_ON_OFF_MESSAGE_MIN_PAYLOAD_LENGHT)
            {
                if (!model_transition_time_is_valid(p_msg_params_packed->transition_time))
                {
                    return;
                }
                in_data_transition.transition_time_ms = model_transition_time_decode(p_msg_params_packed->transition_time);
                in_data_transition.delay_ms = model_delay_decode(p_msg_params_packed->delay);
                __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Server: transition_time_ms = %X\n", in_data_transition.transition_time_ms);
                __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Server: delay_ms = %X\n", in_data_transition.delay_ms);
            }
            /*Call the callback function in application*/
            p_server->setting.p_callbacks->set_cb(p_server,
                                                  &p_rx_msg->meta_data,
                                                  &in_data,
                                                  (p_rx_msg->length == USER_ON_OFF_MESSAGE_MIN_PAYLOAD_LENGHT) ? NULL : &in_data_transition,
                                                  (p_rx_msg->opcode.opcode == USER_ON_OFF_OPCODE_SET) ? &status_out_data : NULL);

            if (p_rx_msg->opcode.opcode == USER_ON_OFF_OPCODE_SET)
            {
                (void) status_send(p_server, p_rx_msg, &status_out_data);
            }
        }
    }
}

static inline bool get_params_validate(const access_message_rx_t * p_rx_msg)
{
    return (p_rx_msg->length == 0);
}

static void handle_get(access_model_handle_t model_handle, const access_message_rx_t * p_rx_msg, void * p_args)
{
    user_on_off_server_t * p_server = (user_on_off_server_t *) p_args;
    user_on_off_status_msg_pkt_t out_data = {0};

    if (get_params_validate(p_rx_msg))
    {
        p_server->setting.p_callbacks->get_cb(p_server, &p_rx_msg->meta_data, &out_data);
        (void) status_send(p_server, p_rx_msg, &out_data);
    }
}

static const access_opcode_handler_t m_opcode_handlers[] =
{
    {ACCESS_OPCODE_VENDOR(USER_ON_OFF_OPCODE_SET, USER_ON_OFF_COMPANY_ID), handle_set},
    {ACCESS_OPCODE_VENDOR(USER_ON_OFF_OPCODE_SET_UNRELIABLE, USER_ON_OFF_COMPANY_ID), handle_set},
    {ACCESS_OPCODE_VENDOR(USER_ON_OFF_OPCODE_GET, USER_ON_OFF_COMPANY_ID), handle_get},
};


/** Interface functions */
uint32_t user_on_off_server_init(user_on_off_server_t * p_server, uint8_t element_index)
{
    if (p_server == NULL ||
        p_server->setting.p_callbacks == NULL ||
        p_server->setting.p_callbacks->set_cb == NULL ||
        p_server->setting.p_callbacks->get_cb == NULL )
    {
        return NRF_ERROR_NULL;
    }
    access_model_add_params_t init_params =
    {
        .model_id = ACCESS_MODEL_VENDOR(USER_ON_OFF_SERVER_MODE_ID, USER_ON_OFF_COMPANY_ID),
        .element_index =  element_index,
        .p_opcode_handlers = &m_opcode_handlers[0],
        .opcode_count = ARRAY_SIZE(m_opcode_handlers),
        .p_args = p_server,
        .publish_timeout_cb = periodic_publish_cb
    };

    uint32_t status = access_model_add(&init_params, &p_server->model_handle);
    if (status == NRF_SUCCESS)
    {
        status = access_model_subscription_list_alloc(p_server->model_handle);
    }
    else
    {
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Add model Failed-ErrorCode:0x%02x\r\n", status);
    }
    return status;
}

uint32_t user_on_off_server_status_publish(user_on_off_server_t * p_server, const user_on_off_status_msg_pkt_t * p_params)
{
    if (p_server == NULL ||
        p_params == NULL)
    {
        return NRF_ERROR_NULL;
    }

    return status_send(p_server, NULL, p_params);
}

uint32_t user_on_off_server_state_set(user_on_off_server_t * p_server, bool onoff)
{
    user_on_off_msg_set_t in_data = {0};

    in_data.on_off = onoff;
    p_server->setting.p_callbacks->set_cb(p_server, NULL, &in_data, NULL, NULL);

    return NRF_SUCCESS;
}
