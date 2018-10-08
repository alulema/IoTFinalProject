#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "tc_error.h"
#include "tc_config.h"

bool starts_with(const char *a, const char *b) {
    if(strncmp(a, b, strlen(b)) == 0) return 1;
    return 0;
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