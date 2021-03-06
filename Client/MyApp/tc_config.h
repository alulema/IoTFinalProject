/**
 * Utility methods to read properties file.
 */
#include <stdbool.h>

#ifndef MYAPP_TC_CONFIG_H
#define MYAPP_TC_CONFIG_H

static const char *url_base = "http://iotfinalproject-test.us-east-1.elasticbeanstalk.com/api/thermostat";
static const char *url_online = "http://iotfinalproject-test.us-east-1.elasticbeanstalk.com/api/thermostat/online";

static char* ENDPOINT;
static char* LOG_FILE;
static char* DEVICE_ID;
static char* UNIT;

void read_device_properties(char ** endpoint, char ** log_file, char ** device_id, char ** unit);
void read_device_properties_from_file(char* file, char ** endpoint, char ** log_file, char ** device_id, char ** unit);
bool process_arguments(int argc, char *argv[]);

#endif
