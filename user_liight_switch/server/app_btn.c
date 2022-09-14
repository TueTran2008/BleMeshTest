#include "Hardware.h"
#include "app_timer.h"
#include "InitSystem.h"
/* Logging and RTT */
#include "log.h"
#include "rtt_input.h"
#include "nrf_gpiote.h"
//#include "nrf_gpio.h"


#include "nrf.h"
#include "nrf_error.h"
#include "boards.h"
#include "nrf_delay.h"
#include "nrfx_gpiote.h"
#include "sdk_common.h"
//#include "nrf_drv_clock.h"

#include "app_btn.h"
#include "timer.h"
#include "nrf_mesh_defines.h"
#include "simple_hal.h"

/******************************************************************************
                                   STATIC VARIABLES					    			 
 ******************************************************************************/
static hal_button_handler_cb_t m_button_handler_cb;

static uint8_t button_array[] = BTN_ARR;

static uint32_t m_last_button_pressed = 0;

static uint8_t button_name[2][16] = {"BUTTON_ALARM","BUTTON_USER"};



/******************************************************************************
                                   STATIC FUNCTION					    			 
******************************************************************************/

 static void button_handle(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(button_array); i++)
    {
        if (pin == button_array[i])
        {
            /* Check that the event was generated by a button press, and reject if it's too soon (debounce).
             */
            //if ((sys_get_ms() - m_last_button_pressed) > 50)
            //{
            //     m_last_button_pressed = sys_get_ms();
            //     DEBUG_INFO("button_name:%s\r\n",button_name[i]);
            //     //DEBUG_INFO("Button presse: %d\r\n", i);
            //    //m_last_button_pressed = timer_now();
            //    //m_button_handler_cb(i);
            //    //DEBUG_INFO("Button Pressed\r\n");
            //}
            if (TIMER_DIFF(m_last_button_pressed, timer_now()) > 2000000)
            {
                m_button_handler_cb(i);
                DEBUG_INFO("button_name:%s\r\n",button_name[i]);
                //m_last_button_pressed = timer_now();
                //m_button_handler_cb(i);
            }
        }
    }
}

 uint32_t user_button_init(hal_button_handler_cb_t cb)
{
    if (cb == NULL)
    {
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "No NR Callback BUtton\r\n");
        return NRF_ERROR_NULL;
    }
    if (GPIOTE_CONFIG_NUM_OF_LOW_POWER_EVENTS < BUTTONS_NUMBER)
    {
        return NRF_ERROR_NOT_SUPPORTED;
    }
    m_button_handler_cb = cb;

    if (!nrfx_gpiote_is_init())
    {
        APP_ERROR_CHECK(nrfx_gpiote_init());
    }

    nrfx_gpiote_in_config_t in_config = {
                                            .sense = NRF_GPIOTE_POLARITY_HITOLO,
                                            .pull = NRF_GPIO_PIN_PULLUP,
                                            .is_watcher = false,
                                            .hi_accuracy = false,
                                            .skip_gpio_setup = false
                                        };
    for (uint32_t i = 0; i < sizeof(button_array); i++)
    {
        APP_ERROR_CHECK(nrfx_gpiote_in_init(button_array[i], &in_config, button_handle));
        nrfx_gpiote_in_event_enable(button_array[i], true);
        //uint8_t a = nrf_gpio_pin_read(30);
    }
    //return NRF_SUCCESS;
}