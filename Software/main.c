/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <vector>
#include "string.h"
#include "sentences.hpp"
#include "stringslib.h"
#include "gnss.h"
#include "ubx.hpp"
#include "buffer.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart1_rx;

/* USER CODE BEGIN PV */

#define MAIN_BUFF_SIZE 2048
#define READ_SIZE 16	// The number of characters to read before the interrupt is called - must be a factor of the MAIN_BUFF_SIZE
static volatile char MainBuf[MAIN_BUFF_SIZE + 1];	// + 1 so that the last character is always \0
static volatile uint8_t temp[READ_SIZE];
static volatile uint32_t saveIdx = 0;
static volatile uint32_t nmeaIdx = 0;
static volatile uint32_t ubxIdx = 0;
static volatile uint16_t nCompletions = 0;
static volatile uint32_t totalRead = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
#ifdef __cplusplus
extern "C"{
#endif

#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)	// Using this can lead to errors arising in the transmitting of data
static void printUART(const char * __restrict format, ...) _ATTRIBUTE((__format__ (__printf__, 1, 2)));

#ifdef __cplusplus
}
#endif

char * findBufString(char * haystack, const char * needle, size_t bufferStart, size_t bufferLength);
void configureDYNMODEL();

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void printArr(uint8_t * arr, uint16_t size)
{
	char * str = (char *) calloc(3*size + 1, sizeof(char));

	uint16_t i = 0;
	for(i = 0; i < size - 1; i++)
	{
		sprintf(str + 3*i, "%02X ", arr[i]);
		//printf("%2x", arr[i]);
	}

	sprintf(str + 3*size - 3, "%02X", arr[size - 1]);

	printf("%s", str);

	free(str);
}

