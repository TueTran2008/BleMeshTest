#ifndef _APP_MESH_PROCESS_DATA_H_ 
#define _APP_MESH_PROCESS_DATA_H_
uint8_t app_queue_mesh_read_node(app_beacon_data_t *p_beacon);

void app_queue_process_polling_message();
#endif