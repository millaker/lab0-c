#pragma once

typedef void(task_callback_t)(void *arg);

/* Add a task to scheduling list */
void task_add(task_callback_t *func, void *param);

/* Start scheduling the tasks added*/
int task_start();

#define task_printf(...)     \
    ({                       \
        preempt_disable();   \
        printf(__VA_ARGS__); \
        preempt_enable();    \
    })
