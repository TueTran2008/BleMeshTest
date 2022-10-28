/*****************************************************************************
 * Includes
 *****************************************************************************/
#include "nrf.h"
#include "nrf_drv_saadc.h"
#include "log.h"
#include "nrf_log.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"
#include "app_btn.h"
#include "adc_button.h"
#include "led_driver.h"
#include "DataDefine.h"
#include "nrf_queue.h"
#include "nrf_log_ctrl.h"
#include "nrf_gpiote.h"
#include "nrfx_gpiote.h"
#include "ble_uart_service.h"

/*****************************************************************************
 * Macros
 *****************************************************************************/
#define SFUL_PRO_ADV_BUTTON    NRF_SAADC_INPUT_AIN5
#define SFUL_PRO_ADC_3V8       NRF_SAADC_INPUT_AIN6
#define SFUL_PRO_ADC_PW        NRF_SAADC_INPUT_AIN7
#define SFUL_PRO_ADC_VBAT      NRF_SAADC_INPUT_AIN3

#define ADC_BUTTON_CHANNEL_INDEX  0
#define ADC_3V8_CHANNEL_INDEX     1
#define ADC_PW_CHANNEL_INDEX      2
#define ADC_VBAT_CHANNEL_INDEX    3

#define ADC_TOTAL_CHANNEL         4
#define ADC_CONFIG_GAIN           4
#define ADC_RESOLUTION_TO_VALUE            4095


#define ADC_BUTTON_0_MIN_VALUE  270 //(450)
#define ADC_BUTTON_0_MAX_VALUE  320

#define ADC_BUTTON_1_MIN_VALUE  380 //623
#define ADC_BUTTON_1_MAX_VALUE  420

#define ADC_BUTTON_2_MIN_VALUE  460
#define ADC_BUTTON_2_MAX_VALUE  510

#define ADC_BUTTON_3_MIN_VALUE  740
#define ADC_BUTTON_3_MAX_VALUE  790

#define ADC_BUTTON_4_MIN_VALUE  2200
#define ADC_BUTTON_4_MAX_VALUE  2500

#define ADC_BUTTON_5_MIN_VALUE  970
#define ADC_BUTTON_5_MAX_VALUE  1050

#define ADC_MAX_CELL_VALUE      2960

#define DEBUG_ADC   0
/*****************************************************************************
 * Typedef Decleration
 *****************************************************************************/
typedef union
{
  struct{
    uint8_t btn0 : 1;
    uint8_t btn1 : 1;
    uint8_t btn2 : 1;
    uint8_t btn3 : 1;
    uint8_t btn4 : 1;
    uint8_t btn5 : 1;
    uint8_t reserved : 2;
   }name;
   uint8_t value;
}adc_button_t;

/*****************************************************************************
 * Static Variables
 *****************************************************************************/
static custom_buttom_state_t button_transmit_state[6] =
{
  BUTTON_IDLE, BUTTON_IDLE, BUTTON_IDLE, BUTTON_IDLE, BUTTON_IDLE, BUTTON_IDLE
};
static custom_buttom_state_t m_last_button_transmit_state[6] =
{
  BUTTON_IDLE, BUTTON_IDLE, BUTTON_IDLE, BUTTON_IDLE, BUTTON_IDLE, BUTTON_IDLE
};
static uint16_t m_vbat_voltage = 0;
static uint16_t m_pw_voltage = 0;
static uint16_t m_pw_3V8 = 0;
bool protect_button = false;
static uint32_t m_button_get_ms = 0;
static nrf_saadc_value_t     m_buffer_pool[4];
static const nrf_drv_timer_t m_timer = NRF_DRV_TIMER_INSTANCE(1);
static nrf_ppi_channel_t     m_ppi_channel = NRF_PPI_CHANNEL4;
static uint32_t              m_adc_evt_counter;
static adc_button_t m_adc_button;
static app_btn_hw_config_t button_hw_cfg[] =
{
  {0, 1, 0},
  {1, 1, 0},
  {2, 1, 0},
  {3, 1, 0},
  {4, 1, 0},
  {5, 1, 0}
};

//const static nrf_saadc_channel_config_t[] = {SFUL_PRO_ADV_BUTTON, SFUL_PRO_ADC_3V8, SFUL_PRO_ADC_PW, SFUL_RPO_ADC_VBAT};
/*****************************************************************************
 * Forward Declaration
 *****************************************************************************/
