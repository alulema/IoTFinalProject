#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <syslog.h>
#include <zconf.h>
#include <errno.h>

#include "tc_error.h"
#include "tc_config.h"

bool starts_with(const char *a, const char *b) {
    if(strncmp(a, b, strlen(b)) == 0) return 1;
    return 0;
}

static tc_error_t _write_config(char* device_config) {
    // Open the file, check the return value.
    FILE* fp = fopen("./config.json", "wb");
    if (fp == NULL) {
        return NO_OPEN;
    }

    // Write config
    fprintf(fp, "%s\n", device_config);

    // Closing and returning.
    fclose(fp);
    return OK;
}

tc_error_t tc_write_config(char* device_config) {
    // Setting the initial interval.
    char retry_interval = 1;

    // The default return result.
    tc_error_t result = OK;

    // This do loop supports retrying the writes if we can't
    // acquire a file handle for some reason.
    do {
        result = _write_config(device_config);

        // If we have an error, and we haven't used all our retries,
        // give it another shot.
        if (result != OK && retry_interval < 64) {

            // Put an entry in syslog that we had a failure.
            syslog(LOG_INFO,
                   "failed temp write (%s); retry with interval (%d)",
                   strerror(errno),
                   retry_interval << 1
            );

            // Sleep, set retry interval, and try again.
            sleep(retry_interval);
            retry_interval = retry_interval << 1;
        }

    } while(result != OK && retry_interval < 64);

    // return the final result.
    return result;
}

void read_device_properties(char ** device_id, char ** unit) {
    FILE* fp = fopen("./properties.info", "r");
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    tc_error_t retcode = OK;

    // Test that we can open the file, return if not.
    if (fp == NULL) {
        printf("No properties file is present. Exiting with error.");
        exit(EXIT_FAILURE);
    }

    const char delims[] = "=";
    while ((read = getline(&line, &len, fp)) != -1) {
        printf("Retrieved line of length %zu :\n", read);
        strtok(line, "\n");
        printf("%s\n", line);
        bool is_device_id = false;
        bool is_unit = false;

        do {
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

            if (starts_with(line, "device_id="))
                is_device_id = true;
            else
                is_device_id = false;

            if (starts_with(line, "unit="))
                is_unit = true;
            else
                is_unit = false;

            size_t field_len = strcspn(line, delims);
            printf("%.*s\n", (int)field_len, line);
            line += field_len;
        } while (*line++);
    }

    fclose(fp);
}