#ifndef _APP_GATEWAY_MESSAGE_H_
#define _APP_GATEWAY_MESSAGE_H_



#define  GW_MESH_MSQ_DATA_MAX_SIZE      (24)
#define  GW_MESH_MSQ_ID_ALARM_SYSTEM    0x00
#define  GW_MESH_MSG_ID_UNKNOWN_SYSTEM  0x01

#define MAX_MESSAGE_RECEIVE_FROM_MESH_NETWORK_IN_QUEUE          (24)


typedef union __attribute((packed))
{
    struct __attribute((packed))
    {
      uint8_t smoke_alarm : 1;
      uint8_t temperature : 7;
    } temperature_smoke;
    uint8_t raw;
} alarm_reserve_t;

typedef struct 
{
    uint8_t             mac[6];
    uint16_t            mesh_id;
    uint8_t             data[GW_MESH_MSQ_DATA_MAX_SIZE];
    uint8_t             len;
    uint8_t             msg_id;

}gw_mesh_msq_t;


/*                32 bit raw value               */
/* [31-24]            [23 - 16]           [15 - 8]                   [7 -- 0] */
/*Reserver        -   msg_type------------device_type---------------Bit 7 : charge status, Bit[6-0] : battery value [0 - 100]*/
typedef struct __attribute((packed))
{
    uint8_t fw_version;    // 0 : Discard, 1 : Charging
    uint8_t battery_value;    // From 0 - 100, first 1 byte is battery_charge status
    uint8_t dev_type;
    uint8_t msg_type;
    alarm_reserve_t reserve;      // for combine temperature + smoke sensor, bit 7 is smoke alarm
} app_alarm_message_structure_t;

typedef struct __attribute((packed))
{
    uint8_t mac[6];
    app_alarm_message_structure_t  alarm;
} app_mesh_alarm_message_structure_t;


/** @brief  Write TX Message to Queue
 *
 * @param[in]   
 *
 * @retval     
 */
uint32_t app_mesh_gw_queue_push(void *p_data);
/** @brief  Read Message from Queue
 *
 * @param[in]   
 *
 * @retval     
 */
uint32_t app_mesh_gw_queue_pop(void* p_data);
/** @brief  Check if Queue is available to read
 *
 * @retval     
 */
bool app_mesh_gw_queue_is_empty();
/** @brief  Check if Queue is available to write
 * 
 * @retval     
 */
bool app_mesh_gw_queue_is_full();
/** @brief  Create message gateway Type
 * 
 * @retval     
 */
void app_mesh_gw_create_message(uint8_t msg_type, uint8_t msg_id, uint8_t mesh_id, gw_mesh_msq_t *p_out);
#endif