static void timer_handler(nrf_timer_event_t event_type, void * p_context);
static uint16_t adc_raw_to_mv(int16_t raw_value);
static uint32_t btn_read(uint32_t pin);
static uint32_t button_get_100ms();
static void saadc_sampling_event_init(void);
static void adc_value_button_handle(uint16_t button_mv);
static void on_btn_pressed(int index, int event, void* p_args);
static void on_btn_release(int index, int event, void* p_args);
static void on_btn_hold_so_long(int index, int event, void* p_args);
static void on_btn_hold(int index, int event, void* p_args);
/*<Button Queue Function>*/
static uint32_t app_button_adc_queue_push(void *p_data);
static uint32_t app_button_adc_queue_pop(void* p_data);
static bool app_button_adc_queue_pop_is_empty();
static bool app_button_adc_queue_pop_is_full();
 /*****************************************************************************
 * Function Implementation 
 *****************************************************************************/
/**
 * @brief Creat connection between Timer and ADC Sampling
 */
void saadc_sampling_event_enable(void)
{
    ret_code_t err_code = nrf_drv_ppi_channel_enable(m_ppi_channel);
    APP_ERROR_CHECK(err_code);
}
/**
 * @brief Timer Initilization 100ms Cycle
 */
static void saadc_sampling_event_init(void)
{
    ret_code_t err_code;

    err_code = nrf_drv_ppi_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_32;
    err_code = nrf_drv_timer_init(&m_timer, &timer_cfg, timer_handler);
    APP_ERROR_CHECK(err_code);

    /* setup m_timer for compare event every 400ms */
    uint32_t ticks = nrf_drv_timer_ms_to_ticks(&m_timer, 100);
    nrf_drv_timer_extended_compare(&m_timer,
                                   NRF_TIMER_CC_CHANNEL0,
                                   ticks,
                                   NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK,
                                   false);
    nrf_drv_timer_enable(&m_timer);

    uint32_t timer_compare_event_addr = nrf_drv_timer_compare_event_address_get(&m_timer,
                                                                                NRF_TIMER_CC_CHANNEL0);
    uint32_t saadc_sample_task_addr   = nrf_drv_saadc_sample_task_get();
    /*

      NOTE:
      I had changed the NRFX_PPI Driver . Alloc channel start from Channel_0 to Channel_2.
      Because Channel_0 used for PA Pin and Channel_1 used for LNA_Pin => I don't know why 
      nrf_drv_ppi_channel_alloc couldn't detect those used channel.  
    */
    /* setup ppi channel so that timer compare event is triggering sample task in SAADC */
    err_code = nrf_drv_ppi_channel_alloc(&m_ppi_channel);
    APP_ERROR_CHECK(err_code);
    //m_ppi_channel = NRF_PPI_CHANNEL4;
    err_code = nrf_drv_ppi_channel_assign(m_ppi_channel,
                                          timer_compare_event_addr,
                                          saadc_sample_task_addr);
    APP_ERROR_CHECK(err_code);
}
/**
 * @brief Handle SAADC Event 
 */
void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
    if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
    {
        ret_code_t err_code;
        err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, 4);
        APP_ERROR_CHECK(err_code);
        uint16_t mv[4] = {0};
        //NRF_LOG_INFO("ADC event number: %d", (int)m_adc_evt_counter);
        for (int i = 0; i < 4; i++)
        {   
            mv[i] = adc_raw_to_mv(p_event->data.done.p_buffer[i]);
            if(i == 0)
            {
#if(DEBUG_ADC)
              NRF_LOG_INFO("Button Voltage:%d", mv[0]);
#endif
              adc_value_button_handle(mv[i]);
            }
            //else if(i == 4)
            //{
            //  m_vbat_voltage = mv[i];
            //}
        }
        //NRF_LOG_INFO("\r\nButtonVoltage:%d\n3V8Voltage:%d\r\nPowerVoltage:%d\r\nBattery Voltage:%d", mv[0]
        //                                                                                             , mv[1]
        //                                                                                             , mv[2]
        //                                                                                             , mv[3]);
    }
    else if(p_event->type == NRFX_SAADC_EVT_CALIBRATEDONE)
    {
    }
}
/**
 * @brief 
 * @param[in] Init SAADC with 4 channels
 */
