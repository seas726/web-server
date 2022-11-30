/*
 CSCI 4211 Socket Programming Project
 Shane Stodolka | Stodo050
 10-26-22
 */

// standard
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

//network
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

//threads
#include <pthread.h>

// helper files
#include "util.h"
#include "topic.h"
#include "path.h"
#include "client.h"

// define global max variables to use
#define MAX_CONNECTIONS 5

/* global variables */

struct Client *clients[MAX_CONNECTIONS]; // an array to keep track of all connected clients
struct Topic *root;
int *topic_subscriptions[MAX_TOPICS]; // store topics subscriptions based on topicID

int num_topic_subscriptions[MAX_TOPICS];
int num_clients = 0; // keep track of how many clients are currently connected

// functions
int subscribe(struct Client *client, struct Topic *topic);
int un_subscribe(struct Client *client, struct Topic *topic);

// load select topics from the designated topics file and add them to hierarchy
void init_topics() {

    root = newTopic("/"); // initialize the root topic, every topic will be under this

    FILE *f;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    
    f = fopen("topics.txt", "r"); // open file that contains topics
    if(f == NULL)
        return;
    
    while((read = getline(&line, &len, f)) != -1) {
        //printf("line: %s\n", line);
        add_topic(line, root); // add the path as a new topic
    }
    
    fclose(f); // close file
    
    if(line) { // free memory
        free(line);
    }
    
    for(int i = 0; i < MAX_TOPICS; i++) {
        num_topic_subscriptions[i] = 0; // initialize topic subscription arrays
        topic_subscriptions[i] = NULL;
    }
}

// safley close out client connection
void drop_client(struct Client **client) {
    
    struct TopicList *current = (*client)->subs; // reference to
    
    while(current != NULL) {
        if(un_subscribe((*client), current->topic) == FAILURE) {
            //printf("could not unsub %s\n", current->topic->name);
        }
        current = current->next;
    }
    
    clients[(*client)->client_id] = NULL; // mark this client as useable
    num_clients--;
    close((*client)->socket_address); // close out socket so that it dosent use system recources
    if((*client))
        free((*client));
}

// Initialize client array. All unused clients should be NULL
void init_clients() {
    for(int i = 0; i < MAX_CONNECTIONS; i++) {
        clients[i] = NULL;
    }
}

// returns the next un-used client structure. if all clients are in use, return  -1 for error
int next_free_client() {
    if(num_clients < MAX_CONNECTIONS) {
        for(int i = 0; i < MAX_CONNECTIONS; i++) {
            if(clients[i] == NULL) // if the client is currently not active, then give it out.
                return i;
        }
    }
    return -1;
}


/* CLIENT SUBSCRIPTION FUNCTIONS */
/* --------------------------------------------------------------------------------------------------------------------------------------- */

void list_subscriptions(struct Client *client) {

    struct TopicList *current = client->subs;
    
    while(current != NULL) {
        send(client->socket_address, current->topic->name, sizeof(current->topic->name), 0);
        current = current->next;
    }
}


// this function will subscribe to all grandchildren of topic with name t_name
int sub_single_level_wildcard(struct Client *client, struct Topic *topic, char *t_name) {
    
    for(int i = 0; i < topic->num_children; i++) {
        struct Topic *child = topic->children[i];
        
        if(child != NULL) {
            for(int j = 0; j < child->num_children; j++) {
                struct Topic *grandchild = child->children[j];
                if(grandchild != NULL) {
                    if(strcmp(grandchild->name, t_name) == SUCCESS)
                        subscribe(client, grandchild);
                }
                else return FAILURE;
            }
        }
        else {
            return FAILURE;
        }
    }
    return SUCCESS;
}

int sub_wildcard(struct Client *client, struct Topic *root) {
    
    for(int i = 0; i < root->num_children; i++) {
       // printf("root: %s | child: %s\n", root->name, root->children[i]->name );
        if(subscribe(client, root->children[i]) == FAILURE){
            printf("failed to subscribe %s.\n", root->children[i]->name);
        }
        sub_wildcard(client, root->children[i]);
    }
    return SUCCESS;
}

