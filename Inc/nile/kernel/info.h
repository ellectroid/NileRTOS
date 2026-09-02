#ifndef NILE_KERNEL_INFO_H_
#define NILE_KERNEL_INFO_H_

#include "nile/stdtypes.h"
#include "nile/kernel_config.h"

typedef struct nile_kernel_info{
	uint32_t kernel_size;
	uint16_t version_major;
	uint16_t version_minor;
	uint32_t version_patch;
	uint8_t infotext[NILE_KERNEL_INFO_INFOTEXT_SIZE];
}nile_kernel_info;


#endif /* NILE_KERNEL_INFO_H_ */
