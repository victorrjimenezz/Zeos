
#ifndef __TASK_H__
#define __TASK_H__

#include <sched.h>
void task_switch(union task_union *new);
void ebp_switch(int *old_esp, int *new_esp);

#endif
