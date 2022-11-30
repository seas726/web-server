
#include "util.h"

#ifndef PATH
#define PATH

struct Path {
    char **elements;
    int depth;
};

//void parse_path_rec(char *path, struct Path **new_path, int depth, int len);
struct Path *newPath(char *path);
void deletePath(struct Path *p);

#endif
