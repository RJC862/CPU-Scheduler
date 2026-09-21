#include <stdlib.h>
#include "linked_list.h"

// Creates and returns a new list
// If compare is NULL, list_insert just inserts at the head
list_t* list_create(compare_fn compare)
{
    list_t* new_list = (list_t*)malloc(sizeof(list_t));
    if (new_list == NULL) {
        return NULL;
    }

    new_list->head = NULL;
    new_list->tail = NULL;
    new_list->count = 0;
    new_list->compare = compare;

    return new_list;
}

// Destroys a list
void list_destroy(list_t* list)
{
    if (list == NULL) {
        return;
    }

    list_node_t* curr = list->head;
    while (curr != NULL) {
        list_node_t* next = curr->next;
        free(curr);
        curr = next;
    }

    free(list);
}

// Returns head of the list
list_node_t* list_head(list_t* list)
{
    if (list == NULL) {
        return NULL;
    }
    return list->head;
}

// Returns tail of the list
list_node_t* list_tail(list_t* list)
{
    if (list == NULL) {
        return NULL;
    }
    return list->tail;
}

// Returns next element in the list
list_node_t* list_next(list_node_t* node)
{
    if (node == NULL) {
        return NULL;
    }
    return node->next;
}

// Returns prev element in the list
list_node_t* list_prev(list_node_t* node)
{
    if (node == NULL) {
        return NULL;
    }
    return node->prev;
}

// Returns end of the list marker
list_node_t* list_end(list_t* list)
{
    if (list == NULL || list->tail == NULL) {
        return NULL;
    }

    return list->tail->next;
}

// Returns data in the given list node
void* list_data(list_node_t* node)
{
    if (node == NULL) {
        return NULL;
    }
    return node->data;
}

// Returns the number of elements in the list
size_t list_count(list_t* list)
{
    if (list == NULL) {
        return 0;
    }
    return list->count;
}

// Finds the first node in the list with the given data
// Returns NULL if data could not be found
list_node_t* list_find(list_t* list, void* data)
{
    if (data == NULL || list == NULL) {
        return NULL;
    }

    list_node_t* curr = list->head;

    while (curr != NULL) {
        if (curr->data == data) {
            return curr;
        }

        curr = curr->next;
    }

    return NULL;
}

// Inserts a new node in the list with the given data
// Returns new node inserted
list_node_t* list_insert(list_t* list, void* data){
    if (list == NULL || data == NULL) {
        return NULL;
    }

    list_node_t* new_node = (list_node_t*)malloc(sizeof(list_node_t));
    if (new_node == NULL) {
        return NULL;
    }

    new_node->data = data;
    new_node->next = NULL;
    new_node->prev = NULL;

    if (list->head == NULL) {
        list->head = new_node;
        list->tail = new_node;
        list->count = 1;
    }

    // if the compare function is NULL, insert at the head
    if (list->compare == NULL){
        new_node->next = list->head;
        list->head->prev = new_node;
        list->head = new_node;
        list->count++;
        return new_node;
    }

    list_node_t* curr = list->head;
    
    // Inserts at the first position where the new node is less than or equal to the current node
    while (curr != NULL) {

        // compare the new node and the current node
        int comp_res = list->compare(new_node->data, curr->data);

        if (comp_res <= 0) { // insert before current node
            new_node->next = curr;
            
            if (curr->prev != NULL) {
                new_node->prev = curr->prev;
                curr->prev->next = new_node;
            }
            curr->prev = new_node;

            list->count++;
            return new_node;
        }

        curr = curr->next;
    }

    if (curr == NULL) { // insert at tail
        new_node->prev = list->tail;
        list->tail->next = new_node;
        list->tail = new_node;
        new_node->prev = list->tail;
        new_node->next = NULL;
        list->count++;
    }
    return new_node;
}

// Removes a node from the list and frees the node resources
void list_remove(list_t* list, list_node_t* node)
{
    if (list == NULL || node == NULL) {
        return;
    }

    if (node->prev != NULL ) {
        node->prev->next = node->next;
        node->prev = NULL;
    }

    if (node->next != NULL) {
        node->next->prev = node->prev;
        node->next = NULL;
    }

    free(node);
}
