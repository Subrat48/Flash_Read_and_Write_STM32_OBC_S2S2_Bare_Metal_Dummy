/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "flash_memory.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// Address to store the Flash ID structure (Start of the 11th 4KB block)
#define FLASH_ID_STORAGE_ADDR 0x0000A000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi3;

UART_HandleTypeDef huart7;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_UART7_Init(void);
static void MX_SPI3_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_UART7_Init();
	MX_SPI3_Init();
	/* USER CODE BEGIN 2 */

	// --- FLASH MEMORY COMPREHENSIVE TEST ---
	// Initialization: Ensure CS is high
	HAL_GPIO_WritePin(MAIN_FM_CS_GPIO_Port, MAIN_FM_CS_Pin, GPIO_PIN_SET);
	HAL_Delay(100);

	char uart_buf[256]; // Increased buffer for summary table
	uint32_t data_addresses[10];
	// Define 10 Addresses for data (4KB spacing starting from 0)
	for (int i = 0; i < 10; i++) {
		data_addresses[i] = i * 0x1000;
	}

	sprintf(uart_buf,
			"\r\n=== NXT GEN CUBUS OBC: Flash Memory Comprehensive Test ===\r\n");
	HAL_UART_Transmit(&huart7, (uint8_t*) uart_buf, strlen(uart_buf), 100);

	// =================================================================
	// --- PART 1: READ, STORE, and VERIFY FLASH CHIP ID ("Who am I") ---
	// =================================================================
	sprintf(uart_buf, "\r\n--- PART 1: Flash Chip ID Operations ---\r\n");
	HAL_UART_Transmit(&huart7, (uint8_t*) uart_buf, strlen(uart_buf), 100);

	DEVICE_ID chip_id;
	DEVICE_ID readback_id;

	// 1. Read ID directly from the chip's internal registers
	Read_ID(&hspi3, &chip_id);
	sprintf(uart_buf,
			"Read Chip ID directly: Manuf=0x%02X, Type=0x%02X, Cap=0x%02X\r\n",
			chip_id.MAN_ID, chip_id.M_TYPE, chip_id.M_CAP);
	HAL_UART_Transmit(&huart7, (uint8_t*) uart_buf, strlen(uart_buf), 100);

	// 2. Store this ID structure into the Flash Memory itself
	sprintf(uart_buf, "Storing Chip ID to address: 0x%08lX...\r\n",
			FLASH_ID_STORAGE_ADDR);
	HAL_UART_Transmit(&huart7, (uint8_t*) uart_buf, strlen(uart_buf), 100);

	// A. Erase the 4KB sector at the storage address
	Sector_Erase(&hspi3, FLASH_ID_STORAGE_ADDR, 4);
	HAL_Delay(200); // Ensure erase is complete

	// B. Write the entire DEVICE_ID structure
	Page_Write(&hspi3, FLASH_ID_STORAGE_ADDR, (uint8_t*) &chip_id,
			sizeof(DEVICE_ID));
	HAL_Delay(10); // Ensure write is complete
	HAL_UART_Transmit(&huart7, (uint8_t*) "ID stored successfully.\r\n", 25,
			100);

	// 3. Read back the stored ID from Flash to verify
	memset(&readback_id, 0, sizeof(DEVICE_ID)); // Clear buffer
	Bulk_Read(&hspi3, FLASH_ID_STORAGE_ADDR, (uint8_t*) &readback_id,
			sizeof(DEVICE_ID));

	sprintf(uart_buf,
			"Verified ID from Flash : Manuf=0x%02X, Type=0x%02X, Cap=0x%02X\r\n",
			readback_id.MAN_ID, readback_id.M_TYPE, readback_id.M_CAP);
	HAL_UART_Transmit(&huart7, (uint8_t*) uart_buf, strlen(uart_buf), 100);

	// =================================================================
	// --- PART 2: MIXED DATA TYPE READ/WRITE TEST ---
	// =================================================================
	sprintf(uart_buf, "\r\n--- PART 2: Mixed Data Type Test ---\r\n");
	HAL_UART_Transmit(&huart7, (uint8_t*) uart_buf, strlen(uart_buf), 100);

	// --- WRITE SEQUENCE ---
	sprintf(uart_buf, "\r\n[WRITING DATA...]\r\n");
	HAL_UART_Transmit(&huart7, (uint8_t*) uart_buf, strlen(uart_buf), 100);

	for (int i = 0; i < 10; i++) {
		// Erase Sector (4KB)
		Sector_Erase(&hspi3, data_addresses[i], 4);
		HAL_Delay(200); // Safety delay for erase

		// Write Data based on index
		if (i == 0) {
			char data[] = "Subrat";
			Page_Write(&hspi3, data_addresses[i], (uint8_t*) data,
					strlen(data) + 1);
			sprintf(uart_buf, "Addr 0x%06lX: Wrote String 'Subrat'\r\n",
					data_addresses[i]);
		} else if (i == 1) {
			int data = 12345;
			Page_Write(&hspi3, data_addresses[i], (uint8_t*) &data,
					sizeof(int));
			sprintf(uart_buf, "Addr 0x%06lX: Wrote Int %d\r\n",
					data_addresses[i], data);
		} else if (i == 2) {
			float data = 3.14159f;
			Page_Write(&hspi3, data_addresses[i], (uint8_t*) &data,
					sizeof(float));
			sprintf(uart_buf, "Addr 0x%06lX: Wrote Float %.4f\r\n",
					data_addresses[i], data);
		} else if (i == 3) {
			short data = -500;
			Page_Write(&hspi3, data_addresses[i], (uint8_t*) &data,
					sizeof(short));
			sprintf(uart_buf, "Addr 0x%06lX: Wrote Short %d\r\n",
					data_addresses[i], data);
		} else if (i == 4) {
			long data = 987654321;
			Page_Write(&hspi3, data_addresses[i], (uint8_t*) &data,
					sizeof(long));
			sprintf(uart_buf, "Addr 0x%06lX: Wrote Long %ld\r\n",
					data_addresses[i], data);
		} else if (i == 5) {
			char data = 'X';
			Page_Write(&hspi3, data_addresses[i], (uint8_t*) &data,
					sizeof(char));
			sprintf(uart_buf, "Addr 0x%06lX: Wrote Char '%c'\r\n",
					data_addresses[i], data);
		} else {
			int data = i * 1111;
			Page_Write(&hspi3, data_addresses[i], (uint8_t*) &data,
					sizeof(int));
			sprintf(uart_buf, "Addr 0x%06lX: Wrote Int %d\r\n",
					data_addresses[i], data);
		}
		HAL_UART_Transmit(&huart7, (uint8_t*) uart_buf, strlen(uart_buf), 100);
		HAL_Delay(10); // Delay for write cycle
	}

	// --- READ SEQUENCE ---
	sprintf(uart_buf, "\r\n[READING BACK DATA...]\r\n");
	HAL_UART_Transmit(&huart7, (uint8_t*) uart_buf, strlen(uart_buf), 100);

	for (int i = 0; i < 10; i++) {
		uint8_t rx_buffer[32];
		memset(rx_buffer, 0, 32);
		Bulk_Read(&hspi3, data_addresses[i], rx_buffer, 32);

		if (i == 0) {
			sprintf(uart_buf, "Addr 0x%06lX: Read String '%s'\r\n",
					data_addresses[i], (char*) rx_buffer);
		} else if (i == 1) {
			sprintf(uart_buf, "Addr 0x%06lX: Read Int %d\r\n",
					data_addresses[i], *(int*) rx_buffer);
		} else if (i == 2) {
			sprintf(uart_buf, "Addr 0x%06lX: Read Float %.4f\r\n",
					data_addresses[i], *(float*) rx_buffer);
		} else if (i == 3) {
			sprintf(uart_buf, "Addr 0x%06lX: Read Short %d\r\n",
					data_addresses[i], *(short*) rx_buffer);
		} else if (i == 4) {
			sprintf(uart_buf, "Addr 0x%06lX: Read Long %ld\r\n",
					data_addresses[i], *(long*) rx_buffer);
		} else if (i == 5) {
			sprintf(uart_buf, "Addr 0x%06lX: Read Char '%c'\r\n",
					data_addresses[i], *(char*) rx_buffer);
		} else {
			sprintf(uart_buf, "Addr 0x%06lX: Read Int %d\r\n",
					data_addresses[i], *(int*) rx_buffer);
		}
		HAL_UART_Transmit(&huart7, (uint8_t*) uart_buf, strlen(uart_buf), 100);
	}

	// =================================================================
	// --- PART 3: FINAL ADDRESS SUMMARY TABLE ---
	// =================================================================
	sprintf(uart_buf, "\r\n\r\n=== FINAL ADDRESS SUMMARY TABLE ===\r\n");
	HAL_UART_Transmit(&huart7, (uint8_t*) uart_buf, strlen(uart_buf), 100);
	sprintf(uart_buf,
			"| Content Description | Exact Address (HEX) | Exact Address (DEC) |\r\n");
	HAL_UART_Transmit(&huart7, (uint8_t*) uart_buf, strlen(uart_buf), 100);
	sprintf(uart_buf,
			"|---------------------|---------------------|---------------------|\r\n");
	HAL_UART_Transmit(&huart7, (uint8_t*) uart_buf, strlen(uart_buf), 100);

	// Row for the stored ID
	sprintf(uart_buf, "| Flash Chip ID Struct| 0x%08lX        | %-19lu |\r\n",
			FLASH_ID_STORAGE_ADDR, FLASH_ID_STORAGE_ADDR);
	HAL_UART_Transmit(&huart7, (uint8_t*) uart_buf, strlen(uart_buf), 100);

	// Rows for the mixed data
	char *descriptions[] = { "String 'Subrat'", "Integer (12345)",
			"Float (3.1415...)", "Short (-500)", "Long (987654321)",
			"Char ('X')", "Integer (6666)", "Integer (7777)", "Integer (8888)",
			"Integer (9999)" };

	for (int i = 0; i < 10; i++) {
		sprintf(uart_buf, "| %-20s| 0x%08lX        | %-19lu |\r\n",
				descriptions[i], data_addresses[i], data_addresses[i]);
		HAL_UART_Transmit(&huart7, (uint8_t*) uart_buf, strlen(uart_buf), 100);
	}
	sprintf(uart_buf,
			"===================================================================\r\n");
	HAL_UART_Transmit(&huart7, (uint8_t*) uart_buf, strlen(uart_buf), 100);
	sprintf(uart_buf, "\r\n=== Test Complete ===\r\n");
	HAL_UART_Transmit(&huart7, (uint8_t*) uart_buf, strlen(uart_buf), 100);

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 25;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief SPI3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SPI3_Init(void) {

	/* USER CODE BEGIN SPI3_Init 0 */

	/* USER CODE END SPI3_Init 0 */

	/* USER CODE BEGIN SPI3_Init 1 */

	/* USER CODE END SPI3_Init 1 */
	/* SPI3 parameter configuration*/
	hspi3.Instance = SPI3;
	hspi3.Init.Mode = SPI_MODE_MASTER;
	hspi3.Init.Direction = SPI_DIRECTION_2LINES;
	hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi3.Init.NSS = SPI_NSS_SOFT;
	hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi3.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&hspi3) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN SPI3_Init 2 */

	/* USER CODE END SPI3_Init 2 */

}

