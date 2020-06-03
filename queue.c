
#include "queue.h"

/**
* This function will create node and is taken and modified from 
* https://www.hackerearth.com/practice/data-structures/linked-list/singly-linked-list/tutorial/
*/
node createNode(int data){
    node temp;
    temp = (node)malloc(sizeof(struct node_t));
    assert(temp);
    temp->value = data;
    temp->next = NULL;
    return temp;
}

/**
* This function will add a new node in the lis and is taken and modified from 
* https://www.hackerearth.com/practice/data-structures/linked-list/singly-linked-list/tutorial/
*/
node push_node(node head, int value) {
    node new_node, temp ;
    new_node = createNode(value);
     
    if(head == NULL){
        head = new_node;
    }
    else{
        temp = head;
        while(temp->next != NULL){
            temp = temp->next;
        }
        temp->next = new_node;
    }
    return head;
}

/**
* This function will remove the next queue from the list and is taken from 
* https://www.learn-c.org/en/Linked_lists
*/
int pop(node * head) {
    int retval = -1;
    node next_node = NULL;

    if (*head == NULL) {
        return -1;
    }

    next_node = (*head)->next;
    retval = (*head)->value;
    free(*head);
    *head = next_node;

    return retval;
}