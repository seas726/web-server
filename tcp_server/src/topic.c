#include "topic.h"

int num_topics = 0;

/*
 initialize a new topic with name n
 this funcntion mallocs and
 returns a pointer to a topic struct
 */

struct Topic *newTopic(char *n) {
    
    if(num_topics < 0 || n == NULL)
        return NULL;
    
    struct Topic *topic = malloc(sizeof(struct Topic));
    int i = 0;
    
    if(topic == NULL) // check malloc error
        return NULL;
    
    
    topic->topic_id = num_topics;
    strcpy(topic->name, n);
    topic->num_children = 0;
    
    for(i = 0; i < MAX_CHILDREN; i++) {
        topic->children[i] = malloc(sizeof(struct Topic*));
    }
    
    num_topics = num_topics + 1; // increment the global topic counter
    return topic;
}

struct TopicList *newTopicList(struct Topic *t) {
    if(t == NULL) {
        return NULL;
    }
    
    struct TopicList *newList = malloc(sizeof(struct TopicList)); // allocate enough memory for the structure
    
    newList->topic = t;
    newList->next = NULL; // since this is always being added onto the end it should be null.
    
    return newList;
}


void addChild(struct Topic *root, struct Topic *child) {
    
    if(root->num_children >= MAX_CHILDREN) {
        printf("Max children reached.\n");
        return;
    }
    
    root->children[root->num_children] = child;
    root->num_children++;
}

// Delete a topic and free its memeory
void delete_topic(struct Topic *t) {
    
    for(int i = 0; i < t->num_children; i++) {
        delete_topic(t->children[i]);
    }
    
    if( t!= NULL)
        free(t);
}
void deleteTopicListElement(struct TopicList *tl) {
    if(tl == NULL)
        return;
    tl->topic = NULL;
    tl->next = NULL;
    free(tl);
}


/*
 ADD TOPIC
 
 Add a topic to a given tree. Topic must be given in the form of a path structure.

 */

void add_topic_rec(struct Path *p, struct Topic *r, int depth) {

    if(r->num_children >= 10) {
        printf("Cannot add topic %s, the tree is full.\n", p->elements[depth]);
        return;
    }
    
    // recursive end case, only explore as deep as the path goes
    if(depth > (p->depth - 1)) {
        return;
    }
    
    if(p->elements[depth][0] == '#' || p->elements[depth][0] == '/' || p->elements[depth][0] == '+') {
        printf("cannot add '#', '+', or '/' to tree.\n");
        return;
    }
     
    // if there are children, explore.
    if(r->num_children > 0) {
        for(int i = 0; i < r->num_children; i++) {
            char *c_name = r->children[i]->name; // ref to current levels name
            //printf("%s | %s\n", c_name, p->elements[0])
            if(strcmp(c_name, p->elements[depth]) == 0) {
                // yay path exists
                add_topic_rec(p, r->children[i], depth + 1); // continue on
                return; // exit
            }
            else {
                //printf("path does not exist, adding it now\n");
                continue;
            }
        }
    }

    /*
     Since the given topic does NOT already exist we must make it.
     */
    
    struct Topic *child = newTopic(p->elements[depth]); // new topic is child of the root
    //printf("topic added: %s\n", p->elements[depth]);
    addChild(r, child); // add a child to the root because it does not exist yet
    //topics[num_topics] = child;
    add_topic_rec(p, child, depth + 1); // child-> new root :)
    
    return;
}

// starter function for above recursive implementation
void add_topic(char *path, struct Topic *r) {
    struct Path *p = newPath(path);
    add_topic_rec(p, r, 0);
    deletePath(p); // free path because the structure is no longer needed
}

int is_valid_topic(struct Path *path, struct Topic *root, struct Topic **ret) {
    
    if(path == NULL || root == NULL) return FAILURE;
    
    // go through each entry in the path
    for(int i = 0; i < path->depth; i++) {
        if(path->elements[i][0] == '#') {
            printf("WILDCARD DETECTED at %d\n", i);
            *ret = root;
            return WILDCARD;
        }
        else if(path->elements[i][0] == '+') {
            printf("MULTI_LEVEL DETECTED at %d\n", i);
            *ret = root;
            return SINGLE_LEVEL_WILDCARD;
        }
        
        // loop through children of each entry
        for(int j = 0; j < root->num_children; j++) {
            
            if(strcmp(root->children[j]->name, path->elements[i]) == 0) {
                root = root->children[j]; // assign next root to found child
                if(i == (path->depth) - 1) {
                    *ret = root; // set return topic to the found match
                    printf("SUCESS: %s\n", (*ret)->name);
                    
                    return 0;
                }
                break;
            }
        }
    }
    *ret = NULL;
    return -1;
}


// if its a valid topic, return the topic structure
struct Topic *is_valid_topic_return(struct Path *path, struct Topic *root, int depth) {
    
    if(depth >= path->depth) {
        return  NULL;
    }
    if(path->elements[depth][0] == '#') {
        printf("WILDCARD DETECTED\n"); // if a wildcard is detected we must subscribe to all children of all children and shit.
    }
    
    // loop through all the children. if none of the children match at depth
    for(int i = 0; i < root->num_children; i++) {
       // printf("topic: %s | %d\n", root->children[i]->name, depth);
        //printf("path: %s | %d\n", path->elements[depth], path->depth);
        if(strcmp(root->children[i]->name, path->elements[depth]) == 0) {
            if(depth == (path->depth) - 1) {
                return root->children[i];
            }
            return is_valid_topic_return(path, root->children[i], depth + 1);
        }
    }
    return NULL;
}

struct Topic *get_topic_by_id(int t_id, struct Topic *root) {
    
    if(root == NULL)
        return NULL;
    
    int num = root->num_children;

    
    for(int i = 0; i < num; i++) {
        int num = root->children[i]->topic_id;

        if(num == t_id) {
            printf("MATCH: %s\n", root->children[i]->name);
            return root->children[i];
        }
        get_topic_by_id(t_id, root->children[i]);
    }
}


// recursivley print topics in a hierarchy
void printTopics(struct Topic *root, int depth) {
    
    for(int i = 0; i < depth; i++) {
        printf("  ");
    }
    printf("%s | %d\n", root->name, root->topic_id);
    
    for(int i = 0; i < root->num_children; i++) {
        printTopics(root->children[i], depth + 1);
    }
}


void printTopicsArray(struct Topic **t, int num) {
    
    for(int i = 0; i < num; i++) {
        printf("%s | %d\n", t[i]->name, t[i]->topic_id);
    }
}

// ********* PATH
