#define _GNU_SOURCE
#include "linked_list.h"

pthread_rwlock_t head_lock;

/// @brief Initializes the list
/// @param head list head
void list_init(Node** head, size_t size) {
    mem_init(size);
    *head = NULL;
    int init_result = pthread_rwlock_init(&head_lock, NULL);
    if (init_result != 0) {
        perror("pthread_rwlock_init failed");
        exit(EXIT_FAILURE);
    }
}

/// @brief inserts last in linked list
/// @param head list head
/// @param data data for the new node
void list_insert(Node** head, uint16_t data) {
    Node* new_node = mem_alloc(sizeof(Node));
    pthread_mutex_init(&new_node->lock, NULL);
    if (!new_node) return;
    new_node->data = data;
    new_node->next = NULL;
    pthread_rwlock_wrlock(&head_lock);
    if (*head == NULL) {
        *head = new_node;
        pthread_rwlock_unlock(&head_lock);
        return;
    }
    Node* walker = *head;
    pthread_mutex_lock(&walker->lock);
    pthread_rwlock_unlock(&head_lock);

    while (walker->next) {
        pthread_mutex_unlock(&walker->lock);
        pthread_mutex_lock(&walker->next->lock);
        walker = walker->next;
    }
    walker->next = new_node;
    pthread_mutex_unlock(&walker->lock);
    return;
}

/// @brief Inserts a node after prev_node
/// @param prev_nodenode that will be before new node
/// @param data data for the new node
void list_insert_after(Node* prev_node, uint16_t data) {
    if (prev_node == NULL) return;
    Node* new_node = mem_alloc(sizeof(Node));
    pthread_mutex_init(&new_node->lock, NULL);

    if (!new_node) return;
    new_node->data = data;
    pthread_mutex_lock(&prev_node->lock);
    new_node->next = prev_node->next;
    prev_node->next = new_node;
    pthread_mutex_unlock(&prev_node->lock);
}

/// @brief inserts before a node
/// @param head list head
/// @param next_node node that will be after new node
/// @param data data for the new node
void list_insert_before(Node** head, Node* next_node, uint16_t data) {
    pthread_rwlock_wrlock(&head_lock);

    if (*head == NULL) {
        pthread_rwlock_unlock(&head_lock);
        return;  // ERROR
    }
    pthread_mutex_lock(&(*head)->lock);
    pthread_rwlock_unlock(&head_lock);

    Node* new_node = mem_alloc(sizeof(Node));
    if (!new_node) {
        pthread_mutex_unlock(&(*head)->lock);
        return;
    }
    pthread_mutex_init(&new_node->lock, NULL);
    pthread_mutex_lock(&new_node->lock);

    if (next_node == *head) {
        new_node->data = data;
        new_node->next = *head;
        *head = new_node;
        pthread_mutex_unlock(&new_node->lock);
        pthread_mutex_unlock(&new_node->next->lock);
        return;
    }

    Node* walker = *head;
    while (walker->next != next_node && walker->next != NULL) {
        pthread_mutex_lock(&walker->next->lock);
        pthread_mutex_t *old_lock = &walker->lock;
        walker = walker->next;
        pthread_mutex_unlock(old_lock);
    }
    if (walker->next == NULL) {
        pthread_mutex_unlock(&walker->lock);
        pthread_mutex_unlock(&new_node->lock);
        mem_free(new_node);
        return;  // ERRROR
    }
    walker->next = new_node;
    walker->next->next = next_node;
    walker->next->data = data;
    pthread_mutex_unlock(&new_node->lock);
    pthread_mutex_unlock(&walker->lock);
    return;
}

/// @brief deletes the Node with data
/// @param head list head
/// @param data
void list_delete(Node** head, uint16_t data) {
    pthread_rwlock_wrlock(&head_lock);
    if (*head == NULL){
        pthread_rwlock_unlock(&head_lock);
        return;
    }

    if ((*head)->data == data) {
        pthread_mutex_lock(&(*head)->lock);
        Node* temp = *head;
        *head = (*head)->next;
        pthread_mutex_unlock(&temp->lock);
        mem_free(temp);
        pthread_rwlock_unlock(&head_lock);
        return;
    }
    Node* walker = *head;
    pthread_mutex_lock(&walker->lock);
    pthread_rwlock_unlock(&head_lock);

    while (walker->next != NULL && walker->next->data != data) {
        pthread_mutex_lock(&walker->next->lock);
        pthread_mutex_t *old_lock = &walker->lock;
        walker = walker->next;
        pthread_mutex_unlock(old_lock);
    }
    if (walker->next == NULL) {
        pthread_mutex_unlock(&walker->lock);
        return;
    }

    Node* temp = walker->next;
    pthread_mutex_lock(&temp->lock);
    walker->next = temp->next;
    pthread_mutex_unlock(&temp->lock);
    pthread_mutex_unlock(&walker->lock);
    mem_free(temp);
    return;
}

/// @brief return the pointer to node with data or NULL if not found
/// @param head list head
/// @param data value to search for
/// @return Node* or NULL if node not found
Node* list_search(Node** head, uint16_t data) {
    pthread_rwlock_rdlock(&head_lock);
    Node* walker = *head;
    while (walker != NULL) {
        if (walker->data == data) {
            pthread_rwlock_unlock(&head_lock);
            return walker;
        }
        walker = walker->next;
    }
    pthread_rwlock_unlock(&head_lock);
    return NULL;
}

/// @brief displays all nodes
/// @param head list head
void list_display(Node** head) { list_display_range(head, NULL, NULL); }

/// @brief Displays the nodes in the range, including start and end
/// @param head list head
/// @param start_node first node to display
/// @param end_node last node to display
void list_display_range(Node** head, Node* start_node, Node* end_node) {
    pthread_rwlock_rdlock(&head_lock);
    if (end_node) end_node = end_node->next;
    if (!start_node) start_node = *head;
    printf("[");
    while (start_node != NULL && start_node != end_node) {
        printf("%d", start_node->data);
        start_node = start_node->next;
        if (start_node && start_node != end_node) printf(", ");
    }
    printf("]");
    pthread_rwlock_unlock(&head_lock);
}

/// @brief returns the number of nodes
/// @param head list head
/// @return int
int list_count_nodes(Node** head) {
    pthread_rwlock_rdlock(&head_lock);
    Node* walker = *head;
    int counter = 0;
    while (walker != NULL) {
        counter++;
        walker = walker->next;
    }
    pthread_rwlock_unlock(&head_lock);
    return counter;
}

/// @brief frees all used memory
/// @param head list head
void list_cleanup(Node** head) {
    *head = NULL;
    mem_deinit();
    pthread_rwlock_destroy(&head_lock);
}