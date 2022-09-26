#ifndef _DATA_DEFINE_H_
#define _DATA_DEFINE_H_

#define KEY_LENGHT    16
#include "device_state_manager.h"
/*Mesh network info exchange between node and gateway*/
typedef struct
{
   uint8_t app_key[KEY_LENGHT];
   uint8_t net_key[KEY_LENGHT];
   dsm_local_unicast_address_t unicast_address;
   //uint32_t sum32;
}mesh_network_info_t;


typedef void(*app_flash_get_success_cb_t)(void* p_arg);


#endif