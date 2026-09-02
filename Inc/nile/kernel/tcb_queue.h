#ifndef NILE_KERNEL_TCB_QUEUE_H_
#define NILE_KERNEL_TCB_QUEUE_H_

#include "nile/stdtypes.h"
#include "nile/kernel/tcb.h"

void tcb_queue_tcb_remove(
    nile_kernel_tcb **queue_first_elem,
    nile_kernel_tcb **queue_last_elem,
    nile_kernel_tcb *elem_to_remove);
void tcb_queue_tcb_add_before(
    nile_kernel_tcb **queue_first_elem,
    nile_kernel_tcb **queue_last_elem,
    nile_kernel_tcb *elem_to_insert,
    nile_kernel_tcb *elem_to_insert_before);
void tcb_queue_tcb_add_after(
    nile_kernel_tcb **queue_first_elem,
    nile_kernel_tcb **queue_last_elem,
    nile_kernel_tcb *elem_to_insert,
    nile_kernel_tcb *elem_to_insert_after);

void tcb_queue_task_init_add(nile_kernel_tcb*);


#endif /* NILE_KERNEL_TCB_QUEUE_H_ */
