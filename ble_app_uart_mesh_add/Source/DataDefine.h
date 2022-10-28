#ifndef _DATA_DEFINE_H_
#define _DATA_DEFINE_H_

#include "led_driver.h"
#include "user_on_off_client.h"
#include "user_on_off_common.h"

#include "device_state_manager.h"
#include "user_on_off_client.h"
#include "adc_button.h"
#include "adc_button.h"

#define KEY_LENGHT    16
#define BEACON_DEFAULT_TOKEN 0x1C345378u
#define BEACON_ID           0x01u
#define APP_MAX_BEACON      32


#define HW_RF_PA_PIN                23 		//PA RX EN
#define HW_RF_LNA_PIN               24 		//PA TX EN


#define FIRMWARE_VERSION  001 
#define HARDWARE_VERSION  002

#define DEV_TEST  1

#define MESH_APP_COMMON_INVALID_UNICAST_ADDR      (0xFFFF)

#define APP_MESH_MIN_VALID_UNICAST_ADD    0x0001
#define APP_MESH_MAX_VALID_UNICAST_ADD    0xFFFF


#define APP_UNACK_MSG_REPEAT_COUNT (3)

#define  GW_MESH_MSQ_ID_ALARM_SYSTEM    0x00

#define  GW_MESH_MSG_ID_UNKNOWN_SYSTEM  0x01


#define    PAIR_ID_DEVICE_REQUEST_INFO                      (0x01)        // Device request information from gateway
#define    PAIR_ID_GATEWAY_REQUEST_INFO                     (0x02)        // Usecase : replace gw by other gw, the new gw will request information from other nodes in mesh network
#define    PAIR_ID_GATEWAY_ACCEPT_PAIR_REQUEST              (0x03)
#define    PAIR_ID_GATEWAY_REJECT_PAIR_REQUEST              (0x04)
#define    PAIR_ID_PROCESS_DONE                             (0xFE)
#define    PAIR_ID_MAX                                      (0xFF)

#define    PAIR_INFO_GW_UNICAST_ADDR                     (0x0002)
#define    PAIR_INFO_GW_MAX_NODE_SUPPORT                 (2048)       // including beacon


///*TODO: Fix*/
//#define   APP_MSG_TYPE_NO_ALARM       0x11
//#define   APP_MSG_TYPE_SOS_ALARM      0x22
//#define   APP_MSG_TYPE_SPEAKER_ALARM  0x33
//#define   APP_MSG_TYPE_SMOKE_ALARM    0x44
//#define   APP_MSG_TYPE_TEMP_ALARM     0x55

/* Device type */
#define APP_DEVICE_GW                               (0x00)      /*<Message in mesh network sent from gateway>*/
#define APP_DEVICE_SENSOR_FIRE_DETECTOR             (0x01)      
#define APP_DEVICE_SENSOR_SMOKE_DETECTOR            (0x02)
#define APP_DEVICE_BUTTON_SOS                       (0x03)
#define APP_DEVICE_SPEAKER                          (0x04)
#define APP_DEVICE_SENSOR_TEMP_DETECTOR             (0x05)
#define APP_DEVICE_SENSOR_DOOR_DETECTOR             (0x06)
#define APP_DEVICE_SENSOR_PIR_DETECTOR              (0x07)
#define APP_DEVICE_TOUCH_LIGHT                      (0x08)
#define APP_DEVICE_TOUCH_WATER_WARMER               (0x09)
#define APP_DEVICE_TOUCH_AIR_CON                    (0x0A)
#define APP_DEVICE_SENSOR_TEMP_SMOKE                (0x0B)
#define APP_DEVICE_POWER_METER                      (0x0C)
#define APP_DEVICE_UNKNOWN                          (0xFE)
#define APP_DEVICE_MAX                              (0xFF)

