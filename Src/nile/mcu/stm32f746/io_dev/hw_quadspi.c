#include "nile/kernel.h"
#include "nile/irq.h"
#include "nile/mcu/stm32f746/io_dev/gpio.h"
#include "app/io_dev_irq_priority.h"
#include "nile/mcu/stm32f746/io_dev/quadspi.h"
#include "nile/barriers.h"
#include "nile/cache.h"
#include "nile/external_cache.h"

/* BASIC OPERATIONS */

void mcu_hw_qspi_register_read(uint8_t instruction, uint8_t dataLinesMode, uint8_t *destination, uint8_t dtr) {

	while (QUADSPI->SR & QUADSPI_SR_BUSY); //Make sure no operation is going on

	QUADSPI->FCR = 0xFFFFFFFF; //Clear all flags
	QUADSPI->DLR = 1U - 1U; //Set data length to 1
	QUADSPI->CR = (QUADSPI->CR & ~(0x1F << 8)) | (0x00 << 8); //Set FIFO threshold to 1
	QUADSPI->CR = (QUADSPI->CR & ~QUADSPI_CR_FTHRES_Msk);
	/*
	 * Set communication configuration register
	 * Functional mode: 			indirect read
	 * Data mode: 					as per param dataLinesMode
	 * Instruction mode: 			as per param dataLinesMode
	 * Instruction: 				as per param instruction
	 *
	 */
	QUADSPI->CCR = ((dtr != 0) << 31) | (QSPI_FMODE_INDIRECT_READ << 26) | (dataLinesMode << 24) | (dataLinesMode << 8) | (instruction << 0);

	/* ---------- Communication Starts Automatically ----------*/

	while (!(QUADSPI->SR & QUADSPI_SR_FLEVEL_Msk)); //Wait for the byte of data to arrive
	*destination = *((uint8_t*) (&(QUADSPI->DR))); //Read byte from data register and place it into provided memory; Byte access
	while (QUADSPI->SR & QUADSPI_SR_BUSY); //Make sure no operation is going on

}

/*
 * This function writes a single instruction via extended SPI, Dual SPI or Quad SPI
 *
 * @param		instruction		8-bit instruction
 * @param		dataLinesMode	1 = 1 data line
 * 								2 = 2 data lines
 * 								3 = 4 data lines
 * */
void mcu_hw_qspi_instruction_write_indirect(uint8_t instruction, uint8_t dataLinesMode, uint8_t dtr) {

	while (QUADSPI->SR & QUADSPI_SR_BUSY); //Make sure no operation is going on
	QUADSPI->FCR = 0xFFFFFFFF; //Clear all flags

	/*
	 * Set communication configuration register
	 * Functional mode: 			indirect write
	 * Instruction mode: 			as per param dataLines
	 * Instruction: 				as per param instruction
	 *
	 */
	QUADSPI->CCR = ((dtr != 0) << 31) | (QSPI_FMODE_INDIRECT_WRITE << 26) | (dataLinesMode << 8) | (instruction << 0);

	/* ---------- Communication Starts Automatically ----------*/

	while (!(QUADSPI->SR & QUADSPI_SR_TCF)); //Wait for operation to complete
	//while (QUADSPI->SR & QUADSPI_SR_BUSY);
}

/*
 * This function writes a byte to a register of QSPI Flash via extended SPI, Dual SPI or Quad SPI
 *
 * @param		instruction		8-bit instruction (write to register command)
 * @param		registerContent	New register value
 * @param		dataLinesMode	1 = 1 data line
 * 								2 = 2 data lines
 * 								3 = 4 data lines
 * */
void mcu_hw_qspi_register_write_indirect(uint8_t instruction, uint8_t dataLinesMode, uint8_t registerContent, uint8_t dtr) {

	while (QUADSPI->SR & QUADSPI_SR_BUSY); //Make sure no operation is going on
	QUADSPI->FCR = 0xFFFFFFFF; //Clear all flags

	QUADSPI->DLR = 1U - 1U; //Set number of bytes to write: 1
	QUADSPI->CR = (QUADSPI->CR & ~QUADSPI_CR_FTHRES_Msk) | (0x00 << QUADSPI_CR_FTHRES_Pos); //Set FIFO threshold to 1

	/*
	 * Set communication configuration register
	 * Functional mode: 			indirect write
	 * Data mode: 					as per param dataLinesMode
	 * Instruction mode: 			as per param dataLinesMode
	 * Instruction: 				as per param instruction
	 *
	 * Load data
	 *
	 */
	QUADSPI->CCR = ((dtr != 0) << 31) | (QSPI_FMODE_INDIRECT_WRITE << 26) | (dataLinesMode << 24) | (dataLinesMode << 8) | (instruction << 0);
	QUADSPI->DR = registerContent; //Load new register value into data register

	/* ---------- Communication Starts Automatically ----------*/

	while (!(QUADSPI->SR & QUADSPI_SR_TCF)); //Wait for operation to complete
	//while (QUADSPI->SR & QUADSPI_SR_BUSY);
}

/*
 * This function reads QSPI memory via Quad SPI
 *
 * @param		address			address of the first byte to read
 * @param		length			how many bytes to read
 * @param		destination		destination array
 *
 * */
