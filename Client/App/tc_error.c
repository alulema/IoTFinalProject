/**
 * Contains a function that maps an error code to a string.
 */
#include <stdlib.h>

#include "tc_error.h"

/**
 * Map an error code to a string.
 *
 * @param err The error code.
 * @return A string related to the error code.
 */
const char* tc_error_to_msg(const tc_error_t err) {
    char* msg = NULL;
    switch(err) {
        case OK:
            msg = "Everything is just fine.";
            break;
        case NO_OPEN:
            msg = "You tried to open a file but things did not go well.";
            break;
        case NO_FORK:
            msg = "Unable to fork a child process.";
            break;
        case NO_SETSID:
            msg = "Unable to set the session id.";
            break;
        case RECV_SIGTERM:
            msg = "Received a termination signal; exiting.";
            break;
        case RECV_SIGKILL:
            msg = "Received a kill signal; exiting.";
            break;
        case WEIRD_EXIT:
            msg = "An unexptected condition has come up, exiting.";
            break;
        case UNKNOWN_HEATER_STATE:
            msg = "Encountered an unknown heater state!";
            break;
        default:
            msg = "You submitted some kind of wackadoodle error code. What's up with you?";
    }
    return msg;
}