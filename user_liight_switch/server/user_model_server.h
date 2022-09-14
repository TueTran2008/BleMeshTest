#ifndef SIMPLE_ON_OFF_SERVER_H__
#define SIMPLE_ON_OFF_SERVER_H__

#include <stdint.h>
#include <stdbool.h>
#include "access.h"
#include "user_model_common.h"




typedef struct __simple_on_off_server simple_on_off_server_t;

typedef void(*simple_on_off_server_get)(simple_on_off_server_t *p_itself);
typedef void(*simple_on_off_server_set)(simple_on_off_server_t *p_itself);


struct __simple_on_off_server
{
  access_model_handle_t model_handle; 
  simple_on_off_server_get get_callback;
  simple_on_off_server_set set_callback;
};
/*Opcode Handler Typedef Function Pointer*/
typedef void (*access_opcode_handler_cb_t)(access_model_handle_t handle,
                                           const access_message_rx_t * p_message,
                                           void * p_args);

#endif

