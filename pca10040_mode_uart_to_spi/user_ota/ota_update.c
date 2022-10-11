#include "ota_update.h"

#include <string.h>
#include "flash_if.h"
#include "nrf_log.h"


#define OTA_FUNC_PLACE(x)				x		// RAMFUNCTION_SECTION_CODE
#define OTA_TIMEOUT_MS                  (60000)

typedef struct
{
    __attribute__((aligned(4))) uint8_t data[FLASH_IF_SECTOR_SIZE];      // ensure align 4
    uint32_t size;
}ota_bytes_remain_t;

static uint32_t m_expected_fw_size = 0;
static bool m_found_header = false;
static uint32_t m_current_write_size = 0;
static bool m_ota_is_running = false;
static ota_bytes_remain_t m_ota_remain;
volatile uint32_t ota_timeout_ms;
uint32_t m_crc;

bool ota_update_is_running(void)
{
	return m_ota_is_running;
}

uint8_t ota_get_downloaded_percent(void)
{
    if (m_expected_fw_size == 0)
    {
        return 0;
    } 

    return (m_current_write_size * 100)/ m_expected_fw_size;
}

bool ota_update_start(uint32_t expected_size)
{
    bool retval = true;
    // NRF_LOG_INFO("Firmware size %u bytes, download to addr 0x%08X\r\n", expected_size, OTA_UPDATE_DOWNLOAD_IMAGE_START_ADDR);
    if (ota_timeout_ms == 0)
    {
        ota_timeout_ms = OTA_TIMEOUT_MS;
        m_crc = 0;
    }
    if (expected_size > OTA_UPDATE_APPLICATION_SIZE)
    {
        retval = false;
    }
    else
    {
        m_expected_fw_size = expected_size; // - OTA_UPDATE_DEFAULT_HEADER_SIZE;
        m_current_write_size = 0;
        memset(&m_ota_remain, 0, sizeof(m_ota_remain));
        if (retval)
        {
            flash_if_error_t err = FLASH_IF_OK;
            if (err != FLASH_IF_OK)
            {
                NRF_LOG_INFO("Erase flash error\r\n");
                retval = false;
            }

            m_found_header = false;
            m_ota_is_running = true;
        }
    }
    return retval;
}

 /**
  * Download firmware region detail
  * -------------------------------
  *         16 bytes header
  * -------------------------------  
  *         Raw firmware
  * -------------------------------
  *         4 bytes CRC32
  * -------------------------------
  */
bool ota_update_write_next(uint8_t *data, uint32_t length)
{
    // TODO write data to flash
    // ASSERT(length > 16)
		m_ota_is_running = true;
    ota_timeout_ms = 15000;
    // Step1 : Header must has same hardware version and same firmware type
    // Step2 : Write data to flash, exclude checksum
    if (m_found_header == false 
		&& m_current_write_size < sizeof(ota_image_header_t))
    {
        // TODO : verify length of data >= 16 byte
        ota_image_header_t image_info;
        memcpy(&image_info, data, sizeof(ota_image_header_t));
        if (memcmp(image_info.name.header, 
                    OTA_UPDATE_DEFAULT_HEADER_DATA_FIRMWARE_TYPE, 
                    strlen(OTA_UPDATE_DEFAULT_HEADER_DATA_FIRMWARE_TYPE)))
        {
            //NRF_LOG_INFO("Firmware header err\r\n");
            return false;
        }
        if (memcmp(image_info.name.hardware_version, 
                    OTA_UPDATE_DEFAULT_HEADER_DATA_HARDWARE, 
                    strlen(OTA_UPDATE_DEFAULT_HEADER_DATA_HARDWARE)))
        {
            //NRF_LOG_INFO("Hardware header err\r\n");
            return false;
        }

        
        m_found_header = true;
        // Skip header
//        length -= OTA_UPDATE_DEFAULT_HEADER_SIZE;
//        data += OTA_UPDATE_DEFAULT_HEADER_SIZE;
        //NRF_LOG_INFO("Found header\r\n");
    }
    
    if (!m_found_header)
    {
        //NRF_LOG_INFO("Not found firmware header\r\n");
        return false;
    }
    
    while (length)
    {
        uint32_t bytes_need_copy = FLASH_IF_SECTOR_SIZE - m_ota_remain.size;

        if (bytes_need_copy > length)
        {
            bytes_need_copy = length;
        }
        
        memcpy(&m_ota_remain.data[m_ota_remain.size], data, bytes_need_copy);
        length -= bytes_need_copy;
        data += bytes_need_copy;
        m_ota_remain.size += bytes_need_copy;
        
        if (m_ota_remain.size == FLASH_IF_SECTOR_SIZE)
        {
            
            NRF_LOG_INFO("Total write size %u, at addr 0x%08X\r\n", m_current_write_size + m_ota_remain.size, 
                                                                OTA_UPDATE_DOWNLOAD_IMAGE_START_ADDR + m_current_write_size);
            flash_if_error_t err = FLASH_IF_OK;
#if OTA_ERASE_ALL_FLASH_BEFORE_WRITE == 0
            err = flash_if_erase(OTA_UPDATE_DOWNLOAD_IMAGE_START_ADDR + m_current_write_size, FLASH_IF_SECTOR_SIZE);
#endif
            err += flash_if_copy(OTA_UPDATE_DOWNLOAD_IMAGE_START_ADDR + m_current_write_size,
            											(uint32_t*)&m_ota_remain.data[0],
														m_ota_remain.size/4);
            if (err != FLASH_IF_OK)
            {
                NRF_LOG_INFO("Write data to flash error\r\n");
                return false;
            }
           // NRF_LOG_INFO("DONE\r\n");

            m_current_write_size += m_ota_remain.size;
            m_ota_remain.size = 0;
        }
        else
        {
            break;
        }
    }

    if (m_current_write_size >= m_expected_fw_size)
    {
        NRF_LOG_INFO("All data received\r\n");
    }
    
    return true;
}

