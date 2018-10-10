#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <errno.h>
#include <stdlib.h>

#include "tc_json.h"
#include "tc_config.h"
#include "tc_state.h"

char* create_online_request(char * device, char * unit) {
    char json[50];
    strcpy(json, "{ \"device-id\": \"");
    strcat(json, device);
    strcat(json, "\", \"unit\": \"");
    strcat(json, unit);
    strcat(json, "\", \"type\": \"Thermostat\" }");

    return &json;
}

char* create_post_data_request(char * device, char * unit, tc_heater_state_t heater_state, char * temperature) {
    char *json = (char *) malloc(sizeof(char) * 1000);
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
