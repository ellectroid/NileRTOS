#include "nile/stdtypes.h"
#include "nile/kernel.h"

void tcb_queue_tcb_remove(nile_kernel_tcb **queue_first_elem, nile_kernel_tcb **queue_last_elem, nile_kernel_tcb *elem_to_remove) {
	// Update the previous node's next pointer
	if (elem_to_remove->tcb_prev)
		((nile_kernel_tcb*) elem_to_remove->tcb_prev)->tcb_next = elem_to_remove->tcb_next;
	else
		*queue_first_elem = (nile_kernel_tcb*) elem_to_remove->tcb_next;

	// Update the next node's previous pointer
	if (elem_to_remove->tcb_next)
		((nile_kernel_tcb*) elem_to_remove->tcb_next)->tcb_prev = elem_to_remove->tcb_prev;
	else
		*queue_last_elem = (nile_kernel_tcb*) elem_to_remove->tcb_prev;

	// Clear the removed element's links
	elem_to_remove->tcb_prev = NULLPTR;
	elem_to_remove->tcb_next = NULLPTR;
}

void tcb_queue_tcb_add_before(nile_kernel_tcb **queue_first_elem, nile_kernel_tcb **queue_last_elem, nile_kernel_tcb *elem_to_insert,
		nile_kernel_tcb *elem_to_insert_before) {
	// Defensive clear in case elem_to_insert was linked elsewhere
	elem_to_insert->tcb_prev = NULLPTR;
	elem_to_insert->tcb_next = NULLPTR;

	if (elem_to_insert_before == NULLPTR) {
		// Prepend to front
		if (elem_to_insert != *queue_first_elem) {
			elem_to_insert->tcb_next = *queue_first_elem;

			if (*queue_first_elem)
				(*queue_first_elem)->tcb_prev = elem_to_insert;
			else
				*queue_last_elem = elem_to_insert;

			*queue_first_elem = elem_to_insert;

		}
	} else {
		elem_to_insert->tcb_prev = elem_to_insert_before->tcb_prev;
		elem_to_insert->tcb_next = elem_to_insert_before;

		if (elem_to_insert_before->tcb_prev)
			((nile_kernel_tcb*) elem_to_insert_before->tcb_prev)->tcb_next = elem_to_insert;
		else
			*queue_first_elem = elem_to_insert;

		elem_to_insert_before->tcb_prev = elem_to_insert;
	}
}

void tcb_queue_tcb_add_after(nile_kernel_tcb **queue_first_elem, nile_kernel_tcb **queue_last_elem, nile_kernel_tcb *elem_to_insert,
		nile_kernel_tcb *elem_to_insert_after) {
	if (elem_to_insert_after == NULLPTR) {
		// Append at the end
		if (elem_to_insert != *queue_last_elem) {
			elem_to_insert->tcb_prev = *queue_last_elem;
			elem_to_insert->tcb_next = NULLPTR;

			if (*queue_last_elem)
				(*queue_last_elem)->tcb_next = elem_to_insert;
			else
				*queue_first_elem = elem_to_insert; // was empty

			*queue_last_elem = elem_to_insert;

		}
	} else {
		// Insert after the given element
		elem_to_insert->tcb_prev = elem_to_insert_after;
		elem_to_insert->tcb_next = (nile_kernel_tcb*) elem_to_insert_after->tcb_next;

		elem_to_insert_after->tcb_next = elem_to_insert;

		if (elem_to_insert->tcb_next)
			((nile_kernel_tcb*) elem_to_insert->tcb_next)->tcb_prev = elem_to_insert;
		else
			*queue_last_elem = elem_to_insert;
	}
}

void tcb_queue_task_init_add(nile_kernel_tcb *new_task) {
	nile_kernel *kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;

	nile_kernel_tcb *ready_it = kk->scheduler.tcb_ready_queue_first;
	while (ready_it != NULLPTR) {
		if (ready_it->scheduling_priority_current < new_task->scheduling_priority_current) {
			break;
		}
		ready_it = (nile_kernel_tcb*) ready_it->tcb_next;
	}
	if (ready_it)
		tcb_queue_tcb_add_before(&kk->scheduler.tcb_ready_queue_first, &kk->scheduler.tcb_ready_queue_last, new_task, ready_it);
	else
		tcb_queue_tcb_add_after(&kk->scheduler.tcb_ready_queue_first, &kk->scheduler.tcb_ready_queue_last, new_task, ready_it);

	new_task->flags |= NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_ENABLED |
	NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_READY; //TCB is enabled
}
