#ifndef NILE_SYSCALL_KERNEL_HANDLERS_H_
#define NILE_SYSCALL_KERNEL_HANDLERS_H_

#include "nile/stdtypes.h"

/* NONE */
uint32_t system_call_none(uint32_t* arg_list, uint32_t arg_cnt);

/* Kernel Info / Debug */
uint32_t system_call_kernel_info_version(uint32_t* arg_list, uint32_t arg_cnt);
uint32_t system_call_kernel_info_timing(uint32_t* arg_list, uint32_t arg_cnt);
uint32_t system_call_kernel_log(uint32_t* arg_list, uint32_t arg_cnt);
uint32_t system_call_kernel_reserved0(uint32_t* arg_list, uint32_t arg_cnt);
uint32_t system_call_kernel_reserved1(uint32_t* arg_list, uint32_t arg_cnt);
uint32_t system_call_kernel_reserved2(uint32_t* arg_list, uint32_t arg_cnt);
uint32_t system_call_kernel_reserved3(uint32_t* arg_list, uint32_t arg_cnt);

/* Task Management */
uint32_t system_call_task_yield(uint32_t* arg_list, uint32_t arg_cnt);
uint32_t system_call_task_block(uint32_t* arg_list, uint32_t arg_cnt);
uint32_t system_call_task_reserved0(uint32_t* arg_list, uint32_t arg_cnt);
uint32_t system_call_task_reserved1(uint32_t* arg_list, uint32_t arg_cnt);
uint32_t system_call_task_reserved2(uint32_t* arg_list, uint32_t arg_cnt);
uint32_t system_call_task_reserved3(uint32_t* arg_list, uint32_t arg_cnt);
uint32_t system_call_task_reserved4(uint32_t* arg_list, uint32_t arg_cnt);
uint32_t system_call_task_reserved5(uint32_t* arg_list, uint32_t arg_cnt);

/* IO */
uint32_t system_call_io_char_dev_ioctl(uint32_t *arg_list, uint32_t arg_cnt);
uint32_t system_call_io_char_dev_open(uint32_t *arg_list, uint32_t arg_cnt);
uint32_t system_call_io_char_dev_close(uint32_t *arg_list, uint32_t arg_cnt);
uint32_t system_call_io_char_dev_read(uint32_t *arg_list, uint32_t arg_cnt);
uint32_t system_call_io_char_dev_write(uint32_t *arg_list, uint32_t arg_cnt);

uint32_t system_call_io_block_dev_ioctl(uint32_t *arg_list, uint32_t arg_cnt);
uint32_t system_call_io_block_dev_open(uint32_t *arg_list, uint32_t arg_cnt);
uint32_t system_call_io_block_dev_close(uint32_t *arg_list, uint32_t arg_cnt);
uint32_t system_call_io_block_dev_read(uint32_t *arg_list, uint32_t arg_cnt);
uint32_t system_call_io_block_dev_write(uint32_t *arg_list, uint32_t arg_cnt);
uint32_t system_call_io_block_dev_erase(uint32_t *arg_list, uint32_t arg_cnt);

uint32_t system_call_io_reserved0(uint32_t *arg_list, uint32_t arg_cnt);
uint32_t system_call_io_reserved1(uint32_t *arg_list, uint32_t arg_cnt);
uint32_t system_call_io_reserved2(uint32_t *arg_list, uint32_t arg_cnt);
uint32_t system_call_io_reserved3(uint32_t *arg_list, uint32_t arg_cnt);
uint32_t system_call_io_reserved4(uint32_t *arg_list, uint32_t arg_cnt);

#endif /* NILE_SYSCALL_KERNEL_HANDLERS_H_ */