void mcu_hw_qspi_read_indirect(uint32_t address, uint32_t length, uint8_t *destination, uint8_t dtr) {

	while (QUADSPI->SR & QUADSPI_SR_BUSY); //Make sure no operation is going on
	QUADSPI->FCR = 0xFFFFFFFF; //Clear all flags
	QUADSPI->DLR = length - 1U; //Set number of bytes to read
	QUADSPI->CR = (QUADSPI->CR & ~QUADSPI_CR_FTHRES_Msk) | (0x00 << QUADSPI_CR_FTHRES_Pos); //Set FIFO threshold to 1

	/*
	 * Set communication configuration register
	 * Functional mode: 			Indirect read
	 * Data mode: 					4 Lines
	 * Instruction mode: 			4 Lines
	 * Address mode:				4 Lines
	 * Address size:				24 Bits
	 * Dummy cycles:				6 Cycles
	 * Instruction: 				Quad Output Fast Read
	 *
	 * Set 24-bit Address
	 *
	 */

//	QUADSPI->CCR = ((dtr != 0) << 31) | (QSPI_FMODE_INDIRECT_READ << 26) | (QSPI_QUAD << 24) | (QSPI_QUAD << 8) | (QSPI_QUAD << 10) | (QSPI_ADSIZE_24 << 12) | (0x06 << 18)
//			| (MT25QL128ABA1EW9_COMMAND_QUAD_OUTPUT_FAST_READ << 0);
	QUADSPI->CCR = ((dtr != 0) << 31) | (QSPI_FMODE_INDIRECT_READ << 26) | (QSPI_QUAD << 24) | (QSPI_QUAD << 8) | (QSPI_QUAD << 10) | (QSPI_ADSIZE_24 << 12)
			| (0x06 << 18) | (MT25QL128ABA1EW9_COMMAND_FAST_READ << 0);
	QUADSPI->AR = (0x00FFFFFF) & address;

	/* ---------- Communication Starts Automatically ----------*/

	while (QUADSPI->SR & QUADSPI_SR_BUSY) {
		if (QUADSPI->SR & QUADSPI_SR_FTF) {
			*destination = *((uint8_t*) &(QUADSPI->DR)); //Read a byte from data register, byte access
			destination++;
		}
	}
}

/*
 * This function erases QSPI memory via Quad SPI
 *
 * @param		address			address of the first byte of subsector to erase
 * @param		instruction		8-bit instruction (various erase commands)
 *
 * */
void mcu_hw_qspi_erase(uint8_t instruction, uint32_t address, uint8_t dtr) {

	while (QUADSPI->SR & QUADSPI_SR_BUSY); //Make sure no operation is going on
	mcu_hw_qspi_instruction_write_indirect(MT25QL128ABA1EW9_COMMAND_WRITE_ENABLE,
	QSPI_QUAD, dtr); //erase requires write enable command first
	QUADSPI->FCR = 0xFFFFFFFF; //Clear all flags

	/*
	 * Set communication configuration register
	 * Functional mode: 			indirect write
	 * Address mode:				4 lines
	 * Address size:				24 bits
	 * Address:						as per param address (any address in subsector is valid)
	 * Instruction mode: 			4 lines
	 * Instruction: 				as per param instruction
	 *
	 * Set 24-bit Address
	 *
	 */
	QUADSPI->CCR = ((dtr != 0) << 31) | (QSPI_FMODE_INDIRECT_WRITE << 26) | (QSPI_QUAD << 10) | (0x02 << 12) | (QSPI_QUAD << 8) | (instruction << 0);
	QUADSPI->AR = 0x00FFFFFF & address;

	/* ---------- Communication Starts Automatically ----------*/

	while (QUADSPI->SR & QUADSPI_SR_BUSY); //Make sure no operation is going on

	/* ---------- Waiting for erase process to end ---------- */

	uint8_t eraseEnded = 0x00;
	do {
		mcu_hw_qspi_register_read(MT25QL128ABA1EW9_COMMAND_READ_FLAG_STATUS_REGISTER,
		QSPI_QUAD, &eraseEnded, dtr);
		eraseEnded &= 0x80;
	} while (eraseEnded == 0x00);

}

