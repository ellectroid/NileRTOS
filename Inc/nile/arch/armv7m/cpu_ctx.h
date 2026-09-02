#ifndef NILE_ARCH_ARMV7M_CPU_CTX_H_
#define NILE_ARCH_ARMV7M_CPU_CTX_H_

#include "nile/stdtypes.h"

/* ----------------------------- */
/* GP register context           */
/* ----------------------------- */

typedef struct nile_cpu_gp_context {
    union {
        struct {
            uint32_t R4;
            uint32_t R5;
        };
        uint64_t R4_R5_aligned;
    };

    uint32_t R6;
    uint32_t R7;
    uint32_t R8;
    uint32_t R9;
    uint32_t R10;
    uint32_t R11;
    uint32_t SP;
} nile_cpu_gp_context;

/* ----------------------------- */
/* FPU register context          */
/* ----------------------------- */

typedef struct nile_cpu_fpu_context {
    union {
        struct {
            uint32_t S16;
            uint32_t S17;
        };
        uint64_t S16_S17_aligned;
    };

    uint32_t S18;
    uint32_t S19;
    uint32_t S20;
    uint32_t S21;
    uint32_t S22;
    uint32_t S23;
    uint32_t S24;
    uint32_t S25;
    uint32_t S26;
    uint32_t S27;
    uint32_t S28;
    uint32_t S29;
    uint32_t S30;
    uint32_t S31;
} nile_cpu_fpu_context;

/* ----------------------------- */
/* Stackframe GP context         */
/* ----------------------------- */

typedef struct nile_cpu_stackframe_gp {
    uint32_t R0;
    uint32_t R1;
    uint32_t R2;
    uint32_t R3;
    uint32_t R12;
    uint32_t LR;
    uint32_t PC;
    uint32_t xPSR;
} nile_cpu_stackframe_gp;

/* ----------------------------- */
/* Stackframe FPU context        */
/* ----------------------------- */

typedef struct nile_cpu_stackframe_fpu {
    uint32_t S0;
    uint32_t S1;
    uint32_t S2;
    uint32_t S3;
    uint32_t S4;
    uint32_t S5;
    uint32_t S6;
    uint32_t S7;
    uint32_t S8;
    uint32_t S9;
    uint32_t S10;
    uint32_t S11;
    uint32_t S12;
    uint32_t S13;
    uint32_t S14;
    uint32_t S15;
    uint32_t FPSCR;
} nile_cpu_stackframe_fpu;

static NILE_FORCEINLINE uint32_t tcb_get_sp(nile_cpu_gp_context* ctx){
	return ctx->SP;
}

#endif /* NILE_ARCH_ARMV7M_CPU_CTX_H_ */
