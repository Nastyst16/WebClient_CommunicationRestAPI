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

    char username[BUFLEN];
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
        debug("Invalid command:  ", -1);
        debug(command, -1);
        return -1;
    }
}


void register_command(t_client client)
{
    int sockfd = client.sockfd;

    printf("username=");
    memset(client.username, 0, BUFLEN);
    scanf("%s", client.username);

    char secret_password[BUFLEN];
    printf("password=");
    memset(secret_password, 0, BUFLEN);
    scanf("%s", secret_password);

    // create the message
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);

    json_object_set_string(root_object, "username", client.username);
    json_object_set_string(root_object, "password", secret_password);

    strcpy(client.message, json_serialize_to_string_pretty(root_value));

    char *data[] = {client.message};
    client.request = compute_post_request(HOST, "/api/v1/tema/auth/register", "application/json", data, 1, NULL, 0, NULL);

    send_to_server(sockfd, client.request);

    strcpy(client.response, receive_from_server(sockfd));

    // searching for the ERROR string in the response
    if (!verify_error(&client)) {
        printf("Success, user: %s registered.\n", client.username);
    } else {
        return;
    }

}

void login_command(t_client *client)
{
    int sockfd = client->sockfd;

    printf("username=");
    memset(client->username, 0, BUFLEN);
    scanf("%s", client->username);

    char secret_password[BUFLEN];
    printf("password=");
    memset(secret_password, 0, BUFLEN);
    scanf("%s", secret_password);

    // create the message
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);

    json_object_set_string(root_object, "username", client->username);
    json_object_set_string(root_object, "password", secret_password);

    strcpy(client->message, json_serialize_to_string_pretty(root_value));

    char *data[] = {client->message};
    client->request = compute_post_request(HOST, "/api/v1/tema/auth/login", "application/json", data, 1, NULL, 0, NULL);

    send_to_server(sockfd, client->request);

    strcpy(client->response, receive_from_server(sockfd));

    // searching for the ERROR string in the response
    if (!verify_error(client)) {

        printf("Success, user: %s logged in.\n", client->username);
    } else {
        return;
    }


    client->tokens[0] = '\0';

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
        printf("Success, user: %s entered the library.\n", client->username);
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

    if (strlen(client->cookies) == 0) {
        printf("You must be logged in to get the books.\n");
        return;
    }

    if (strlen(client->tokens) == 0) {
        printf("You must enter the library to get the books.\n");
        return;
    }

    // create the message
    client->request = compute_get_request(HOST, "/api/v1/tema/library/books", NULL, NULL, 0, client->tokens);

    send_to_server(sockfd, client->request);

    strcpy(client->response, receive_from_server(sockfd));

    debug(client->response, -1);

    // searching for the ERROR string in the response
    if (!verify_error(client)) {

        char *json_start = strstr(client->response, "[");

        JSON_Value *root_value = json_parse_string(json_start);

        char to_print[BUFLEN];
        memset(to_print, 0, BUFLEN);
        strcpy(to_print, json_serialize_to_string_pretty(root_value));

        if (strlen(to_print) == 2) {
            printf("No books in the library.\n");
            return;
        }

        printf("%s\n", to_print);

    } else {
        return;
    }
}


void get_book_command(t_client *client) {

    if (strlen(client->cookies) == 0) {
        printf("You must be logged in to get a book.\n");
        return;
    }
    if (strlen(client->tokens) == 0) {
        printf("You must enter the library to get a book.\n");
        return;
    }

    printf("id=");
    char *id = (char *) malloc(11);
    memset(id, 0, 11);
    scanf("%s", id);

    int sockfd = client->sockfd;

    char path[BUFLEN];
    memset(path, 0, BUFLEN);
    strcpy(path, "/api/v1/tema/library/books/");
    strcat(path, id);


    // create the message
    client->request = compute_get_request(HOST, path, NULL, NULL, 0, client->tokens);

    send_to_server(sockfd, client->request);

    strcpy(client->response, receive_from_server(sockfd));

    free(id);

    if (!verify_error(client)) {
        char *json_start = strstr(client->response, "{");

        JSON_Value *root_value = json_parse_string(json_start);
        printf("%s\n", json_serialize_to_string_pretty(root_value));

    } else {
        return;
    }
}




