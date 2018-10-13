/**
 * Utility methods to help assembling JSON requests.
 */
#ifndef MYAPP_TC_JSON_H
#define MYAPP_TC_JSON_H

#include "tc_state.h"

char* create_online_request(char * device, char * unit);
char* create_post_data_request(char * device, char * unit, tc_heater_state_t heater_state, char * temperature);

#endif
