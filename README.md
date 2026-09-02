# NileRTOS
Embedded RTOS

Real-time operating system. Architecture info:
- Architecture-agnostic design with stubs for other architectures, e.g. RISC-V
- The existing example is for ARMv7-M, device used in example is STM32F746 (on a Discovery board)

OS features:
- Memory protection
- Cache management
- Easy system calls
- IO via system calls
- IO as char devices and block devices
- Lazy FPU stacking
- TLSF kernel memory allocator (O(1) insertion, O(1) deletion)
- Superflexible scheduler (round-robin, fair, unfair, any combination)
- Pipes for inter-process communication (as char devices)
- Tickless idle naturally supported by architecture

Existing example on STM32F746 Discovery board includes:
- UART1 as char device
- Pipe0 as char device
- QSPI Flash in QSPI DDR XIP (no instruction) mode for reading, indirect write for erase/write
- Task without FPU, task with lazy stacking FPU


TODO:
- Replace scheduler's task linked lists with minheap structures

TODO MCU-specific:
- Finish GPIO as char device, with IO system call as digital writes and reads and IOCTL system call for GPIO function config
