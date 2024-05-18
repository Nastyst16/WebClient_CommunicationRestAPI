#pragma once

#include "common.h"
#include "commands.c"


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

        int command = parse_command(argc, argv);

        switch(command) {
            case REGISTER:
                register_command(client);
                break;

            case LOGIN:
                login_command(client);

                break;

            case ENTER_LIBRARY:

                break;

            case GET_BOOKS:


                break;

            case GET_BOOK:

                break;

            case ADD_BOOK:

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
