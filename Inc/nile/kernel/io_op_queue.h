#ifndef NILE_KERNEL_IO_OP_QUEUE_H_
#define NILE_KERNEL_IO_OP_QUEUE_H_

#include "nile/stdtypes.h"
#include "nile/kernel_config.h"
#include "nile/syscall_api.h"

#define NILE_KERNEL_IO_DEV_OP_FLAG_OPERATION_POS   0U
#define NILE_KERNEL_IO_DEV_OP_FLAG_OPERATION_MASK  (0x03U << NILE_KERNEL_IO_DEV_OP_FLAG_OPERATION_POS)

#define NILE_KERNEL_IO_DEV_OP_FLAG_READ            (0U << NILE_KERNEL_IO_DEV_OP_FLAG_OPERATION_POS)
#define NILE_KERNEL_IO_DEV_OP_FLAG_WRITE           (1U << NILE_KERNEL_IO_DEV_OP_FLAG_OPERATION_POS)
#define NILE_KERNEL_IO_DEV_OP_FLAG_ERASE           (2U << NILE_KERNEL_IO_DEV_OP_FLAG_OPERATION_POS)

#define NILE_KERNEL_IO_DEV_OP_FLAG_ABORTED         (1U << 29)
#define NILE_KERNEL_IO_DEV_OP_FLAG_PENDING         (1U << 30)
#define NILE_KERNEL_IO_DEV_OP_FLAG_ACTIVE          (1U << 31)

#define NILE_KERNEL_IO_DEV_OP_FLAG_SYSTEM_MASK     \
    (NILE_KERNEL_IO_DEV_OP_FLAG_OPERATION_MASK |   \
     NILE_KERNEL_IO_DEV_OP_FLAG_ABORTED        |   \
     NILE_KERNEL_IO_DEV_OP_FLAG_PENDING        |   \
     NILE_KERNEL_IO_DEV_OP_FLAG_ACTIVE)

typedef struct {
	uint32_t flags;
	void *owner_tcb;
	uint8_t *data_buffer;
	uint32_t data_len;
	uint32_t cursor;
	volatile uint32_t *io_hw_op_finished_code_ptr; //inside nile_syscall_params_io_op
} nile_kernel_io_dev_op;

typedef struct {
	nile_kernel_io_dev_op op;
} nile_kernel_io_char_dev_op;

typedef struct {
	nile_kernel_io_dev_op op;
	uint32_t block_dev_mem_off;
} nile_kernel_io_block_dev_op;

#define IO_OP_RING_BUFFER_FLAGS_IS_BLOCK_DEV_POS  0
#define IO_OP_RING_BUFFER_FLAGS_IS_BLOCK_DEV_MASK (1 << IO_OP_RING_BUFFER_FLAGS_IS_BLOCK_DEV_POS)
#define IO_OP_RING_BUFFER_FLAGS_IS_BLOCK_DEV      (1 << IO_OP_RING_BUFFER_FLAGS_IS_BLOCK_DEV_POS)
#define IO_OP_RING_BUFFER_FLAGS_IS_CHAR_DEV_POS   1
#define IO_OP_RING_BUFFER_FLAGS_IS_CHAR_DEV_MASK  (1 << IO_OP_RING_BUFFER_FLAGS_IS_CHAR_DEV_POS)
#define IO_OP_RING_BUFFER_FLAGS_IS_CHAR_DEV       (1 << IO_OP_RING_BUFFER_FLAGS_IS_CHAR_DEV_POS)

#define IO_OP_RING_BUFFER_FLAGS_IS_TX_POS         2
#define IO_OP_RING_BUFFER_FLAGS_IS_TX_MASK        (1 << IO_OP_RING_BUFFER_FLAGS_IS_TX_POS)
#define IO_OP_RING_BUFFER_FLAGS_IS_TX             (1 << IO_OP_RING_BUFFER_FLAGS_IS_TX_POS)

#define IO_OP_RING_BUFFER_FLAGS_IS_RX_POS         3
#define IO_OP_RING_BUFFER_FLAGS_IS_RX_MASK        (1 << IO_OP_RING_BUFFER_FLAGS_IS_RX_POS)
#define IO_OP_RING_BUFFER_FLAGS_IS_RX             (1 << IO_OP_RING_BUFFER_FLAGS_IS_RX_POS)

#define IO_OP_RING_BUFFER_FLAGS_IS_EX_POS         4
#define IO_OP_RING_BUFFER_FLAGS_IS_EX_MASK        (1 << IO_OP_RING_BUFFER_FLAGS_IS_EX_POS)
#define IO_OP_RING_BUFFER_FLAGS_IS_EX             (1 << IO_OP_RING_BUFFER_FLAGS_IS_EX_POS)

typedef struct {
	uint32_t flags;
	uint16_t queue_head; //first empty slot
	uint16_t queue_tail; //first full slot
	uint16_t slot_capacity;
	uint16_t slot_used_count;
} io_op_ringbuffer_queue;

typedef struct nile_kernel_io_char_dev_op_queue {
	io_op_ringbuffer_queue queue;
	nile_kernel_io_char_dev_op slot[NILE_KERNEL_IO_CHAR_DEV_OP_QUEUE_CAPACITY];
} nile_kernel_io_char_dev_op_queue;

typedef struct nile_kernel_io_block_dev_op_queue {
	io_op_ringbuffer_queue queue;
	nile_kernel_io_block_dev_op slot[NILE_KERNEL_IO_BLOCK_DEV_OP_QUEUE_CAPACITY];
} nile_kernel_io_block_dev_op_queue;

/* -------------------------------
 CHAR DEVICE OPERATION QUEUE API
 ------------------------------- */

uint16_t io_char_dev_queue_level(nile_kernel_io_char_dev_op_queue *queue);
uint16_t io_char_dev_queue_capacity(nile_kernel_io_char_dev_op_queue *queue);
uint16_t io_char_dev_queue_push(nile_kernel_io_char_dev_op_queue *queue, nile_syscall_params_io_op *params, void *owner_tcb);
nile_kernel_io_char_dev_op* io_char_dev_queue_peek(nile_kernel_io_char_dev_op_queue *queue);
uint16_t io_char_dev_queue_pop(nile_kernel_io_char_dev_op_queue *queue);

/* -------------------------------
 BLOCK DEVICE OPERATION QUEUE API
 ------------------------------- */

uint16_t io_block_dev_queue_level(nile_kernel_io_block_dev_op_queue *queue);
uint16_t io_block_dev_queue_capacity(nile_kernel_io_block_dev_op_queue *queue);
uint16_t io_block_dev_queue_push(nile_kernel_io_block_dev_op_queue *queue, nile_syscall_params_io_op *params, void *owner_tcb);
nile_kernel_io_block_dev_op* io_block_dev_queue_peek(nile_kernel_io_block_dev_op_queue *queue);
uint16_t io_block_dev_queue_pop(nile_kernel_io_block_dev_op_queue *queue);

#endif /* NILE_KERNEL_IO_OP_QUEUE_H_ */