/* Message alarm type bit, may be multi alarm type pack in to one message */
#define    APP_MSG_TYPE_FIRE_ALARM                  (0x01)
#define    APP_MSG_TYPE_SPEAKER_ALARM               (0x02)
#define    APP_MSG_TYPE_SMOKE_ALARM                 (0x03)
#define    APP_MSG_TYPE_SOS_ALARM                   (0x04)
#define    APP_MSG_TYPE_KEEP_ALIVE                  (0x05)
#define    APP_MSG_TYPE_NO_ALARM                    (0x06)      /*<Message Indicate no alarm source>*/
#define    APP_MSG_TYPE_SET_ALARM_SPEAKER           (0x07)
#define    APP_MSG_TYPE_GET_ALARM_SPEAKER           (0x08)
#define    APP_MSG_TYPE_PING_NODE                   (0x09)
#define    APP_MSG_TYPE_PING_ALL                    (0x0A)
#define    APP_MSG_TYPE_ACK                         (0x0B)
#define    APP_MSG_TYPE_REBOOT                      (0x0C)
#define    APP_MSG_TYPE_TEMP_ALARM                  (0x0D)
#define    APP_MSG_TYPE_DOOR_ALARM                  (0x0E)
#define    APP_MSG_TYPE_PIR_ALARM                   (0x0F)
#define    APP_MSG_TYPE_SELF_TEST_ALARM             (0x10)     /*<Message indicate this is manual alarm >*/
#define    APP_MSG_TYPE_MESH_CORE_DEBUG             (0x11)
#define    APP_MSG_TYPE_TEMP_SMOKE_ALARM            (0x12)    // combine sensor
#define    APP_MSG_TYPE_MAX                         (0xFF)



#define APP_MESH_ID_LEAVE_NETWORK (0x00)
#define APP_MESH_ID_PUBLISH (0x01)
#define APP_MESH_ID_SUBSCRIBE (0x02)
#define APP_MESH_ID_DISABLE (0x03)
#define APP_MESH_ID_DO_PING (0x03)
#define APP_MESH_ID_BEACON (0x04)


#define  GW_MESH_MSQ_ID_ALARM_SYSTEM    0x00
#define  GW_MESH_MSG_ID_UNKNOWN_SYSTEM  0x01

#define APP_MESH_EVT_MAX_DATA_LEN (14)

#define APP_TOKEN (0x1234)
#define APP_BEACON_COMPANY_ID 0x5900

typedef union __attribute((packed))
{
    struct __attribute((packed))
    {
        uint8_t tid : 4;
        uint8_t version : 4;    // limited 16 version
    } info;
    uint8_t value;
} app_beacon_tid_t;

typedef struct __attribute((packed))
{
    uint16_t unicast_addr;
    uint8_t tid;
} app_transaction_t;
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


/*Mesh network info exchange between node and gateway*/
typedef struct
{
   //bool available; /*check if inf is saved in flash*/
   uint8_t app_key[KEY_LENGHT];
   uint8_t net_key[KEY_LENGHT];
   dsm_local_unicast_address_t unicast_address;
}mesh_network_info_t;
/*<>*/
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

typedef struct 
{
   uint8_t device_mac[6];
   uint8_t device_type;
   bool pair_success;
}beacon_pair_info_t;

typedef union {
    struct Alarms_t {
        uint8_t EnableBuzzer : 1;
        uint8_t EnableSyncAlarm : 1;
        uint8_t EnableAlarmPower : 1;
        uint8_t reserver : 5;
    } Name;
    uint8_t Value;
} AlarmConfig_t;

typedef struct
{
    uint8_t Params_CenterMuteTime;
    uint8_t SpeakerVolume;
    //uint16_t FrequencyNonSos;
    //uint16_t FrequencySoS;
    AlarmConfig_t AlarmConfig;
}Parameter_Common_t;


typedef struct __attribute((packed))
{
    pair_info_t pair_information;
 /*<Onl;y count which one using UART NUS service requests data>*/
    Parameter_Common_t config_parameter;
    uint16_t total_provisioned_node; 
}FlashParameter_t;

typedef union {
    struct AlarmState_t {
      uint16_t ALARM_CENTER_BUTTON : 1;				
      uint16_t ALARM_REMOTE : 1;		
      uint16_t ALARM_SMOKE : 1;		
      uint16_t ALARM_DOOR : 1;	
      uint16_t ALARM_PIR : 1;		
      uint16_t ALARM_TEMP : 1;		
      uint16_t ALARM_SFUL02_IO : 1;
      uint16_t ALARM_SFUL02_DTMF : 1;
      uint16_t ALARM_SFUL02_FIREBOX_FAULTY : 1;
      uint16_t ALARM_SFUL02_ISO_IN3 : 1;
      uint16_t ALARM_SFUL02_ISO_IN4 : 1;
      uint16_t ALARM_FIRE : 1;
      uint16_t ALARM_BUZZER_LAMP : 1;
      uint16_t ALARM_COMBINE_TEMP_SMOKE : 1;
      uint16_t NA : 2;
    } Name;
    uint16_t Value;
} DeviceAlarmStatus_t;

typedef union __attribute((packed))
{
    struct Properties_t
    {
        uint16_t deviceType : 6;	
        uint16_t alarmState : 1;
        uint16_t isNewMsg : 1;
        uint16_t isPairMsg : 1;
        //uint16_t fwVersion : 5;
        uint16_t comboSensor : 1;
        uint16_t reserve : 10;
    } Name;
    uint16_t Value;
}node_proprety_t;

