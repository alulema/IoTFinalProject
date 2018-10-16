/**
 * This module contains the function to read the properties file
 * which describes the device's basic properties: device id and
 * measurement unit
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <syslog.h>
#include <zconf.h>
#include <errno.h>

#include "tc_error.h"
#include "tc_config.h"

/**
 * Utility method to check if string starts with another string.
 * @param a Base string
 * @param b String to be searched
 * @return True is base string starts with the other string.
 */
bool starts_with(const char *a, const char *b) {
    if(strncmp(a, b, strlen(b)) == 0) return 1;
    return 0;
}

/**
 * Reads properties file from the executing folder, if properties
 * file is not present the aborts the execution. If properties file
 * is present, the the device id and measurement unit are read.
 * @param endpoint The cloud services endpoint.
 * @param log_file The log filename.
 * @param device_id The device id reference to be settled.
 * @param unit The measurement unit reference to be settled.
 */
void read_device_properties(char ** endpoint, char ** log_file, char ** device_id, char ** unit) {
    read_device_properties_from_file("/etc/init.d/properties.info", endpoint, log_file, device_id, unit);
}

/**
 * Reads properties file from the executing folder, if properties
 * file is not present the aborts the execution. If properties file
 * is present, the the device id and measurement unit are read.
 * @param endpoint The cloud services endpoint.
 * @param log_file The log filename.
 * @param device_id The device id reference to be settled.
 * @param unit The measurement unit reference to be settled.
 */
void read_device_properties_from_file(char* file, char ** endpoint, char ** log_file, char ** device_id, char ** unit) {
    FILE* fp = fopen(file, "r");
    char * line = NULL;
    size_t len = 0;

    // Test that we can open the file, return if not.
    if (fp == NULL) {
        printf("No properties file is present. Exiting with error.");
        exit(EXIT_FAILURE);
    }

    const char delims[] = "=";
    while (getline(&line, &len, fp) != -1) {
        strtok(line, "\n");
        bool is_endpoint = false;
        bool is_log_file = false;
        bool is_device_id = false;
        bool is_unit = false;

        do {
            if (is_endpoint) {
                *endpoint = (char *)malloc(strlen(line)+1);
                (*endpoint)[strlen(line)] = '\n';
                strcpy(*endpoint, line);
            }
            if (is_log_file) {
                *log_file = (char *)malloc(strlen(line)+1);
                (*log_file)[strlen(line)] = '\n';
                strcpy(*log_file, line);
            }
            if (is_device_id) {
                *device_id = (char *)malloc(strlen(line)+1);
                (*device_id)[strlen(line)] = '\n';
                strcpy(*device_id, line);
            }
            if (is_unit) {
                *unit = (char *)malloc(strlen(line)+1);
                (*unit)[strlen(line)] = '\n';
                strcpy(*unit, line);
            }

            if (starts_with(line, "endpoint="))
                is_endpoint = true;
            else
                is_endpoint = false;

            if (starts_with(line, "log_file="))
                is_log_file = true;
            else
                is_log_file = false;

            if (starts_with(line, "device_id="))
                is_device_id = true;
            else
                is_device_id = false;

            if (starts_with(line, "unit="))
                is_unit = true;
            else
                is_unit = false;

            size_t field_len = strcspn(line, delims);
            line += field_len;
        } while (*line++);
    }

    fclose(fp);
}

/**
 * Process command line arguments to decide to show help or use config file
 * @param argc Command line arguments number
 * @param argv Command line arguments
 * @return True to allow to run as command
 */
bool process_arguments(int argc, char *argv[]) {
    int isHelp;
    int isConfig;
    const char *tz = getenv("TZ");

    if (tz != NULL) {
        syslog(LOG_ERR, "The app doesn't support changing timezone.");
        exit(OK);
    }

    if (argc > 3) {
        printf("Too many arguments\n");
        printf("App is intended to run as daemon\n");
        printf("Type --help or -h to display online help about start it from console.\n");
        exit(OK);
    }
    switch (argc) {
        case 1:
            printf("App is intended to run as daemon\n");
            printf("Type --help or -h to display online help about start it from console.\n");
            return false;
        case 2:
            if (strcmp(argv[1], "start") == 0) {
                syslog(LOG_INFO, "CONTROLLER: Running as daemon");
                return false;
            }

            isConfig = strcmp(argv[1], "--config_file") == 0 || strcmp(argv[1], "-c") == 0;
            isHelp = strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0;

            if (isHelp) {
                printf("Thermostat Device App. This application monitors thermocouple's temperature and status, decides to turn on/off the actuator, and sends data to cloud services.\n\n");
                printf("Usage: main_controller [options]\n");
                printf("\t-c, --config <filepath> Send POST request to specified url, with additional arguments\n");
                printf("\t-h, --help              Obtains help information about this command\n");

                exit(OK);
            } else if (isConfig) {
                printf("Incomplete arguments, you are trying to use --config_file option but a parameter is missing.\n");
                printf("Type --help or -h to display online help about start it from console.\n");

                exit(OK);
            }

            printf("Not recognized command argument\n");
            printf("Type --help or -h to display online help about start it from console.\n");
            exit(OK);
        case 3:
            isConfig = strcmp(argv[1], "--config_file") == 0 || strcmp(argv[1], "-c") == 0;

            if (isConfig) {
                printf("Reading configuration from %s\n", argv[2]);
                read_device_properties_from_file(argv[2], &ENDPOINT, &LOG_FILE, &DEVICE_ID, &UNIT);

                if (ENDPOINT == NULL || LOG_FILE == NULL || DEVICE_ID == NULL || UNIT == NULL) {
                    printf("Some information is not correct in config file, please use the following structure:\n\n");
                    printf("endpoint=http://services_url:port\n");
                    printf("log_file=<logfile path>\n");
                    printf("device_id=<device id>\n");
                    printf("unit=C\n");

                    exit(OK);
                }

                if (ENDPOINT != NULL && LOG_FILE != NULL && DEVICE_ID != NULL && UNIT != NULL)
                    return true;

                return false;
            } else {
                printf("Wrong arguments.\n");
                printf("Type --help or -h to display online help about start it from console.\n");

                exit(OK);
            }
        default:
            return true;
    }
}