int subscribe(struct Client *client, struct Topic *topic) {
    
    if(client == NULL || topic == NULL)
        return FAILURE;
    
    if(num_topic_subscriptions[topic->topic_id] >= MAX_SUBSCRIBERS)
        return FAILURE;
    
    int t_id = topic->topic_id; // reference to topic id
    
    if(addSub(&client, topic) == SUCCESS) {
        
        if(topic_subscriptions[t_id] == NULL) {
            topic_subscriptions[t_id] = (int*)malloc(sizeof(int) * MAX_SUBSCRIBERS);
        }
        if(num_topic_subscriptions[topic->topic_id] >= MAX_SUBSCRIBERS) {
            return -1; // max subscribers reached, return error
        }

        num_topic_subscriptions[t_id] = num_topic_subscriptions[t_id] + 1; // increase num subscriptions
        topic_subscriptions[t_id][num_topic_subscriptions[t_id] - 1] = client->client_id; // add client_id to subscribers
        
        printf("client %d subscribed to to: %s | %d\n", client->client_id, topic->name, topic->topic_id);
        
        send(client->socket_address, topic->message, sizeof(topic->message), 0);
        
        return SUCCESS; // success
    }
    return FAILURE;
}

/*
 UNSUBSCRIBE will validate that a client is subscribed to a given topic
 and then remove that subscription from both the clients array and the global array
 
 */

int un_subscribe(struct Client *client, struct Topic *topic) {
    
    if(client == NULL || topic == NULL)
        return FAILURE;
    
    if(num_topic_subscriptions[topic->topic_id] == 0) {
        return FAILURE; // error, no topic to remove
    }
    
    
    int t_id = topic->topic_id; // reference to topic id
        
    if(removeSub(&client, topic) == SUCCESS) {
        int num_subs = num_topic_subscriptions[t_id]; // reference to number of clients subscribed to a given topic. used for readability.
        
        for(int j = 0; j < num_subs; j++) {
            if(topic_subscriptions[t_id][j] == client->client_id) {
                
                topic_subscriptions[t_id][j] = topic_subscriptions[t_id][num_subs - 1];
                topic_subscriptions[t_id][num_subs - 1] = -1; // remove the client subscription from global list
                num_topic_subscriptions[t_id] = num_subs - 1;
            }
        }
        //printf("NUM SUB AFTER UNSUB: %d\n", num_topic_subscriptions[topic_id]);
        
        /*
        for (int i = 0; i < num_topic_subscriptions[t_id]; i++) {
            printf("subscriptions after unsub:");
            if(topic_subscriptions[t_id][i] >= 0)
               printf(" %d\n", topic_subscriptions[t_id][i]);
        }
         */
        
        printf("client %d unsubscribed from: %s | %d\n", client->client_id, topic->name, topic->topic_id);
        
        return SUCCESS; // success - maybe should return the topic_id unsubeed from.. wait i dont need to do this
    }
    return FAILURE; // error return code
}


// publish a message to all subscribers of a topic
int publish(struct Topic *topic, char *message) {
    
    if(topic == NULL || message == NULL) {
        return -1;
    }
    
    int* subscribed_clients = topic_subscriptions[topic->topic_id]; // reference to array of all subscribed client_ids
    int num = num_topic_subscriptions[topic->topic_id];
    
    for(int i = 0; i < num; i++) {
        struct Client *c = clients[subscribed_clients[i]]; // reference to client structure
        
        if(c != NULL){
            if(send(c->socket_address, message, sizeof(message), 0) > -1) { // send message of acceptance
                printf("published message to client %d\n", c->client_id);
                continue;
            }
        }
    }
    return 0;
}

/* --------------------------------------------------------------------------------------------------------------------------------------- */

/*
 Gets the first word seperated by a space in a string.
 
 This is a helper function for reading in client commands as
 they are seperated by a space in this scenario
 
 */

