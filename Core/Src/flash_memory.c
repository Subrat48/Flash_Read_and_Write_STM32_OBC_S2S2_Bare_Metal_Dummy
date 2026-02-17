/*
 * flash_memory.c
 *
 * Created on: Feb 12, 2025
 * Author: root
 */

#include "main.h"
#include "flash_memory.h"

// --- DELAY FUNCTION ---
void delay_us(uint32_t us) {
	HAL_Delay(1); // Simple 1ms delay is safe and sufficient for these ops
}

// --- Chip Select Functions ---
void FM_Enable(SPI_HandleTypeDef *SPI) {
	HAL_GPIO_WritePin(MAIN_FM_CS_GPIO_Port, MAIN_FM_CS_Pin, GPIO_PIN_RESET);
	delay_us(1);
}

void FM_Disable(SPI_HandleTypeDef *SPI) {
	HAL_GPIO_WritePin(MAIN_FM_CS_GPIO_Port, MAIN_FM_CS_Pin, GPIO_PIN_SET);
	delay_us(1);
}

// --- Operations ---

void Read_ID(SPI_HandleTypeDef *SPI, DEVICE_ID *FM_ID) {
	uint8_t cmd = READ_ID;
	DEVICE_ID buff;
	uint8_t data[20];
	int i;

	FM_Enable(SPI);
	HAL_SPI_Transmit(SPI, &cmd, 1, 300);
	HAL_SPI_Receive(SPI, data, 20, 1000);
	FM_Disable(SPI);

	buff.MAN_ID = data[0];
	buff.M_TYPE = data[1];
	buff.M_CAP = data[2];
	buff.REM_BYTES = data[3];
	buff.EXT_ID = data[4];
	buff.DEV_INFO = data[5];
	for (i = 6; i < 20; i++) {
		buff.UID[i] = data[i];
	}
	*FM_ID = buff;
}

void Page_Write(SPI_HandleTypeDef *SPI, uint32_t address, uint8_t *data,
		uint16_t size) {
	uint8_t cmd = PAGE_PROGRAM;
	uint8_t command[3];

	// Handle data larger than one page (256 bytes)
	while (size > PAGE_SIZE) {
		command[0] = (uint8_t) (address >> 16 & 0xff);
		command[1] = (uint8_t) (address >> 8 & 0xff);
		command[2] = (uint8_t) (address & 0xff);

		Write_Enable(SPI);
		FM_Enable(SPI);
		HAL_SPI_Transmit(SPI, &cmd, 1, 100);
		HAL_SPI_Transmit(SPI, command, 3, 200);
		HAL_SPI_Transmit(SPI, data, PAGE_SIZE, 200);
		FM_Disable(SPI);

		HAL_Delay(5); // Internal Write Cycle Time ~3ms

		size = size - PAGE_SIZE;
		data = data + 256;
		address = address + 256;
	}

	// Handle remaining data
	command[0] = (uint8_t) (address >> 16 & 0xff);
	command[1] = (uint8_t) (address >> 8 & 0xff);
	command[2] = (uint8_t) (address & 0xff);

	Write_Enable(SPI);
	FM_Enable(SPI);
	HAL_SPI_Transmit(SPI, &cmd, 1, 100);
	HAL_SPI_Transmit(SPI, command, 3, 200);
	HAL_SPI_Transmit(SPI, data, size, 200);
	FM_Disable(SPI);

	HAL_Delay(5);
}

// --- FIXED SECTOR ERASE FUNCTION ---
uint8_t Sector_Erase(SPI_HandleTypeDef *SPI, uint32_t address,
		uint8_t sector_size) {
	uint8_t addr[3]; // Changed to 3 bytes for standard addressing
	uint8_t cmd = 0;

	// FIX: Correct 24-bit Address Calculation
	addr[0] = (uint8_t) (address >> 16 & 0xff);
	addr[1] = (uint8_t) (address >> 8 & 0xff);
	addr[2] = (uint8_t) (address & 0xFF);

	switch (sector_size) {
	case 64:
		cmd = SECTOR_ERASE;
		Write_Enable(SPI);
		FM_Enable(SPI);
		HAL_SPI_Transmit(SPI, &cmd, 1, 100);
		HAL_SPI_Transmit(SPI, addr, 3, 200);
		FM_Disable(SPI);
		HAL_Delay(500); // 64KB Erase takes time
		return 1;
	case 32:
		cmd = SUBSECTOR_ERASE_32KB;
		Write_Enable(SPI);
		FM_Enable(SPI);
		HAL_SPI_Transmit(SPI, &cmd, 1, 100);
		HAL_SPI_Transmit(SPI, addr, 3, 200);
		FM_Disable(SPI);
		HAL_Delay(400); // 32KB Erase takes time
		return 2;
	case 4:
		cmd = SUBSECTOR_ERASE_4KB;
		Write_Enable(SPI);
		FM_Enable(SPI);
		HAL_SPI_Transmit(SPI, &cmd, 1, 100);
		HAL_SPI_Transmit(SPI, addr, 3, 200);
		FM_Disable(SPI);
		HAL_Delay(200); // 4KB Erase takes ~50-100ms
		return 3;
	default:
		return 0;
	}
}

void Write_Enable(SPI_HandleTypeDef *SPI) {
	uint8_t cmd = WRITE_ENABLE;
	FM_Enable(SPI);
	HAL_SPI_Transmit(SPI, &cmd, 1, 500);
	FM_Disable(SPI);
	delay_us(1);
}

void Bulk_Read(SPI_HandleTypeDef *SPI, uint32_t address, uint8_t *data,
		uint16_t size) {
	uint8_t cmd[4];
	cmd[0] = READ;
	// FIX: Ensure Read matches Write/Erase addressing
	cmd[1] = (uint8_t) (address >> 16 & 0xff);
	cmd[2] = (uint8_t) (address >> 8 & 0xff);
	cmd[3] = (uint8_t) (address & 0xff);

	FM_Enable(SPI);
	HAL_SPI_Transmit(SPI, cmd, 4, 100);
	HAL_SPI_Receive(SPI, data, size, 200);
	FM_Disable(SPI);
	delay_us(1);
}

// --- Unused Stubs ---
void Byte_Write_4B(SPI_HandleTypeDef *SPI, uint32_t address, uint8_t data) {
}
uint8_t Sector_Erase_4B(SPI_HandleTypeDef *SPI, uint32_t address,
		uint8_t sector_size) {
	return 0;
}
void Page_Write_4B(SPI_HandleTypeDef *SPI, uint32_t address, uint8_t *data,
		uint16_t size) {
}
void Chip_Erase(SPI_HandleTypeDef *SPI) {
	uint8_t cmd = CHIP_ERASE;
	Write_Enable(SPI);
	FM_Enable(SPI);
	HAL_SPI_Transmit(SPI, &cmd, 1, 100);
	FM_Disable(SPI);
	HAL_Delay(50000);
}
void Bulk_Read_4B(SPI_HandleTypeDef *SPI, uint32_t address, uint8_t *data,
		uint16_t size) {
}
uint8_t Read_Byte_4B(SPI_HandleTypeDef *SPI, uint32_t address) {
	return 0;
}
