#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (head != NULL) {
        INIT_LIST_HEAD(head);
    }
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;
    element_t *cur, *next;
    list_for_each_entry_safe (cur, next, head, list) {
        q_release_element(cur);
    }
    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *new_element = malloc(sizeof(element_t));
    if (!new_element)
        return false;

    new_element->value = strdup(s);
    if (!new_element->value) {
        free(new_element);
        return false;
    }

    list_add(&new_element->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *new_element = malloc(sizeof(element_t));
    if (!new_element)
        return false;

    new_element->value = strdup(s);
    if (!new_element->value) {
        free(new_element);
        return false;
    }

    list_add_tail(&new_element->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (head && !list_empty(head)) {
        element_t *to_remove = list_first_entry(head, element_t, list);
        list_del(&to_remove->list);

        if (sp) {
            strncpy(sp, to_remove->value, bufsize - 1);
            sp[bufsize - 1] = '\0';
        }
        return to_remove;
    }

    return NULL;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (head && !list_empty(head)) {
        element_t *to_remove = list_last_entry(head, element_t, list);
        list_del(&to_remove->list);

        if (sp) {
            strncpy(sp, to_remove->value, bufsize - 1);
            sp[bufsize - 1] = '\0';
        }
        return to_remove;
    }

    return NULL;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;

    int count = 0;
    struct list_head *temp;
    list_for_each (temp, head)
        count++;

    return count;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    if (!head || list_empty(head)) {
        return false;
    }

    struct list_head *slow = head->next;
    struct list_head *fast = head->next;

    while (fast != head && fast->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }

    if (fast == head) {
        slow = slow->prev;
    }

    element_t *middle_element = list_entry(slow, element_t, list);

    list_del(slow);

    free(middle_element->value);
    free(middle_element);

    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    if (!head || list_empty(head)) {
        return false;
    }

    struct list_head *current, *safe;
    element_t *entry, *next_entry;
    bool mark_del = false;

    list_for_each_safe (current, safe, head) {
        if (current->next == head) {
            if (mark_del) {
                entry = list_entry(current, element_t, list);
                list_del(current);
                q_release_element(entry);
            }
            break;
        }

        entry = list_entry(current, element_t, list);
        next_entry = list_entry(current->next, element_t, list);

        if (!strcmp(entry->value, next_entry->value)) {
            list_del(current);
            q_release_element(entry);
            mark_del = true;
        } else if (mark_del) {
            list_del(current);
            q_release_element(entry);
            mark_del = false;
        }
    }

    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head *node = head->next;
    while (node != head && node->next != head) {
        struct list_head *nextNode = node->next;

        list_del(node);
        list_del(nextNode);

        list_add(nextNode, node->prev);
        list_add(node, nextNode);

        node = node->next;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head)) {
        return;
    }

    struct list_head *current = head;
    struct list_head *temp = NULL;

    do {
        temp = current->next;
        current->next = current->prev;
        current->prev = temp;

        current = current->prev;
    } while (current != head);
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    if (!head || list_empty(head) || k <= 1)
        return;

    int len = 0;
    struct list_head *current = head->next;
    len = q_size(head);

    struct list_head partition, *temp_head = head, *tail;
    INIT_LIST_HEAD(&partition);

    for (int i = 0; i < (len / k); i++) {
        current = temp_head->next;
        for (int j = 0; j < k; j++) {
            tail = current;
            current = current->next;
        }
        list_cut_position(&partition, temp_head, tail);
        q_reverse(&partition);
        list_splice_init(&partition, temp_head);
        temp_head = current->prev;
    }
    return;
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
}

void merge(struct list_head *l_head,
           struct list_head *r_head,
           struct list_head *head)
{
    struct list_head temp_list;
    INIT_LIST_HEAD(&temp_list);

    while (!list_empty(l_head) || !list_empty(r_head)) {
        struct list_head *chosen;

        if (list_empty(l_head)) {
            chosen = r_head;
        } else if (list_empty(r_head)) {
            chosen = l_head;
        } else {
            element_t *l_entry = list_entry(l_head->next, element_t, list);
            element_t *r_entry = list_entry(r_head->next, element_t, list);
            chosen =
                (strcmp(l_entry->value, r_entry->value) <= 0) ? l_head : r_head;
        }

        list_move_tail(chosen->next, &temp_list);
    }

    list_splice_tail_init(&temp_list, head);
}
void merge_sort_recursive(struct list_head *head, int length)
{
    if (length <= 1)
        return;

    int mid = length / 2;
    struct list_head left, right;
    INIT_LIST_HEAD(&left);
    INIT_LIST_HEAD(&right);

    struct list_head *current = head->next;
    for (int i = 0; i < mid; i++) {
        struct list_head *next = current->next;
        list_move_tail(current, &left);
        current = next;
    }
    list_splice_tail_init(head, &right);

    merge_sort_recursive(&left, mid);
    merge_sort_recursive(&right, length - mid);

    INIT_LIST_HEAD(head);
    merge(&left, &right, head);
}

void merge_sort(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    int length = 0;
    struct list_head *pos;
    list_for_each (pos, head) {
        length++;
    }

    merge_sort_recursive(head, length);
}

void q_sort(struct list_head *head, bool descend)
{
    merge_sort(head);
    if (descend) {
        q_reverse(head);
    }
}


/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    if (!head || list_empty(head)) {
        return 0;
    }

    struct list_head *current, *tmp, *next;

    list_for_each_safe (current, tmp, head) {
        char *current_value = list_entry(current, element_t, list)->value;
        bool has_smaller_right = false;

        for (next = current->next; next != head; next = next->next) {
            char *next_value = list_entry(next, element_t, list)->value;
            if (strcmp(current_value, next_value) > 0) {
                has_smaller_right = true;
                break;
            }
        }

        if (has_smaller_right) {
            list_del(current);
            q_release_element(list_entry(current, element_t, list));
        }
    }

    return q_size(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    if (!head || list_empty(head)) {
        return 0;
    }

    struct list_head *current, *tmp, *next;

    list_for_each_safe (current, tmp, head) {
        char *current_value = list_entry(current, element_t, list)->value;
        bool has_greater_right = false;

        for (next = current->next; next != head; next = next->next) {
            char *next_value = list_entry(next, element_t, list)->value;
            if (strcmp(current_value, next_value) < 0) {
                has_greater_right = true;
                break;
            }
        }

        if (has_greater_right) {
            list_del(current);
            element_t *to_remove_element = list_entry(current, element_t, list);
            q_release_element(to_remove_element);
        }
    }

    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in
 * ascending/descending order */
int q_merge(struct list_head *head, bool descend)
{
    if (!head || list_empty(head)) {
        return 0;
    }

    queue_contex_t *base_queue = list_first_entry(head, queue_contex_t, chain);
    if (list_is_singular(head)) {
        return base_queue->size;
    }

    queue_contex_t *queue_to_merge;
    struct list_head *current, *next;

    list_for_each_safe (current, next, head) {
        if (current == &base_queue->chain) {
            continue;
        }
        queue_to_merge = list_entry(current, queue_contex_t, chain);
        list_splice_tail_init(queue_to_merge->q, base_queue->q);
        base_queue->size += queue_to_merge->size;
    }

    q_sort(base_queue->q, descend);
    return base_queue->size;
}

