#include "http.h"

#include <stdio.h>
#include <string.h>

int parse_http_request(
    const uint8_t *payload,
    int payload_len,
    http_request_t *req)
{
    if (payload == NULL ||
        req == NULL ||
        payload_len <= 0)
    {
        return -1;
    }

    memset(req, 0, sizeof(*req));

    char buffer[1024];

    if (payload_len >= (int)sizeof(buffer))
        payload_len = sizeof(buffer) - 1;

    memcpy(buffer, payload, payload_len);
    buffer[payload_len] = '\0';

    char version[32];

    if (sscanf(buffer,
               "%7s %255s %31s",
               req->method,
               req->path,
               version) != 3)
    {
        return -1;
    }

    if (strncmp(version, "HTTP/", 5) != 0)
        return -1;

    return 0;
}


int build_http_response(
    char *buffer,
    int buffer_size)
{
    const char *body =
        "Hello from Mini TCP/IP Stack!\n";

    int body_length = strlen(body);

    int written = snprintf(
        buffer,
        buffer_size,

        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",

        body_length,
        body);

    if (written < 0 ||
        written >= buffer_size)
    {
        return -1;
    }

    return written;
}
