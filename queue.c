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
    struct list_head *pos = b->prev;
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

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

static struct list_head *merge(
    void *priv,
    bool (*cmp)(void *priv, struct list_head *, struct list_head *, bool),
    struct list_head *a,
    struct list_head *b,
    bool descend)
{
    struct list_head *head = NULL, **tail = &head;

    for (;;) {
        /* if equal, take 'a' -- important for sort stability */
        if (!cmp(priv, a, b, descend)) {
            *tail = a;
            tail = &a->next;
            a = a->next;
            if (!a) {
                *tail = b;
                break;
            }
        } else {
            *tail = b;
            tail = &b->next;
            b = b->next;
            if (!b) {
                *tail = a;
                break;
            }
        }
    }
    return head;
}

static void merge_final(
    void *priv,
    bool (*cmp)(void *priv, struct list_head *, struct list_head *, bool),
    struct list_head *head,
    struct list_head *a,
    struct list_head *b,
    bool descend)
{
    struct list_head *tail = head;
    size_t count = 0;

    for (;;) {
        /* if equal, take 'a' -- important for sort stability */
        if (!cmp(priv, a, b, descend)) {
            tail->next = a;
            a->prev = tail;
            tail = a;
            a = a->next;
            if (!a)
                break;
        } else {
            tail->next = b;
            b->prev = tail;
            tail = b;
            b = b->next;
            if (!b) {
                b = a;
                break;
            }
        }
    }

    /* Finish linking remainder of list b on to tail */
    tail->next = b;
    do {
        if (unlikely(!++count))
            cmp(priv, b, b, descend);
        b->prev = tail;
        tail = b;
        b = b->next;
    } while (b);

    /* And the final links to make a circular doubly-linked list */
    tail->next = head;
    head->prev = tail;
}

void list_sort(
    void *priv,
    struct list_head *head,
    bool (*cmp)(void *priv, struct list_head *, struct list_head *, bool),
    bool descend)
{
    struct list_head *list = head->next, *pending = NULL;
    size_t count = 0; /* Count of pending */

    if (list == head->prev) /* Zero or one elements */
        return;

    /* Convert to a null-terminated singly-linked list. */
    head->prev->next = NULL;

    do {
        size_t bits;
        struct list_head **tail = &pending;

        /* Find the least-significant clear bit in count */
        for (bits = count; bits & 1; bits >>= 1)
            tail = &(*tail)->prev;
        /* Do the indicated merge */
        if (likely(bits)) {
            struct list_head *a = *tail, *b = a->prev;

            a = merge(priv, cmp, b, a, descend);
            /* Install the merged result in place of the inputs */
            a->prev = b->prev;
            *tail = a;
        }

        /* Move one element from input list to pending */
        list->prev = pending;
        pending = list;
        list = list->next;
        pending->next = NULL;
        count++;
    } while (list);

    /* End of input; merge together all the pending lists. */
    list = pending;
    pending = pending->prev;
    for (;;) {
        struct list_head *next = pending->prev;

        if (!next)
            break;
        list = merge(priv, cmp, pending, list, descend);
        pending = next;
    }
    /* The final merge, rebuilding prev links */
    merge_final(priv, cmp, head, pending, list, descend);
}

/* Timsort */

typedef bool (*list_cmp_func_t)(void *,
                                struct list_head *,
                                struct list_head *,
                                bool);
static inline size_t run_size(struct list_head *head)
{
    if (!head)
        return 0;
    if (!head->next)
        return 1;
    return (size_t) (head->next->prev);
}

struct pair {
    struct list_head *head, *next;
};

static size_t stk_size;

static void build_prev_link(struct list_head *head,
                            struct list_head *tail,
                            struct list_head *list)
{
    tail->next = list;
    do {
        list->prev = tail;
        tail = list;
        list = list->next;
    } while (list);

    /* The final links to make a circular doubly-linked list */
    tail->next = head;
    head->prev = tail;
}

static struct pair find_run(void *priv,
                            struct list_head *list,
                            list_cmp_func_t cmp)
{
    size_t len = 1;
    struct list_head *next = list->next, *head = list;
    struct pair result;

    if (!next) {
        result.head = head, result.next = next;
        return result;
    }

    if (cmp(priv, list, next, 0) > 0) {
        /* decending run, also reverse the list */
        struct list_head *prev = NULL;
        do {
            len++;
            list->next = prev;
            prev = list;
            list = next;
            next = list->next;
            head = list;
        } while (next && cmp(priv, list, next, 0) > 0);
        list->next = prev;
    } else {
        do {
            len++;
            list = next;
            next = list->next;
        } while (next && cmp(priv, list, next, 0) == 0);
        list->next = NULL;
    }
    head->prev = NULL;
    head->next->prev = (struct list_head *) len;
    result.head = head, result.next = next;
    return result;
}

static struct list_head *merge_at(void *priv,
                                  list_cmp_func_t cmp,
                                  struct list_head *at)
{
    size_t len = run_size(at) + run_size(at->prev);
    struct list_head *prev = at->prev->prev;
    struct list_head *list = merge(priv, cmp, at->prev, at, 0);
    list->prev = prev;
    list->next->prev = (struct list_head *) len;
    --stk_size;
    return list;
}

static struct list_head *merge_force_collapse(void *priv,
                                              list_cmp_func_t cmp,
                                              struct list_head *tp)
{
    while (stk_size >= 3) {
        if (run_size(tp->prev->prev) < run_size(tp)) {
            tp->prev = merge_at(priv, cmp, tp->prev);
        } else {
            tp = merge_at(priv, cmp, tp);
        }
    }
    return tp;
}

static struct list_head *merge_collapse(void *priv,
                                        list_cmp_func_t cmp,
                                        struct list_head *tp)
{
    int n;
    while ((n = stk_size) >= 2) {
        if ((n >= 3 &&
             run_size(tp->prev->prev) <= run_size(tp->prev) + run_size(tp)) ||
            (n >= 4 && run_size(tp->prev->prev->prev) <=
                           run_size(tp->prev->prev) + run_size(tp->prev))) {
            if (run_size(tp->prev->prev) < run_size(tp)) {
                tp->prev = merge_at(priv, cmp, tp->prev);
            } else {
                tp = merge_at(priv, cmp, tp);
            }
        } else if (run_size(tp->prev) <= run_size(tp)) {
            tp = merge_at(priv, cmp, tp);
        } else {
            break;
        }
    }

    return tp;
}

void timsort(void *priv, struct list_head *head, list_cmp_func_t cmp)
{
    stk_size = 0;

    struct list_head *list = head->next, *tp = NULL;
    if (head == head->prev)
        return;

    /* Convert to a null-terminated singly-linked list. */
    head->prev->next = NULL;

    do {
        /* Find next run */
        struct pair result = find_run(priv, list, cmp);
        result.head->prev = tp;
        tp = result.head;
        list = result.next;
        stk_size++;
        tp = merge_collapse(priv, cmp, tp);
    } while (list);

    /* End of input; merge together all the runs. */
    tp = merge_force_collapse(priv, cmp, tp);

    /* The final merge; rebuild prev links */
    struct list_head *stk0 = tp, *stk1 = stk0->prev;
    while (stk1 && stk1->prev)
        stk0 = stk0->prev, stk1 = stk1->prev;
    if (stk_size <= 1) {
        build_prev_link(head, head, stk0);
        return;
    }
    merge_final(priv, cmp, head, stk1, stk0, 0);
}