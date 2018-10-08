/**
 * Reading and writing from supported files. This is the API
 * for the reading and writing module.
 */
#include "tc_error.h"

#ifndef __TC_STATE_H__
#define __TC_STATE_H__

/**
 * An enum type describing heater state.
 */
typedef enum {
    ON,
    OFF
} tc_heater_state_t;

tc_error_t  tc_write_temperature(const char* filename, float t);
tc_error_t  tc_read_state(const char* filename, tc_heater_state_t* state);

#endif