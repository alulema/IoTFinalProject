/**
 * The main entry point of the thermocouple simulation program.
 *
 * This program simulates a thermocouple as a deamon process.
 * It is designed to be started at system startup in the init
 * (or alternative) hierarchy under linux. This module contains
 * the entry point (main()) at the bottom of the file. All
 * other functions are module private functions (i.e. static),
 * and not part of the exposed API. The only exposed function
 * is main().
 */
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "tc_error.h"
#include "tc_state.h"
#include "tc_math.h"

static const char*  URL             = "http://localhost:5000/api/thermostat";
static const char*  DAEMON_NAME     = "tcsimd";
static const char*  TEMP_FILENAME   = "/tmp/temp";
static const char*  STATE_FILENAME  = "/tmp/status";
static const char*  WORKING_DIR     = "/";
static const char*  DEVICE_ID       = "THE-001";
static const char*  UNIT            = "F";

static const long   SLEEP_DELAY     = 5;

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

/**
 * This is the signal hander we set on the daemon
 * process after initialization. This way, we can
 * intercept and handle signals from the OS.
 *
 * @param signal The signal from the OS.
 */
static void _signal_handler(const int signal) {
    switch(signal) {
        case SIGHUP:
            break;
        case SIGTERM:
            _exit_process(RECV_SIGTERM);
            break;
        default:
            syslog(LOG_INFO, "received unhandled signal");
    }
}

/**
 * When we start a daemon process, we need to fork from the
 * parent so we can appropriately configure the process
 * as a standalone, daemon process with approrpiate stdin,
 * stdout, and the like. Here, we handle errors if we are
 * unable to fork or we are the parent process and the fork
 * worked. If the fork failed, we record that and exit.
 * Otherwise, we exit the parent cleanly.
 *
 * @param pid The process ID of th enew process.
 */
static void _handle_fork(const pid_t pid) {
    // For some reason, we were unable to fork.
    if (pid < 0) {
        _exit_process(NO_FORK);
    }

    // Fork was successful so exit the parent process.
    if (pid > 0) {
        exit(OK);
    }
}

/**
 * Here, we handle the details of daemonizing a process.
 * This involves forking, opening the syslog connection,
 * configuring signal handling, and closing standard file
 * descriptors.
 */
static void _daemonize(void) {
    // Fork from the parent process.
    pid_t pid = fork();

    // Open syslog with the specified logmask.
    openlog(DAEMON_NAME, LOG_PID | LOG_NDELAY | LOG_NOWAIT, LOG_DAEMON);

    // Handle the results of the fork.
    _handle_fork(pid);

    // Now become the session leader.
    if (setsid() < -1) {
        _exit_process(NO_SETSID);
    }

    // Set our custom signal handling.
    signal(SIGTERM, _signal_handler);
    signal(SIGHUP, _signal_handler);

    // New file persmissions on this process, they need to be permissive.
    //umask(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    //umask(666);

    // Change to the working directory.
    chdir(WORKING_DIR);

    // Closing file descriptors (STDIN, STDOUT, etc.).
    for (long x = sysconf(_SC_OPEN_MAX); x>=0; x--) {
        close(x);
    }
}

static bool _commit_data(tc_heater_state_t heater_state, float temp) {
    CURL *curl;
    CURLcode res;

    char str_temp[6];
    ftoa(temp, str_temp, 1);

    char json[100];
    strcpy(json, "{ \"id\": \"");
    strcat(json, DEVICE_ID);
    strcat(json, "\", \"status\": \"");
    strcat(json, heater_state == ON ? "ON" : "OFF");
    strcat(json, "\", \"measurement\": ");
    strcat(json, str_temp);
    strcat(json, ", \"unit\": \"");
    strcat(json, UNIT);
    strcat(json, "\" }");

    curl = curl_easy_init();

    if (curl) {
        struct curl_slist *hs=NULL;
        hs = curl_slist_append(hs, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_URL, URL);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hs);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(json));
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            return false;
        }

        curl_easy_cleanup(curl);
    } else {
        return false;
    }

    return true;
}

/**
 * This runs the simulation. We essentially have a loop which
 * reads the heater state, adjust the temperature based on this
 * information, and writes the new temperature to the appropriate
 * location.
 */
static void _run_simulation(void) {

    // It's a bit cold! Note we're using a float in case we want to be
    // more sophisticated with the temperature management in the future.
    // Right now we just use a linear model.
    float temp = 64;

    syslog(LOG_INFO, "beginning thermocouple simulation");
    while(true) {
        // Read the heater state.
        tc_heater_state_t heater_state = OFF;
        tc_error_t err = tc_read_state(STATE_FILENAME, &heater_state);
        if (err != OK) _exit_process(err);

        // Is the heater on? then increase the temperature one degree.
        // Otherwise, it's getting colder!
        temp = (heater_state == ON) ? temp + 1 : temp - 1;

        // Write the temp to the file.
        err = tc_write_temperature(TEMP_FILENAME, temp);
        if (err != OK) _exit_process(err);

        _commit_data(heater_state, temp);
        // Take a bit of a nap.
        sleep(SLEEP_DELAY);
    }
}

/**
 * A utility function to test for file existance.
 *
 * @param filename The name of the file to check.
 */
static bool _file_exists(const char* filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0) ? true : false;
}

/**
 * A utility function to create a file.
 *
 * @param name The name of the file to create.
 */
static void _create_file(const char* name) {
    FILE* fp = fopen(name, "w");
    if (fp == NULL) {
        _exit_process(NO_OPEN);
    }
    fclose(fp);
}

/**
 * When we first start up, the various files we need to
 * use may not exist. If that is the case, we create them
 * here for future use.
 */
static void _configure(void) {
    if (!_file_exists(STATE_FILENAME)) {
        syslog(LOG_INFO, "no state file; creating.");
        _create_file(STATE_FILENAME);
    }

    if (!_file_exists(TEMP_FILENAME)) {
        syslog(LOG_INFO, "no temp file; creating.");
        _create_file(TEMP_FILENAME);
    }
    syslog(LOG_INFO, "test finished.");
}

/**
 * The daemon entry point.
 */
int main(void) {
    // Daemonize the process.
    // _daemonize();

    // Set up appropriate files if they don't exist.
    _configure();

    // Execute the primary daemon routines.
    _run_simulation();

    // If we get here, something weird has happened.
    return OK;
}