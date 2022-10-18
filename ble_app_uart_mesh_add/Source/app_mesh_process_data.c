#include "DataDefine.h"
#include "app_mesh_gateway_msg.h"
#include "app_mesh_message_queue.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "ble_config.h"
#include "led_driver.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*****************************************************************************
 * MACROS Decleration
 *****************************************************************************/
#define MAX_ALARM_SUPPORT 12

#define NODE_INVALID_INDEX 0xFF

#define ALARM_NODE_TIMEOUT 45000
/*****************************************************************************
 * Structure Decleration
 *****************************************************************************/
 /*<Structure containt imformation of current alarm node>*/
typedef struct
{
  uint8_t mac_add[6];
  uint16_t mesh_id;
  uint32_t appropriate_tick_get_alarm; /*Assign systick when receive alarm message*/
  uint8_t is_alarm;     
  uint8_t is_valid_data;
}node_alarm_store_t;

/*****************************************************************************
 * Privates Variables
 *****************************************************************************/
static node_alarm_store_t m_list_alarm[MAX_ALARM_SUPPORT] = {0};
uint16_t m_last_alarm_addr = 0xFFFF;

/*****************************************************************************
 * Forward Decleration
 *****************************************************************************/
static inline bool is_msg_alarm(uint8_t msg_type);
static int8_t node_alarm_find_and_write(node_alarm_store_t *alarm_node);
static void app_queue_mesh_insert_node(uint8_t *mac, 
                                        app_alarm_message_structure_t *alarm_sos_structure,
                                        uint8_t alarm_type,
                                        uint8_t *custom_data,
                                        uint8_t custom_data_size);
static void no_alarm_anymore(void);
static bool node_detect_available_alarm();

/*****************************************************************************
 * Implementation
 *****************************************************************************/
