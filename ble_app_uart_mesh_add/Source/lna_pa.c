#include "lna_pa.h"
#include "mesh_pa_lna.h"
#include "nrf_error.h"
#include "app_error.h"
#include <string.h>
static ble_common_opt_pa_lna_t m_pa_lna_params;
static ble_opt_t sd_opt;
void pa_lna_gpio_config(uint8_t gpio_pa_pin, uint8_t gpio_lna_pin)
{
    /* Configure PA/LNA for mesh */
    //ble_common_opt_pa_lna_t m_pa_lna_params;
    //memset(&m_pa_lna_params, 0, sizeof(ble_common_opt_pa_lna_t));

    //PA config
    m_pa_lna_params.pa_cfg.enable = 1;
    m_pa_lna_params.pa_cfg.active_high = 1;
    m_pa_lna_params.pa_cfg.gpio_pin = gpio_pa_pin;

    //LNA config
    m_pa_lna_params.lna_cfg.enable = 1;
    m_pa_lna_params.lna_cfg.active_high = 1;
    m_pa_lna_params.lna_cfg.gpio_pin = gpio_lna_pin;

    // Common PA/LNA config
    m_pa_lna_params.gpiote_ch_id  = 0;        // GPIOTE channel
    m_pa_lna_params.ppi_ch_id_clr = 1;        // PPI channel for pin clearing
    m_pa_lna_params.ppi_ch_id_set = 0;        // PPI channel for pin setting

    app_lna_pa_enable_module();

//    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "mesh_pa_lna_enable: %d\n", err);


    /* Config PA/LNA for BLE */	    
    //ble_opt_t opt;
    memset(&sd_opt, 0, sizeof(ble_opt_t));
	
    // Common PA/LNA confi0
    sd_opt.common_opt.pa_lna.gpiote_ch_id  = 0;        // GPIOTE channel
    sd_opt.common_opt.pa_lna.ppi_ch_id_clr = 1;        // PPI channel for pin clearing
    sd_opt.common_opt.pa_lna.ppi_ch_id_set = 0;        // PPI channel for pin setting
	
    // PA config
    sd_opt.common_opt.pa_lna.pa_cfg.active_high = 1;                // Set the pin to be active high
    sd_opt.common_opt.pa_lna.pa_cfg.enable      = 1;                // Enable toggling
    sd_opt.common_opt.pa_lna.pa_cfg.gpio_pin    = gpio_pa_pin;      // The GPIO pin to toggle
  
    // LNA config
    sd_opt.common_opt.pa_lna.lna_cfg.active_high  = 1;              // Set the pin to be active high
    sd_opt.common_opt.pa_lna.lna_cfg.enable       = 1;              // Enable toggling
    sd_opt.common_opt.pa_lna.lna_cfg.gpio_pin     = gpio_lna_pin;   // The GPIO pin to toggle

    app_lna_pa_enable_in_ble_stack();
}


void app_lna_pa_enable_module()
{
    uint32_t err = mesh_pa_lna_gpiote_enable(&m_pa_lna_params);
    APP_ERROR_CHECK(err);
}
void app_lna_pa_enable_in_ble_stack()
{
    uint32_t err_code = sd_ble_opt_set(BLE_COMMON_OPT_PA_LNA, &sd_opt);
    APP_ERROR_CHECK(err_code);
}
