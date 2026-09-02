#include "nile/kernel.h"
#include "nile/mcu/stm32f746/io_dev/quadspi.h"
#include "nile/kernel/io_op_queue.h"
#include "app/io_dev_id.h"

void QuadSPI_IRQHandler() {
	nile_kernel *kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
	nile_kernel_io_block_dev *dev = &kk->io_block_dev[BLOCK_DEV_ID_QUADSPI];

	nile_kernel_io_block_dev_op *op = io_block_dev_queue_peek(dev->rqx);

	if (!op) {
		QUADSPI->CR &= ~(QUADSPI_CR_FTIE | QUADSPI_CR_TEIE | QUADSPI_CR_SMIE | QUADSPI_CR_TCIE);
		QUADSPI->FCR = 0xFFFFFFFF;
		kk->status.kernel_log.mem[0]++;
		goto end;
	}
	uint8_t operation_type = op->op.flags & NILE_KERNEL_IO_DEV_OP_FLAG_OPERATION_MASK;

	uint32_t isr = QUADSPI->SR;

	if (isr & (QUADSPI_SR_TEF)) {
		QUADSPI->FCR = QUADSPI_FCR_CTEF;
		QUADSPI->CR &= ~(QUADSPI_CR_FTIE | QUADSPI_CR_TEIE | QUADSPI_CR_SMIE | QUADSPI_CR_TCIE);
		//transfer error
		quadspi_operation_error_handler(dev, QSPI_DEV_ERRCODE_TRANSFER_ERR);
		kk->status.kernel_log.mem[1]++;
		goto end;
	}

	if (isr & (QUADSPI_SR_TCF)) {
		//disable interrupt source, clear the flag
		QUADSPI->CR &= ~QUADSPI_CR_TCIE;
		QUADSPI->FCR = QUADSPI_FCR_CTCF;
		QUADSPI->CR &= ~QUADSPI_CR_EN;
		if (op->op.cursor == 0) {
			kk->status.kernel_log.mem[2]++;
			//configure polling match pattern for Write Latch Enable
			QUADSPI->CR |= QUADSPI_CR_APMS; //automatic polling stop
			QUADSPI->CR |= QUADSPI_CR_PMM; //any flag matches (OR match mode)
			QUADSPI->PSMKR = 1UL << 1; //bit 1 is write latch of QSPI Flash
			QUADSPI->PSMAR = 1UL << 1; //the value has to be 1 (write latch ON)
			QUADSPI->DLR = 1U - 1U;
			QUADSPI->PIR = 8;
			QUADSPI->CR |= QUADSPI_CR_SMIE; //match interrupt enable
			QUADSPI->CR |= QUADSPI_CR_EN;
			QUADSPI->CCR = (QSPI_FMODE_AUTOMATIC_POLLING << QUADSPI_CCR_FMODE_Pos) | (QSPI_QUAD << QUADSPI_CCR_DMODE_Pos)
					| (QSPI_QUAD << QUADSPI_CCR_IMODE_Pos) | (MT25QL128ABA1EW9_COMMAND_READ_STATUS_REGISTER << 0);


//			QUADSPI->CR |= QUADSPI_CR_EN;
//			QUADSPI->DLR = 0; // 1 byte
//			QUADSPI->CCR = (QSPI_FMODE_INDIRECT_READ << QUADSPI_CCR_FMODE_Pos) | (QSPI_SINGLE << QUADSPI_CCR_DMODE_Pos) | (QSPI_SINGLE << QUADSPI_CCR_IMODE_Pos)
//					| (QSPI_SKIP << QUADSPI_CCR_ADMODE_Pos) | (MT25QL128ABA1EW9_COMMAND_READ_STATUS_REGISTER << QUADSPI_CCR_INSTRUCTION_Pos);
//
//			// wait for TCF, then read DR
//			while (!(QUADSPI->SR & QUADSPI_SR_TCF)) {
//			}
//			uint8_t sr = *(volatile uint8_t*) &QUADSPI->DR;
//			uint8_t sr2 = *(volatile uint8_t*) &QUADSPI->DR;

//			QUADSPI->CR = (QUADSPI->CR & ~(0x1F << 8)) | (0x00 << 8); //Set FIFO threshold to 1
//			QUADSPI->CR = (QUADSPI->CR & ~QUADSPI_CR_FTHRES_Msk);
//			/*
//			 * Set communication configuration register
//			 * Functional mode: 			indirect read
//			 * Data mode: 					as per param dataLinesMode
//			 * Instruction mode: 			as per param dataLinesMode
//			 * Instruction: 				as per param instruction
//			 *
//			 */
//			while(QUADSPI->SR & QUADSPI_SR_FLEVEL_Msk) {uint8_t dummy = *(uint8_t*)&QUADSPI->DR;}
//			uint8_t dataLinesMode = QSPI_QUAD;
//			uint8_t instruction = MT25QL128ABA1EW9_COMMAND_READ_STATUS_REGISTER;
//			//uint8_t instruction = MT25QL128ABA1EW9_COMMAND_READ_ENHANCED_VOLATILE_CONFIGURATION_REGISTER;
//			uint8_t dtr = 0;
//			QUADSPI->CCR = ((dtr != 0) << 31) | (QSPI_FMODE_INDIRECT_READ << 26) | (dataLinesMode << 24) | (dataLinesMode << 8) | (instruction << 0);
//
//			/* ---------- Communication Starts Automatically ----------*/
//
//			while (!(QUADSPI->SR & QUADSPI_SR_FLEVEL_Msk)); //Wait for the byte of data to arrive
//			uint8_t sr = *(volatile uint8_t*) &QUADSPI->DR;
//			uint8_t sr2 = *(volatile uint8_t*) &QUADSPI->DR;

		}

		if (op->op.cursor == op->op.data_len) {
			kk->status.kernel_log.mem[3]++;
			//configure polling match pattern for Write In Progress
			QUADSPI->CR |= QUADSPI_CR_APMS; //automatic polling stop
			QUADSPI->CR |= QUADSPI_CR_PMM; //any flag matches (OR match mode)
			QUADSPI->PSMKR = 1UL << 0; //bit 0 is write in progress of QSPI Flash
			QUADSPI->PSMAR = 0UL << 0; //the value has to be 0 (write in progress OFF)
			QUADSPI->DLR = 1U - 1U;
			QUADSPI->PIR = 8;
			QUADSPI->CR |= QUADSPI_CR_SMIE; //match interrupt enable
			QUADSPI->CR |= QUADSPI_CR_EN;
			QUADSPI->CCR = (QSPI_FMODE_AUTOMATIC_POLLING << QUADSPI_CCR_FMODE_Pos) | (QSPI_QUAD << QUADSPI_CCR_DMODE_Pos)
					| (QSPI_QUAD << QUADSPI_CCR_IMODE_Pos) | (MT25QL128ABA1EW9_COMMAND_READ_STATUS_REGISTER << 0);
		}
		goto end;
	}

	if (isr & (QUADSPI_SR_SMF)) {
		//disable interrupt source, clear the flag
		QUADSPI->CR &= ~QUADSPI_CR_SMIE;
		QUADSPI->FCR = QUADSPI_FCR_CSMF;
		if (op->op.cursor == 0) {
			kk->status.kernel_log.mem[4]++;
			if (operation_type == NILE_KERNEL_IO_DEV_OP_FLAG_WRITE) {
				QUADSPI->CR = (QUADSPI->CR & ~QUADSPI_CR_FTHRES_Msk) | ((16UL - 1) << QUADSPI_CR_FTHRES_Pos);
				QUADSPI->CR |= QUADSPI_CR_FTIE; //threshold interrupt
				quadspi_write_cmd_issue(dev); //will issue write command, but no data write (FIFO Thr. does data)
			}
			if (operation_type == NILE_KERNEL_IO_DEV_OP_FLAG_ERASE) {
				QUADSPI->CR |= QUADSPI_CR_TCIE; //transfer complete interrupt
				quadspi_erase_cmd_issue(dev); //will issue erase command, advance cursor
			}
		} else if (op->op.cursor == op->op.data_len) {
			kk->status.kernel_log.mem[5]++;
			//write in progress bit was cleared
			quadspi_cmd_finished(dev);
		}
		goto end;
	}

	if (isr & (QUADSPI_SR_FTF)) {
		//flag managed by hardware
		kk->status.kernel_log.mem[6]++;
		quadspi_write_cmd_load_next_data(dev); //loads data, advances cursor, autoclears the flag
		if (op->op.cursor == op->op.data_len) {
//			op->op.cursor = op->op.data_len;
			QUADSPI->CR &= ~QUADSPI_CR_FTIE;
			QUADSPI->CR |= QUADSPI_CR_TCIE; //transfer complete interrupt
		}
	}

	end: return;
}
