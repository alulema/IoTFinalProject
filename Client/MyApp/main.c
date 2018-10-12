#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <syslog.h>
#include <zconf.h>

#include "tc_error.h"
#include "tc_config.h"
#include "tc_json.h"
#include "tc_httpclient.h"
#include "tc_state.h"

static const char *URL = "http://52.67.91.154:5000/api/thermostat";
static const char *URL_ONLINE = "http://52.67.91.154:5000/api/thermostat/online";
static const char *URL_GETCONFIG = "http://52.67.91.154:5000/api/thermostat/getdeviceconfig?deviceId=";
static const char *TEMP_FILENAME = "/tmp/temp";
static const char *STATE_FILENAME = "/tmp/status";
static const char *WORKING_DIR = "/";

/**
 * If we exit the process, we want to sent information on
 * the reason for the exit to syslog, and then close
 * the log. This is a way for us to centralize cleanup
 * when we leave the daemon process.
 *
 * @param err The error code we exit under.
 */
static void _exit_process(const tc_error_t err) {
    syslog(LOG_INFO, "%s", tc_error_to_msg(err));
    closelog();
    exit(err);
}


int main() {
    // 0. read properties file
    read_device_properties(&DEVICE_ID, &UNIT);

    // 1. send device info to cloud to indicate it is online
    //char* online_request = create_online_request(DEVICE_ID, UNIT); // throwing memerror!!!
    char online_request[50];
    strcpy(online_request, "{ \"device_id\": \"");
    strcat(online_request, DEVICE_ID);
    strcat(online_request, "\", \"unit\": \"");
    strcat(online_request, UNIT);
    strcat(online_request, "\", \"type\": \"Thermostat\" }");

    while (1) {
        send_post(online_request, URL_ONLINE);

        // 2. send enqueued data to cloud to indicate it is online
        // Read the heater state.
        tc_heater_state_t heater_state = OFF;
        char *temperature;
        tc_error_t err = tc_read_state(STATE_FILENAME, &heater_state);
        tc_error_t err2 = tc_read_temperature(TEMP_FILENAME, &temperature);
        strtok(temperature, "\n");

        if (err != OK) {
            _exit_process(err);
        }

        //char * post_data_request = create_post_data_request(DEVICE_ID, UNIT, heater_state, temperature);
        char post_data_request[200];
        strcpy(post_data_request, "{ \"id\": \"");
        strcat(post_data_request, DEVICE_ID);
        strcat(post_data_request, "\", \"status\": \"");
        strcat(post_data_request, heater_state == ON ? "ON" : "OFF");
        strcat(post_data_request, "\", \"measurement\": ");
        strcat(post_data_request, temperature);
        strcat(post_data_request, ", \"unit\": \"");
        strcat(post_data_request, UNIT);
        strcat(post_data_request, "\" }");

        send_post(post_data_request, URL);

        // 3. read configs from cloud
        char get_config_url[100];
        strcpy(get_config_url, URL_GETCONFIG);
        strcat(get_config_url, DEVICE_ID);

        char *tst = send_get(get_config_url);
        printf("%s", tst);
        sleep(1);
    }

    // 4. if configs are new, then overwrite
    // 5. write configs in local
    // 6. send flag to indicate it was configured
    printf("Hello, World!\n");
    return 0;
}