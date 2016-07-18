#ifndef LINKEDLIST_H
#define LINKEDLIST_H
#include "error.h"
#include "pa.h"

typedef struct LinkedListNode{
 Pa *data;
 struct LinkedListNode *next;
} LinkedListNode;

typedef struct LinkedList{
 LinkedListNode *head;
 LinkedListNode *tail;
} LinkedList;

ERROR_CODE newLinkedList( LinkedList **list);
ERROR_CODE enqueue( LinkedList *list, Pa *data);
ERROR_CODE dequeue( LinkedList *list, Pa **data);
ERROR_CODE deleteLinkedList( LinkedList *list);
int isEmpty( LinkedList *list);

#endif