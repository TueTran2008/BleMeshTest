#include "flash_if.h"
#include "ota_update.h"
#include "flash_if.h"
#include "nrf_dfu_flash.h"
#include "nrf_error.h"
#include "ota_update.h"
#include <stdint.h>
#include <stdio.h>
/*This is implementation for nRF52*/

#define ABS_RETURN(x, y) (((x) < (y)) ? (y) : (x))

bool flash_if_init(void)
{
    return true;
}

uint32_t flash_if_erase(uint32_t addr, uint32_t pages)
{
    uint32_t ret_code = 0;
    uint32_t number_of_page_erase = pages / FLASH_IF_PAGE_SIZE;
    ret_code = nrf_dfu_flash_erase(addr, number_of_page_erase, NULL);
    return ret_code;
}
uint32_t flash_if_copy(uint32_t destination, uint32_t *source, uint32_t nb_of_word)
{
    uint32_t ret_code = 0;
    ret_code = nrf_dfu_flash_store(destination, source, nb_of_word, NULL);
    return ret_code;
}
