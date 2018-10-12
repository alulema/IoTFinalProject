#include <stdbool.h>

#ifndef MYAPP_TC_HTTPCLIENT_H
#define MYAPP_TC_HTTPCLIENT_H

struct string {
    char *ptr;
    size_t len;
};

bool send_post(char* json, const char * url);
char* send_get(char* url);

#endif //MYAPP_TC_HTTPCLIENT_H
