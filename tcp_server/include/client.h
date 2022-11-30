
#ifndef CLIENT
#define CLIENT

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#include "util.h"
#include "topic.h"


#define MAX_SUBSCRIPTIONS 50

struct Client {
    int client_id; // keep track of the clients by ID
    //int active; // boolean, active = 1, inactive = 0;
    int socket_address; // the socket that the client is communicating from
    pthread_t thread_id; // the thread that the client is running on
    
    int num_subscriptions;
    int subscriptions[MAX_SUBSCRIPTIONS]; // array of topic Ids that client has subscribed to. -1 means no valid topic as 0 can be a valid topic.
    
    struct TopicList *subs; // Linked list of topics currenty subscribed to
    
};


struct Client *newClient(int client_id, int socket); // port num to initialize?

int addSub(struct Client **client, struct Topic *topic);
int removeSub(struct Client **client, struct Topic *topic);


//void list_subscriptions(struct Client *c);


#endif
