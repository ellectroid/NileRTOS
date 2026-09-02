#include "nile/stdtypes.h"
#include "nile/kernel.h"
#include "nile/syscall_api.h"

uint32_t syscall_handler_dispatch(uint32_t system_call_id, uint32_t* arg_list, uint32_t arg_cnt){
	nile_kernel *kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
	uint32_t retval = NILE_SYSCALL_RETVAL_ERR_INVALID_HANDLER;
	if(system_call_id < kk->syscall_vtable.vector_count){
		if(kk->syscall_vtable.vector[system_call_id]){
			retval = kk->syscall_vtable.vector[system_call_id](arg_list, arg_cnt);
		}
	}
	return retval;
}

