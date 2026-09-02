#include "nile/stdtypes.h"
#include "nile/kernel.h"
#include "nile/kernel/io_op_queue.h"

uint16_t io_char_dev_queue_level(nile_kernel_io_char_dev_op_queue *queue) {
	return queue->queue.slot_used_count;
}
uint16_t io_char_dev_queue_capacity(nile_kernel_io_char_dev_op_queue *queue) {
	return queue->queue.slot_capacity;
}
uint16_t io_char_dev_queue_push(nile_kernel_io_char_dev_op_queue *queue, nile_syscall_params_io_op *params, void *owner_tcb) {
	io_op_ringbuffer_queue *q = &queue->queue;

	if (q->slot_used_count >= q->slot_capacity) {
		return 0;
	}

	uint16_t index = q->queue_head;
	nile_kernel_io_char_dev_op *slot = &queue->slot[index];
	nile_kernel_io_dev_op *op = &slot->op;

	/* Fill operation descriptor */
	op->owner_tcb = owner_tcb;
	op->data_buffer = (uint8_t*) params->data_buffer;
	op->data_len = params->data_len;
	op->cursor = 0;
	op->io_hw_op_finished_code_ptr = &params->retval_io_hw_op_finished_code;

	/* Set flags */
	op->flags |= NILE_KERNEL_IO_DEV_OP_FLAG_PENDING;

	/* Advance ring buffer */
	q->queue_head = (index + 1) % q->slot_capacity;
	q->slot_used_count++;

	return op->data_len;
}

nile_kernel_io_char_dev_op* io_char_dev_queue_peek(nile_kernel_io_char_dev_op_queue *queue) {
	if (queue->queue.slot_used_count == 0)
		return NULL;

	return &queue->slot[queue->queue.queue_tail];
}

uint16_t io_char_dev_queue_pop(nile_kernel_io_char_dev_op_queue *queue) {
	io_op_ringbuffer_queue *q = &queue->queue;

	if (q->slot_used_count == 0)
		return 0;

	uint16_t index = q->queue_tail;
	nile_kernel_io_char_dev_op *slot = &queue->slot[index];
	nile_kernel_io_dev_op *op = &slot->op;

	uint16_t len = op->data_len;

	/* Clear descriptor */
//    op->owner_tcb     = NULL;
//    op->data_buffer   = NULL;
//    op->data_len      = 0;
//    op->cursor        = 0;
//    op->io_retval_ptr = NULL;
//    op->flags         = 0;
	/* Advance ring buffer */
	q->queue_tail = (index + 1) % q->slot_capacity;
	q->slot_used_count--;

	return len;
}

uint16_t io_block_dev_queue_level(nile_kernel_io_block_dev_op_queue *queue) {
    return queue->queue.slot_used_count;
}

uint16_t io_block_dev_queue_capacity(nile_kernel_io_block_dev_op_queue *queue) {
    return queue->queue.slot_capacity;
}

uint16_t io_block_dev_queue_push(
        nile_kernel_io_block_dev_op_queue *queue,
        nile_syscall_params_io_op *params,
        void *owner_tcb)
{
    io_op_ringbuffer_queue *q = &queue->queue;

    if (q->slot_used_count >= q->slot_capacity) {
        return 0;
    }

    uint16_t index = q->queue_head;
    nile_kernel_io_block_dev_op *slot = &queue->slot[index];
    nile_kernel_io_dev_op *op = &slot->op;

    /* Fill operation descriptor */
    op->owner_tcb     = owner_tcb;
    op->data_buffer   = (uint8_t*) params->data_buffer;
    op->data_len      = params->data_len;
    op->cursor        = 0;
    op->io_hw_op_finished_code_ptr = &params->retval_io_hw_op_finished_code;

    /* Block-device specific field */
    slot->block_dev_mem_off = params->block_dev_memaddr;

    /* Set flags */
    op->flags |= NILE_KERNEL_IO_DEV_OP_FLAG_PENDING;

    /* Advance ring buffer */
    q->queue_head = (index + 1) % q->slot_capacity;
    q->slot_used_count++;

    return op->data_len;
}

nile_kernel_io_block_dev_op* io_block_dev_queue_peek(
        nile_kernel_io_block_dev_op_queue *queue)
{
    if (queue->queue.slot_used_count == 0)
        return NULL;

    return &queue->slot[queue->queue.queue_tail];
}

uint16_t io_block_dev_queue_pop(nile_kernel_io_block_dev_op_queue *queue)
{
    io_op_ringbuffer_queue *q = &queue->queue;

    if (q->slot_used_count == 0)
        return 0;

    uint16_t index = q->queue_tail;
    nile_kernel_io_block_dev_op *slot = &queue->slot[index];
    nile_kernel_io_dev_op *op = &slot->op;

    uint16_t len = op->data_len;

    /* Advance ring buffer */
    q->queue_tail = (index + 1) % q->slot_capacity;
    q->slot_used_count--;

    return len;
}
