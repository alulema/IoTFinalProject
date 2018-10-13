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
 * @param device_id The device id reference to be settled.
 * @param unit The measurement unit reference to be settled.
 */
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