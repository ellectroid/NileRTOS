#ifndef NILE_ARCH_ARMV7M_CTX_INIT_H_
#define NILE_ARCH_ARMV7M_CTX_INIT_H_

#include "nile/stdtypes.h"
#include "nile/compiler.h"
#include "nile/kernel/tcb.h"


/* GP-only startup context (no FPU frame) */
static NILE_FORCEINLINE void ctx_init_gp(nile_kernel_tcb *tcb,
                                         nile_kernel_tcb_startup_frame_gp *ctx_val)
{
    uint32_t *stack_top = (uint32_t*) tcb->stack_base_address;

    /* Exception frame: 8 words */
    const uint32_t frame_words = 8U;
    uint32_t *frame = stack_top - frame_words;

    /* Correct ARMv7-M exception frame layout (descending stack) */
    frame[0] = ctx_val->startup_gp_reg[0];      /* R0  */
    frame[1] = ctx_val->startup_gp_reg[1];      /* R1  */
    frame[2] = ctx_val->startup_gp_reg[2];      /* R2  */
    frame[3] = ctx_val->startup_gp_reg[3];      /* R3  */
    frame[4] = ctx_val->startup_gp_reg[12];     /* R12 */
    frame[5] = ctx_val->startup_gp_reg[14];     /* LR  */
    frame[6] = ctx_val->startup_pc | 1U;        /* PC  */
    frame[7] = 0x01000000U;                     /* xPSR */

    /* Callee saved registers R4 to R11 */
    for (uint32_t r = 4; r <= 11; ++r) {
        ((uint32_t*)&tcb->saved_context_gp)[r - 4] = ctx_val->startup_gp_reg[r];
    }

    /* Set SP to the start of the exception frame */
    tcb->saved_context_gp.SP = (uint32_t) frame;
}

static NILE_FORCEINLINE void ctx_init_gp_fpu(
    nile_kernel_tcb_fpu *tcb,
    nile_kernel_tcb_startup_frame_gp_fpu *ctx_val)
{
    uint32_t *stack_top = (uint32_t*) tcb->tcb.stack_base_address;

    /* GP frame: 8 words
       FP frame: 18 words (S0..S15 + FPSCR + align)
       Total: 26 words */
    const uint32_t gp_words = 8U;
    const uint32_t fp_words = 18U;
    const uint32_t frame_words = gp_words + fp_words;

    uint32_t *frame = stack_top - frame_words;

    /* GP frame first (lowest addresses) */
    frame[0] = ctx_val->startup_gp_reg[0];      /* R0  */
    frame[1] = ctx_val->startup_gp_reg[1];      /* R1  */
    frame[2] = ctx_val->startup_gp_reg[2];      /* R2  */
    frame[3] = ctx_val->startup_gp_reg[3];      /* R3  */
    frame[4] = ctx_val->startup_gp_reg[12];     /* R12 */
    frame[5] = ctx_val->startup_gp_reg[14];     /* LR  */
    frame[6] = ctx_val->startup_pc | 1U;        /* PC  */
    frame[7] = 0x01000000U;                     /* xPSR */

    /* FP frame next (higher addresses) */
    for (uint32_t i = 0; i < 16U; ++i) {
        frame[8 + i] = ctx_val->startup_fpu_reg[i]; /* S0..S15 */
    }

    frame[24] = 0U; /* FPSCR */
    frame[25] = 0U; /* alignment word */

    /* Callee-saved GP registers R4..R11 */
    for (uint32_t r = 4; r <= 11; ++r) {
        ((uint32_t*)&tcb->tcb.saved_context_gp)[r - 4] =
            ctx_val->startup_gp_reg[r];
    }

    /* Callee-saved FPU registers S16..S31 */
    for (uint32_t s = 16; s <= 31; ++s) {
        ((uint32_t*)&tcb->saved_context_fpu)[s - 16] =
            ctx_val->startup_fpu_reg[s];
    }

    /* SP must point to R0 (lowest address) */
    tcb->tcb.saved_context_gp.SP = (uint32_t) frame;
}

#endif /* NILE_ARCH_ARMV7M_CTX_INIT_H_ */
