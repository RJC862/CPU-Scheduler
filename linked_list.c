#include <stdlib.h>
#include "linked_list.h"

// Creates and returns a new list
// If compare is NULL, list_insert just inserts at the head
list_t* list_create(compare_fn compare)
{
    /* IMPLEMENT THIS */
    return NULL;
}

// Destroys a list
void list_destroy(list_t* list)
{
    /* IMPLEMENT THIS */
}

// Returns head of the list
list_node_t* list_head(list_t* list)
{
    return list->head;
}

// Returns tail of the list
list_node_t* list_tail(list_t* list)
{
    return list->tail;
}

// Returns next element in the list
list_node_t* list_next(list_node_t* node)
{
    return node->next;
}

// Returns prev element in the list
list_node_t* list_prev(list_node_t* node)
{
    return node->prev;
}

// Returns end of the list marker
list_node_t* list_end(list_t* list)
{
    return NULL;
}

// Returns data in the given list node
void* list_data(list_node_t* node)
{
    return node->data;
}

// Returns the number of elements in the list
size_t list_count(list_t* list)
{
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
    list_node_t* new_node = (list_node_t*)malloc(sizeof(list_node_t));
    return NULL;

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