void mcu_hw_qspi_write_indirect(uint32_t address, uint32_t length, uint8_t data[], uint8_t dtr) {

	while (QUADSPI->SR & QUADSPI_SR_BUSY); //Make sure no operation is going on
	mcu_hw_qspi_instruction_write_indirect(MT25QL128ABA1EW9_COMMAND_WRITE_ENABLE,
	QSPI_QUAD, dtr); //program requires write enable
	QUADSPI->FCR = 0xFFFFFFFF; //Clear all flags

	QUADSPI->DLR = length - 1U; //Set number of bytes to write
	QUADSPI->CR = (QUADSPI->CR & ~QUADSPI_CR_FTHRES_Msk) | (0x00 << QUADSPI_CR_FTHRES_Pos); //Set FIFO threshold to 1

	/*
	 * Set communication configuration register
	 * Functional mode: 			Indirect write
	 * Data mode: 					4 lines
	 * Instruction mode: 			4 lines
	 * Address mode:				4 lines
	 * Address size:				24 bits
	 * Instruction: 				Quad Input Fast Program
	 *
	 * Write 24-bit Address
	 *
	 */
	QUADSPI->CCR = ((dtr != 0) << 31) | (QSPI_FMODE_INDIRECT_WRITE << 26) | (QSPI_QUAD << 24) | (QSPI_QUAD << 8) | (QSPI_QUAD << 10) | (QSPI_ADSIZE_24 << 12)
			| (MT25QL128ABA1EW9_COMMAND_QUAD_INPUT_FAST_PROGRAM << 0);
	QUADSPI->AR = 0x00FFFFFF & address;
	uint32_t data_pointer = 0;
	do {
		*((uint8_t*) (&QUADSPI->DR)) = data[data_pointer]; //place data - byte access
		data_pointer++;
		while ((QUADSPI->SR & QUADSPI_SR_FLEVEL_Msk) != 0x00); //wait for the data to be shifted out
	} while (QUADSPI->SR & QUADSPI_SR_BUSY);

	/* ---------- Communication Starts Automatically, ends at this point ----------*/

	/* ---------- Checking status register to make sure writing operation has finished ---------- */
	uint8_t readyStatusFlag = 0x00;
	QUADSPI->FCR = 0xFFFFFFFF; //Clear all flags
	do {
		mcu_hw_qspi_register_read(MT25QL128ABA1EW9_COMMAND_READ_FLAG_STATUS_REGISTER,
		QSPI_QUAD, &readyStatusFlag, dtr);
		readyStatusFlag &= 0x80;
	} while (readyStatusFlag == 0x00);

}

void quadspi_memory_mapped_mode_enable(struct nile_kernel_io_block_dev *dev) {
	(void) dev;
	//ERRATUM es0290 2.4.3
	QUADSPI->AR = 0;
	QUADSPI->CR |= QUADSPI_CR_ABORT;
	while (QUADSPI->CR & QUADSPI_CR_ABORT);
	//End of ERRATUM
	QUADSPI->CR &= ~QUADSPI_CR_EN;
	QUADSPI->DLR = 0;
	QUADSPI->CR |= ((3U - 1U) << QUADSPI_CR_PRESCALER_Pos); //Default clock prescaler (3; max 216MHz/3 = 72MHz)
	QUADSPI->CR |= (QUADSPI_CR_EN);

	//XIP mode initialization
	//read volatile configuration register
	uint8_t VCR;
	mcu_hw_qspi_register_read(
	MT25QL128ABA1EW9_COMMAND_READ_VOLATILE_CONFIGURATION_REGISTER,
	QSPI_QUAD, &VCR, 0);
	//modify (set XIP enable bit - bit 3 set to 0)
	VCR = VCR & ~(1 << 3); //XIP enable
	VCR &= ~0xF0;
	VCR |= ((uint8_t) MT25QL128ABA1EW9_QIO_DTR_DUMMYCYC_CNT << 4);
	//write volatile configuration register
	mcu_hw_qspi_instruction_write_indirect(MT25QL128ABA1EW9_COMMAND_WRITE_ENABLE,
	QSPI_QUAD, 0);
	mcu_hw_qspi_register_write_indirect(
	MT25QL128ABA1EW9_COMMAND_WRITE_VOLATILE_CONFIGURATION_REGISTER,
	QSPI_QUAD, VCR, 0);
	//operation completed (on the physical level)

	// Make sure there is no write in progress
	uint8_t sr_wip;
	do {
		mcu_hw_qspi_register_read(MT25QL128ABA1EW9_COMMAND_READ_STATUS_REGISTER, QSPI_QUAD, &sr_wip, 0);
	} while (sr_wip & (1 << 0));

	//Performing indirect read to activate XIP in the Flash IC

	QUADSPI->FCR = 0xFFFFFFFF; //Clear all flags
	QUADSPI->DLR = 1U - 1U; //Set number of bytes to read
	QUADSPI->CR = (QUADSPI->CR & ~QUADSPI_CR_FTHRES_Msk) | (0x00 << QUADSPI_CR_FTHRES_Pos); //Set FIFO threshold to 1

	/*
	 * Set communication configuration register
	 * Functional mode: 			Indirect read
	 * Data mode: 					4 Lines
	 * Instruction mode: 			4 Lines
	 * Address mode:				4 Lines
	 * Alternate byte mode/size     4 Lines/1 byte
	 * Address size:				24 Bits
	 * SDR/DTR:                     DTR
	 * Dummy cycles:				6 Cycles - 1byte*1cycle/byte = 5 Cycles
	 * Instruction: 				Quad Output Fast Read
	 *
	 * Set 24-bit Address
	 *
	 */
	QUADSPI->CR &= ~QUADSPI_CR_EN;
	QUADSPI->CR = (QUADSPI->CR & ~QUADSPI_CR_PRESCALER_Msk) | ((3U - 1U) << QUADSPI_CR_PRESCALER_Pos); //Default clock prescaler (3; max 216MHz/3 = 72MHz)
	QUADSPI->DCR |= ((2 - 1) << QUADSPI_DCR_CSHT_Pos); //chip select high time for read ops 20ns min; 50ns for write/prog/erase
	QUADSPI->CR &= ~(QUADSPI_CR_SSHIFT_Msk); //Sample shift 1/2 cycle disabled (for DTR)
	uint8_t XIP_activation_readout;
	QUADSPI->CCR = (1 << QUADSPI_CCR_DDRM_Pos) | (1 << QUADSPI_CCR_DHHC_Pos) | (QSPI_FMODE_INDIRECT_READ << QUADSPI_CCR_FMODE_Pos)
			| (QSPI_QUAD << QUADSPI_CCR_DMODE_Pos) | (QSPI_QUAD << QUADSPI_CCR_IMODE_Pos) | (QSPI_QUAD << QUADSPI_CCR_ADMODE_Pos)
			| (QSPI_ADSIZE_24 << QUADSPI_CCR_ADSIZE_Pos) | ((MT25QL128ABA1EW9_QIO_DTR_DUMMYCYC_CNT - 1) << QUADSPI_CCR_DCYC_Pos)
			| (MT25QL128ABA1EW9_COMMAND_DTR_QUAD_OUTPUT_FAST_READ << 0);
	QUADSPI->ABR = 0x00000000; //XIP bit set to 0 (XIP enable)
	QUADSPI->CCR = (QUADSPI->CCR & ~QUADSPI_CCR_ABMODE_Msk) | (QSPI_QUAD << QUADSPI_CCR_ABMODE_Pos); //alt byte 4 IO
	QUADSPI->CCR = (QUADSPI->CCR & ~QUADSPI_CCR_ABSIZE_Msk) | (0 << QUADSPI_CCR_ABSIZE_Pos); //1 alt byte
	QUADSPI->CR |= (QUADSPI_CR_EN);
	QUADSPI->AR = 0;

	/* ---------- Communication Starts Automatically ----------*/

	while (QUADSPI->SR & QUADSPI_SR_BUSY) {
		if (QUADSPI->SR & QUADSPI_SR_FTF) {
			XIP_activation_readout = *((uint8_t*) &(QUADSPI->DR)); //Read a byte from data register
		}
	}
	(void) XIP_activation_readout;
	QUADSPI->FCR = 0xFFFFFFFF; //Clear all flags

	//ERRATUM es0290 2.4.3
	QUADSPI->CR &= ~QUADSPI_CR_EN;
	QUADSPI->AR = 0;
	QUADSPI->CR |= (QUADSPI_CR_EN);
	QUADSPI->CR |= QUADSPI_CR_ABORT;
	while (QUADSPI->CR & QUADSPI_CR_ABORT);
	//End of ERRATUM

	//Enable memory-mapped XIP mode
	QUADSPI->CCR = (QUADSPI->CCR & ~(QUADSPI_CCR_FMODE_Msk | QUADSPI_CCR_IMODE_Msk)) | (QSPI_FMODE_MEMORY_MAPPED << QUADSPI_CCR_FMODE_Pos)
			| (QSPI_SKIP << QUADSPI_CCR_IMODE_Pos);

}

