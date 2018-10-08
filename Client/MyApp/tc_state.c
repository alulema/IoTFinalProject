/**
 * This module contains the functions to read and write to the
 * various status files. The API of this module consists of the
 * tc_write_temperature(filename, temp) and the
 * tc_read_state(filename, state_buffer) functions. Each
 * function returns an error value on error via the tc_error_t
 * type.
 *
 * Each of these functions are divided into an upper and lower
 * function. The upper function, in the public API call,
 * handles things like errors and retry logic (we will retry
 * reads and writes on a staggered interval, increasing the
 * attempt interval by powers of 2 until we exceed 64 seconds).
 * The lower function contains the file reading logic.
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <errno.h>
#include <stdlib.h>

#include "tc_state.h"

/**
 * The lower read function. We read the value, indicate the
 * result in the passed result buffer, and log the state to
 * syslog.
 *
 * @param filename The file from which to read.
 * @param result The heater state buffer.
 */
static tc_error_t _read_state(const char* filename, tc_heater_state_t* result) {

    // Open the file.
    FILE* fp = fopen(filename, "rb");

    // Set our default return code.
    tc_error_t retcode = OK;

    // Test that we can open the file, return if not.
    if (fp == NULL) {
        return NO_OPEN;
    }

    // Read the value of the temperature into the supplied
    // buffer and compare the results to ON or OFF. Then
    // Translate into the appropriate enum type.
    char buffer[32];
    fgets(buffer, sizeof(buffer), fp);
    if (strstr(buffer, "ON")) {
        syslog(LOG_INFO, "HEATER IS ON");
        *result = ON;
    } else if (strstr(buffer, "OFF")) {
        syslog(LOG_INFO, "HEATER IS OFF");
        *result = OFF;
    } else {
        retcode = UNKNOWN_HEATER_STATE;
    }

    // Close the file and return.
    fclose(fp);
    return retcode;
}

static tc_error_t _read_temperature(const char* filename, char ** t) {

    // Open the file.
    FILE* fp = fopen(filename, "rb");

    // Set our default return code.
    tc_error_t retcode = OK;

    // Test that we can open the file, return if not.
    if (fp == NULL) {
        return NO_OPEN;
    }

    // Read the value of the temperature into the supplied
    // buffer and compare the results to ON or OFF. Then
    // Translate into the appropriate enum type.
    char buffer[32];
    fgets(buffer, sizeof(buffer), fp);

    *t = (char *)malloc(strlen(buffer)+1);
    (*t)[strlen(buffer)] = '\n';
    strcpy(*t, buffer);

    syslog(LOG_INFO, "TEMPERATURE IS %s", t);

    // Close the file and return.
    fclose(fp);
    return retcode;
}

/**
 * Write the temperature to the file.
 *
 * @param filename The file to which to write.
 * @param t The temperature we're writing to the file.
 * @return The error conditions.
 */
static tc_error_t _write_temperature(const char* filename, float t) {
    // Open the file, check the return value.
    FILE* fp = fopen(filename, "wb");
    if (fp == NULL) {
        return NO_OPEN;
    }

    // Write that temperature.
    fprintf(fp, "%f\n", t);

    // Closing and returning.
    fclose(fp);
    return OK;
}

/**
 * The upper writing function. Here we handle error conditions
 * and retry logic.
 *
 * @param filename The name fo the file to which to write.
 * @param t The temperature to write to the file.
 * @return The error conditions.
 */
tc_error_t tc_write_temperature(const char* filename, float t) {
    // Setting the initial interval.
    char retry_interval = 1;

    // The default return result.
    tc_error_t result = OK;

    // This do loop supports retrying the writes if we can't
    // acquire a file handle for some reason.
    do {
        result = _write_temperature(filename, t);

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

/**
 * The upper reading function. Handles retries, errors, and
 * the like while the lower function handles the mechanics of
 * actually reading from the file.
 *
 * @param filename The file from which to read.
 * @param state The state buffer.
 * @return The error conditions.
 */
tc_error_t  tc_read_state(const char* filename, tc_heater_state_t* state) {
    // Initial retry interval.
    char retry_interval = 1;

    // Initial error conditions.
    tc_error_t result = OK;

    // This loop supports retry logic.
    do {
        // Attempt a read.
        result = _read_state(filename, state);

        // Take a nap based on the retry interval.
        sleep(retry_interval);

        // Is the result unknown? If so, assume the heater is ON.
        // Otherwise, if still in error, retry the read and make
        // a syslog entry.
        if (result == UNKNOWN_HEATER_STATE) {
            syslog(LOG_INFO, "we have an unknown heater state; assuming to ON.");
            *state = ON;
            result = OK;
        } else if (result != OK && retry_interval < 64) {
            syslog(LOG_INFO,
                   "failed state read with (%s); retry with interval (%d)",
                   strerror(errno),
                   retry_interval << 1
            );
            sleep(retry_interval);
            retry_interval = retry_interval << 1;
        }

    } while (result != OK && retry_interval < 64);

    // Return the error result.
    return result;
}

tc_error_t tc_read_temperature(const char* filename, char ** t) {
    // Initial retry interval.
    char retry_interval = 1;

    // Initial error conditions.
    tc_error_t result = OK;

    // This loop supports retry logic.
    do {
        // Attempt a read.
        result = _read_temperature(filename, t);

        // Take a nap based on the retry interval.
        sleep(retry_interval);

    } while (result != OK && retry_interval < 64);

    // Return the error result.
    return result;
}