static void saadc_init(void)
{
    ret_code_t err_code;
    /*Channel Config*/
    nrf_saadc_channel_config_t adc_button = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(SFUL_PRO_ADV_BUTTON);
    nrf_saadc_channel_config_t adc_3v8 = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(SFUL_PRO_ADC_3V8);
    nrf_saadc_channel_config_t adc_pw = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(SFUL_PRO_ADC_PW);
    nrf_saadc_channel_config_t adc_vbat = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(SFUL_PRO_ADC_VBAT);
    adc_button.gain = NRF_SAADC_GAIN1_6;
    /*SAADC Init*/
    nrf_saadc_channel_config_t adc_channel_cfg[] = {adc_button, adc_3v8, adc_pw,adc_vbat};
    err_code = nrf_drv_saadc_init(NULL, saadc_callback);
    APP_ERROR_CHECK(err_code);
    /*ADC Channel Config*/
    for(uint8_t i = 0; i < ADC_TOTAL_CHANNEL; i++)
    {
      err_code = nrf_drv_saadc_channel_init(i, &adc_channel_cfg[i]);
      APP_ERROR_CHECK(err_code);
    }
    for(uint8_t i = 0; i < 1; i++)
    {
      err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool, 4);
      NRF_LOG_INFO("Config ADC index :%d", i);
      APP_ERROR_CHECK(err_code);
    }
}
/**
 * @brief Button initialize callback. No Implemetation needed
 */
static void button_initialize(uint32_t button_num)
{
    //NO Implemetation needed
    //return
}
/**
 * @brief Monitoring state of all Button to send to the host
 * @return Buffer state of ADC Button
 */
 custom_buttom_state_t * app_button_adc()
 {
    return button_transmit_state;
 }
/**
 * @brief 
 * @param[in] 
 * @param[in] 
 * @param[in] 
 */
void adc_init()
{
  app_btn_config_t m_button_config;
  m_button_config.btn_initialize = NULL;
  m_button_config.btn_count = 6;
  m_button_config.config = /*(button_hw_cfg*)*/button_hw_cfg;
  m_button_config.btn_initialize = button_initialize;
  m_button_config.btn_read = btn_read;
  m_button_config.get_tick_cb = nrf_get_tick;
  app_btn_initialize(&m_button_config);
  app_btn_register_callback(APP_BTN_EVT_HOLD, on_btn_hold, NULL);
  app_btn_register_callback(APP_BTN_EVT_HOLD_SO_LONG, on_btn_hold_so_long, NULL);
  app_btn_register_callback(APP_BTN_EVT_PRESSED, on_btn_pressed, NULL);
  app_btn_register_callback(APP_BTN_EVT_RELEASED, on_btn_release, NULL);    
  saadc_init();
  saadc_sampling_event_init();
  saadc_sampling_event_enable();
}
/**
 * @brief Transfer Raw ADC Data to millies voltage level
 * @param[in] raw_value: ADC Raw measured value
 * @return Voltage level after calculated
 */
static uint16_t adc_raw_to_mv(int16_t raw_value)
{
  if(raw_value < 0)
  {
      raw_value = 0;
  }
  int16_t ret_value = 0;
  ret_value = (600  * ADC_CONFIG_GAIN) * raw_value / ADC_RESOLUTION_TO_VALUE;
  return ret_value;
}
/**
 * @brief: Read the pin state
 * @param[in]: Pin index
 */
static uint32_t btn_read(uint32_t pin)
{
  switch(pin)
  {
      case 0:
        return m_adc_button.name.btn0;
      case 1:
        return m_adc_button.name.btn1;
      case 2:
        return m_adc_button.name.btn2;
      case 3:
        return m_adc_button.name.btn3;
      case 4:
        return m_adc_button.name.btn4;
      case 5:
        return m_adc_button.name.btn5;
      default:
        return 0;
  }
}
/**
 * @brief: Determine which button is pressded
 * @param[in]: Pin index
 */
static void adc_value_button_handle(uint16_t button_mv)
{
  if(button_mv >= ADC_BUTTON_0_MIN_VALUE && button_mv <= ADC_BUTTON_0_MAX_VALUE)
  {
      m_adc_button.value = 0;
      m_adc_button.name.btn0 = 1;
  }
  else if(button_mv >= ADC_BUTTON_1_MIN_VALUE && button_mv <= ADC_BUTTON_1_MAX_VALUE)
  {
      m_adc_button.value = 0;
      m_adc_button.name.btn1 = 1;
  }
  else if(button_mv >= ADC_BUTTON_2_MIN_VALUE && button_mv <= ADC_BUTTON_2_MAX_VALUE)
  {
      m_adc_button.value = 0;
      m_adc_button.name.btn2 = 1;
  }
  else if(button_mv >= ADC_BUTTON_3_MIN_VALUE && button_mv <= ADC_BUTTON_3_MAX_VALUE)
  {
      m_adc_button.value = 0;
      m_adc_button.name.btn3 = 1;
  }
  else if(button_mv >= ADC_BUTTON_4_MIN_VALUE && button_mv <= ADC_BUTTON_4_MAX_VALUE)
  {
      m_adc_button.value = 0;
      m_adc_button.name.btn4 = 1;
  }
  else if(button_mv >= ADC_BUTTON_5_MIN_VALUE && button_mv <= ADC_BUTTON_5_MAX_VALUE)
  {
      m_adc_button.value = 0;
      m_adc_button.name.btn5 = 1;
  }
  else
  {
      m_adc_button.value = 0;
  }
}
//APP_TIMER_DEF(btn_timer);
//static void test(void *p_args)
//{
//  nrfx_gpiote_out_set(SFUL01_LED_ALARM_PIN);
//  app_ble_enter_pair_mode();
//}
/**
 * @brief Button Pressed Event Handle
 * @param[in]: Index - Pin Number 
 * @param[in]: Event - Event Handle type 
 * @param[in]: p_args - Pointer to arguments need for handling event
 */