void quadspi_memory_mapped_mode_disable(struct nile_kernel_io_block_dev *dev) {
	(void) dev;
	nile_external_cache_prefetch_disable();
	nile_external_cache_disable();

	nile_branch_prediction_disable();

	nile_cache_invalidate_icache_all();
	nile_cache_disable_icache();

	nile_cache_clean_dcache_all();
	nile_cache_disable_dcache();
	nile_dsb();
	nile_isb();
	QUADSPI->CR &= ~QUADSPI_CR_EN;
	QUADSPI->ABR = 0xFFFFFFFF; //disable XIP mode bit
	QUADSPI->CR |= (QUADSPI_CR_EN);
	//Perform a read (memory-mapped is enabled)
	nile_dsb();
	volatile uint8_t dummy_mem_read = *(uint8_t*) QSPI_MEMMAPPED_ADDRESS;
	(void) dummy_mem_read;
	nile_dsb();
	nile_isb();
	QUADSPI->CR &= ~QUADSPI_CR_EN;
	QUADSPI->CCR = (QUADSPI->CCR & ~QUADSPI_CCR_FMODE_Msk) | (QSPI_FMODE_INDIRECT_READ << QUADSPI_CCR_FMODE_Pos);
	QUADSPI->CCR = (QUADSPI->CCR & ~QUADSPI_CCR_IMODE_Msk) | (QSPI_QUAD << QUADSPI_CCR_IMODE_Pos);
	QUADSPI->DLR = 0;
	QUADSPI->CR |= (QUADSPI_CR_EN);

	nile_cache_invalidate_dcache_all();
	nile_cache_enable_dcache();

	nile_cache_enable_icache();

	nile_branch_prediction_enable();

	nile_external_cache_reset(); //re-enables
	nile_external_cache_prefetch_enable();

}

