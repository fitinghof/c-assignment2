#include "linked_list.h"

void list_init(Node** head){
    head = malloc(sizeof(Node*));
    *head = NULL;
}

void list_insert(Node** head, int data){
    Node* newNode = malloc(sizeof(Node));
    newNode->value = data;
    newNode->next = *head;
    *head = newNode;
}

void list_insert_after(Node* prev_node, int data){
    Node* newNode = malloc(sizeof(Node));
    newNode->next = prev_node->next;
    newNode->value = data;
    prev_node->next = newNode;
}

void list_insert_before(Node** head, Node* next_node, int data){
    Node* walker = *head;
    if(next_node == *head) return list_insert(head, data);

    while (walker->next != next_node && walker->next != NULL)
    {
        walker = walker->next;
    }
    if (walker->next == NULL) return -1; // ERRROR

    walker->next = malloc(sizeof(Node));
    walker->next->next = next_node;
    walker->next->value = data;
}

void list_delete(Node** head, int data);

Node* list_search(Node** head, int data);

void list_display(Node** head, Node* start_node, Node* end_node);

int list_count_nodes(Node** head);

void list_cleanup(Node** head);