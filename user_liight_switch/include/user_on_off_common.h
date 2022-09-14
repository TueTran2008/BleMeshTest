

#ifndef USER_ON_OFF_COMMON_H__
#define USER_ON_OFF_COMMON_H__

#include <stdint.h>
#include "access.h"

/**
 * @defgroup SIMPLE_ON_OFF_MODEL Simple OnOff model
 * This model implements the message based interface required to
 * set the 1 bit value on the server.
 *
 * This implementation of a simple OnOff model can be used to switch things
 * on or off by manipulating a single on/off state. The intention of this model
 * is to have a simple example model that can be used as a baseline for constructing
 * your own model.
 *
 * Do not confuse the simple OnOff model with the Generic OnOff Model specified
 * in @tagMeshMdlSp. The Generic OnOff Model provides additional
 * features such as control over when and for how long the transition between
 * the on/off state should be performed.
 *
 * @note When the server has a publish address set (as in the light switch example),
 * the server will publish its state to its publish address every time its state changes.
 *
 * For more information about creating models, see
 * @ref md_doc_user_guide_modules_models_creating.
 *
 * Model Identification
 * @par
 * Company ID: @ref SIMPLE_ON_OFF_COMPANY_ID
 * @par
 * Simple OnOff Client Model ID: @ref SIMPLE_ON_OFF_CLIENT_MODEL_ID
 * @par
 * Simple OnOff Server Model ID: @ref SIMPLE_ON_OFF_SERVER_MODEL_ID
 *
 * List of supported messages:
 * @par
 * @copydoc SIMPLE_ON_OFF_OPCODE_SET
 * @par
 * @copydoc SIMPLE_ON_OFF_OPCODE_GET
 * @par
 * @copydoc SIMPLE_ON_OFF_OPCODE_SET_UNRELIABLE
 * @par
 * @copydoc SIMPLE_ON_OFF_OPCODE_STATUS
 *
 * @ingroup MESH_API_GROUP_VENDOR_MODELS
 * @{
 * @defgroup SIMPLE_ON_OFF_COMMON Common Simple OnOff definitions
 * Types and definitions shared between the two Simple OnOff models.
 * @{
 */
/*lint -align_max(push) -align_max(1) */

/** Vendor specific company ID for Simple OnOff model */
#define SIMPLE_ON_OFF_COMPANY_ID    (ACCESS_COMPANY_ID_NORDIC)


/*Define Models Opcode*/
#define USER_ON_OFF_COMPANY_ID     0x1234
#define USER_ON_OFF_SWITCH_LIGHT_OPCODE   0x9876

/*Define User Raw Data Lenght*/

#define USER_ON_OFF_MESSAGE_MIN_PAYLOAD_LENGHT  3
#define USER_ON_OFF_MESSAGE_MAX_PAYLOAD_LENGHT  5


/** Simple OnOff opcodes. */
typedef enum
{
    USER_ON_OFF_OPCODE_SET = 0xC1,            /**< Simple OnOff Acknowledged Set. */
    USER_ON_OFF_OPCODE_GET = 0xC2,            /**< Simple OnOff Get. */
    USER_ON_OFF_OPCODE_SET_UNRELIABLE = 0xC3, /**< Simple OnOff Set Unreliable. */
    USER_ON_OFF_OPCODE_STATUS = 0xC4          /**< Simple OnOff Status. */
} user_on_off_opcode_t;


#define BYTECH_COMPANY_OPCODE 0xABCD

/** Message format for the Simple OnOff Set message. */
typedef struct __attribute((packed))
{
    uint8_t on_off; /**< State to set > **/
    uint8_t tid;    /**< Transaction number. */
    uint8_t pwm_period; /**<lightless of led>*/
    uint8_t transition_time; /*BLE Transition time*/
    uint8_t delay; /*Delay in encoded msg*/
} user_on_off_msg_set_pkt_t;

/** Message format for th Simple OnOff Set Unreliable message. */
typedef struct __attribute((packed))
{
    uint8_t on_off; /**< State to set. */
    uint8_t pwm_period;
    uint8_t tid;    /**< Transaction number. */
} user_on_off_msg_set_t;


/** Message format for the generic_onoff Status message. */
typedef struct __attribute((packed))
{
    uint8_t present_on_off;                                 /**< The present value of the Generic OnOff state */
    uint8_t target_on_off;                                  /**< The target value of the Generic OnOff state (optional) */
    uint8_t remaining_time;                                 /**< Encoded remaining time */
} user_on_off_status_msg_pkt_t;
/*lint -align_max(pop) */

/** @} end of SIMPLE_ON_OFF_COMMON */
/** @} end of SIMPLE_ON_OFF_MODEL */
#endif /* SIMPLE_ON_OFF_COMMON_H__ */