void quadspi_hw_clock_reset() {
	RCC->AHB3ENR &= ~(RCC_AHB3ENR_QSPIEN); //QUADSPI Clock
	while (RCC->AHB3ENR & RCC_AHB3ENR_QSPIEN);

	RCC->AHB3ENR |= (RCC_AHB3ENR_QSPIEN); //QUADSPI Clock

	RCC->AHB3RSTR |= RCC_AHB3RSTR_QSPIRST;
	RCC->AHB3RSTR &= ~RCC_AHB3RSTR_QSPIRST;
}
void quadspi_hw_gpio_config() {
	RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOBEN) | (RCC_AHB1ENR_GPIODEN) | (RCC_AHB1ENR_GPIOEEN); //GPIOB, GPIOD, GPIOE Clock
	/*
	 QuadSPI:
	 NCS -  PB6
	 CLK -  PB2
	 D0  - PD11
	 D1  - PD12
	 D2  -  PE2
	 D3  - PD13
	 */

	mcu_hw_gpio_alternate_function_set(GPIOB, 2, 9U);
	mcu_hw_gpio_push_pull_set(GPIOB, 2);
	mcu_hw_gpio_pullup_pulldown_disable(GPIOB, 2);
	mcu_hw_gpio_drive_speed_set(GPIOB, 2, 0xFF);

	mcu_hw_gpio_alternate_function_set(GPIOB, 6, 10U);
	mcu_hw_gpio_push_pull_set(GPIOB, 6);
	mcu_hw_gpio_pullup_pulldown_disable(GPIOB, 6);
	mcu_hw_gpio_drive_speed_set(GPIOB, 6, 0xFF);

	mcu_hw_gpio_alternate_function_set(GPIOD, 11, 9U);
	mcu_hw_gpio_push_pull_set(GPIOD, 11);
	mcu_hw_gpio_pullup_pulldown_disable(GPIOD, 11);
	mcu_hw_gpio_drive_speed_set(GPIOD, 11, 0xFF);

	mcu_hw_gpio_alternate_function_set(GPIOD, 12, 9U);
	mcu_hw_gpio_push_pull_set(GPIOD, 12);
	mcu_hw_gpio_pullup_pulldown_disable(GPIOD, 12);
	mcu_hw_gpio_drive_speed_set(GPIOD, 12, 0xFF);

	mcu_hw_gpio_alternate_function_set(GPIOD, 13, 9U);
	mcu_hw_gpio_push_pull_set(GPIOD, 13);
	mcu_hw_gpio_pullup_pulldown_disable(GPIOD, 13);
	mcu_hw_gpio_drive_speed_set(GPIOD, 13, 0xFF);

	mcu_hw_gpio_alternate_function_set(GPIOE, 2, 9U);
	mcu_hw_gpio_push_pull_set(GPIOE, 2);
	mcu_hw_gpio_pullup_pulldown_disable(GPIOE, 2);
	mcu_hw_gpio_drive_speed_set(GPIOE, 2, 0xFF);
}

void quadspi_hw_ic_init() {
	//Full reset regardless of Flash IC current state
	mcu_hw_qspi_instruction_write_indirect(MT25QL128ABA1EW9_COMMAND_RESET_ENABLE,
	QSPI_QUAD, 1);
	mcu_hw_qspi_instruction_write_indirect(MT25QL128ABA1EW9_COMMAND_RESET_MEMORY,
	QSPI_QUAD, 1);
	mcu_hw_qspi_instruction_write_indirect(MT25QL128ABA1EW9_COMMAND_RESET_ENABLE,
	QSPI_QUAD, 0);
	mcu_hw_qspi_instruction_write_indirect(MT25QL128ABA1EW9_COMMAND_RESET_MEMORY,
	QSPI_QUAD, 0);
	mcu_hw_qspi_instruction_write_indirect(MT25QL128ABA1EW9_COMMAND_RESET_ENABLE,
	QSPI_SINGLE, 0);
	mcu_hw_qspi_instruction_write_indirect(MT25QL128ABA1EW9_COMMAND_RESET_MEMORY,
	QSPI_SINGLE, 0);

	//Dummy cycles
	//XIP (no instruction) capability: ON (must be enabled via mode bit)
	//Wrap-around: continuous
	uint8_t VCR = ((uint8_t) MT25QL128ABA1EW9_QIO_DTR_DUMMYCYC_CNT << 4 | 0x00 << 3 | 0x03 << 0);
	mcu_hw_qspi_instruction_write_indirect(MT25QL128ABA1EW9_COMMAND_WRITE_ENABLE,
	QSPI_SINGLE, 0);
	mcu_hw_qspi_register_write_indirect(
	MT25QL128ABA1EW9_COMMAND_WRITE_VOLATILE_CONFIGURATION_REGISTER,
	QSPI_SINGLE, VCR, 0);

	uint8_t VCR_readback = 0;
	mcu_hw_qspi_register_read(
	MT25QL128ABA1EW9_COMMAND_READ_VOLATILE_CONFIGURATION_REGISTER,
	QSPI_SINGLE, &VCR_readback, 0);

	//Quad IO enable (0 << 7)
	//Double IO enable (0 << 6)
	//DTR enable (0 << 5)
	//Reser/hold enable (1 << 4)
	//Reserved (1 << 3)
	//30 Ohm (default) impedance (0x07 << 0)
	uint8_t EVCR = (0 << 7) | (1 << 6) | (1 << 5) | (0 << 4) | (1 << 3) | (0x07 << 0);
	mcu_hw_qspi_instruction_write_indirect(MT25QL128ABA1EW9_COMMAND_WRITE_ENABLE,
	QSPI_SINGLE, 0); //Enable write to register
	mcu_hw_qspi_register_write_indirect(
	MT25QL128ABA1EW9_COMMAND_WRITE_ENHANCED_VOLATILE_CONFIGURATION_REGISTER,
	QSPI_SINGLE, EVCR, 0);

	uint8_t EVCR_readback = 0;
	//QUADSPI->CR &= (~QUADSPI_CR_SSHIFT_Msk); //sample shift off for DTR
	//MT25QL128ABA1EW9_COMMAND_READ_ENHANCED_VOLATILE_CONFIGURATION_REGISTER
	mcu_hw_qspi_register_read(
	MT25QL128ABA1EW9_COMMAND_READ_ENHANCED_VOLATILE_CONFIGURATION_REGISTER,
	QSPI_QUAD, &EVCR_readback, 0);

	//mcu_hw_qspi_erase(MT25QL128ABA1EW9_COMMAND_4KB_SUBSECTOR_ERASE, 0, 0); //test

	QUADSPI->CR |= QUADSPI_CR_APMS; //automatic stop for polling

	QUADSPI->CR &= (~QUADSPI_CR_SSHIFT_Msk); //sample shift off for DTR
	QUADSPI->DCR &= ~QUADSPI_DCR_CSHT_Msk;
	QUADSPI->DCR |= ((2 - 1) << QUADSPI_DCR_CSHT_Pos); //chip select high time for read ops 20ns min; 50ns for write/prog/erase

	//mcu_hw_qspi_erase(MT25QL128ABA1EW9_COMMAND_4KB_SUBSECTOR_ERASE, 4096, 0); //test

}
/* Exposed functions */

