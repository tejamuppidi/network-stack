#ifndef HTTP_H
#define HTTP_H

#include <stdint.h>

#define HTTP_METHOD_MAX 8
#define HTTP_PATH_MAX   256

typedef struct
{
    char method[HTTP_METHOD_MAX];
    char path[HTTP_PATH_MAX];
} http_request_t;

/* Parse the first line of an HTTP request */
int parse_http_request(
    const uint8_t *payload,
    int payload_len,
    http_request_t *req);

/*
 * Build a simple HTTP response.
 *
 * Returns:
 *   number of bytes written
 *   -1 on error
 */
int build_http_response(
    char *buffer,
    int buffer_size);

#endif
