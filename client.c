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
#include "parson.h"

#define SERVER_IP "34.246.184.49"
#define SERVER_PORT 8080
#define HOST "34.246.184.49:8080"
#define BUFMAX 256
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

    char data0[BUFLEN];
    char data1[BUFLEN];
    char login[BUFLEN];
    char logout[BUFLEN];
    char message[BUFLEN];
    char response[BUFLEN];
    char cookies[BUFLEN];
    char *request;
    char tokens[BUFLEN];

} t_client;

int verify_error(t_client *client) {

    int ok_error = 0;

    if (strstr(client->response, "error") != NULL) {

        char *error = strstr(client->response, "error");
        error += 8;
        error[strlen(error) - 2] = '\0'; // remove the last "}

        printf("%s\n", error);

        ok_error = 1;
    }

    return ok_error;
}

int parse_command(char *argv[])
{
    char command[BUFMAX];
    scanf("%s", command);

    if (strcmp(command, "register") == 0) {
        return REGISTER;
    } else if (strcmp(command, "login") == 0) {
        return LOGIN;
    } else if (strcmp(command, "enter_library") == 0) {
        return ENTER_LIBRARY;
    } else if (strcmp(command, "get_books") == 0) {
        return GET_BOOKS;
    } else if (strcmp(command, "get_book") == 0) {
        return GET_BOOK;
    } else if (strcmp(command, "add_book") == 0) {
        return ADD_BOOK;
    } else if (strcmp(command, "delete_book") == 0) {
        return DELETE_BOOK;
    } else if (strcmp(command, "logout") == 0) {
        return LOGOUT;
    } else if (strcmp(command, "exit") == 0) {
        return EXIT;
    } else {
        return -1;
    }
}


void register_command(t_client client) // done
{
    int sockfd = client.sockfd;

    printf("username=");
    memset(client.data0, 0, BUFLEN);
    scanf("%s", client.data0);

    printf("password=");
    memset(client.data1, 0, BUFLEN);
    scanf("%s", client.data1);

    // create the message
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);

    json_object_set_string(root_object, "username", client.data0);
    json_object_set_string(root_object, "password", client.data1);

    strcpy(client.message, json_serialize_to_string_pretty(root_value));

    char *data[] = {client.message};
    client.request = compute_post_request(HOST, "/api/v1/tema/auth/register", "application/json", data, 1, NULL, 0, NULL);

    send_to_server(sockfd, client.request);

    strcpy(client.response, receive_from_server(sockfd));

    // searching for the ERROR string in the response
    if (strstr(client.response, "error") != NULL) {

        char *error = strstr(client.response, "error");
        error += 8;
        error[strlen(error) - 2] = '\0'; // remove the last "}

        printf("%s\n", error);

    } else {
        printf("Success, user: %s registered.\n", client.data0);
    }


}

void login_command(t_client *client)
{
    int sockfd = client->sockfd;

    printf("username=");
    memset(client->data0, 0, BUFLEN);
    scanf("%s", client->data0);

    printf("password=");
    memset(client->data1, 0, BUFLEN);
    scanf("%s", client->data1);

    // create the message
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);

    json_object_set_string(root_object, "username", client->data0);
    json_object_set_string(root_object, "password", client->data1);

    strcpy(client->message, json_serialize_to_string_pretty(root_value));

    char *data[] = {client->message};
    client->request = compute_post_request(HOST, "/api/v1/tema/auth/login", "application/json", data, 1, NULL, 0, NULL);

    send_to_server(sockfd, client->request);

    strcpy(client->response, receive_from_server(sockfd));

    // searching for the ERROR string in the response
    if (!verify_error(client)) {
        printf("Success, user: %s logged in.\n", client->data0);
    } else {
        return;
    }



    // no error. save the cookie
    char *cookie = strstr(client->response, "Set-Cookie: ");

    if (cookie != NULL) {
        cookie += 12;
        char *end = strstr(cookie, ";");
        end[0] = '\0';
        strcpy(client->cookies, cookie);

    } else {
        printf("No cookie received.\n");
    }

}


void enter_library_command(t_client *client) {

    int sockfd = client->sockfd;

    if (strlen(client->cookies) == 0) {
        printf("You must be logged in to enter the library.\n");
        return;
    }

    char *data[] = {client->cookies};
    client->request = compute_get_request(HOST, "/api/v1/tema/library/access", NULL, data, 1, NULL);

    send_to_server(sockfd, client->request);

    strcpy(client->response, receive_from_server(sockfd));

    // searching for the ERROR string in the response
    if (!verify_error(client)) {
        printf("Success, user: %s entered the library.\n", client->data0);
    } else {
        return;
    }


    // saving the token
    char *token = strstr(client->response, "token");
    token += 8;
    token[strlen(token) - 2] = '\0';

    memset(client->tokens, 0, BUFLEN);
    strcpy(client->tokens, token);
}


