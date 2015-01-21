#ifndef HEADER_LINKEDLIST_H
#define HEADER_LINKEDLIST_H

#include <unistd.h>

typedef struct linkedList {
    struct linkedList *next;
    void *data;
    size_t size;
} LinkedList;

LinkedList *linkedList_init();

ssize_t linkedList_push(LinkedList *head, void *data, size_t size);

ssize_t linkedList_pop(LinkedList *head, void *buf, size_t size);

void *linkedList_getIndexOf(LinkedList *head, int index);

void linkedList_removeIndexOf(LinkedList *head, int index);

void linkedList_remove(LinkedList *head, void *data, size_t size);

int linkedList_getLength(LinkedList *head);

void linkedList_free(LinkedList *head);

#endif