#ifndef OTA_UPDATE_H
#define OTA_UPDATE_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Bootloader memory organization
 *
 *   Address					Size					Desc
 * 	0x80000000				    8KB					Bootloader code
 * 	0x80002000				    27KB				Application
 * 	0x80008C00				    27KB				Download firmware
 * 	0x8000F800				    0x1000			    OTA download information
 */
 
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
 
#define OTA_UPDATE_BOOTLOADER_SIZE                                              (4096)
#define OTA_UPDATE_APPLICATION_SIZE						0x5A000U
#define OTA_UPDATE_DOWNLOAD_FIRMWARE_SIZE                                       OTA_UPDATE_APPLICATION_SIZE
#define OTA_UPDATE_INFORMATION_SIZE						0x400


/*Soft device Flash Macros*/
#define OTA_SOFTDEVICE_FLASH_SIZE                                               0x25000U
#define OTA_SOFTDEVICE_FLASH_START_ADDR                                         0x1000U
#define OTA_UPDATE_FLASH_BASE							(OTA_SOFTDEVICE_FLASH_START_ADDR + OTA_SOFTDEVICE_FLASH_START_ADDR)
#define OTA_UPDATE_APPLICATION_START_ADDR                                       (OTA_UPDATE_FLASH_BASE)
#define OTA_UPDATE_DOWNLOAD_IMAGE_START_ADDR                                    (OTA_UPDATE_FLASH_BASE)
#define OTA_INFORMATION_START_ADDR						(OTA_UPDATE_FLASH_BASE + OTA_UPDATE_APPLICATION_SIZE)


#define OTA_UPDATE_FLAG_UPDATE_NEW_FIRMWARE				0x12345688
#define OTA_UPDATE_FLAG_UPDATE_COMPLETE					0x98761234
#define OTA_UPDATE_FLAG_INVALID							0xFFFFFFFF

#define OTA_UPDATE_MD5_CHECKSUM_SIZE					16
#define OTA_UPDATE_DEFAULT_HEADER_SIZE					16

#ifndef OTA_UPDATE_DEFAULT_HEADER_DATA_FIRMWARE_TYPE
#define OTA_UPDATE_DEFAULT_HEADER_DATA_FIRMWARE_TYPE  	"SFUL01"       // must 3 bytes
#endif

#ifndef OTA_UPDATE_DEFAULT_HEADER_DATA_HARDWARE
#define OTA_UPDATE_DEFAULT_HEADER_DATA_HARDWARE         "001"       // must 3 bytes
#endif

#ifndef OTA_UPDATE_FW_VERSION
#define OTA_UPDATE_FW_VERSION                           "001"       // must 3 bytes
#endif

#define OTA_UPDATE_CHECK_HEADER							1   

#ifndef OTA_ERASE_ALL_FLASH_BEFORE_WRITE
#define OTA_ERASE_ALL_FLASH_BEFORE_WRITE                0
#endif

typedef union  __attribute__((packed))
{
    struct
    {
        char header[3];
        char firmware_version[3];
        char hardware_version[3];
        uint32_t firmware_size;         // fw size =  image_size + 16 byte md5, fw excluded header
        uint8_t release_year;           // From 2000
        uint8_t release_month;
        uint8_t release_date;
    } __attribute__((packed)) name;
    uint8_t raw[16];
}ota_image_header_t;



typedef struct
{
    uint32_t ota_flag;
    uint32_t size;
    uint32_t crc32;
} ota_information_t;

/*!
 * @brief		Start ota update process
 * @param[in]	expected_size : Firmware size, included header signature
 * @retval 		TRUE : Operation success
 *         		FALSE : Operation failed
 */
bool ota_update_start(uint32_t expected_size);

/*!
 * @brief		Write data to flash
 * @param[in]	data : Data write to flash
 * @param[in]	length : Size of data in bytes
 * @note 		Flash write address will automatic increase inside function
 * @retval		TRUE : Operation success
 *				FALSE : Operation failed
 */
bool ota_update_write_next(uint8_t *data, uint32_t length);

/*!
 * @brief       Finish ota process
 * @param[in]   status TRUE : All data downloaded success
 *                   FALSE : A problem occurs
 * @retval		TRUE : Operation success
 *				FALSE : Operation failed
 */
bool ota_update_finish(bool status);

/*!
 * @brief		Check ota update status
 * @retval		TRUE : OTA is running
 * 				FALSE : OTA is not running
 */
bool ota_update_is_running(void);

/*!
 * @brief		Get current ota update
 * @retval		OTA config in flash
 */
ota_information_t *ota_update_get_config(void);

/*!
 * @brief		Set ota image size
 * @param[in]	size : Size of image
 */
void ota_update_set_expected_size(uint32_t size);

/*!
 * @brief		Write all remain data into flash
 */
bool ota_update_commit_flash(void);

/*!
 * @brief		Verify image checksum
 * @param[in]	addr : Start addr of image
 * @param[in]	length : Size of image, included 16 bytes checksum at the end
 * @retval		TRUE Valid image
 * 				FALSE Invalid image
 */
bool ota_update_verify_checksum(uint32_t addr, uint32_t length);

/*!
 * @brief		Get downloaded firmware in percent
 * @retval		Downloaded percent
 */
uint8_t ota_get_downloaded_percent(void);

/*!
 * @brief		Rollback to factory firmware
 */
void ota_update_rollback_to_factory_firmware(void);

/**
 * @brief		Write ota firmware header to flash
 * @param[in]	addr Address will stored data
 * @param[in]	header Header data
 */
void ota_update_write_header(uint32_t addr, ota_information_t *header);

/*!
 * @brief		Check if all data received
 * @retval		TRUE All data received
 *              FALSE Firmware data is not complete
 */
bool app_ota_is_all_data_received(void);

#endif /* OTA_UPDATE_H */