static void on_btn_pressed(int index, int event, void* p_args)
{
  NRF_LOG_ERROR("Button %d press", index);
  memset(button_transmit_state, 0, sizeof(button_transmit_state));
  //button_transmit_state[index] = BUTTON_PRESSED;
  queue_button_t local_button;
  memset(&local_button, 0, sizeof(local_button));
  //local_button.name.btn_state = BUTTON_PRESSED;
  local_button.button_state[index] = BUTTON_PRESSED;

  if(index == 1)
  {
    if(!(xSystem.alarm_value.Value))
    {
      //gw_mesh_msq_t send_to_devices;
      app_alarm_message_structure_t alarm_struct_sos;
      memset(&alarm_struct_sos, 0, sizeof(alarm_struct_sos));
      alarm_struct_sos.battery_value = app_adc_get_battery_percent();
      alarm_struct_sos.fw_version = FIRMWARE_VERSION;
      alarm_struct_sos.dev_type = APP_DEVICE_GW;
      alarm_struct_sos.msg_type = APP_MSG_TYPE_SELF_TEST_ALARM;

      gw_mesh_msq_t to_msg_queue;
      memset(&to_msg_queue, 0, sizeof(to_msg_queue));
      memcpy(to_msg_queue.mac, app_ble_get_mac(), 6);
      to_msg_queue.mesh_id = PAIR_INFO_GW_UNICAST_ADDR;
      memcpy(to_msg_queue.data, &alarm_struct_sos, sizeof(alarm_struct_sos));
      to_msg_queue.msg_id = GW_MESH_MSQ_ID_ALARM_SYSTEM;
      to_msg_queue.len = sizeof(alarm_struct_sos);
      NRF_LOG_WARNING("Gateway Self Alarm");
      if(app_mesh_gw_queue_push(&to_msg_queue))
      {
        gw_mesh_msq_t read_msg;
        app_mesh_gw_queue_pop(&read_msg);
        app_mesh_gw_queue_push(&to_msg_queue);
      }
    }
    else if(xSystem.alarm_value.Name.ALARM_CENTER_BUTTON)
    {
       NRF_LOG_INFO("/++++++++++++++++NO CENTER ALARM BUTTON ***************/");
       app_alarm_message_structure_t alarm_struct_sos;
       memset(&alarm_struct_sos, 0, sizeof(alarm_struct_sos));
       alarm_struct_sos.battery_value = app_adc_get_battery_percent();
       alarm_struct_sos.fw_version = FIRMWARE_VERSION;
       alarm_struct_sos.dev_type = APP_DEVICE_GW;
       alarm_struct_sos.msg_type = APP_MSG_TYPE_NO_ALARM;

       gw_mesh_msq_t to_msg_queue;
       memset(&to_msg_queue, 0, sizeof(to_msg_queue));
       memcpy(to_msg_queue.mac, app_ble_get_mac(), 6);
       to_msg_queue.mesh_id = PAIR_INFO_GW_UNICAST_ADDR;
       memcpy(to_msg_queue.data, &alarm_struct_sos, sizeof(alarm_struct_sos));
       to_msg_queue.msg_id = GW_MESH_MSQ_ID_ALARM_SYSTEM;
       to_msg_queue.len = sizeof(alarm_struct_sos);
       if(app_mesh_gw_queue_push(&to_msg_queue))
       {
          gw_mesh_msq_t read_msg;
          app_mesh_gw_queue_pop(&read_msg);
          app_mesh_gw_queue_push(&to_msg_queue);
       }
     }
    
  else
  {
      //gw_mesh_msq_t send_to_devices;
      NRF_LOG_WARNING("Gateway No Alarm");
      //app_mesh_gw_create_message(APP_MSG_TYPE_SELF_TEST_ALARM, GW_MESH_MSQ_ID_ALARM_SYSTEM, PAIR_INFO_GW_UNICAST_ADDR, &send_to_devices);
   }
 }


  if(!app_button_adc_queue_push(&local_button))
  {
    queue_button_t read_button;
    app_button_adc_queue_pop(&read_button);
    app_button_adc_queue_push(&local_button);
  }
 
}
/**
 * @brief Button Pressed Event Handle
 * @param[in]: Index - Pin Number 
 * @param[in]: Event - Event Handle type 
 * @param[in]: p_args - Pointer to arguments need for handling event
 */
