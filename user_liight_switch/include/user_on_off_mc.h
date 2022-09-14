
#ifndef _USER_ONOFF_MC_H__
#define _USER_ONOFF_MC_H__

#include <stdint.h>

//#include "generic_onoff_common.h"
#include "user_on_off_common.h"
#include "mesh_config.h"
#include "mesh_opt.h"
#include "nrf_mesh_assert.h"
#include "model_config_file.h"

#if (SCENE_SETUP_SERVER_INSTANCES_MAX > 0) || (DOXYGEN)
#include "scene_common.h"
#endif

/**
 * @defgroup GENERIC_ONOFF_MC Persistence module for the Generic OnOff Server model related states
 * @ingroup GENERIC_ONOFF_MODEL
 *
 * This module provides APIs for handling persistence of the Generic OnOff Server model related states.
 *
 * @
 */

/** Number of entry instances required to store the current state and state for each scene
 *  @note If @ref SCENE_SETUP_SERVER_INSTANCES_MAX is equal to 0, then this is equal to @ref
 *  GENERIC_ONOFF_SERVER_INSTANCES_MAX.
 */
#if (SCENE_SETUP_SERVER_INSTANCES_MAX > 0) || (DOXYGEN)
#define USER_ONOFF_SERVER_STORED_WITH_SCENE_STATES (GENERIC_ONOFF_SERVER_INSTANCES_MAX + \
                                (SCENE_REGISTER_ARRAY_SIZE * GENERIC_ONOFF_SERVER_INSTANCES_MAX))
#else
#define USER_ONOFF_SERVER_STORED_WITH_SCENE_STATES \
                                    (GENERIC_ONOFF_SERVER_INSTANCES_MAX)
#endif

#define USER_ONOFF_EID_START         (MESH_APP_MODEL_USER_ON_OFF_ID_START)
#define USER_ONOFF_EID_END           (USER_ONOFF_EID_START + USER_ONOFF_SERVER_STORED_WITH_SCENE_STATES - 1)

NRF_MESH_STATIC_ASSERT(USER_ONOFF_EID_END <= MESH_APP_MODEL_USER_ON_OFF_ID_END);

/** Generic OnOff state entry ID */
#define USER_ONOFF_EID           MESH_CONFIG_ENTRY_ID(MESH_OPT_MODEL_FILE_ID, USER_ONOFF_EID_END)

/** Set internal OnOff state variable.
 *
 * @param[in] index     An index to identify an instance of a state variable.
 * @param[in] value     Value to set.
 *
 * @retval NRF_SUCCESS              The value was successfully set.
 * @retval NRF_ERROR_NOT_FOUND      The given index is unknown.
 * @retval NRF_ERROR_INVALID_DATA   The value is invalid.
 */
uint32_t user_onoff_mc_onoff_state_set(uint8_t index, bool value);

/** Get internal OnOff state variable.
 *
 * @param[in]  index     An index to identify an instance of a state variable.
 * @param[out] p_value   Pointer to a buffer to copy the value into. Cannot be NULL.
 *
 * @retval NRF_SUCCESS              The entry value was successfully copied into @p p_value.
 * @retval NRF_ERROR_NULL           A parameter is NULL.
 * @retval NRF_ERROR_NOT_FOUND      The given index is unknown.
 * @retval NRF_ERROR_INVALID_STATE  The given index is known, but has no data associated with it.
 */
uint32_t user_onoff_mc_onoff_state_get(uint8_t index, bool * p_value);

#if (SCENE_SETUP_SERVER_INSTANCES_MAX > 0) || (DOXYGEN)

/** Stores the given OnOff value for the specific scene index.
 * @note Available only if @ref SCENE_SETUP_SERVER_INSTANCES_MAX is equal or larger than 1.
 *
 * @param[in] index         An index to identify an instance of a model.
 * @param[in] scene_index   A scene index for which given value is saved.
 * @param[in] value         Value to set.
 *
 * @retval NRF_SUCCESS              The value was successfully set.
 * @retval NRF_ERROR_NOT_FOUND      The given index is unknown.
 * @retval NRF_ERROR_INVALID_DATA   The value is invalid.
 */
uint32_t user_onoff_mc_scene_onoff_store(uint8_t index, uint8_t scene_index, bool value);

/** Restores the given OnOff value for the specific scene index.
 * @note Available only if @ref SCENE_SETUP_SERVER_INSTANCES_MAX is equal or larger than 1.
 *
 * @param[in] index         An index to identify an instance of a model.
 * @param[in] scene_index   A scene index for which value is to be recalled.
 * @param[in] p_value       Pointer to a buffer to copy the value into.
 *
 * @retval NRF_SUCCESS              The value was successfully set.
 * @retval NRF_ERROR_NOT_FOUND      The given index is unknown.
 * @retval NRF_ERROR_INVALID_DATA   The value is invalid.
 */
uint32_t user_onoff_mc_scene_onoff_recall(uint8_t index, uint8_t scene_index, bool * p_value);

#endif /* (SCENE_SETUP_SERVER_INSTANCES_MAX > 0) || (DOXYGEN) */

/** Create an instance of the Generic OnOff Server model states and return the corresponding handle.
 *
 * @param[out] p_handle Pointer to a buffer to copy the handle into to access internal state instance.
 *
 * @retval NRF_SUCCESS              The new instance is successfully created.
 * @retval NRF_ERROR_NULL           A parameter is NULL.
 * @retval NRF_ERROR_RESOURCES      No more instances can be created.
 *                                  In that case, increase value of
 *                                  @ref GENERIC_ONOFF_SERVER_INSTANCES_MAX.
 */
uint32_t user_onoff_mc_open(uint8_t * p_handle);

/**
 * Clear all stored data and reset state contexts to default values.
 */
void user_onoff_mc_clear(void);

/**
 * Initialize the Generic OnOff Server persistent memory.
 */
void user_onoff_mc_init(void);

/** @} end of GENERIC_ONOFF_MC */

#endif /* GENERIC_ONOFF_MC_H__ */