void get_books_command(t_client *client) {

    int sockfd = client->sockfd;

    if (strlen(client->tokens) == 0) {
        printf("You must be logged in to enter the library.\n");
        return;
    }

    // create the message
    client->request = compute_get_request(HOST, "/api/v1/tema/library/books", NULL, NULL, 0, client->tokens);

    send_to_server(sockfd, client->request);

    strcpy(client->response, receive_from_server(sockfd));

    // searching for the ERROR string in the response
    if (!verify_error(client)) {

        char *json_start = strstr(client->response, "[");

        JSON_Value *root_value = json_parse_string(json_start);
        printf("%s", json_serialize_to_string_pretty(root_value));

    } else {
        return;
    }
}


void get_book_command(t_client *client) {

    printf("id=");
    char *id = (char *) malloc(11);
    memset(id, 0, 11);
    scanf("%s", id);

    int sockfd = client->sockfd;

    if (strlen(client->tokens) == 0) {
        printf("You must be logged in to get a book.\n");
        return;
    }

    char path[BUFLEN];
    memset(path, 0, BUFLEN);
    strcpy(path, "/api/v1/tema/library/books/");
    strcat(path, id);


    // create the message
    client->request = compute_get_request(HOST, path, NULL, NULL, 0, client->tokens);

    send_to_server(sockfd, client->request);

    strcpy(client->response, receive_from_server(sockfd));

    if (!verify_error(client)) {
        char *json_start = strstr(client->response, "{");

        JSON_Value *root_value = json_parse_string(json_start);
        printf("%s", json_serialize_to_string_pretty(root_value));

    } else {
        return;
    }
}




void add_book_command(t_client *client) {

    int sockfd = client->sockfd;

    if (strlen(client->tokens) == 0) {
        printf("You must be logged in to add a book.\n");
        return;
    }

    // create the message
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);

    char book[BUFLEN];
    fgets(book, BUFLEN, stdin);

    printf("title=");
    memset(book, 0, BUFLEN);
    fgets(book, BUFLEN, stdin);
    // remove the newline character
    book[strlen(book) - 1] = '\0';
    json_object_set_string(root_object, "title", book);

    printf("author=");
    memset(book, 0, BUFLEN);
    fgets(book, BUFLEN, stdin);
    // remove the newline character
    book[strlen(book) - 1] = '\0';
    json_object_set_string(root_object, "author", book);

    printf("genre=");
    memset(book, 0, BUFLEN);
    fgets(book, BUFLEN, stdin);
    // remove the newline character
    book[strlen(book) - 1] = '\0';
    json_object_set_string(root_object, "genre", book);

    printf("publisher=");
    memset(book, 0, BUFLEN);
    fgets(book, BUFLEN, stdin);
    // remove the newline character
    book[strlen(book) - 1] = '\0';
    json_object_set_string(root_object, "publisher", book);

    printf("page_count=");
    memset(book, 0, BUFLEN);
    fgets(book, BUFLEN, stdin);
    // remove the newline character
    book[strlen(book) - 1] = '\0';
    json_object_set_number(root_object, "page_count", atoi(book));

    strcpy(client->message, json_serialize_to_string_pretty(root_value));

    char *data[] = {client->message};
    client->request = compute_post_request(HOST, "/api/v1/tema/library/books", "application/json",
                                            data, 1, NULL, 0, client->tokens);

    send_to_server(sockfd, client->request);

    strcpy(client->response, receive_from_server(sockfd));

    // searching for the ERROR string in the response
    if (!verify_error(client)) {
        printf("Success, user: %s added a book.\n", client->data0);
    } else {
        printf("%s", client->response);
        return;
    }
}




int main(int argc, char *argv[])
{
    t_client client;

    while (1) {

        client.sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
        if(client.sockfd < 0){
            error("Socket not available. It has other missions to accomplish.");
        }
        if (argc > 1) {
            printf("Invalid command.\n");
        }

        int command = parse_command(argv);

        // checkpoint debug

        switch(command) {
            case REGISTER:
                register_command(client);
                break;

            case LOGIN:
                login_command(&client);

                break;

            case ENTER_LIBRARY:
                enter_library_command(&client);

                break;

            case GET_BOOKS:
                get_books_command(&client);

                break;

            case GET_BOOK:
                get_book_command(&client);

                break;

            case ADD_BOOK:
                add_book_command(&client);

                break;

            case DELETE_BOOK:

                break;

            case LOGOUT:

                break;

            case EXIT:


                break;

            default:
                printf("Invalid command.\n");
                break;



        }

        close_connection(client.sockfd);
    }



    // free the allocated data at the end!
    close_connection(client.sockfd);
    // free(client.message);
    // free(response);

    return 0;
}
