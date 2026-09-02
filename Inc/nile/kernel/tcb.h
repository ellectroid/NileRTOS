#ifndef NILE_KERNEL_TCB_H_
#define NILE_KERNEL_TCB_H_

#include "nile/stdtypes.h"
#include "nile/compiler.h"
#include "nile/kernel_config.h"
#include "nile/cache.h"
#include "nile/external_cache.h"
#include "nile/memprot.h"

/* ----------------------------- */
/* Architecture-specific CPU ctx */
/* ----------------------------- */

#if NILE_CPU_ARCH_ARMV7M
#include "nile/arch/armv7m/cpu_ctx.h"
#elif NILE_CPU_ARCH_RISCV
#include "nile/arch/riscv/cpu_ctx.h"
#else
#error "No CPU context implementation for this architecture"
#endif

/* ----------------------------- */
/* Task control block flags      */
/* ----------------------------- */

/* Bit positions */
#define NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_ENABLED_POS          0U
#define NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_RUNNING_POS          1U
#define NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_READY_POS            2U
#define NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_POS          3U
#define NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_SUSPENDED_POS        4U
#define NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_TERMINATED_POS       5U
#define NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_ARRIVED_POS          6U
#define NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_USES_FPU_POS         8U
#define NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_FPU_LAZY_STACKING_POS 9U

#define NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_IO_POS       10U
#define NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_DELAY_POS    11U
#define NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_MUTEX_POS    12U

/* Individual flags */
#define NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_ENABLED        (1U << NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_ENABLED_POS)
#define NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_RUNNING        (1U << NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_RUNNING_POS)
#define NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_READY          (1U << NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_READY_POS)
#define NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED        (1U << NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_POS)
#define NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_SUSPENDED      (1U << NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_SUSPENDED_POS)
#define NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_TERMINATED     (1U << NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_TERMINATED_POS)
#define NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_ARRIVED        (1U << NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_ARRIVED_POS)

#define NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_USES_FPU       (1U << NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_USES_FPU_POS)
#define NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_FPU_LAZY_STACKING (1U << NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_FPU_LAZY_STACKING_POS)

#define NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_IO     (1U << NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_IO_POS)
#define NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_DELAY  (1U << NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_DELAY_POS)

/* Masks */
#define NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_ALL_MASK \
    ( NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED      \
    | NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_IO   \
    | NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_DELAY )
 //   | NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_MUTEX )

/* ----------------------------- */
/* Portable memprot region model */
/* ----------------------------- */

typedef struct nile_kernel_task_memprot_region {
	uintptr_t base;
	size_t size;
	uint32_t perms; /* NILE_MEMPROT_PERM_* */
	uint32_t type; /* NILE_MEMPROT_MEM_* */
	bool executable; /* true = executable, false = XN */
} nile_kernel_task_memprot_region;

/* ----------------------------- */
/* Task control block            */
/* ----------------------------- */

typedef struct nile_kernel_tcb {
	uint32_t task_id;
	uint32_t flags;

	void *tcb_prev;
	void *tcb_next;

	uint32_t scheduling_priority_base;
	uint32_t scheduling_priority_current;
	uint32_t scheduling_ready_queue_wait_time_accumulator;
	uint32_t scheduling_dynamic_priority_wait_time_counter;
	uint32_t scheduling_dynamic_priority_wait_time_prescaler;
	uint32_t scheduling_dynamic_priority_step;
	uint32_t scheduling_burst_time_base;
	uint32_t scheduling_burst_time_remaining;

	nile_kernel_task_memprot_region memprot_regions[NILE_KERNEL_TASK_MEMPROT_REGION_COUNT];


	volatile uint32_t *volatile scheduling_blocked_release_src;
	uint32_t scheduling_blocked_os_tick_release_timestamp;
	void* volatile scheduling_blocking_io_op;
	void* volatile scheduling_blocking_io_op_queue;
	void* volatile scheduling_blocking_io_op_dev;

	uint32_t stack_base_address;
	uint32_t stack_size;

	nile_cpu_gp_context saved_context_gp;
} nile_kernel_tcb;

typedef struct nile_kernel_tcb_fpu {
	nile_kernel_tcb tcb;
	nile_cpu_fpu_context saved_context_fpu;
} nile_kernel_tcb_fpu;

typedef struct nile_kernel_tcb_startup_ctx_gp{
		uint32_t startup_pc;
		uint32_t startup_sp;
		uint32_t special_reg[8];
		uint32_t startup_gp_reg[32];
}nile_kernel_tcb_startup_frame_gp;

typedef struct nile_kernel_tcb_startup_ctx_gp_fpu{
		uint32_t startup_pc;
		uint32_t startup_sp;
		uint32_t special_reg[8];
		uint32_t startup_gp_reg[32];
		union{
			uint32_t startup_fpu_reg[32];
			float startup_fpu_reg_f[32];
		};
}nile_kernel_tcb_startup_frame_gp_fpu;

static NILE_FORCEINLINE void tcb_set_fpu_config(nile_kernel_tcb *tcb, bool use_fpu, bool lazy_stacking){
	if (use_fpu) {
			tcb->flags |= NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_USES_FPU;

			if (lazy_stacking) {
				tcb->flags |= NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_FPU_LAZY_STACKING;
			} else {
				tcb->flags &= ~NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_FPU_LAZY_STACKING;
			}
		} else {
			tcb->flags &= ~NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_USES_FPU;
			tcb->flags &= ~NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_FPU_LAZY_STACKING;
		}
}

static NILE_FORCEINLINE void apply_memprot_settings(nile_kernel_tcb *tcb)
{
    nile_dsb();
    nile_cache_clean_invalidate_dcache_all();
    nile_dsb();
    nile_isb();

    nile_cache_invalidate_icache_all();

    nile_external_cache_reset();

    nile_memprot_disable();
    nile_dsb();
    nile_isb();

    /* Apply new region settings */
    for (uint32_t i = 0U; i < NILE_KERNEL_TASK_MEMPROT_REGION_COUNT; ++i) {
        const nile_kernel_task_memprot_region *r = &tcb->memprot_regions[i];

        if (r->size == 0U) {
            nile_memprot_region_disable(i);
            continue;
        }

        nile_memprot_region_configure(
            i,
            r->base,
            r->size,
            r->perms,
            r->type,
            r->executable
        );
        nile_memprot_region_enable(i);
    }

    nile_dsb();
    nile_isb();
    nile_memprot_enable_background();
    nile_memprot_enable();
    nile_dsb();
    nile_isb();

    nile_cache_clean_invalidate_dcache_all();
    nile_dsb();
    nile_isb();
    nile_cache_invalidate_icache_all();

    nile_external_cache_reset();
}

static NILE_FORCEINLINE uint32_t tcb_get_sp(nile_cpu_gp_context* ctx);


#endif /* NILE_KERNEL_TCB_H_ */
