/**
 * Utility methods to perform HTTP POST requests to specified urls.
 */
#include <stdbool.h>

#ifndef MYAPP_TC_HTTPCLIENT_H
#define MYAPP_TC_HTTPCLIENT_H

/**
 * An enum type to help receiving strings from POST request.
 */
struct string {
    char *ptr;
    size_t len;
};

bool send_post(char* json, const char * url);
char* send_post_str(char* json, const char * url);

#endif
