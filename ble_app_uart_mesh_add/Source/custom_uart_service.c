///**
// * Copyright (c) 2014 - 2021, Nordic Semiconductor ASA
// *
// * All rights reserved.
// *
// * Redistribution and use in source and binary forms, with or without modification,
// * are permitted provided that the following conditions are met:
// *
// * 1. Redistributions of source code must retain the above copyright notice, this
// *    list of conditions and the following disclaimer.
// *
// * 2. Redistributions in binary form, except as embedded into a Nordic
// *    Semiconductor ASA integrated circuit in a product or a software update for
// *    such product, must reproduce the above copyright notice, this list of
// *    conditions and the following disclaimer in the documentation and/or other
// *    materials provided with the distribution.
// *
// * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
// *    contributors may be used to endorse or promote products derived from this
// *    software without specific prior written permission.
// *
// * 4. This software, with or without modification, must only be used with a
// *    Nordic Semiconductor ASA integrated circuit.
// *
// * 5. Any software provided in binary form under this license must not be reverse
// *    engineered, decompiled, modified and/or disassembled.
// *
// * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
// * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
// * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
// * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
// * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// *
// */
///** @file
// *
// * @defgroup ble_sdk_uart_over_ble_main main.c
// * @{
// * @ingroup  ble_sdk_app_nus_eval
// * @brief    UART over BLE application main file.
// *
// * This file contains the source code for a sample application that uses the Nordic UART service.
// * This application uses the @ref srvlib_conn_params module.
// */


//#include <stdint.h>
//#include <string.h>
//#include "nordic_common.h"
//#include "nrf.h"
//#include "ble_hci.h"
//#include "ble_advdata.h"
//#include "ble_advertising.h"
//#include "ble_conn_params.h"
//#include "nrf_sdh.h"
//#include "nrf_sdh_soc.h"
//#include "nrf_sdh_ble.h"
//#include "nrf_ble_gatt.h"
//#include "nrf_ble_qwr.h"
//#include "app_timer.h"
//#include "ble_nus.h"
//#include "app_uart.h"
//#include "app_util_platform.h"
//#include "bsp_btn_ble.h"
//#include "nrf_pwr_mgmt.h"

//#if defined (UART_PRESENT)
//#include "nrf_uart.h"
//#endif
//#if defined (UARTE_PRESENT)
//#include "nrf_uarte.h"
//#include "log.h"
//#endif

//#include "nrf_log.h"
//#include "nrf_log_ctrl.h"
//#include "nrf_log_default_backends.h"



///*user Include */
//#include "user_input.h"
////#include "led_driver.h"
////#include "user_on_off_client.h"
///* Mesh */
//#include "nrf_mesh.h"
//#include "mesh_adv.h"
//#include "ble_config.h"
//#include "ble_softdevice_support.h"
//#include "app_sef_provision.h"


//#include <string.h>



///*Node accesses service on Gateway */


//#define KEY_LENGHT    16

///*Mesh network info exchange between node and gateway*/
//typedef struct
//{
//   uint8_t app_key[KEY_LENGHT];
//   uint8_t net_key[KEY_LENGHT];
//   uint32_t unicast_address;
//   uint32_t sum32;
//}mesh_network_info_t;
//mesh_network_info_t gateway_info;


//static bool m_is_node_request_data = false;

//void node_nus_data_handler(ble_nus_evt_t *p_evt)
//{
//  if(p_evt->type == BLE_NUS_EVT_RX_DATA)
//  {
//    if(p_evt->params.rx_data.p_data && p_evt->params.rx_data.length)
//    {
//        NRF_LOG_DEBUG("Received data from BLE NUS. Writing data on UART.");
//        NRF_LOG_HEXDUMP_DEBUG(p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);
//        if(strstr(p_evt->params.rx_data.p_data, "REQ"))
//        {
//          m_is_node_request_data = true;
//        }
//    }
//  }
//  else if(p_evt->type == BLE_NUS_EVT_COMM_STARTED)
//  {
//    /**/
//    if(m_is_node_request_data)
//    {
//      ble_nus_data_send(&m_nus, (uint8_t*)&gateway_info, sizeof(mesh_network_info_t), m_conn_handle);
//    }
//  }
//}



