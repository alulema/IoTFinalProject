#include <stdbool.h>

#ifndef MYAPP_TC_CONFIG_H
#define MYAPP_TC_CONFIG_H

static char* DEVICE_ID;
static char* UNIT;


bool starts_with(const char *a, const char *b);
void read_device_properties(char ** device_id, char ** unit);
tc_error_t tc_write_config(char* device_config);

#endif //MYAPP_TC_CONFIG_H
