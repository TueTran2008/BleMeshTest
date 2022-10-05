#ifndef _APP_SELF_PROVISION_H_
#define _APP_SELF_PROVISION_H_
#include "access.h"
#include "access_config.h"

#define LOCAL_ADDRESS_START 0x0001
#define MESH_TOPIC_ALL 0xC001
#define MESH_TOPIC_CONTROL 0xC002   /*Gateway send to this topic*/
#define MESH_TOPIC_WARNING 0xC003   /*Gateway control, all node sub*/
#define MESH_TOPIC_REPORT  0xC004   /*Gateway sub, Node send*/

#define MESH_TOPIC_CONTROL_ACK 0xC005 /*Node ping*/
#define MESH_TOPIC_WARNING_ACK 0xC006
/*Netkey*/
/*Netkey*/
//#define APP_NETKEY {0x8C, 0x5C, 0x10, 0x23, 0xC9, 0x77, 0x8D, 0xFA, 0xC3, 0x69, 0x9F, 0x1C, 0x9E, 0x4A, 0x25, 0x28}
//#define APP_NETKEY_INDEX 0
///*Device Key*/
//#define APP_DEVKEY  {}
//#define APP_DEVKEY_INDEX 

#define NET_KEY {0x8C, 0x5C, 0x10, 0x23, 0xC9, 0x77, 0x8D, 0xFA, 0xC3, 0x69, 0x9F, 0x1C, 0x9E, 0x4A, 0x25, 0x28}
#define DEV_KEY {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}
#define APP_KEY {0x5C, 0x44, 0xD9, 0x7D, 0xDA, 0x11, 0x53, 0x57, 0x49, 0x7D, 0x23, 0xBF, 0x5B, 0xCF, 0xAD, 0xD9}


void app_self_provision(access_model_handle_t model_handle,
                        int8_t *p_app_key, uint8_t *p_netkey, 
                        dsm_local_unicast_address_t m_start_unicast_add);

void subcribe_topic(access_model_handle_t model);
                       
void app_gateway_get_current_info();

void app_get_sequence_number_and_iv_index(uint32_t *iv_index, uint32_t *sequence_number);

void app_generate_random_keys(uint8_t *app_key, uint8_t *net_key);
#endif