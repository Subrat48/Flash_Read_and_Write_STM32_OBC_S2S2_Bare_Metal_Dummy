/*
 * flash_memory.h
 *
 * Created on: Feb 12, 2025
 * Author: root
 */

#ifndef INC_FLASH_MEMORY_H_
#define INC_FLASH_MEMORY_H_

#include "main.h"

// --- Commands ---
#define		WRITE_ENABLE					0x06
#define		WRITE_DISABLE					0x04
#define 	READ_ID							0x9E
#define 	READ							0x03
#define		PAGE_PROGRAM					0x02
#define		SECTOR_ERASE					0xD8
#define		SUBSECTOR_ERASE_32KB			0x52
#define		SUBSECTOR_ERASE_4KB				0x20
#define		CHIP_ERASE						0xC4

#define 	PAGE_SIZE						256

// --- Unused Commands (Kept for compatibility) ---
#define		BYTE_4_READ						0x13
#define		BYTE_4_PAGE_PROGRAM				0x12
#define		SECTOR_ERASE_4_BYTE				0xDC
#define		SUBSECTOR_ERASE_32KB_4_BYTE		0x5C
#define		SUBSECTOR_ERASE_4KB_4_BYTE		0x21

typedef struct {
	uint8_t MAN_ID;
	uint8_t M_TYPE;
	uint8_t M_CAP;
	uint8_t REM_BYTES;
	uint8_t EXT_ID;
	uint8_t DEV_INFO;
	uint8_t UID[14];
} DEVICE_ID;

// --- Function Prototypes ---
void delay_us(uint32_t us);

void Read_ID(SPI_HandleTypeDef *SPI, DEVICE_ID *FM_ID);
void Bulk_Read(SPI_HandleTypeDef *SPI, uint32_t address, uint8_t *data,
		uint16_t size);
void Page_Write(SPI_HandleTypeDef *SPI, uint32_t address, uint8_t *data,
		uint16_t size);
uint8_t Sector_Erase(SPI_HandleTypeDef *SPI, uint32_t address,
		uint8_t sector_size);

void FM_Disable(SPI_HandleTypeDef *SPI);
void FM_Enable(SPI_HandleTypeDef *SPI);
void Write_Enable(SPI_HandleTypeDef *SPI);
void Chip_Erase(SPI_HandleTypeDef *SPI);

// Unused 4B prototypes kept to prevent link errors if referenced
void Byte_Write_4B(SPI_HandleTypeDef *SPI, uint32_t address, uint8_t data);
uint8_t Sector_Erase_4B(SPI_HandleTypeDef *SPI, uint32_t address,
		uint8_t sector_size);
void Page_Write_4B(SPI_HandleTypeDef *SPI, uint32_t address, uint8_t *data,
		uint16_t size);
void Bulk_Read_4B(SPI_HandleTypeDef *SPI, uint32_t address, uint8_t *data,
		uint16_t size);
uint8_t Read_Byte_4B(SPI_HandleTypeDef *SPI, uint32_t address);

#endif /* INC_FLASH_MEMORY_H_ */