char* get_next_word(char** request) {
    
    if(*request == NULL ) return NULL;
    
    char *arg = malloc(sizeof(char) * MAX_MESSAGE_LEN); // allocate enough memory to store a reponse
    
    for(int i = 0; i < strlen(*request); i++ ) {
        if((*request)[i] == ' ') {
            strncpy(arg, (*request), i);
            memmove((*request), (*request) + i + 1, MAX_MESSAGE_LEN); // this line will remove the first arg from request. add 1 for space
            return arg;
        }
    }
    
    // no space, only word.
    
    strcpy(arg,(*request));
    (*request) = NULL;
    arg[strcspn(arg, "\n")] = 0; // remove the newline char
    
    return arg;
}

/*
 process_client_request takes in a clent request as a string of arguements and processes each arguement.
 A response string is returned to the client thread detailing either success or failure of
 the requested operation
 */

char* process_client_request(char* request, struct Client *client) {
    
    int ret_val; // use this to store all return values
    
    char *arg1; // store the first arguement from client
    char *response = malloc(sizeof(char) * MAX_MESSAGE_LEN); // allocate memory for the response string
    
    arg1 = get_next_word(&request); // get the first client arguement from the request
    
    

    if(strcmp(arg1, "disconnect") == 0) {
        strcpy(response, "disconnect_ok"); // "disconnect_ok is seen by the client as an ACK of disconnection"
    }
    else if (strcmp(arg1, "list") == 0) {
        list_subscriptions(client);
    }
    else {
        
        char *arg2; // store topic / arg 2 here
        arg2 = get_next_word(&request); // get topic
        struct Path *p = newPath(arg2); // this is a malloc, free please
        printf("MADE IT HERE\n");

        struct Topic *t = NULL;
        
        ret_val = is_valid_topic(p, root, &t);
        
        if(t == NULL || ret_val == -1) {
            strcpy(response, "please enter a valid topic"); // exits after this
            // free memory
            if(arg2 != NULL)
                free(arg2);
            if(arg1 != NULL)
                free(arg1);
            return response;
        }
            
        if(strncmp(arg1, "publish", 7) == 0) {
            
            char *arg3, *arg4; // publish requires a third arguement, the message to send
            arg3 = get_next_word(&request);
            arg4 = get_next_word(&request);
            
            if(arg3 != NULL) {
                printf("arg3: %s\n", arg3);
            }
            
            if(arg4 != NULL) {
                printf("arg4: %s\n", arg4);
            }
            
            if(publish(t, arg3) == SUCCESS) {
                if((arg4 != NULL) && (strcmp(arg4, "-r") == SUCCESS)) {
                    strcpy(t->message, arg3);
                    strcpy(response, "message published and retained to ");
                }
                else {
                    strcpy(response, "message published to ");
                }
            }
            else {
                strcpy(response, "error, could not publish to ");
            }
            if(arg3 != NULL)
                free(arg3); // free the memory malloced by get_next_word
        }
        else if(strncmp(arg1, "subscribe", 9) == 0) {
            // add third arg to subscribe / unsubscribe that will account for wildcards??
            
            // WILDCARD CASE
            if(ret_val == 1) {
                if(sub_wildcard(client,t) == 0) {
                    strcpy(response, "you have subscribed to all topics under ");
                }
                else {
                    strcpy(response, "error, could not subscribe wildcard ");
                }
            }
            // SINGLE LEVEL WILDCARD CASE
            else if(ret_val == 2) {
                
                char *t_name = p->elements[p->depth - 1];
                
                if(sub_single_level_wildcard(client,t, t_name ) == SUCCESS) {
                    strcpy(response, "you have subscribed topics under ");
                }
                else {
                    strcpy(response, "error, could not subscribe wildcard ");
                }
            }
            // SINGLE SUBSCRIPTION CASE
            else {
                if(subscribe(client, t) == 0) {
                    strcpy(response, "you have subscribed to ");
                }
                else {
                    strcpy(response, "error, could not subscribe to ");
                }
            }
        }
        else if(strncmp(arg1, "unsubscribe", 11) == 0) {
            
            // wildcards not supported for unsubscribing
            if(ret_val == 1 || ret_val == 2) {
                strcpy(response, "error, cannot use wildcards to unsubscribe ");
            }
            else {
                // OK, no wildcard detected
                if((ret_val = un_subscribe(client, t)) == 0) {
                    strcpy(response, "you have unsubscribed from ");
                }
                else if (ret_val == -2){
                    strcpy(response, "error, not currently subscribed to ");
                }
                else {
                    strcpy(response, "error, could not unsubscribe from ");
                }
            }
        }
        else { // not valid action
            strcpy(response, "command not found '"); // update to include the faulty command?
            strcat(response, arg1); // add topic onto the end of message
            strcat(response, "'"); // add topic onto the end of message

            if(arg2 != NULL)
                free(arg2);
            if(arg1 != NULL)
                free(arg1);
            return response;
        }
        
        printf("command: %s\n", arg1);
        printf("arg: %s\n", arg2);
        strcat(response, arg2); // add topic onto the end of message, respond to the useer
        
        
        if(p != NULL) {
            free(p);
        }
        
        // free memory
        if(arg2 != NULL)
            free(arg2);
        if(arg1 != NULL)
            free(arg1);
        
        return response;
    }


    if(arg1 != NULL)
        free(arg1); // free memory allocated by get_next_word
    return response;
}

