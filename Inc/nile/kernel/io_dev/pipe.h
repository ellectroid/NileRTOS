#ifndef NILE_KERNEL_IO_DEV_PIPE_H_
#define NILE_KERNEL_IO_DEV_PIPE_H_

void pipe_hw_init(struct nile_kernel_io_char_dev *dev);
void pipe_hw_reset(struct nile_kernel_io_char_dev *dev);
void pipe_hw_tx_start(struct nile_kernel_io_char_dev *dev);
void pipe_hw_rx_start(struct nile_kernel_io_char_dev *dev);
void pipe_hw_tx_stop(struct nile_kernel_io_char_dev *dev);
void pipe_hw_rx_stop(struct nile_kernel_io_char_dev *dev);

#endif /* NILE_KERNEL_IO_DEV_PIPE_H_ */