/** @brief    
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
static void process_sos_data(gw_mesh_msq_t *p_data)
{
   app_alarm_message_structure_t * alarm_struct_sos = (app_alarm_message_structure_t*) p_data->data;
   if(is_msg_alarm(alarm_struct_sos->msg_type))
   {
      NRF_LOG_ERROR("Alarm sos message type:%u\r\n",alarm_struct_sos->msg_type);
      /*LOG INFOMATION*/
      m_last_alarm_addr = p_data->mesh_id;
      if(xSystem.flash_parameter.config_parameter.AlarmConfig.Name.EnableSyncAlarm
         || alarm_struct_sos->dev_type == APP_DEVICE_GW)
      {

        /*Set Alarm Lamp on the alarm's source*/
        app_mesh_publish_set_lamp(p_data->mesh_id, true);

        /*Wait for ACK*/
        app_mesh_publish_set_sos_ack();
        /*Set ACK*/
        app_queue_mesh_insert_node(p_data->mac, alarm_struct_sos, 1, NULL, 0);
        if(alarm_struct_sos->dev_type != APP_DEVICE_GW)
        {
          node_alarm_store_t dev;
          dev.is_alarm = 1;
          dev.appropriate_tick_get_alarm = nrf_get_tick();
          memcpy(dev.mac_add, p_data->mac, 6);
          dev.mesh_id = p_data->mesh_id;
          dev.is_valid_data = 1;
          if(node_alarm_find_and_write(&dev) == NODE_INVALID_INDEX)
          {
            NRF_LOG_ERROR("Device alarm already existed in the list\r\n");
          }
        }
      }

   }/*Set the System syate*/
   else if(alarm_struct_sos->msg_type == APP_MSG_TYPE_KEEP_ALIVE)
   {
     //TODO Update Note information
     app_queue_mesh_insert_node(p_data->mac, alarm_struct_sos, 0, NULL, 0);
   }
   else if(alarm_struct_sos->msg_type == APP_MSG_TYPE_ACK)
   {
      //TODO Reset ack
   }
   /*Ðây là tru?ng h?p node g?i tr?ng thái không báo d?ng n?a*/
   else if(alarm_struct_sos->dev_type != APP_DEVICE_GW &&
           alarm_struct_sos->msg_type == APP_MSG_TYPE_NO_ALARM)
   {
    //TODO Reset Alarm cache 
    app_queue_mesh_insert_node(p_data->mac, alarm_struct_sos, 0, NULL, 0);
    if(alarm_struct_sos->dev_type != APP_DEVICE_GW)
    {
      node_alarm_store_t dev;
      dev.is_alarm = 0;
      dev.appropriate_tick_get_alarm = nrf_get_tick();
      memcpy(dev.mac_add, p_data->mac, 6);
      dev.mesh_id = p_data->mesh_id;
      dev.is_valid_data = 0;
      if(node_alarm_find_and_write(&dev) == NODE_INVALID_INDEX)
      {
        NRF_LOG_ERROR("Device alarm already existed in the list\r\n");
      }
    }
   }
   /*Ðây là tru?ng h?p b?m nút và thay d?i tr?ng thái chuông denf*/
   else if(alarm_struct_sos->dev_type == APP_DEVICE_GW &&
           alarm_struct_sos->msg_type == APP_MSG_TYPE_NO_ALARM)
   {
    //TODO Reset Alarm cach
      app_queue_mesh_insert_node(p_data->mac, alarm_struct_sos, 0, NULL, 0);
   }
   else
   {
       NRF_LOG_WARNING("Mesh don't Process this message\r\n");
   }
}
/** @brief    
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
static void process_non_sos_data(gw_mesh_msq_t *p_data)
{
  if(p_data->data[1] == APP_DEVICE_POWER_METER)
  {
    //Update Node in the node array
    app_queue_mesh_insert_node(p_data->mac, NULL, 0, p_data->data, p_data->len);
  }
  else
  {
    NRF_LOG_ERROR("Unknown Device\r\n");
  }
}
/** @brief    
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
static inline bool is_msg_alarm(uint8_t msg_type)
{
    if (msg_type == APP_MSG_TYPE_FIRE_ALARM
      || msg_type == APP_MSG_TYPE_SMOKE_ALARM
      || msg_type == APP_MSG_TYPE_SOS_ALARM
      || msg_type == APP_MSG_TYPE_SPEAKER_ALARM
      || msg_type == APP_MSG_TYPE_TEMP_ALARM
      || msg_type == APP_MSG_TYPE_DOOR_ALARM
      || msg_type == APP_MSG_TYPE_PIR_ALARM
      || msg_type == APP_MSG_TYPE_SELF_TEST_ALARM
      || msg_type == APP_MSG_TYPE_TEMP_SMOKE_ALARM)
    {
        return true;
    }

    return false;
}
/** @brief   Insert the node alarm 
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
static int8_t node_alarm_find_and_write(node_alarm_store_t *alarm_node)
{
  bool is_already_exist = false;
  bool found_free_node = false;
  int8_t free_node_index = NODE_INVALID_INDEX;

  for(uint8_t i = 0; i < MAX_ALARM_SUPPORT; i++)
  {
    if(m_list_alarm[i].is_valid_data == 0
       && found_free_node == false)
    {
       found_free_node = true;
       free_node_index = i;
    }
    if(memcmp(alarm_node->mac_add, m_list_alarm[i].mac_add, 6) == 0 ||
       (alarm_node->mesh_id == m_list_alarm[i].mesh_id && alarm_node->mesh_id != 0xFF)) 
    {
      free_node_index  = NODE_INVALID_INDEX;
      is_already_exist = true;
      memcpy(&m_list_alarm[i], alarm_node , sizeof(node_alarm_store_t));
    }
  }
  if(is_already_exist == false)
  {
    memcpy(&m_list_alarm[free_node_index], alarm_node , sizeof(node_alarm_store_t));
  }
  return free_node_index;
}
/** @brief   Find the Alarm Node 
 *
 * @param[out]    
 * @param[in]   
 *
 * @retval     
 */
 static bool node_detect_available_alarm()
 {
    bool alarm = false;
    uint32_t current_tick = nrf_get_tick();
    for(uint8_t i = 0; i < MAX_ALARM_SUPPORT; i++)
    {  
       if(m_list_alarm[i].is_valid_data)
       {
         if(current_tick - m_list_alarm[i].appropriate_tick_get_alarm > ALARM_NODE_TIMEOUT)
         {
            memset(&m_list_alarm[i], 0, sizeof(node_alarm_store_t));
         }
         if(m_list_alarm[i].is_alarm)
         {
            alarm = true;
         }
       }
    }
    return alarm;
 }

 static void app_queue_mesh_insert_node(uint8_t *mac, 
                                        app_alarm_message_structure_t *alarm_sos_structure,
                                        uint8_t alarm_type,
                                        uint8_t *custom_data,
                                        uint8_t custom_data_size)
 {
    app_beacon_data_t local_node;
    
    if(!custom_data)
    {
      /*Copy MAC*/
      memcpy(local_node.device_mac, mac, 6);
      local_node.battery = alarm_sos_structure->battery_value;
      local_node.fw_verison = alarm_sos_structure->fw_version;
      local_node.smoke_value = alarm_sos_structure->reserve.temperature_smoke.smoke_alarm;
      local_node.teperature_value = alarm_sos_structure->reserve.temperature_smoke.temperature;
     // local_node.timestamp = Get RTC Value
      local_node.propreties.Name.alarmState = alarm_type & 0x01;
      local_node.propreties.Name.deviceType = alarm_sos_structure->dev_type;
      local_node.custom_data.IsCustom = 0; /*Khong phai custom data*/
      if(alarm_type || alarm_sos_structure->msg_type == APP_MSG_TYPE_NO_ALARM)
      {
         xSystem.alarm_value.Value &= 0x01;
         switch(alarm_sos_structure->dev_type)
         {
          case APP_DEVICE_SENSOR_FIRE_DETECTOR:
            xSystem.alarm_value.Name.ALARM_FIRE = 1;
            break;
          case APP_DEVICE_SENSOR_SMOKE_DETECTOR:
            xSystem.alarm_value.Name.ALARM_SMOKE = 1;
            break;
          case APP_DEVICE_BUTTON_SOS:
            xSystem.alarm_value.Name.ALARM_REMOTE = 1;
            break;
          case APP_DEVICE_SPEAKER:
            xSystem.alarm_value.Name.ALARM_BUZZER_LAMP = 1;
            break;
          case APP_DEVICE_SENSOR_TEMP_DETECTOR:
            xSystem.alarm_value.Name.ALARM_TEMP = 1;
            break;         
          case APP_DEVICE_SENSOR_DOOR_DETECTOR:
            xSystem.alarm_value.Name.ALARM_DOOR = 1;
            break;         
          case APP_DEVICE_SENSOR_PIR_DETECTOR:
            xSystem.alarm_value.Name.ALARM_PIR = 1;
            break;         
          case APP_DEVICE_SENSOR_TEMP_SMOKE:
            xSystem.alarm_value.Name.ALARM_COMBINE_TEMP_SMOKE = 1;
            break;         
           case APP_DEVICE_GW:
            xSystem.alarm_value.Name.ALARM_CENTER_BUTTON = 1;
            /*Gat way thi khong publish "D:*/
            break;
         }
      }         
    }
    else
    {
      memcpy(local_node.device_mac, mac, 6);
      //local_node.propreties.Name.deviceType = custom_data[0];
      if(custom_data[1] == APP_DEVICE_POWER_METER)
      {
        local_node.propreties.Name.deviceType = APP_DEVICE_POWER_METER;
        local_node.custom_data.Len = custom_data_size;
        local_node.custom_data.IsCustom = 1;
        //local_node.timestamp 
        local_node.propreties.Name.isNewMsg = 1;
        local_node.propreties.Name.isPairMsg = 0;
      }
      else
      {
        NRF_LOG_ERROR("Sorry!! We doesn't support this device\r\n");
      }
    }


    /*Write data to the list*/
    for(uint16_t index = 0; index <= xSystem.sensor_count; index++)
    {
      if(memcmp(&xSystem.Node_list[index].device_mac, mac, 6) == 0 )
      {
        memcpy(&xSystem.Node_list[index], &local_node, sizeof(app_beacon_data_t));
        xSystem.Node_list[index].is_data_valid = 1;
        if(alarm_sos_structure->dev_type == APP_DEVICE_SENSOR_TEMP_SMOKE)
        {
          xSystem.Node_list[index].propreties.Name.comboSensor = 1;
        }
        else if (alarm_sos_structure->dev_type == APP_DEVICE_GW) 
        {
          /*Just dont sent this message*/
          xSystem.Node_list[index].is_data_valid = 1;
        }
        return;
      }
    }
    xSystem.Node_list[xSystem.sensor_count].is_data_valid = 1;
    memcpy(&xSystem.Node_list[xSystem.sensor_count], &local_node, sizeof(app_beacon_data_t));
    if(xSystem.sensor_count >= APP_MAX_BEACON)
    {
      NRF_LOG_ERROR("Beacon Queue Full\r\n");
      return;
    }
    xSystem.sensor_count++;
    NRF_LOG_INFO("Insert new node data to the LIST: %u\r\n", xSystem.sensor_count);
 }