static void on_btn_release(int index, int event, void* p_args)
{
  NRF_LOG_WARNING("Button %d release", index);
  memset(button_transmit_state, 0, sizeof(button_transmit_state));
  //button_transmit_state[index] =  BUTTON_IDLE;
  queue_button_t local_button;
  memset(&local_button, 0, sizeof(local_button));
  //local_button.name.btn_state = BUTTON_PRESSED;
  local_button.button_state[index] = BUTTON_IDLE;
  if(!app_button_adc_queue_push(&local_button))
  {
    queue_button_t read_button;
    app_button_adc_queue_pop(&read_button);
    app_button_adc_queue_push(&local_button);
  }
}
/**
 * @brief Button Hold so long Event Handle
 * @param[in]: Index - Pin Number 
 * @param[in]: Event - Event Handle type 
 * @param[in]: p_args - Pointer to arguments need for handling event
 */
static void on_btn_hold_so_long(int index, int event, void* p_args)
{
  NRF_LOG_WARNING("Button %d hold so long", index);
  if(index == 0 )
  {
    nrfx_gpiote_out_set(SFUL01_LED_ALARM_PIN);
    app_ble_enter_pair_mode();
  }
  memset(button_transmit_state, 0, sizeof(button_transmit_state));
  queue_button_t local_button;
  memset(&local_button, 0, sizeof(local_button));
  //local_button.name.btn_state = BUTTON_PRESSED;
  local_button.button_state[index] = BUTTON_HOLD;
  if(!app_button_adc_queue_push(&local_button))
  {
    queue_button_t read_button;
    app_button_adc_queue_pop(&read_button);
    app_button_adc_queue_push(&local_button);
  }
}
/**
 * @brief Button Hold Event Handle
 * @param[in]: Index - Pin Number 
 * @param[in]: Event - Event Handle type 
 * @param[in]: p_args - Pointer to arguments need for handling event
 */
static void on_btn_hold(int index, int event, void* p_args)
{
  NRF_LOG_ERROR("Button %d HOLD ONLY", index);
  memset(button_transmit_state, 0, sizeof(button_transmit_state));
  button_transmit_state[index] = BUTTON_HOLD;
}

/*Timer 100ms tick handler*/
static void timer_handler(nrf_timer_event_t event_type, void * p_context)
{
  /*Increase Every 100ms*/
  m_button_get_ms++;
  NRF_LOG_INFO("button_get_ms");
}
static uint32_t button_get_100ms()
{
  return m_button_get_ms;
}
/*Button Queue*/

/*
*/
NRF_QUEUE_DEF(queue_button_t, app_button_queue_q, 32, NRF_QUEUE_MODE_OVERFLOW);
static uint32_t app_button_adc_queue_push(void *p_data)
{
  uint32_t retval = nrf_queue_push((nrf_queue_t*)&app_button_queue_q, p_data);
  if(retval != NRF_SUCCESS)
  {
    NRF_LOG_ERROR("Button queue failed:%d at func :%s", retval, __FUNCTION__);
    NRF_LOG_FLUSH();
  }
  return retval;
}
static uint32_t app_button_adc_queue_pop(void* p_data)
{
  uint32_t retval = nrf_queue_pop((nrf_queue_t*)&app_button_queue_q, p_data);
  if(retval != NRF_SUCCESS)
  {
    //NRF_LOG_ERROR("Queue error:%d at func :%s", retval, __FUNCTION__);
  }
  return retval;
}
static bool app_button_adc_queue_pop_is_empty()
{
  return nrf_queue_is_empty((nrf_queue_t*)&app_button_queue_q);
}
static bool app_button_adc_queue_pop_is_full()
{
  return nrf_queue_is_full((nrf_queue_t*)&app_button_queue_q);
}

bool app_button_get_state(queue_button_t *p_btn_state)
{
  if(app_button_adc_queue_pop(p_btn_state))
  {
    return false;
  }
  else
  {
    return true;
  }
}
uint8_t app_adc_get_battery_percent()
{
  uint16_t temp;
  temp = (m_vbat_voltage * 100)/ (ADC_MAX_CELL_VALUE);
  return (uint8_t)(temp/100);
}



