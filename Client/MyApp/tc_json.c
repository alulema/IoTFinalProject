/**
 * This module contains utility method to help to assemble the JSON requests
 * that the device will make to cloud services.
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <errno.h>
#include <stdlib.h>

#include "tc_json.h"
#include "tc_config.h"
#include "tc_state.h"

/**
 * Assembles ping request to advise to the cloud that the
 * device is on-line.
 * @param device The device ID
 * @param unit Measurement unit.
 * @return Assembled JSON request
 */
char* create_online_request(char * device, char * unit) {
    char *json = malloc(80);
    strcpy(json, "{ \"device_id\": \"");
    strcat(json, device);
    strcat(json, "\", \"unit\": \"");
    strcat(json, unit);
    strcat(json, "\", \"type\": \"Thermostat\" }");

    return json;
}

/**
 * Assembles the request to send the device's temperature and heater state.
 * @param device The device ID
 * @param unit Measurement unit.
 * @param heater_state The heater state
 * @param temperature The temperature
 * @return Assembled JSON request
 */
char* create_post_data_request(char * device, char * unit, tc_heater_state_t heater_state, char * temperature) {
    char *json = malloc(80);
    strcpy(json, "{ \"id\": \"");
    strcat(json, device);
    strcat(json, "\", \"status\": \"");
    strcat(json, heater_state == ON ? "ON" : "OFF");
    strcat(json, "\", \"measurement\": ");
    strcat(json, temperature);
    strcat(json, ", \"unit\": \"");
    strcat(json, unit);
    strcat(json, "\" }");

    return json;
}

