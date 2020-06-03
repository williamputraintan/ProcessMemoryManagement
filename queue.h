#include "constant.h"

typedef struct node_t {
    int value;
    struct node_t * next;
} *node;


node createNode(int value);
node push_node(node head, int value);
int pop(node * head);