void quadspi_hw_init(struct nile_kernel_io_block_dev *dev) {
	(void) dev;
	quadspi_hw_clock_reset();
	quadspi_hw_gpio_config();

	QUADSPI->CR = (QUADSPI->CR & ~QUADSPI_CR_PRESCALER_Msk) | ((12U - 1U) << QUADSPI_CR_PRESCALER_Pos); //Default clock prescaler (3; max 216MHz/3 = 72MHz)
	QUADSPI->CR |= (QUADSPI_CR_SSHIFT_Msk); //sample shift off for STR
	QUADSPI->DCR |= ((4 - 1) << QUADSPI_DCR_CSHT_Pos); //chip select high time for read ops 20ns min; 50ns for write/prog/erase
	QUADSPI->DCR |= (24 - 1) << QUADSPI_DCR_FSIZE_Pos; //16MiB
	QUADSPI->FCR = 0xFFFFFFFF; //Clear all flags

	QUADSPI->CR |= QUADSPI_CR_EN;

	quadspi_hw_ic_init();

	nile_irq_disable(QUADSPI_IRQn);
	nile_irq_clear_pending(QUADSPI_IRQn);
	nile_irq_set_priority(QUADSPI_IRQn, nile_irq_get_raw_priority_value(QUADSPI_IRQ_LEVEL));
	nile_irq_enable(QUADSPI_IRQn);

	//QUADSPI->CR |= QUADSPI_CR_EN;

	//Memory-mapped XIP DTR
	quadspi_memory_mapped_mode_enable(0);
}

void quadspi_hw_rqx_start(struct nile_kernel_io_block_dev *dev) {

	nile_kernel_io_block_dev_op *new_op = io_block_dev_queue_peek(dev->rqx);
	uint8_t operation_type;
	uint8_t op_length_is_valid;
	if (!new_op)
		return;
	//either write or erase
	restart_pending: operation_type = new_op->op.flags & NILE_KERNEL_IO_DEV_OP_FLAG_OPERATION_MASK;
	op_length_is_valid = 0;

	if (operation_type == NILE_KERNEL_IO_DEV_OP_FLAG_WRITE) {
		if (new_op->op.data_len == dev->write_block_size) {
			op_length_is_valid = 1;
		}
	}
	if (operation_type == NILE_KERNEL_IO_DEV_OP_FLAG_ERASE) {
		if ((new_op->op.data_len == 4 * 1024) || (new_op->op.data_len == 32 * 1024) || (new_op->op.data_len == 64 * 1024)
				|| (new_op->op.data_len == 16 * 1024 * 1024)) {
			op_length_is_valid = 1;
		}
	}
	if (!op_length_is_valid) {
		nile_kernel_io_block_dev_op *op = io_block_dev_queue_peek(dev->rqx);
		*op->op.io_hw_op_finished_code_ptr = (QSPI_DEV_ERRCODE_BAD_OP_LENGTH << IO_HW_OP_FINISHED_CODE_ERRCODE_POS);

		io_block_dev_queue_pop(dev->rqx);
		nile_kernel_io_block_dev_op *next_op = io_block_dev_queue_peek(dev->rqx);
		while (next_op && (next_op->op.flags & NILE_KERNEL_IO_DEV_OP_FLAG_ABORTED)) {
			io_block_dev_queue_pop(dev->rqx);
			next_op = io_block_dev_queue_peek(dev->rqx);
		}
		if (next_op == NULL) {
			if (dev->hw_stop != NULL) {
				dev->hw_stop(dev);
				dev->hw_memmapped_enable(dev);
				dev->flags |= NILE_KERNEL_IO_BLOCK_DEV_FLAG_DEV_MEMMAPPED;
			}
			dev->flags &= ~NILE_KERNEL_IO_BLOCK_DEV_FLAG_DEV_HW_ACTIVE;
		} else {
			//valid operation found
			goto restart_pending;
		}

	}
	new_op->op.flags = (new_op->op.flags & ~NILE_KERNEL_IO_DEV_OP_FLAG_PENDING) | NILE_KERNEL_IO_DEV_OP_FLAG_ACTIVE;

	if (dev->hw_memmapped_disable) {
		dev->hw_memmapped_disable(dev);
		dev->flags &= ~NILE_KERNEL_IO_BLOCK_DEV_FLAG_DEV_MEMMAPPED;
	}
	QUADSPI->CR &= ~QUADSPI_CR_EN;
	QUADSPI->DLR = 0;
	QUADSPI->CR = (QUADSPI->CR & QUADSPI_CR_FTHRES_Msk) | ((32UL - 1) << QUADSPI_CR_FTHRES_Pos);
	QUADSPI->DCR = (QUADSPI->DCR & ~QUADSPI_DCR_CSHT_Msk) | ((2 - 1) << QUADSPI_DCR_CSHT_Pos); //min 50ns; prescaler 12
	QUADSPI->CR = (QUADSPI->CR & ~QUADSPI_CR_PRESCALER_Msk) | ((12U - 1U) << QUADSPI_CR_PRESCALER_Pos); //216/12 = 18MHz, 1 cycle is 1/1.8*10^7 ~ 60ns
	QUADSPI->FCR = 0xFFFFFFFF;
	QUADSPI->CR |= (QUADSPI_CR_SSHIFT_Msk); //Sample shift 1/2 cycle enabled (for SDR)
	//QUADSPI->CR &= ~(QUADSPI_CR_SSHIFT_Msk);
	QUADSPI->CR |= (QUADSPI_CR_EN);

	//Both write and erase need write enable, same approach for both
	QUADSPI->CCR = (QSPI_FMODE_INDIRECT_WRITE << QUADSPI_CCR_FMODE_Pos) | (QSPI_QUAD << QUADSPI_CCR_IMODE_Pos) | (QSPI_SKIP << QUADSPI_CCR_ADMODE_Pos)
			| (QSPI_SKIP << QUADSPI_CCR_DMODE_Pos) | (MT25QL128ABA1EW9_COMMAND_WRITE_ENABLE << QUADSPI_CCR_INSTRUCTION_Pos);
	QUADSPI->CR |= QUADSPI_CR_TCIE;

}

