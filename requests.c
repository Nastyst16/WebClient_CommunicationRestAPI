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

char *compute_get_delete_request(char *host, char *url, char *query_params, char **cookies,
                                int cookies_count, char *token, int getOrDelete)
{
    int i;
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    if (!getOrDelete)
        if (query_params != NULL) {
            sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
        } else {
            sprintf(line, "GET %s HTTP/1.1", url);
        }
    else
        if (query_params != NULL) {
            sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
        } else {
            sprintf(line, "DELETE %s HTTP/1.1", url);
        }

    compute_message(message, line);


    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    sprintf(line, "User-Agent: Mozilla/5.0");
    compute_message(message, line);

    sprintf(line, "Connection: keep-alive");
    compute_message(message, line);

    if (cookies != NULL) {
        sprintf(line, "Cookie: ");

        for(i = 0; i < cookies_count - 1; i++){
            strcat(line, cookies[i]);
            strcat(line, "; ");
        }
        strcat(line, cookies[cookies_count - 1]);

        compute_message(message, line);
    }

    if(token != NULL) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    compute_message(message, "");
    return message;
}

char *compute_post_request(char *host, char *url, char* content_type, char **body_data,
                            int body_data_fields_count, char **cookies, int cookies_count, char *token)
{
    int i, len;
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *body_data_buffer = calloc(LINELEN, sizeof(char));

    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);

    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    sprintf(line, "Connection: keep-alive");
    compute_message(message, line);

    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    len = 0;
    for(i = 0; i < body_data_fields_count - 1; i++){
        len += strlen(body_data[i]) + 1;
    }

    len += strlen(body_data[body_data_fields_count - 1]);

    sprintf(line, "Content-Length: %d", len);
    compute_message(message, line);

    if (cookies != NULL) {
       sprintf(line, "Cookie: ");

        for(i = 0; i < cookies_count - 1; i++){
            strcat(line, cookies[i]);
            strcat(line, "; ");
        }

        strcat(line, cookies[cookies_count - 1]);

        compute_message(message, line);
    }

    if(token != NULL){
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    compute_message(message, "");


    for(i = 0; i < body_data_fields_count - 1; i++){
        strcat(body_data_buffer, body_data[i]);
        strcat(body_data_buffer, "&");
    }

    strcat(body_data_buffer, body_data[body_data_fields_count - 1]);

    memset(line, 0, LINELEN);
    compute_message(message, body_data_buffer);

    free(line);
    return message;
}