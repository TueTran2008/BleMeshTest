#ifndef _APP_MESSAGE_QUEUE_H_
#define _APP_MESSAGE_QUEUE_H_
#include "DataDefine.h"

#define MESH_EVENT_SIZE (8)
//#define APP_MESH_EVT_MAX_DATA_LEN 24


typedef struct __attribute((packed))
{
    uint8_t id;
    uint8_t len;
    uint8_t data[24];
}app_mesh_tx_evt_t;

/** @brief  Write TX Message to Queue
 *
 * @param[in]   
 *
 * @retval     
 */

uint32_t app_mesh_tx_evt_message_queue_push(void *p_data);
/** @brief  Read Message from Queue
 *
 * @param[in]   
 *
 * @retval     
 */

uint32_t app_mesh_tx_evt_message_queue_pop(void* p_data);
/** @brief  Check if Queue is available to read
 *
 * @retval     
 */

bool app_mesh_tx_evt_message_queue_is_empty();
/** @brief  Check if Queue is available to write
 * 
 * @retval     
 */
bool app_mesh_tx_evt_message_queue_is_full();


#endif