typedef struct
{
    uint8_t Data[14];
    uint8_t Len;
    uint8_t IsCustom;
} node_custom_data_t;
/*Forware Declaration*/
typedef struct __attribute((packed))
{
  uint8_t device_mac[6];
  app_beacon_tid_t beacon_tid;
  uint8_t battery;
  uint8_t is_data_valid;  
  uint8_t fw_verison;
  uint8_t teperature_value;
  uint8_t smoke_value;
  uint32_t timestamp; /*<Current tick when receive this value>*/
  node_proprety_t propreties; /*Node's Propreties*/
  node_custom_data_t custom_data;
}app_beacon_data_t;
/*<>*/
typedef struct
{
  bool   is_device_adv;
  beacon_pair_info_t beacon_pair_info;
  bool   init_done;
}global_status_t;
/*Data structure of a global variable*/
typedef struct  __attribute((packed))
{
  mesh_network_info_t network_info;
  global_status_t status;
  DeviceAlarmStatus_t alarm_value;
  uint16_t sensor_count;
  bool is_in_pair_mode;
  bool last_time_is_alarm;
  FlashParameter_t flash_parameter;
  LED_DRVIER_T led_driver;
  access_model_handle_t client_model;
  app_beacon_data_t Node_list[APP_MAX_BEACON];

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



typedef union __attribute((packed))
{
    struct __attribute((packed))
    {
        uint8_t smoke_alarm : 1;
        uint8_t temperature : 7;
    } name;
    uint8_t value;
} temp_smoke_t;

typedef struct __attribute((packed))
{
    uint8_t device_type;
    union __attribute((packed))
    {
        uint8_t raw_uuid[16];
        struct __attribute((packed))
        {
            uint16_t unicast_addr;
            uint8_t device_mac[6];
            uint8_t gw_mac[6];
            app_beacon_tid_t beacon_tid;
            temp_smoke_t temperature;
        } detail;
    } uuid;
    uint8_t msg_type;
    uint8_t battery;
    uint16_t token;
} app_beacon_t;




typedef struct __attribute((packed))
{
  DeviceAlarmStatus_t alarm_value;
  uint16_t sensor_count;
  uint8_t gateway_mac[6];
  app_provision_key_t mesh_key;
  uint8_t in_pair_mode;
  uint32_t sequence_number;
  uint32_t iv_index;
  queue_button_t button_state;
}app_beacon_ping_msg_t;

typedef struct __attribute((packed))
{
  uint8_t device_mac[6];
  uint8_t device_type;// Maybe Bitfield
  app_beacon_tid_t beacon_tid;
  uint8_t battery;
}app_beacon_msg_t;

typedef struct __attribute((packed))
{
  uint8_t netkey[16];
  uint8_t appkey[16];
  uint32_t iv_index;
  uint32_t sequence_number;
}app_beacon_config_msg_t;

typedef struct __attribute((packed))
{
    app_beacon_tid_t beacon_tid; 
    uint8_t mac[6];
    uint8_t battery;
    uint8_t msg_type;
    uint8_t device_type;
} mesh_beacon_parse_data_t;

typedef union __attribute((packed))
{
    struct
    {
        uint8_t alarm;
        uint16_t src_addr;
        uint8_t reserve;
    } name;
    uint32_t value;
}app_mesh_get_set_value_t;

typedef struct __attribute((packed))
{
    uint8_t dev_type;
    uint8_t msg_type;
    app_mesh_get_set_value_t value;
}app_mesh_gw_set_structure_t;




typedef struct
{
  uint8_t config_netkey : 1;
  uint8_t config_appkey : 1;
  uint8_t config_IVindex : 1;
  uint8_t config_seqnumber :1;
  uint8_t config_speaker : 1;
  uint8_t config_alarm : 1;
  uint8_t config_mac   : 1;
  uint8_t reseverd     : 1;
  /*Structure padding*/
  uint8_t netkey[16];
  uint8_t appkey[16];
  uint8_t speaker;
  AlarmConfig_t alarm; /*AlarmConfig size 1 byte*/
  /*Structure padding*/
  uint32_t iv_index;
  uint32_t sequence_number;
  //uint8_t config_centermutetime
  uint8_t mac[6];
}GD32Config_t;

typedef struct
{
  uint16_t unicast_add;
}pair_response_info_t;



typedef void(*beacon_data_callback_t)(void *p_args);
extern System_t xSystem;
typedef void(*app_flash_get_success_cb_t)(void* p_arg);


#endif