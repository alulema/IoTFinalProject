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
#include "jsmn.h"

static const char *URL = "http://52.67.91.154:5000/api/thermostat";
static const char *URL_ONLINE = "http://52.67.91.154:5000/api/thermostat/online";
static const char *URL_GETCONFIG = "http://52.67.91.154:5000/api/thermostat/getdeviceconfig?deviceId=";
static const char *TEMP_FILENAME = "/tmp/temp";
static const char *STATE_FILENAME = "/tmp/status";
static const char *WORKING_DIR = "/";

static const char *JSON_STRING =
        "{\"user\": \"johndoe\", \"admin\": false, \"uid\": 1000,\n  "
        "\"groups\": [\"users\", \"wheel\", \"audio\", \"video\"]}";

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
    if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
        strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
        return 0;
    }
    return -1;
}

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
    int r;
    jsmn_parser p;
    jsmntok_t t[2048]; /* We expect no more than 128 tokens */
    jsmn_init(&p);

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

    do {
        //send_post(online_request, URL_ONLINE);

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

        //send_post(post_data_request, URL);

        // 3. read configs from cloud
        char get_config_url[100];
        strcpy(get_config_url, URL_GETCONFIG);
        strcat(get_config_url, DEVICE_ID);

        char *json_config = send_get(get_config_url);
        r = jsmn_parse(&p, json_config, strlen(json_config), t, sizeof(t)/sizeof(t[0]));

        if (r < 0) {
            printf("Failed to parse JSON: %d\n", r);
            return 1;
        }


        /* Assume the top-level element is an object */
        if (r < 1 || t[0].type != JSMN_OBJECT) {
            printf("Object expected\n");
            return 1;
        }

        /* Loop over all keys of the root object */
        for (int i = 1; i < r; i++) {
            if (jsoneq(JSON_STRING, &t[i], "user") == 0) {
                /* We may use strndup() to fetch string value */
                printf("- User: %.*s\n", t[i+1].end-t[i+1].start,
                       JSON_STRING + t[i+1].start);
                i++;
            } else if (jsoneq(JSON_STRING, &t[i], "admin") == 0) {
                /* We may additionally check if the value is either "true" or "false" */
                printf("- Admin: %.*s\n", t[i+1].end-t[i+1].start,
                       JSON_STRING + t[i+1].start);
                i++;
            } else if (jsoneq(JSON_STRING, &t[i], "uid") == 0) {
                /* We may want to do strtol() here to get numeric value */
                printf("- UID: %.*s\n", t[i+1].end-t[i+1].start,
                       JSON_STRING + t[i+1].start);
                i++;
            } else if (jsoneq(JSON_STRING, &t[i], "groups") == 0) {
                int j;
                printf("- Groups:\n");
                if (t[i+1].type != JSMN_ARRAY) {
                    continue; /* We expect groups to be an array of strings */
                }
                for (j = 0; j < t[i+1].size; j++) {
                    jsmntok_t *g = &t[i+j+2];
                    printf("  * %.*s\n", g->end - g->start, JSON_STRING + g->start);
                }
                i += t[i+1].size + 1;
            } else {
                printf("Unexpected key: %.*s\n", t[i].end-t[i].start,
                       JSON_STRING + t[i].start);
            }
        }

        tc_write_config(json_config);
        sleep(1);
    } while (0);

    // 4. if configs are new, then overwrite
    // 5. write configs in local
    // 6. send flag to indicate it was configured
    printf("Hello, World!\n");
    return 0;
}