static void no_alarm_anymore(void)
{
    /*N?u không còn báo cháy thì clear alarm cache */
    if (node_detect_available_alarm() == false)
    {
        memset(m_list_alarm, 0, sizeof(m_list_alarm));
    }
    else
    {
        NRF_LOG_INFO("Node alarm still exist\r\n");
        return;
    }
    if(xSystem.flash_parameter.config_parameter.AlarmConfig.Name.EnableSyncAlarm)
    {
      app_mesh_publish_set_lamp(m_last_alarm_addr, 0);
    }
    for(uint32_t i = 0; i < APP_MAX_BEACON; i++)
    {
      xSystem.Node_list[i].propreties.Name.alarmState = 0;
      xSystem.Node_list[i].propreties.Name.isNewMsg = 0;
      xSystem.Node_list[i].is_data_valid = 0;
    }
    xSystem.alarm_value.Value = 0;
}

uint8_t app_queue_mesh_read_node(app_beacon_data_t *p_beacon)
{
  uint8_t node_index = 0;
  static uint8_t read_index = 0;
  for(uint16_t i = read_index; i < xSystem.sensor_count; i++)
  {
    if(xSystem.Node_list[i].is_data_valid)
    {
      memcpy(p_beacon, &xSystem.Node_list[i], sizeof(app_beacon_data_t));
      NRF_LOG_INFO("Read beacon in list: %d\r\n", i);
      xSystem.Node_list[i].is_data_valid = 0;
      if(read_index == xSystem.sensor_count - 1)
      {
        read_index = 0;
      }
      else
      {
        read_index = i;
      }
      return node_index;
    }
  }
  p_beacon = NULL;
  NRF_LOG_ERROR("Beacon list no data\r\n");
  return 0xFF;
}
void app_queue_process_polling_message()
{
  gw_mesh_msq_t rx_mesh;
  if(!app_mesh_gw_queue_pop(&rx_mesh))
  {
    NRF_LOG_INFO("Get message from queue to process\r\n");
    if(rx_mesh.msg_id == GW_MESH_MSQ_ID_ALARM_SYSTEM)
    {
      process_sos_data(&rx_mesh);
    }
    else if(rx_mesh.msg_id == GW_MESH_MSG_ID_UNKNOWN_SYSTEM)
    {
      //NRF_LOG_ERROR()
      process_non_sos_data(&rx_mesh);
    }
    else
    {
      NRF_LOG_ERROR("unknown message ID:%d at :%s\r\n",__FUNCTION__);
    }
  }
  else
  {
  }
}