// Thread function to manage client connections
void *client_thread(void* client) {
    struct Client *c = (struct Client*)client; // access the client data
    
    printf("Client: %d has joined\n", c->client_id);
    
    char message[MAX_MESSAGE_LEN] = "Welcome to Shane's subscription server!";
    char client_message[MAX_MESSAGE_LEN];
    send(c->socket_address, message, sizeof(message), 0); // send message of acceptance
    
    while(1) {
        // check to see if client is still connected
        printf("Waitng for client: %d\n", c->client_id);
        if(recv(c->socket_address, &client_message, sizeof(client_message), 0) > 0) {
            
            char *r = process_client_request(client_message, c); // retrive response
            send(c->socket_address, r, sizeof(client_message), 0); // send response to client
            printf("Message sent back to client!\n");
            free(r); // free the memory allocated when creating response
        }
        else { // connection has been lost
            printf("Connection has been terminated by client\n");
            drop_client(&c); // safely disconnect client
            pthread_exit(NULL);
            return NULL;
        }
    }
    pthread_exit(NULL);
    return NULL;
}

int main() {
    
    // initialize server values
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9002); // add port numbe as agrument
    server_address.sin_addr.s_addr = INADDR_ANY;
    
    bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));
    
    
    init_topics();
    printTopics(root, 0); // print out all of the topics after they have been initialized
    //printTopicsArray(topics, num_topics);
    
    // server should wait for incomming connections
    
    while(1) {

        printf("Listening for connection\n");
        listen(server_socket, 5); // wait for a client connection
        int new_client_socket;
        // accept incoming connection, returns a new socket descriptor.
        if((new_client_socket = accept(server_socket, NULL, NULL)) > -1) {
            
            int new_client_num = next_free_client(); // gets a client from our 'client tracker'
            
            if(new_client_num < 0) {
                printf("Max connections reached. Please try again later.\n");
                continue; // continue so that clients can keep trying to reach the server
            }
            else {
                struct Client *new_client =  newClient(new_client_num, new_client_socket); // create new client and find pointer
                
                if(new_client == NULL) {
                    printf("ERROR: Could not create client");
                    continue;
                }
                clients[new_client->client_id] = new_client;
                pthread_create(&clients[new_client_num]->thread_id, NULL, &client_thread, clients[new_client_num]);
                num_clients++;
            }
        }
    }
    // close server and end the program
    
    close(server_socket);
    return 0;
}
