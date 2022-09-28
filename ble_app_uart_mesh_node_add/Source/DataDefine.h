#ifndef _DATA_DEFINE_H_
#define _DATA_DEFINE_H_

#define KEY_LENGHT    16
#include "device_state_manager.h"
/*Mesh network info exchange between node and gateway*/
typedef struct
{
   //bool available; /*check if inf is saved in flash*/
   uint8_t app_key[KEY_LENGHT];
   uint8_t net_key[KEY_LENGHT];
   dsm_local_unicast_address_t unicast_address;
   //uint32_t sum32;
}mesh_network_info_t;


typedef struct
{
  bool   is_device_adv; 
}global_status_t;
/*Data structure of a global variable*/
typedef struct
{
  mesh_network_info_t network_info;
  global_status_t status;
}System_t;

extern System_t xSystem;
typedef void(*app_flash_get_success_cb_t)(void* p_arg);


#endif