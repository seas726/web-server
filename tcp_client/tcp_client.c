/*
 CSCI 4211 Socket Programming Project
 Shane Stodolka | Stodo050
 10-26-22
 
 
 The goal of this client/server is to make a Server that returns all String
 messages into ALL CAPS.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>

#include <netinet/in.h>

#define MAX_MESSAGE_LEN 50
#define USER_FD 0


int connect_to_server(int port_num) {
    
    int net_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_num);
    server_address.sin_addr.s_addr = INADDR_ANY;
    
    int response = connect(net_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    
    if(response < 0) {
        perror("Could not connect to server");
        return -1;
    }
    
    return net_socket;
}

int main()
{
    int socket = connect_to_server(9002);
    int ret_val;
    
    fd_set read_fd_set[2];
    
    if(socket < 0){
        return -1;
    }
    
    char user_input[MAX_MESSAGE_LEN];
    char return_input[MAX_MESSAGE_LEN];
    
    recv(socket, &return_input, sizeof(return_input), 0); // wait for message response
    printf("Server: %s\n", return_input);
    //printf("#: ");
    fflush(stdout);

    // infinite loop to read from user/server
    while(1) {
        FD_ZERO(&read_fd_set); // reset the fd pointer
        FD_SET(USER_FD, &read_fd_set); // add in the file descriptors
        FD_SET(socket, &read_fd_set);
        
        ret_val = select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL); // select IO that is active
        
        if(ret_val >= 0) {
            //printf("Select has returned with : %d\n", ret_val);
            
            // STDIN is active
            if(FD_ISSET(USER_FD, &read_fd_set)) {
                
                // user has entered a command
                fgets(user_input, MAX_MESSAGE_LEN, stdin);
                
                if(send(socket, user_input, sizeof(user_input), 0) > 0 ) {
                    //printf("sent message to server");
                    continue;
                }
                else {
                    // uh oh no more server
                    printf("Server is unreachable.\n");
                    fflush(stdout);
                    break;
                }
            }
            // SERVER socket is active
            else if(FD_ISSET(socket, &read_fd_set)) {
                // server has sent a message
                if(recv(socket, &return_input, sizeof(return_input), 0) > 0) {
                    if(strcmp(return_input, "") == 0) {
                        continue;
                    }
                    
                    if(strcmp(return_input, "disconnect_ok") == 0) {
                        break;
                    }
                    printf("Server: %s\n", return_input);
                    fflush(stdout);

                }
                else {
                    printf("Server is unreachable.\n");
                    fflush(stdout);
                    break;
                }
            }
        }
    }

    printf("Exiting server...\n");
    close(socket);
    
    return 0;
}