void quadspi_operation_error_handler(struct nile_kernel_io_block_dev *dev, uint8_t hw_specific_errcode) {
	//Device not ready, so we don't accept further operations
	dev->flags &= ~NILE_KERNEL_IO_BLOCK_DEV_FLAG_DEV_READY;

	//Set device error code
	dev->flags = (dev->flags & ~(NILE_KERNEL_IO_BLOCK_DEV_FLAG_DEV_ERROR | NILE_KERNEL_IO_BLOCK_DEV_FLAG_DEV_ERROR_CODE_MASK));
	dev->flags = (dev->flags | (NILE_KERNEL_IO_BLOCK_DEV_FLAG_DEV_ERROR | (hw_specific_errcode << NILE_KERNEL_IO_BLOCK_DEV_FLAG_DEV_ERROR_CODE_POS)));

	//drain the operation queue, set error codes for all operations in the queue
	nile_kernel_io_block_dev_op *it_op = io_block_dev_queue_peek(dev->rqx);
	*it_op->op.io_hw_op_finished_code_ptr = hw_specific_errcode << IO_HW_OP_FINISHED_CODE_ERRCODE_POS;
	io_block_dev_queue_pop(dev->rqx);
	it_op = io_block_dev_queue_peek(dev->rqx);
	while (it_op != NULL) {
		it_op->op.flags = (it_op->op.flags | NILE_KERNEL_IO_DEV_OP_FLAG_ABORTED) & ~NILE_KERNEL_IO_DEV_OP_FLAG_PENDING;
		*it_op->op.io_hw_op_finished_code_ptr = IO_HW_OP_FINISHED_CODE_ERRCODE_ABORTED << IO_HW_OP_FINISHED_CODE_ERRCODE_POS;
		io_block_dev_queue_pop(dev->rqx);
		it_op = io_block_dev_queue_peek(dev->rqx);
	}

	//Stop reception
	if (dev->hw_stop != NULL) {
		dev->hw_stop(dev);
	}

	dev->flags &= ~NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_ACTIVE;
}

void quadspi_write_cmd_load_next_data(struct nile_kernel_io_block_dev *dev);
void quadspi_write_cmd_issue(struct nile_kernel_io_block_dev *dev) {
	QUADSPI->CR &= ~QUADSPI_CR_EN;
	nile_kernel_io_block_dev_op *op = io_block_dev_queue_peek(dev->rqx);
	QUADSPI->DLR = op->op.data_len - 1U;
	QUADSPI->CR = (QUADSPI->CR & ~QUADSPI_CR_FTHRES_Msk) | ((16UL - 1UL) << QUADSPI_CR_FTHRES_Pos);
	QUADSPI->ABR = 0x55667788;
	op->op.cursor = 0;
	QUADSPI->CR |= QUADSPI_CR_EN;
	QUADSPI->CCR = (QSPI_FMODE_INDIRECT_WRITE << QUADSPI_CCR_FMODE_Pos) | (QSPI_QUAD << QUADSPI_CCR_DMODE_Pos) | (QSPI_QUAD << QUADSPI_CCR_IMODE_Pos)
			| (QSPI_QUAD << QUADSPI_CCR_ADMODE_Pos) | (QSPI_ADSIZE_24 << QUADSPI_CCR_ADSIZE_Pos)
//			| (QSPI_QUAD << QUADSPI_CCR_ABMODE_Pos) | (0x03 << QUADSPI_CCR_ABSIZE_Pos)
			| (MT25QL128ABA1EW9_COMMAND_PAGE_PROGRAM << QUADSPI_CCR_INSTRUCTION_Pos);
	QUADSPI->AR = op->block_dev_mem_off;

	//	while((QUADSPI->SR & QUADSPI_SR_FTF)){
//		quadspi_write_cmd_load_next_data(dev);
//	}
//	quadspi_write_cmd_load_next_data(dev);
//	quadspi_write_cmd_load_next_data(dev);
	//QUADSPI->DR = 0xD00DFACE;
	//hw starts
	QUADSPI->CR |= QUADSPI_CR_FTIE;
}

