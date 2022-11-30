#include <stdlib.h>
#include <string.h>
#include <stdio.h>



#include "path.h"


// create an easy to navigate path structure from a user-inputted path.
struct Path *newPath(char *path) {
    
    if(path == NULL) return NULL;

    int path_len = strlen(path); // get the total length of the path string
    if(path_len <= 0) { return NULL; }
    
    struct Path *p; // reference to new path being made
    p = malloc(sizeof(struct Path)); // malloc the new path

    int total_depth = 0; // keep track of the depth of the path. news/hello/ is a depth of 1

    // count the amount of '/'s that i have???
    for(int i = 0; i < path_len; i++) {
        if((path[i] == '/' && (i + 1) < path_len) || ((i + 1 == path_len) && path[i] != '/')) {
            // malloc this many char pointers
            total_depth++;
        }
    }
    
    
    if(total_depth < 0) {
        return NULL;
    }
    
    p->elements = malloc(total_depth * sizeof(char*)); // initialize the elements
    p->depth = total_depth; // assign the depth
    // loop through the amount of times
    for(int i = 0; i < total_depth; i++) {

        path_len = strlen(path); // recalculated the string
        
        for(int j = 0; j < path_len; j++) { // loop through path
            //printf("segfault: %d | %c\n", j, path[j]);
            if(path[j] == '/') {
                
                if(j == (path_len -1))
                    break;
                
                p->elements[i] = (char*)malloc(sizeof(char*));
               // printf("char: %c | %d\n", path[j], j);
                char *c;
                c = malloc(sizeof(char) * j); // allocate the exact memory needed to store the directory
                strncpy(c, path, j); //
                //printf("path element: %s\n", c);
                p->elements[i] = c; // set depth to point at new path element
                // this makes sure that j is not the last char, then does good things
                if(j != (path_len - 1)) {
                    memmove(path, path + j + 1, path_len); // this line will remove the first arg from 'path'. add 1 to get rid of '/'
                }
                break;
            }
            else if (j == (path_len - 1)) {
                // this means it is the last one OR depth is 0
                if(i == 0) {
                    //printf("depth 1\n");
                    
                }
                p->elements[i] = (char*)malloc(sizeof(char*));
                //printf("segfault: %d\n", j);
                char *c;
                c = malloc(sizeof(char) * j); // allocate the exact memory needed to store the directory
                
                // fix null character discrepencies between file read and input
                if(path[j] != '\n') {
                    j++;
                }
                 

                strncpy(c, path, j); //
                //printf("path element else: %s\n", c);
                //printf("lastchar: %c\n", path[j - 1]);
                p->elements[i] = c; // set depth to point at new new
                //if(i != 0)
                break;
            }
        }
    }
    return p;
}

void deletePath(struct Path *p) {
    
    for(int i = 0; i < p->depth; i++) {
        if(p != NULL)
            free(p->elements[i]);
    }
    free(p);
}
