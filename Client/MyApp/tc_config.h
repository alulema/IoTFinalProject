/**
 * Utility methods to read properties file.
 */
#include <stdbool.h>

#ifndef MYAPP_TC_CONFIG_H
#define MYAPP_TC_CONFIG_H

static char* DEVICE_ID;
static char* UNIT;

void read_device_properties(char ** device_id, char ** unit);

#endif