void quadspi_erase_cmd_issue(struct nile_kernel_io_block_dev *dev) {
	nile_kernel_io_block_dev_op *op = io_block_dev_queue_peek(dev->rqx);
	uint8_t erase_instruction = 0;
	switch (op->op.data_len) {
	case (4 * 1024):
		erase_instruction = MT25QL128ABA1EW9_COMMAND_4KB_SUBSECTOR_ERASE;
		break;
	case (32 * 1024):
		erase_instruction = MT25QL128ABA1EW9_COMMAND_32KB_SUBSECTOR_ERASE;
		break;
	case (64 * 1024):
		erase_instruction = MT25QL128ABA1EW9_COMMAND_SECTOR_ERASE;
		break;
	case (16 * 1024 * 1024):
		erase_instruction = MT25QL128ABA1EW9_COMMAND_BULK_ERASE_1;
		break;
	default:
		return;
	}
	QUADSPI->CCR = (QSPI_FMODE_INDIRECT_WRITE << QUADSPI_CCR_FMODE_Pos) | (QSPI_SKIP << QUADSPI_CCR_DMODE_Pos) | (QSPI_QUAD << QUADSPI_CCR_IMODE_Pos)
			| (QSPI_QUAD << QUADSPI_CCR_ADMODE_Pos) | (QSPI_ADSIZE_24 << QUADSPI_CCR_ADSIZE_Pos) | (erase_instruction << QUADSPI_CCR_INSTRUCTION_Pos);
	QUADSPI->AR = op->block_dev_mem_off;
	//hw starts
	QUADSPI->CR |= QUADSPI_CR_TCIE;
	op->op.cursor = op->op.data_len;
}

void quadspi_cmd_finished(struct nile_kernel_io_block_dev *dev) {
	//remove from queue, find the next
	nile_kernel_io_block_dev_op *op = io_block_dev_queue_peek(dev->rqx);
	*op->op.io_hw_op_finished_code_ptr = (op->op.data_len << IO_HW_OP_FINISHED_CODE_OP_LENGTH_POS) & IO_HW_OP_FINISHED_CODE_OP_LENGTH_MASK;
	io_block_dev_queue_pop(dev->rqx);
	nile_kernel_io_block_dev_op *next_op = io_block_dev_queue_peek(dev->rqx);
	while (next_op && (next_op->op.flags & NILE_KERNEL_IO_DEV_OP_FLAG_ABORTED)) {
		io_block_dev_queue_pop(dev->rqx);
		next_op = io_block_dev_queue_peek(dev->rqx);
	}
	if (next_op == NULL) {
		if (dev->hw_stop != NULL) {
			dev->hw_stop(dev);
			if (dev->hw_memmapped_enable)
				dev->hw_memmapped_enable(dev);
			dev->flags |= NILE_KERNEL_IO_BLOCK_DEV_FLAG_DEV_MEMMAPPED;
		}
		dev->flags &= ~NILE_KERNEL_IO_BLOCK_DEV_FLAG_DEV_HW_ACTIVE;
	} else {
		//valid operation found
		dev->rqx_execute(dev);
	}
}

void quadspi_write_cmd_load_next_data(struct nile_kernel_io_block_dev *dev) {
	uint16_t gran_cfg = dev->write_granularity;
	uint32_t total_bytes = (uint32_t) ((gran_cfg & NILE_KERNEL_IO_BLOCK_DEV_GRN_TOTAL_MASK) >> NILE_KERNEL_IO_BLOCK_DEV_GRN_TOTAL_POS);
	uint32_t total_words = total_bytes >> 2;
	nile_kernel *kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
	nile_kernel_io_block_dev_op *op = io_block_dev_queue_peek(dev->rqx);
	//write two words at once
	for (uint32_t i = 0; i < (total_words >> 1); i++) {
		kk->status.kernel_log.mem[10]++;
		QUADSPI->DR = *(uint32_t*) &op->op.data_buffer[op->op.cursor];
		QUADSPI->DR = *(uint32_t*) &op->op.data_buffer[op->op.cursor + 4];
		op->op.cursor += 2 * sizeof(uint32_t);
		if (op->op.cursor >= op->op.data_len)
			break;
	}
}

void quadspi_hw_stop(struct nile_kernel_io_block_dev *dev) {
	(void) dev;
	QUADSPI->CR &= ~(QUADSPI_CR_FTIE | QUADSPI_CR_TEIE | QUADSPI_CR_SMIE | QUADSPI_CR_TCIE);
	QUADSPI->FCR = 0xFFFFFFFF;
}
