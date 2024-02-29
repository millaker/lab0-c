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
    struct list_head *tmp = (struct list_head *) malloc(sizeof(*tmp));
    if (!tmp) {
        return NULL;
    }
    INIT_LIST_HEAD(tmp);
    return tmp;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;

    element_t *curr, *safe;
    list_for_each_entry_safe (curr, safe, head, list) {
        if (curr->value)
            free(curr->value);
        free(curr);
    }
    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *new_node = (element_t *) malloc(sizeof(*new_node));
    if (!new_node)
        return false;

    size_t len = strlen(s);
    new_node->value = (char *) malloc(sizeof(char) * (len + 1));
    if (!new_node->value) {
        free(new_node);
        return false;
    }

    strncpy(new_node->value, s, len + 1);

    list_add(&new_node->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *new_node = (element_t *) malloc(sizeof(*new_node));
    if (!new_node)
        return false;

    size_t len = strlen(s);
    new_node->value = (char *) malloc(sizeof(char) * (len + 1));
    if (!new_node->value) {
        free(new_node);
        return false;
    }

    strncpy(new_node->value, s, len + 1);

    list_add_tail(&new_node->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || head == head->next)
        return NULL;
    element_t *removed = list_entry(head->next, element_t, list);
    if (sp) {
        strncpy(sp, removed->value, bufsize);
        sp[bufsize - 1] = '\0';
    }

    list_del(&removed->list);
    return removed;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || head == head->next)
        return NULL;
    element_t *removed = list_entry(head->prev, element_t, list);
    if (sp) {
        strncpy(sp, removed->value, bufsize);
        sp[bufsize - 1] = '\0';
    }

    list_del(&removed->list);
    return removed;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;
    struct list_head *mid, *slow = head, *fast = head;
    while (fast->next != head && fast->next->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }
    mid = slow->next;
    element_t *entry = list_entry(mid, element_t, list);
    list_del(mid);
    if (entry->value)
        free(entry->value);
    free(entry);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    // Assume the list is sorted
    if (!head)
        return false;
    if (list_empty(head) || list_is_singular(head))
        return true;
    struct list_head *curr = head->next, *it = head->next->next;
    element_t *curr_entry = list_entry(curr, element_t, list);
    bool dup = false;
    while (curr != head && it != head) {
        element_t *it_entry = list_entry(it, element_t, list);
        if (!strcmp(it_entry->value, curr_entry->value)) {
            // Duplicate
            struct list_head *temp = it->next;
            list_del(it);
            free(it_entry->value);
            free(it_entry);
            it = temp;
            dup = true;
            continue;
        }
        if (dup) {
            list_del(curr);
            free(curr_entry->value);
            free(curr_entry);
        }
        curr = it;
        curr_entry = list_entry(curr, element_t, list);
        it = it->next;
        dup = false;
    }
    // Free curr if is dup
    if (dup) {
        list_del(curr);
        free(curr_entry->value);
        free(curr_entry);
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head))
        return;
    struct list_head *curr = head->next, *next = head->next->next;
    while (curr != head && next != head) {
        list_del(next);
        next->next = curr;
        next->prev = curr->prev;
        curr->prev->next = next;
        curr->prev = next;

        curr = curr->next;
        next = curr->next;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;
    struct list_head *curr = head;
    do {
        struct list_head *temp = curr->next;
        curr->next = curr->prev;
        curr->prev = temp;
        curr = temp;
    } while (curr != head);
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    LIST_HEAD(temp_head);
    struct list_head *reverse_first = head->next;
    struct list_head *curr = head->next;
    int i = k - 1;
    while (curr != head) {
        if (!i) {
            // Store prev and next
            struct list_head *prev = reverse_first->prev, *next = curr->next;
            // Attach to temp head
            temp_head.next = reverse_first;
            temp_head.prev = curr;
            reverse_first->prev = &temp_head;
            curr->next = &temp_head;
            q_reverse(&temp_head);
            // Rebuild link
            temp_head.next->prev = prev;
            temp_head.prev->next = next;
            prev->next = temp_head.next;
            next->prev = temp_head.prev;
            // Restore vars
            i = k - 1;
            reverse_first = next;
            curr = next;
            continue;
        }
        curr = curr->next;
        i--;
    }
}

/* Merge two sorted list into one, the two parameters point to the first
 * element
 */