void add_book_command(t_client *client) {

    int sockfd = client->sockfd;

    if (strlen(client->cookies) == 0) {
        printf("You must be logged in to add a book.\n");
        return;
    }
    if (strlen(client->tokens) == 0) {
        printf("You must enter the library to add a book.\n");
        return;
    }

    // create the message
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);

    char title[BUFLEN], author[BUFLEN], genre[BUFLEN], publisher[BUFLEN], page_count[BUFLEN];
    fgets(title, BUFLEN, stdin);

    printf("title=");
    memset(title, 0, BUFLEN);
    fgets(title, BUFLEN, stdin);
    // remove the newline character
    title[strlen(title) - 1] = '\0';
    json_object_set_string(root_object, "title", title);

    printf("author=");
    memset(author, 0, BUFLEN);
    fgets(author, BUFLEN, stdin);
    // remove the newline character
    author[strlen(author) - 1] = '\0';
    json_object_set_string(root_object, "author", author);

    printf("genre=");
    memset(genre, 0, BUFLEN);
    fgets(genre, BUFLEN, stdin);
    // remove the newline character
    genre[strlen(genre) - 1] = '\0';
    json_object_set_string(root_object, "genre", genre);


    printf("publisher=");
    memset(publisher, 0, BUFLEN);
    fgets(publisher, BUFLEN, stdin);
    // remove the newline character
    publisher[strlen(publisher) - 1] = '\0';
    json_object_set_string(root_object, "publisher", publisher);

    printf("page_count=");
    memset(page_count, 0, BUFLEN);
    fgets(page_count, BUFLEN, stdin);
    // remove the newline character
    page_count[strlen(page_count) - 1] = '\0';

    // verify if the page_count is a number
    for (int i = 0; i < strlen(page_count); i++) {
        if (page_count[i] < '0' || page_count[i] > '9') {
            printf("Incorrect data type for page_count. It must be a number.\n");
            return;
        }
    }
    json_object_set_number(root_object, "page_count", atoi(page_count));


    // verify if all the inputs are correct
    if (strlen(title) == 0 || strlen(author) == 0 || strlen(genre) == 0 || strlen(publisher) == 0 || strlen(page_count) == 0) {
        printf("At least one of the inputs is empty. Cannot add the book.\n");
        return;
    }


    strcpy(client->message, json_serialize_to_string_pretty(root_value));

    char *data[] = {client->message};
    client->request = compute_post_request(HOST, "/api/v1/tema/library/books", "application/json",
                                            data, 1, NULL, 0, client->tokens);

    send_to_server(sockfd, client->request);

    strcpy(client->response, receive_from_server(sockfd));

    // searching for the ERROR string in the response
    if (!verify_error(client)) {
        printf("Success, user: %s added a book.\n", client->username);
    } else {
        return;
    }
}

void delete_book_command(t_client *client) {

    if (strlen(client->cookies) == 0) {
        printf("You must be logged in to delete a book.\n");
        return;
    }
    if (strlen(client->tokens) == 0) {
        printf("You must enter the library to delete a book.\n");
        return;
    }

    printf("id=");
    char *id = (char *) malloc(11);
    memset(id, 0, 11);
    scanf("%s", id);

    int sockfd = client->sockfd;

    char path[BUFLEN];
    memset(path, 0, BUFLEN);
    strcpy(path, "/api/v1/tema/library/books/");
    strcat(path, id);


    // create the message
    client->request = compute_delete_request(HOST, path, NULL, NULL, 0, client->tokens);

    send_to_server(sockfd, client->request);

    strcpy(client->response, receive_from_server(sockfd));

    int book_id = atoi(id);
    free(id);

    if (!verify_error(client)) {

        ////////////// mai trebuie sa pui mesaj de erroare daca id ul nu este valid
        printf("Success, user: %s deleted the book with id: %d.\n", client->username, book_id);

    } else {
        debug("Error in delete_book_command", -1);
        return;
    }
}

void logout_command(t_client *client) {

        int sockfd = client->sockfd;

        if (strlen(client->cookies) == 0) {
            printf("You must be logged in to logout.\n");
            return;
        }

        char *data[] = {client->cookies};
        client->request = compute_get_request(HOST, "/api/v1/tema/auth/logout", NULL, data, 1, NULL);

        send_to_server(sockfd, client->request);

        strcpy(client->response, receive_from_server(sockfd));

        // searching for the ERROR string in the response
        if (!verify_error(client)) {
            printf("Success, user: %s logged out.\n", client->username);
        } else {
            return;
        }

        // reset the cookies
        memset(client->cookies, 0, BUFLEN);
        memset(client->tokens, 0, BUFLEN);
}




int main(int argc, char *argv[])
{
    t_client client;

    while (1) {

        client.sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
        if(client.sockfd < 0){
            printf("Serverul nu este disponibil.\n");
            return 0;
        }

        int command = parse_command(argv);

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
                delete_book_command(&client);
                break;

            case LOGOUT:
                logout_command(&client);
                break;

            case EXIT:
                close_connection(client.sockfd);
                return 0;

            default:
                printf("Invalid command.\n");
                break;
        }

        close_connection(client.sockfd);
    }

    // free the allocated data at the end!
    close_connection(client.sockfd);

    return 0;
}
