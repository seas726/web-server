


#ifndef TOPIC
#define TOPIC

#include "util.h"
#include "path.h"


#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#define MAX_CHILDREN 10
#define MAX_SUBSCRIBERS 10
#define MAX_TOPICS 500

#define WILDCARD 1
#define SINGLE_LEVEL_WILDCARD 2

struct Topic {
    int topic_id;
    char name[MAX_MESSAGE_LEN];
    char message[MAX_MESSAGE_LEN];
    int num_children;
    struct Topic *children[MAX_CHILDREN];
};

struct TopicList {
    struct Topic *topic;
    struct TopicList *next;
};

/* Topic functions */

struct Topic *newTopic(char *n);
struct TopicList *newTopicList(struct Topic *t);


 
void delete_topic(struct Topic *t);
void deleteTopicListElement(struct TopicList *tl); // delete and deallocate memory

void add_topic_rec(struct Path *p, struct Topic *r, int depth);
void add_topic(char *path, struct Topic *r);

int is_valid_topic(struct Path *path, struct Topic *root, struct Topic **ret);
struct Topic *is_valid_topic_return(struct Path *path, struct Topic *root, int depth);
struct Topic *get_topic_by_id(int topic_id, struct Topic *root);

void printTopics(struct Topic *root, int depth);
void printTopicsArray(struct Topic **t, int num);

void addChild(struct Topic *root, struct Topic *child);






#endif
