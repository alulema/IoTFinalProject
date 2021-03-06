/**
 * This module contains the functions to send HTTP POST requests
 * to cloud services. The API of this module consists of the
 * send_post(json, url) and the send_post(json, url) functions.
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "tc_httpclient.h"

/**
 * Struct initialization method
 * @param s Struct to support saving server's response.
 */
void init_string(struct string *s) {
    s->len = 0;
    s->ptr = malloc(s->len + 1);

    if (s->ptr == NULL) {
        fprintf(stderr, "malloc() failed\n");
        exit(EXIT_FAILURE);
    }

    s->ptr[0] = '\0';
}

/**
 * Callback method to be called after the HTTP request returns a
 * JSON string as response
 * @param ptr
 * @param size
 * @param nmemb
 * @param s
 * @return
 */
size_t curl_write(void *ptr, size_t size, size_t nmemb, struct string *s) {
    size_t new_len = s->len + size * nmemb;
    s->ptr = realloc(s->ptr, new_len + 1);

    if (s->ptr == NULL) {
        fprintf(stderr, "realloc() failed\n");
        exit(EXIT_FAILURE);
    }

    memcpy(s->ptr + s->len, ptr, size * nmemb);
    s->ptr[new_len] = '\0';
    s->len = new_len;

    return size * nmemb;
}

/**
 * This method sends an HTTP POST request to the specified url.
 * It doesn't expects any response from server.
 * @param json JSON request body
 * @param url URL to cloud service
 * @return True if everything is ok.
 */
bool send_post(char* json, const char * url) {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();

    if (curl) {
        struct curl_slist *hs=NULL;
        hs = curl_slist_append(hs, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_URL, url);
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
 * This method sends an HTTP POST request to the specified url.
 * It doesn't expects any response from server.
 * @param json JSON request body
 * @param url URL to cloud service
 * @return JSON server's response
 */
char* send_post_str(char* json, const char * url) {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();

    if (curl) {
        struct curl_slist *hs=NULL;
        struct string s;
        init_string(&s);

        hs = curl_slist_append(hs, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
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

        return s.ptr;
    } else {
        return NULL;
    }

    return NULL;
}
