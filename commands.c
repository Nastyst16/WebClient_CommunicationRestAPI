#pragma once

#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

#define SERVER_IP "34.246.184.49"
#define SERVER_PORT 8080
// #define SERVER_API "/api/v1/dummy"
// #define OPENWEATHER_API "api.openweathermap.org"
// #define OPENWEATHER_PORT 80


#define REGISTER 1
#define LOGIN 2
#define ENTER_LIBRARY 3
#define GET_BOOKS 4
#define GET_BOOK 5
#define ADD_BOOK 6
#define DELETE_BOOK 7
#define LOGOUT 8
#define EXIT 9


typedef struct {
    int sockfd;

    char *data[2];
    char *login;
    char *message;
    char *response;
    char *cookies[1];
} t_client;


int parse_command(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: ./client <command>\n");
        exit(0);
    }

    if (strcmp(argv[1], "register") == 0) {
        return REGISTER;
    } else if (strcmp(argv[1], "login") == 0) {
        return LOGIN;
    } else if (strcmp(argv[1], "enter_library") == 0) {
        return ENTER_LIBRARY;
    } else if (strcmp(argv[1], "get_books") == 0) {
        return GET_BOOKS;
    } else if (strcmp(argv[1], "get_book") == 0) {
        return GET_BOOK;
    } else if (strcmp(argv[1], "add_book") == 0) {
        return ADD_BOOK;
    } else if (strcmp(argv[1], "delete_book") == 0) {
        return DELETE_BOOK;
    } else if (strcmp(argv[1], "logout") == 0) {
        return LOGOUT;
    } else if (strcmp(argv[1], "exit") == 0) {
        return EXIT;
    } else {
        printf("Invalid command.\n");
        exit(0);
    }
}


void register_command(t_client client)
{
    int sockfd = client.sockfd;

    printf("username=");
    scanf("%s", client.data[0]);
    printf("password=");
    scanf("%s", client.data[1]);

    char *message = compute_post_request(SERVER_IP, "/api/v1/tema/auth/register", "application/json", client.data, 2, NULL, 0);
    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);
    printf("%s\n", response);

    free(message);
}

void login_command(t_client client)
{
    int sockfd = client.sockfd;

    printf("username=");
    scanf("%s", client.data[0]);
    printf("password=");
    scanf("%s", client.data[1]);

    char *message = compute_post_request(SERVER_IP, "/api/v1/tema/auth/login", "application/json", client.data, 2, NULL, 0);
    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);
    printf("%s\n", response);

    free(message);
}