bool app_ota_is_all_data_received(void)
{
    if (m_expected_fw_size && 
        m_current_write_size >= m_expected_fw_size)
    {
        return true;
    }
    return false;
}

void ota_update_set_expected_size(uint32_t size)
{
    m_expected_fw_size = size;
}

bool ota_update_commit_flash(void)
{
    // TODO write boot information
	bool retval = true;
    if (m_ota_remain.size)
    {
//    NRF_LOG_INFO("Commit flash =>> Write final %u bytes, total %u bytes\r\n", 
//                m_ota_remain.size, 
//                m_current_write_size + m_ota_remain.size);

        for (uint32_t i = m_ota_remain.size; i < FLASH_IF_SECTOR_SIZE; i++)
        {
        	m_ota_remain.data[i] = 0xFF;
        }

        if (FLASH_IF_OK != flash_if_copy(OTA_UPDATE_DOWNLOAD_IMAGE_START_ADDR + m_current_write_size, 
                                        (uint32_t*)&m_ota_remain.data[0], 
                                        FLASH_IF_SECTOR_SIZE/4))
        {
        	retval = false;
        }
        m_ota_remain.size = 0;
    }
    return retval;
}

OTA_FUNC_PLACE(void ota_update_write_header(uint32_t addr, ota_information_t *header))
{
//    NRF_LOG_INFO("Write header at addr 0x%08X\r\n", addr);
    flash_if_erase(addr, OTA_UPDATE_INFORMATION_SIZE);
	memcpy(&m_ota_remain.data[0], (uint8_t*)header, sizeof(ota_information_t));
    if (flash_if_copy(addr, (uint32_t*)&m_ota_remain.data[0], (sizeof(ota_information_t) + 3)/4) != FLASH_IF_OK)
    {
        NRF_LOG_INFO("Write data to flash error\r\n");
    }
    else
    {
        //NRF_LOG_INFO("Write header done\r\n");
    }
}


