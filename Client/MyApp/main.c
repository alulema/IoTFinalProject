/**
 * The main entry point of the thermocouple controller program.
 *
 * This program controlles a thermocouple as a deamon process.
 * It is designed to be started at system startup in the init
 * (or alternative) hierarchy under linux. This module contains
 * the entry point (main()) at the bottom of the file. All
 * other functions are module private functions (i.e. static),
 * and not part of the exposed API. The only exposed function
 * is main().
 */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <syslog.h>
#include <zconf.h>

#include "tc_error.h"
#include "tc_config.h"
#include "tc_httpclient.h"
#include "tc_state.h"
#include "tc_json.h"

static const char *DAEMON_NAME         = "tccontroller";
static const char *URL                 = "http://52.67.91.154:5000/api/thermostat";
static const char *URL_ONLINE          = "http://52.67.91.154:5000/api/thermostat/online";
static const char *TEMP_FILENAME       = "/tmp/temp";
static const char *STATE_FILENAME      = "/tmp/status";
static const char *WORKING_DIR         = "/";
static char *DEVICEID            = "THE-001";

static const float DEFAULT_TEMPERATURE = 65;
static const long  SLEEP_DELAY         = 1;

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

/**
 * This method initiates the device, it reads DEVICE_ID and UNIT
 */
static void _initialize(void) {
    syslog(LOG_INFO, "Starting thermocouple controller");
}

/**
 * This method runs the process. It starts pinging the cloud to say "I'm online".
 * Next, it reads temperature and state, and send it to cloud.
 * Finally, it executes the action on the actuator to turn on/off it.
 */
static void _run_process(void) {

    char* online_request = create_online_request(DEVICEID, "F");

    while (true) {
        // Send ping to cloud to indicate the device is online
        send_post(online_request, URL_ONLINE);

        // Read heater state and temperature.
        tc_heater_state_t heater_state = OFF;
        char *temperature;
        tc_error_t err = tc_read_state(STATE_FILENAME, &heater_state);
        tc_error_t err2 = tc_read_temperature(TEMP_FILENAME, &temperature);
        strtok(temperature, "\n");

        if (err != OK || err2 != OK) {
            _exit_process(err);
        }

        // Sends data to cloud, and receives the action to perform in actuator
        char * post_data_request = create_post_data_request(DEVICEID, "F", heater_state, temperature);
        char* action = send_post_str(post_data_request, URL);

        if (action == NULL) {
            if (atof(temperature) > DEFAULT_TEMPERATURE) {
                err = tc_write_state(STATE_FILENAME, OFF);
                if (err != OK) _exit_process(err);
                syslog(LOG_INFO, "Turning OFF heater");
            } else {
                err = tc_write_state(STATE_FILENAME, ON);
                if (err != OK) _exit_process(err);
                syslog(LOG_INFO, "Turning ON heater");
            }
        } else  if (strcmp(action, "1") == 0) {
            if (heater_state != ON) {
                err = tc_write_state(STATE_FILENAME, ON);
                if (err != OK) _exit_process(err);
                syslog(LOG_INFO, "Turning ON heater");
            }
        } else {
            if (heater_state != OFF) {
                err = tc_write_state(STATE_FILENAME, OFF);
                if (err != OK) _exit_process(err);
                syslog(LOG_INFO, "Turning OFF heater");
            }
        }

        sleep(SLEEP_DELAY);
    }
}

/**
 * The daemon entry point.
 */
int main() {
    // Daemonize the process.
    _daemonize();

    // Initializes device settings
    _initialize();

    // Execute the primary daemon routines.
    _run_process();

    // If we get here, something weird has happened.
    return OK;
}