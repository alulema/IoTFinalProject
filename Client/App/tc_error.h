/**
 * Application specific error codes. The module API for
 * similation errors.
 */
#ifndef __TC_ERROR_H__
#define __TC_ERROR_H__

/**
 * An enumerated type of error codes supported in the simulator.
 */
typedef enum {
    OK,
    NO_OPEN,
    NO_FORK,
    NO_SETSID,
    RECV_SIGTERM,
    RECV_SIGKILL,
    WEIRD_EXIT,
    UNKNOWN_HEATER_STATE
} tc_error_t;

const char* tc_error_to_msg(const tc_error_t err);

#endif