bool ota_update_finish(bool status)
{
    bool retval = false;
    m_found_header = false;
    if (status)
    {
        // Check if remain bytes of data
        if (m_ota_remain.size)
        {
            /*NRF_LOG_INFO("Write final %u bytes, total %u bytes\r\n",
						m_ota_remain.size,
						m_current_write_size + m_ota_remain.size);*/
            
            flash_if_erase(OTA_UPDATE_DOWNLOAD_IMAGE_START_ADDR + m_current_write_size, FLASH_IF_SECTOR_SIZE);
            flash_if_copy(OTA_UPDATE_DOWNLOAD_IMAGE_START_ADDR + m_current_write_size,
            				(uint32_t*)&m_ota_remain.data[0],
							(m_ota_remain.size+3)/4);
        }
        if ((m_current_write_size + m_ota_remain.size) != m_expected_fw_size)
        {
            NRF_LOG_INFO("Firmware download not complete\r\n");
        }
        else        
        {
            m_ota_remain.size = 0;
            // Verify checksum from download area to (firmware size - 4 bytes crc32), last 4 bytes is crc32
            //uint32_t header_size = sizeof(ota_image_header_t);
            if (ota_update_verify_checksum(OTA_UPDATE_DOWNLOAD_IMAGE_START_ADDR, m_expected_fw_size))
            {
                // Write data into ota information page, bootloader will check this page and perform firmware copy
                
                NRF_LOG_INFO("Valid checksum, write OTA flag\r\n");
                __attribute__((aligned(4))) ota_information_t new_cfg;
                new_cfg.ota_flag = OTA_UPDATE_FLAG_UPDATE_NEW_FIRMWARE; // OTA_FLAG_UPDATE_NEW_FW;
                new_cfg.size = m_expected_fw_size;      // 4 is size of CRC32
                new_cfg.crc32 = m_crc;
                ota_update_write_header(OTA_INFORMATION_START_ADDR, &new_cfg);
                retval = true;
            }
            else
            {
                //NRF_LOG_INFO("Invalid checksum\r\n");        
            }       
        }
    }
    else
    {
        //LCD8x2_Display_UpdateOTAFail();
        NRF_LOG_INFO("OTA update failed\r\n");
    }
    ota_timeout_ms = 0;
    m_current_write_size = 0;
    m_ota_remain.size = 0;
    m_ota_is_running = false;
    m_expected_fw_size = 0;
    memset(&m_ota_remain, 0, sizeof(m_ota_remain));
    return retval;
}

uint32_t ota_update_crc_by_sum(const uint8_t* data_p, uint32_t length)
{
    uint32_t crc = 0;
    while (length--)
    {
        crc += *data_p++;
    }
    
    return crc;
}

bool ota_update_verify_checksum(uint32_t begin_addr, uint32_t length)
{
#if 0
	// Last 16 bytes of firmware is the md5 checksum value
	NRF_LOG_INFO("Verify checksum from addr 0x%08X, len %u\r\n", begin_addr, length);
    app_md5_ctx md5_cxt;
    uint32_t checksum_addr;
    uint32_t *page_data = (uint32_t*)&m_ota_remain.data[0];
    __ALIGNED(4) uint8_t expected_md5[16];
    uint32_t number_word = (length - 16) / 4;
    app_md5_init(&md5_cxt);
    uint8_t md5_result[OTA_UPDATE_MD5_CHECKSUM_SIZE];

    // Calculate MD5
    for (uint32_t i = 0; i < number_word; i++)
    {
    	vdm_wdt_feed();
    	flash_if_read(begin_addr, page_data, 4);
        app_md5_update(&md5_cxt, (uint8_t *)begin_addr, 4);
    	begin_addr += 4;
    }
    app_md5_final(md5_result, &md5_cxt);

    // Read md5 in binary firmware
    #_if_read(begin_addr, (uint32_t*)&expected_md5[0], 16);

    // Debug
    NRF_LOG_INFO("Expected md5\r\n");
    for (uint32_t i = 0; i < 16; i++)
    {
    	DEBUG_RAW("%02X ", expected_md5[i]);
    }
    DEBUG_RAW("\r\n");

    NRF_LOG_INFO("Calculated md5\r\n");
    for (uint32_t i = 0; i < 16; i++)
    {
    	DEBUG_RAW("%02X ", md5_result[i]);
    }
    DEBUG_RAW("\r\n");

    // Compare
    if (memcmp(md5_result, (uint8_t *)expected_md5, OTA_UPDATE_MD5_CHECKSUM_SIZE) == 0)
    {
    	NRF_LOG_INFO("Checksum is valid\r\n");
        return true;
    }
    else
    {
    	NRF_LOG_INFO("Checksum is error\r\n");
        return false;
    }
#else
    // First 16 bytes is image header, so skip it
    // Last 4 bytes is CRC32 of firmware
    
    begin_addr += sizeof(ota_image_header_t);
    length -= sizeof(ota_image_header_t);
    length -= 4;        // sizeof crc
    
    uint32_t expected_crc = *((uint32_t*)(begin_addr+length));
   // NRF_LOG_INFO("Cal crc from 0x%08X, size %u bytes, last value 0x%08X, exp %u\r\n", begin_addr, length, *((uint32_t*)(begin_addr+length)), expected_crc);
    uint32_t crc = ota_update_crc_by_sum((uint8_t*)begin_addr, length);
    if (expected_crc == crc)
    {
        //NRF_LOG_INFO("CRC valid\r\n", expected_crc, crc);
        m_crc = expected_crc;
        return true;
    }

    //NRF_LOG_INFO("Expected 0x%08X, calculated 0x%08X\r\n", expected_crc, crc);
    return false;
#endif
}
