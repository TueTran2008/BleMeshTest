#ifndef _DATA_DEFINE_H_
#define _DATA_DEFINE_H_

#define KEY_LENGHT    16
#define BEACON_DEFAULT_TOKEN 0x1C345378u
#define BEACON_ID           0x01u

#define    PAIR_ID_DEVICE_REQUEST_INFO                      (0x01)        // Device request information from gateway
#define    PAIR_ID_GATEWAY_REQUEST_INFO                     (0x02)        // Usecase : replace gw by other gw, the new gw will request information from other nodes in mesh network
#define    PAIR_ID_GATEWAY_ACCEPT_PAIR_REQUEST              (0x03)
#define    PAIR_ID_GATEWAY_REJECT_PAIR_REQUEST              (0x04)

#include "device_state_manager.h"
#include "user_on_off_client.h"
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
typedef struct  __attribute((packed))
{
  mesh_network_info_t network_info;
  global_status_t status;
  dsm_local_unicast_address_t node_provision_add;
}System_t;


/*
  Data structure send to gateway when switch gateway 
  
*/
typedef struct
{
  mesh_network_info_t network_info;
  //uint8_t dev_key[16];
  uint32_t iv_index;
  uint32_t sequence_number;
  struct
  {
    mesh_key_index_t n_value;
    uint32_t net_count;
  }net_key;
  struct
  {
    mesh_key_index_t d_value;
    uint32_t dev_count;
  }dev_key;
  struct
  {
    mesh_key_index_t a_value;
    uint32_t app_count;
  }app_key;
  /*Sequence number and vector index of rx data*/
}mesh_gateway_transfer_t;

typedef enum
{
    PAIR_INFO_STATE_INVALID         = 0x12345678,
    PAIR_INFO_STATE_PROVISIONING    = 0x11112222,
    PAIR_INFO_STATE_DONE            = 0x11113333,
    PAIR_INFO_RENEW                 = 0x12341231
}pair_info_state_t;


typedef struct __attribute((packed))
{
    uint8_t netkey[NRF_MESH_KEY_SIZE];
    uint8_t appkey[NRF_MESH_KEY_SIZE];
} app_provision_key_t;

typedef struct __attribute((packed))
{
    uint8_t pair_mac[6];
    uint16_t next_unicast_addr;
    uint16_t exchange_pair_addr;
    uint16_t topic_all;           // all node will subscribe to this topic, gw will send ping msg to this
    uint16_t topic_control;       // gw control, node sub 
    uint16_t topic_warning;       // gw sub, speaker sub, node send
    uint16_t topic_report;        // for node ping 
    uint16_t topic_ctrol_ack; 
    uint16_t topic_warning_ack;    
    app_provision_key_t key;
    pair_info_state_t state;
}pair_info_t;


typedef struct __attribute((packed))
{
  uint8_t msg_id;
  uint8_t device_type;
  uint8_t mac[6];
  uint32_t token;
}pair_info_request_key_t;

typedef struct
{
  uint8_t data_buffer[64];
  uint8_t index;
  bool is_having_scan_response;/*<This may check by compare the index with MAX_ADV_DATA_LENGHT = 32>*/
}adv_scan_data_t;

extern System_t xSystem;
extern user_on_off_client_t m_client;
typedef void(*app_flash_get_success_cb_t)(void* p_arg);


#endif