struct list_head *merge_list(struct list_head *l1,
                             struct list_head *l2,
                             bool descend)
{
    struct list_head *head = NULL, **ptr = &head, **node;
    for (node = NULL; l1 && l2; *node = (*node)->next) {
        node = ((strcmp(list_entry(l1, element_t, list)->value,
                        list_entry(l2, element_t, list)->value) < 0) ^
                descend)
                   ? &l1
                   : &l2;
        *ptr = *node;
        ptr = &(*ptr)->next;
    }
    if (l1)
        *ptr = l1;
    else
        *ptr = l2;
    return head;
}

/* Mergesort function for q_sort, head points to the first element of the
 * to be sorted list
 */
struct list_head *sort_list(struct list_head *head, bool descend)
{
    if (!head || !head->next)
        return head;
    // Find mid node
    struct list_head *mid, *slow = head, *fast = head;
    while (fast->next && fast->next->next) {
        slow = slow->next;
        fast = fast->next->next;
    }
    mid = slow->next;
    slow->next = NULL;
    struct list_head *left = sort_list(head, descend),
                     *right = sort_list(mid, descend);
    return merge_list(left, right, descend);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head))
        return;
    // Convert list into singly linked list
    head->prev->next = NULL;
    head->next = sort_list(head->next, descend);
    // Rebuild prev links
    struct list_head *prev = NULL, *curr = head;
    while (curr) {
        curr->prev = prev;
        prev = curr;
        curr = curr->next;
    }
    head->prev = prev;
    prev->next = head;
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;
    struct list_head *curr = head->prev;
    char *curr_max = list_entry(curr, element_t, list)->value;
    int i = 0;
    while (curr != head) {
        element_t *temp = list_entry(curr, element_t, list);
        if (strcmp(temp->value, curr_max) <= 0) {
            curr_max = temp->value;
            curr = curr->prev;
            i++;
            continue;
        }
        // Remove current node
        struct list_head *next_node = curr->prev;
        list_del(curr);
        free(temp->value);
        free(temp);
        curr = next_node;
    }
    return i;
}

/* Remove every node which has a node with a strictly greater value anywhere
 * to the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;
    struct list_head *curr = head->prev;
    char *curr_max = list_entry(curr, element_t, list)->value;
    int i = 0;
    while (curr != head) {
        element_t *temp = list_entry(curr, element_t, list);
        if (strcmp(temp->value, curr_max) >= 0) {
            curr_max = temp->value;
            curr = curr->prev;
            i++;
            continue;
        }
        // Remove current node
        struct list_head *next_node = curr->prev;
        list_del(curr);
        free(temp->value);
        free(temp);
        curr = next_node;
    }
    return i;
}

/* Merge all the queues into one sorted queue, which is in
 * ascending/descending order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    if (!head || list_empty(head))
        return 0;
    int count = 0;
    struct list_head *first = head->next;
    struct list_head *curr = head->next->next;
    queue_contex_t *first_q = list_entry(first, queue_contex_t, chain);
    count += first_q->size;
    // Convert to singly linked list
    first_q->q->prev->next = NULL;
    while (curr != head) {
        queue_contex_t *temp_q = list_entry(curr, queue_contex_t, chain);
        // Convert to singly linked list
        temp_q->q->prev->next = NULL;
        count += temp_q->size;
        first_q->q->next =
            merge_list(first_q->q->next, temp_q->q->next, descend);
        temp_q->q->next = temp_q->q;
        temp_q->q->prev = temp_q->q;
        curr = curr->next;
    }
    // Rebuild prev links
    struct list_head *prev = NULL;
    curr = first_q->q;
    while (curr) {
        curr->prev = prev;
        prev = curr;
        curr = curr->next;
    }
    first_q->q->prev = prev;
    prev->next = first_q->q;

    return count;
}

static inline void list_swap(struct list_head *a, struct list_head *b)
{
    struct list_head *pos = b;
    list_del(b);
    // Replace a with b
    b->next = a->next;
    b->next->prev = b;
    b->prev = a->prev;
    b->prev->next = b;
    if (pos == a)
        pos = b;
    list_add(a, pos);
}

void q_shuffle(struct list_head *head)
{
    if (!head || list_empty(head))
        return;
    struct list_head *curr = head->next;
    int size = q_size(head);
    while (size > 1) {
        int num = rand() % size--;
        struct list_head *target, *next_temp = curr->next;
        for (target = curr; num > 0; target = target->next, num--)
            ;
        if (curr != target)
            list_swap(curr, target);
        if (next_temp != target)
            curr = next_temp;
    }
}