void printArr_c(uint8_t * arr, uint16_t size)
{
//	char * str = (char *) calloc(2*size + 1, sizeof(char));
//
//	uint16_t i = 0;
//	for(i = 0; i < size; i++)
//	{
//		sprintf(str + 2*i, "%c", arr[i]);
//		//printf("%2x", arr[i]);
//	}
//
//	printf("%s", str);
//
//	free(str);

	uint16_t i = 0;
	for (i = 0; i < size; i++)
	{
		printf("%c", (char) arr[i]);
	}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

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
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_Delay(1000);

  uint32_t Timer = HAL_GetTick();
  uint16_t i = 0;
  printf("Starting\n");

  // Begin reading from DMA. Must be executed for the callback loop to begin
  HAL_UART_Receive_DMA(&huart1, (uint8_t *) temp, READ_SIZE);

  i = 0;

  configureDYNMODEL();

  uint16_t readIdx = saveIdx;
  uint32_t nCharsToRead = 0;
  uint32_t nCharsPrevRead = 0;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
	  	  uint16_t startIdx_pre = saveIdx, nCompletions_pre = nCompletions;
	  	  uint32_t totalRead_pre = totalRead;

		  char ** arr;
		  uint16_t length;
		  char delim[] = "\r\n";

//		  arr = splitString((const char *) MainBuf, delim, &length);

		  arr = (char **) splitBuff((uint8_t *) MainBuf, MAIN_BUFF_SIZE, readIdx, (uint8_t *) delim, strlen(delim), &length);


		  // TODO: Implement logic to only continue if saveIdx > index of arr[length - 1]

		  if (arr != NULL)
		  {
			  readIdx = (arr[length - 1] - *arr + readIdx) % MAIN_BUFF_SIZE;
//			  nCharsToRead = (saveIdx + readIdx) % MAIN_BUFF_SIZE;
			  nCharsToRead = saveIdx > readIdx ? MAIN_BUFF_SIZE + readIdx - saveIdx : readIdx - saveIdx;

			  uint32_t nRead = totalRead;

//			  printf("nCharsToread is: %ld, whereas nRead is: %ld and nCharsPrevRead is: %ld and difference is: %ld\r\n", nCharsToRead, nRead, nCharsPrevRead, nRead - nCharsPrevRead);
			  if (nRead - nCharsPrevRead >= nCharsToRead)
			  {
				  nCharsPrevRead = nRead;

				  uint16_t j;
				  for (j = 0; j < length; j++)
				  {
					  printf("The current line (%d / %d) is: %s\t\tSave index is: %ld\tRead idx is: %d\r\n", j + 1, length, arr[j], saveIdx, readIdx);


					  Sentence<POS> sentence(arr[j]);
					  Sentence<TIME> sentence_time(arr[j]);
	//				  Sentence<GLL> sentence(arr[j]);
	//				  Sentence<RMC> sentence(arr[j]);

	//				  if (strncmp(arr[j] + 3, (char *) "GLL", 3) == 0)
	//				  {
	//					  printf("\r\nThe sentence is: GLL!!!\r\n %s\r\n", arr[j]);
	//				  }
	//				  else
	//				  {
	//					  printf("The sentence is not GLL! Instead it is: ");
	//
	//					  POS * s = sentence.getSentence();
	//
	//					  if (s != NULL)
	//					  {
	//						  printf("\tNOT NULL!");
	//					  }
	//
	//					  printf("\r\n");
	//				  }

					  POS * sent = sentence.getSentence();
					  TIME * time = sentence_time.getSentence();
	//				  GLL * sent = sentence.getSentence();
	//				  RMC * sent = sentence.getSentence();

					  if (sent != NULL) {

						Field<float_t> lat, lon;
						lat = sent->getLatitude();
						lon = sent->getLongitude();
						volatile float flat = 0.0, flon = 0.0;

						if (lat.getValue() != NULL && lon.getValue() != NULL)
						{
							flat = *lat.getValue();
							flon = *lon.getValue();
						}

	//					printf("Time: %s has Lat: %f and Lon: %f\tWith speed: %f\r\n", sent->getTime().c_str(), lat, lon, sent->getSpeedOverGround());

	//					printf("Latitude actually is: %f\r\n", flat);

						printf("Latitude is: %f, whilst longitude is: %f\r\n", flat, flon);
	//					printf("Current datum is: %s\r\n", sent->getDatum().c_str());

	//					  printf("The current text is: %s\r\n", sent->getText().c_str());

					  } else {
	//					printf("Main buffer is: %s\r\n", MainBuf);
	//					  HAL_UART_Transmit(&huart2, (const uint8_t *) "Given sentence was not the required type!\r\n", 1, 0xFFFF);
	//					  std::cout << "Given sentence was not the required type!\r\n";
	//					  printf("No GLL sentence\r\n");
					  }

					  if (time != NULL)
					  {
						  const std::string * t = time->getTime().getValue();

						  if (t != NULL)
						  {
							  printf("The current time is: %s\r\n", t->c_str());
						  }
					  }
				  }

			  }


			  free(*arr); *arr = NULL;
			  free(arr); arr = NULL;
		  }
		  else
		  {
			  printf("Array is NULL\r\n");
		  }


		  //Timer = HAL_GetTick();
//	  }

		  uint16_t startIdx_post = saveIdx, nCompletions_post = nCompletions;
		  uint32_t totalRead_post = totalRead;

//		  printf("The total number of indexes traversed: %d with the number of completions: %d\t", startIdx_post - startIdx_pre, nCompletions_post - nCompletions_pre);
//		  printf("Total number of characters read: %ld, with total times buffer overflow: %ld\r\n", totalRead_post - totalRead_pre, (totalRead_post - totalRead_pre) / MAIN_BUFF_SIZE);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 38400;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/**
 * @brief  Retargets the C library printf function to the USART.
 *   None
 * @retval None
 */
#ifdef __cplusplus
extern "C" {
#endif

PUTCHAR_PROTOTYPE {
	/* Place your implementation of fputc here */
	/* e.g. write a character to the USART1 and Loop until the end of transmission */
	HAL_UART_Transmit(&huart2, (uint8_t*) &ch, 1, 0xFFFF);

	return ch;
}

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {

	#include <errno.h>
	#include <stddef.h>

	int getentropy(void *buffer, size_t length) {
		(void)buffer;
		(void)length;
		return -ENOSYS;  // Function not implemented
	}
}
#endif


#ifdef __cplusplus
extern "C"
{
#endif

void HAL_UART_RxCpltCallback(UART_HandleTypeDef * huart)
{
	memcpy((void *) (MainBuf + saveIdx), (void *) temp, READ_SIZE);

	saveIdx += READ_SIZE;
	totalRead += READ_SIZE;

	if (saveIdx >= MAIN_BUFF_SIZE)
	{
		saveIdx = 0;
		nCompletions++;
	}

//	MainBuf[saveIdx] = *temp;
//	saveIdx++;
//
//	if (saveIdx >= MAIN_BUFF_SIZE)
//	{
//		saveIdx = 0;
//	}
}

// NOTE: For now, only a max of MAIN_BUFF_SIZE can be printed
// Main timeout of 1000ms
static void printUART(const char * __restrict format, ...)
{
	static char str[MAIN_BUFF_SIZE + 1];
	str[MAIN_BUFF_SIZE] = '\0';	// Ensure final character is string terminator

	va_list args;

	// Start variadic function for format
	va_start(args, format);

	// Put at most MAIN_BUFF_SIZE characters into the string
	vsnprintf(str, MAIN_BUFF_SIZE, format, args);

	// End variadic function for format
	va_end(args);

	HAL_UART_Transmit(&huart2, (uint8_t *) str, strlen(str), 1000);
}

#ifdef __cplusplus
}
#endif



char * findBufString(char * haystack, const char * needle, size_t bufferStart, size_t bufferLength)
{
	char * foundStr = NULL;

	size_t i = 0;

	size_t needleLength = strlen(needle);

	/* Scan the string for instances of the needle, exiting when found */
	for (i = 0; i < bufferLength - needleLength + 1; i++)
	{
		if (strncmp(haystack + ((i + bufferStart) % bufferLength), needle, needleLength) == 0)
		{
			foundStr = haystack + ((i + bufferStart) % bufferLength);
			break;
		}
	}

	return foundStr;
}

HAL_StatusTypeDef sendConfiguration(CFG_VALGET getter,  UART_HandleTypeDef * uartHandle = &huart1, uint16_t timeout = 100)
{
	std::vector<uint8_t> getter_vec = getter.getUBX();

	return HAL_UART_Transmit(uartHandle, getter_vec.data(), getter_vec.size(), timeout);
}

CFG_VALGET getConfiguration(CFG_VALGET getter, uint16_t timeout = 1000)
{
	CFG_VALGET found(CFG::LAYER::RAM, 0x0000, {});

	static const uint8_t configReturnHeader[] = {0xb5, 0x62, 0x06, 0x8b};	// The header to look for when a config message is returned

	HAL_StatusTypeDef status = sendConfiguration(getter);

	uint8_t * ubxReturn = NULL;
	bool completeMsg = false;
	uint16_t length = 0;
	uint8_t * foundUBX = NULL;
	uint16_t i = 0;

	uint32_t timer = HAL_GetTick();

	if (status == HAL_OK)
	{
		while(!completeMsg && HAL_GetTick() - timer < timeout)
		{
			ubxReturn = findInBuff((uint8_t *) MainBuf, configReturnHeader, 4, saveIdx, MAIN_BUFF_SIZE);

			if (ubxReturn != NULL)
			{
				// If the length is contiguous, just grab it
				if (ubxReturn - (uint8_t *) MainBuf <= MAIN_BUFF_SIZE - 6)
				{
					length = UBX_DTYPES::convertU2(ubxReturn + 4);
				}
				// If the length is over the buffer end, get the split version
				else
				{
					uint8_t firstByte = *(MainBuf + (ubxReturn - (uint8_t *) MainBuf + 4) % MAIN_BUFF_SIZE);
					uint8_t secondByte = *(MainBuf + (ubxReturn - (uint8_t *) MainBuf + 5) % MAIN_BUFF_SIZE);

					length = (uint16_t) secondByte << 8 | firstByte;
				}

				foundUBX = (uint8_t *) calloc(length + 8, sizeof(uint8_t));

				// Fill the foundUBX array with actual values
				for (i = 0; i < length + 8; i++)
				{
					// Ensure correct wrapping around of the circular buffer
					uint8_t byte = *(MainBuf + (ubxReturn - (uint8_t *) MainBuf + i) % MAIN_BUFF_SIZE);

					foundUBX[i] = byte;
				}

				found.readUBX(foundUBX);


				if (found.getValidity())
				{
					completeMsg = true;
				}


				free(foundUBX);	// Note: this means that this->payload for the found message will be a floating pointer
			}
		}
	}

	return found;
}

bool checkConfiguration(CFG::LAYER layer, CFG::KEYS key, uint8_t value)
{
	bool correctConfig = false;

	CFG_VALGET returnConfig = getConfiguration({layer, 0x0000, {key}}, 3000);

	CFGData * cfgData = returnConfig.getCFGData();

	if (cfgData != NULL)
	{
		CFGData::CFGDataPair * pair = cfgData->getPair(key);

		if (pair != NULL)
		{
			uint8_t val = pair->getValueU1();

			if (val == value)
			{
				correctConfig = true;
			}
		}
	}

	return correctConfig;
}

// NOTE: At the moment, this function will not differentiate between ACK-NAK and timeout. Anything
//		 that is not ACK-ACK will be considered as not acknowledged and hence, false.
bool setConfiguration(CFG_VALSET setter, uint16_t timeout = 1000)
{
	bool successful = false;
	HAL_StatusTypeDef transmissionSuccess;
	uint8_t * found = NULL;

	std::vector<uint8_t> setter_vec = setter.getUBX();

	transmissionSuccess = HAL_UART_Transmit(&huart1, setter_vec.data(), setter_vec.size(), 100);

	if (transmissionSuccess != HAL_OK)
	{
		return false;
	}

	ACK::ACK ack = ACK::ACK(setter.getClass(), setter.getID());
	std::vector<uint8_t> ack_vec = ack.getUBX();

	uint32_t timer = HAL_GetTick();

	while((found = (uint8_t *) findInBuff((uint8_t *) MainBuf, ack_vec.data(), ack_vec.size(), saveIdx, MAIN_BUFF_SIZE)) == NULL
			&& HAL_GetTick() - timer < timeout)
	{
	}

	if (found != NULL)
	{
		successful = true;
	}

	return successful;
}

// TODO: Implement config value checker to check correct config value.
//bool checkConfigValue(CFG::LAYER layer, CFG::KEYS key, uint8_t value)
//{
//	CFG_VALGET getter(layer, 0x0000, {key});
//}

void configureDYNMODEL()
{
	printf("Checking current Dynamic Platform Model configuration... ");

	// Verify if the current configuration is already the expected one
	bool correctConfig = checkConfiguration(CFG::LAYER::FLASH_, CFG::KEYS::NAVSPG_DYNMODEL, CFG::NAVSPG::AIR4);

	if (!correctConfig)
	{
		printf("Current configuration is not Airborne with < 4g acceleration.\r\n");

		std::vector<uint8_t> setter_vec;

		printf("Setting configuration to Airborne with < 4g acceleration... ");

		// Set airborn in flash
		CFG_VALSET setter_flash(CFG::LAYER::FLASH_, {{CFG::KEYS::NAVSPG_DYNMODEL, CFG::NAVSPG::DYNMODEL::AIR4}});

		bool correctFlash = setConfiguration(setter_flash);

		if (correctFlash)
			printf("Flash config set. ");
		else
			printf("Flash config setting unsuccessful. ");

		// Set airborn in ram
		CFG_VALSET setter_ram(CFG::LAYER::RAM, {{CFG::KEYS::NAVSPG_DYNMODEL, CFG::NAVSPG::DYNMODEL::AIR4}});

		bool correctRAM = setConfiguration(setter_ram);

		if (correctRAM)
			printf("RAM config set. ");
		else
			printf("RAM config setting unsuccessful. ");


		if (correctFlash && correctRAM)
			printf("Configuration successfully set!\r\n");
		else
			printf("Configuration could not be set!\r\n");
	}
	else
	{
		printf("Configuration already set to Airborne with < 4g acceleration!\r\n");
	}

//	correctConfig = checkConfiguration(CFG::LAYER::FLASH_, CFG::KEYS::, CFG::NAVSPG::AIR4);
//
//	if (!correctConfig)
//	{
//		printf("Current configuration is not Airborne with < 4g acceleration.\r\n");
//
//		std::vector<uint8_t> setter_vec;
//
//		printf("Setting configuration to Airborne with < 4g acceleration... ");
//
//		// Set time precision in flash
//		CFG_VALSET setter_flash(CFG::LAYER::FLASH_, {{CFG::KEYS::NAVSPG_DYNMODEL, CFG::NAVSPG::DYNMODEL::AIR4}});
//
//		bool correctFlash = setConfiguration(setter_flash);
//
//		if (correctFlash)
//			printf("Flash config set. ");
//		else
//			printf("Flash config setting unsuccessful. ");
//
//		// Set time precision in ram
//		CFG_VALSET setter_ram(CFG::LAYER::RAM, {{CFG::KEYS::NAVSPG_DYNMODEL, CFG::NAVSPG::DYNMODEL::AIR4}});
//
//		bool correctRAM = setConfiguration(setter_ram);
//
//		if (correctRAM)
//			printf("RAM config set. ");
//		else
//			printf("RAM config setting unsuccessful. ");
//
//
//		if (correctFlash && correctRAM)
//			printf("Configuration successfully set!\r\n");
//		else
//			printf("Configuration could not be set!\r\n");
//	}
//	else
//	{
//		printf("Configuration already set to Airborne with < 4g acceleration!\r\n");
//	}
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  printf("Error occurred!\r\n");

  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
