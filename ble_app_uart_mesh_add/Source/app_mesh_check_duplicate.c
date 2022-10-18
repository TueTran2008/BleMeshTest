#include "app_mesh_check_duplicate.h"
#include "DataDefine.h"
#include <string.h>
#include "app_timer.h"
#include "log.h"
#include "nrf_log.h"

#define TID_TIMEOUT_SECOND      (10)

static volatile uint8_t m_tid_index = 0;
static app_transaction_t m_tid_buffer[32];
APP_TIMER_DEF(app_timer_one_sec);
static uint32_t m_last_update_tid_sec;
static uint32_t m_current_sec = 0;

static inline void app_mesh_clean_tid()
{
    /* Reset TID to default state */
    if (m_tid_index)
    {
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Clean [%d] tid msg\n", m_tid_index);
        m_tid_index = 0;
        memset(&m_tid_buffer, 0, sizeof(m_tid_buffer));
    }
}

static void one_second_timeout()
{
    m_current_sec++;
    if (m_current_sec - m_last_update_tid_sec > TID_TIMEOUT_SECOND)
    {
        app_mesh_clean_tid();
        m_last_update_tid_sec = m_current_sec;
    }
}

static void app_timer_clean_tid_create(void)
{
    uint32_t error_code;

    error_code = app_timer_create(&app_timer_one_sec,
                                   APP_TIMER_MODE_REPEATED,
                                   one_second_timeout);
    APP_ERROR_CHECK(error_code);

    error_code = app_timer_start(app_timer_one_sec,
                                   APP_TIMER_TICKS(1000),
                                   NULL);

    APP_ERROR_CHECK(error_code);
}


void app_mesh_tid_init()
{
    app_mesh_clean_tid();
    app_timer_clean_tid_create();
}

bool app_mesh_tid_is_duplicate(app_transaction_t * id)
{
    uint8_t i;
    for (i = 0; i < 32; i++)
    {
        if (memcmp(&m_tid_buffer[i], id, sizeof(app_transaction_t)) == 0)
        {
            return true;
        }
    }
    return false;
}

void app_mesh_insert_tid(app_transaction_t * id)
{
    // Check node existed in queue
    for (uint8_t i = 0; i <32; i++)
    {
        if (m_tid_buffer[i].unicast_addr == id->unicast_addr)
        {
            m_tid_buffer[i].tid = id->tid;
            return;
        }
    }
    
    // Node not existed in quee, replace new one
    m_tid_buffer[m_tid_index] = *id;
    m_tid_index++;

    if (m_tid_index == 32)
    {
        NRF_LOG_ERROR("TID full\r\n");
        m_tid_index = 0;
    }
    
    m_last_update_tid_sec = m_current_sec;
}



const char * app_mesh_msg_map_device_type_to_string_type(uint8_t device_type)
{

    static const char dev_type[13][4] = {"GW", "FR", "S", "R", "B", "T", "D", "P", "TL", "TW", "TA", "ST", "PM"};
    return &dev_type[device_type][0];
}
