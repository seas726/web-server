#include "client.h"



struct Client *newClient(int c_id, int socket) {
    if(c_id < 0 || socket < 0) {
        return NULL;
    }
    
    struct Client *c = malloc(sizeof(struct Client));
    
    c->client_id = c_id;
    c->socket_address = socket;
    c->thread_id = (pthread_t)(-1); // set -1 until thread is created
    c->subs = NULL;
    
    c->num_subscriptions = 0;
    
    for(int i = 0; i < MAX_SUBSCRIPTIONS; i++) {
        c->subscriptions[i] = -1; // No subscriber here
    }
    
    return c;
}

int addSub(struct Client **client, struct Topic *topic) {
    if(topic == NULL || *client == NULL) {
        return FAILURE;
    }
    if((*client)->num_subscriptions >= MAX_SUBSCRIPTIONS) {
        return FAILURE;
    }

    struct TopicList *temp, *current;
    temp = newTopicList(topic);
    current = (*client)->subs;
    
    // case for empty list. if empty, assign head to new node and return
    if((*client)->subs == NULL) {
        (*client)->subs = temp;
        //printf("YEP ADDED FIRST TO LIST: %s\n", (*client)->subs->topic->name);
        (*client)->num_subscriptions++;
        return SUCCESS;
    } //case for list with one element
    else if(current->next == NULL) {
        if(current->topic == NULL || current->topic->topic_id == topic->topic_id) {
            return FAILURE;
        }
    }
    
    while(current->next != NULL) {
        // check if the topic is already in the linked list
        if(current->topic == NULL || current->topic->topic_id == topic->topic_id) {
            return FAILURE;
        }
        current = current->next;
    }
    
    current->next = temp;

    (*client)->num_subscriptions++;
    return SUCCESS;
}

int removeSub(struct Client **client, struct Topic *topic) {
    if(topic == NULL || (*client) == NULL) {
        return FAILURE;
    }
    if((*client)->num_subscriptions >= MAX_SUBSCRIPTIONS) {
        return FAILURE;
    }
    
    struct TopicList *current, *prev;
    current = (*client)->subs;
    prev = current;
    
    if(current == NULL)
        return FAILURE;
    
    // check the first element for success. removing this is easy.
    if(current->topic->topic_id == topic->topic_id) {
        (*client)->subs = (*client)->subs->next; // remove this as the starting point, shift the list left
        return SUCCESS;
    }
    
    /* its not the first node, lets check all the nodes after that*/
    current = current->next; // go to next element
    
    while(current != NULL && current->topic->topic_id != topic->topic_id) {
        prev = current;
        current = current->next;
    }
    
    if(current == NULL) {
        return FAILURE;
    }
    
    prev->next = current->next; // remove from linked list
    deleteTopicListElement(current); // delete and free memory
    (*client)->num_subscriptions--;
    return SUCCESS;
}



// make sure client ID is assigned properly!!!!
