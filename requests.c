#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(const char *host, const char *url, const char *query_params, char **cookies, int cookies_count, const char *token) {
    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char line[LINELEN];

    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }
    compute_message(message, line);

    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if (token != NULL) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    if (cookies != NULL && cookies_count > 0) {
        sprintf(line, "Cookie: ");
        for (int i = 0; i < cookies_count; i++) {
            strcat(line, cookies[i]);
            if (i < cookies_count - 1) {
                strcat(line, "; ");
            }
        }
        compute_message(message, line);
    }

    compute_message(message, "");
    return message;
}

char *compute_post_request(const char *host, const char *url, const char *content_type, char **body_data, int body_data_fields_count, char **cookies, int cookies_count, const char *token) {
    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char line[LINELEN];

    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);

    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    if (token != NULL) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    if (cookies != NULL && cookies_count > 0) {
        sprintf(line, "Cookie: ");
        for (int i = 0; i < cookies_count; i++) {
            strcat(line, cookies[i]);
            if (i < cookies_count - 1) {
                strcat(line, "; ");
            }
        }
        compute_message(message, line);
    }

    int content_length = 0;
    for (int i = 0; i < body_data_fields_count; i++) {
        content_length += strlen(body_data[i]);
    }
    sprintf(line, "Content-Length: %d", content_length);
    compute_message(message, line);

    compute_message(message, "");

    for (int i = 0; i < body_data_fields_count; i++) {
        strcat(message, body_data[i]);
    }

    return message;
}

char *compute_delete_request(const char *host, const char *url, const char *query_params, char **cookies, int cookies_count, const char *token) {
    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char line[LINELEN];

    if (query_params != NULL) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }
    compute_message(message, line);

    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if (token != NULL) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    if (cookies != NULL && cookies_count > 0) {
        sprintf(line, "Cookie: ");
        for (int i = 0; i < cookies_count; i++) {
            strcat(line, cookies[i]);
            if (i < cookies_count - 1) {
                strcat(line, "; ");
            }
        }
        compute_message(message, line);
    }

    compute_message(message, "");
    return message;
}
