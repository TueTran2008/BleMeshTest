#ifndef _APP_MESH_CHECK_DUPLICATE_H_
#define _APP_MESH_CHECK_DUPLICATE_H_
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "DataDefine.h"



/**
 * @brief Convert device type to string
 * @param device_type   Device type
 * @retval Device type in string format
 */
const char * app_mesh_msg_map_device_type_to_string_type(uint8_t device_type);


///**
// * @brief Unpack raw data received from other device in to application message type
// * @param raw       Raw data received
//   @param result    Pointer to application output data
// */
//void app_mesh_unpack_raw_data(uint32_t raw, app_alarm_message_structure_t * result);
//
///**
// * @brief Pack application message type to raw data  to send into mesh network
// * @param msg       Applicaiton message structure value
//   @param result    Pointer to raw output data
// */
//void app_mesh_pack_raw_data(app_alarm_message_structure_t msg, uint32_t * raw);

/**
 * @brief Init mesh network tid for all message
 */
void app_mesh_tid_init();

/**
 * @brief Check if message is duplicated
 * @param id   Message id
 * @retval true If new message is duplicated
 *         false If new message is not duplicated
 */
bool app_mesh_tid_is_duplicate(app_transaction_t * id);

/**
 * @brief Insert transaction id into list
 * @param id   Message id
 */
void app_mesh_insert_tid(app_transaction_t * id);
#endif