/**
 * @brief UART7 Initialization Function
 * @param None
 * @retval None
 */
static void MX_UART7_Init(void) {

	/* USER CODE BEGIN UART7_Init 0 */

	/* USER CODE END UART7_Init 0 */

	/* USER CODE BEGIN UART7_Init 1 */

	/* USER CODE END UART7_Init 1 */
	huart7.Instance = UART7;
	huart7.Init.BaudRate = 115200;
	huart7.Init.WordLength = UART_WORDLENGTH_8B;
	huart7.Init.StopBits = UART_STOPBITS_1;
	huart7.Init.Parity = UART_PARITY_NONE;
	huart7.Init.Mode = UART_MODE_TX_RX;
	huart7.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart7.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart7) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN UART7_Init 2 */

	/* USER CODE END UART7_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */

	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOI_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(MAIN_FM_CS_GPIO_Port, MAIN_FM_CS_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(EN_GLOBAL_RESET_GPIO_Port, EN_GLOBAL_RESET_Pin,
			GPIO_PIN_RESET);

	/*Configure GPIO pin : MAIN_FM_CS_Pin */
	GPIO_InitStruct.Pin = MAIN_FM_CS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(MAIN_FM_CS_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : PA15 */
	GPIO_InitStruct.Pin = GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pin : EN_GLOBAL_RESET_Pin */
	GPIO_InitStruct.Pin = EN_GLOBAL_RESET_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(EN_GLOBAL_RESET_GPIO_Port, &GPIO_InitStruct);

	/* USER CODE BEGIN MX_GPIO_Init_2 */

	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}
