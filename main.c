/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include <usart_driver.h>
#include <gpio_driver.h>
#include <morse.h>
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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint32_t pressTimestamp = 0;
uint32_t nowTimestamp = 0;
uint32_t timeWithoutPressTimestamp = 0;
const uint32_t longPress = 500;
const uint32_t stopTime = 5000;

uint32_t playTime = 500;
uint8_t inputCode = 0;
char inputCodeLength = 0;
char blinkCodeIndex = 0;
uint32_t blinkCodeTimestamp = 0;
uint32_t blinkDelay = 500;
bool isDiodeWorking = false;

bool transmitRequest = false;
uint8_t transmitLetter = 0;
bool transmitStarted = false;

bool newLetter = false;
uint8_t receivedLetter = 0;
bool receiveStarted = false;
uint8_t buffer[8]; // 8 - buffer size
bool isBufferFull = false;
char bufferIndex = 0;
char bufferLength = 8;
char bufferGetIndex = 0;
char bufferCounter = 0;

uint8_t letterToBlink;
char blinkLetterIndex = 0;
char blinkLetterLength = 0;
uint32_t blinkLetterTimestamp = 0;

bool isEnabledInterrupt = false;


void resetInputCode(){
	inputCode = 0;
	inputCodeLength = 0;
	blinkCodeIndex = 0;
}

void handleButtonClick(){
	bool buttonState = getButtonState();
	if (buttonState){// если нажата
		if (pressTimestamp == 0) nowTimestamp = HAL_GetTick(); // вводится новое в случае долгого ненажатия кнопки
		pressTimestamp = getTimeDifference(nowTimestamp);// getButton определяет время, прошедшее от nowTime
		// на протяжении всего нажатия nowTime не меняется
	} else {
		if (pressTimestamp != 0 && inputCodeLength == 4){
			resetInputCode();
		}
		if (pressTimestamp >= longPress){
			inputCode = inputCode * 10 + 2;
			inputCodeLength++;
			nowTimestamp = HAL_GetTick();
		} else if (pressTimestamp > 0) {
			inputCode = inputCode * 10 + 1;
			inputCodeLength++;
			nowTimestamp = HAL_GetTick();
		} else if (pressTimestamp == 0) {
			timeWithoutPressTimestamp = getTimeDifference(nowTimestamp); // при ненажатии startTime не обновляется
			if (timeWithoutPressTimestamp >= stopTime){
				char letter = codeToLetter(inputCode);
				if (letter != -1){
					transmitLetter = letter;
					transmitRequest = true;
				}
				timeWithoutPressTimestamp = 0;
				resetInputCode();
			}
		}
		pressTimestamp = 0;// т.к. действия обработки pressTime закончены, нужно обнулить
	}
}

void playCodeBlink(){ // вывод на диоды вводимой с кнопки последовательности
	uint32_t timeWithoutBlink = getTimeDifference(blinkCodeTimestamp);
	if (timeWithoutBlink >= blinkDelay){
		uint8_t buff = inputCode;
		uint8_t counter = inputCodeLength - blinkCodeIndex;
		for(int i = 1; i < counter; i++){
			buff = buff / 10;
		}
		buff %= 10; // код 2 или 1
		uint8_t diode = buff == 1? 1: 2; // 1 - yellow, 2 - red, 0 - green
		if (getDiodeState(diode)){
			turnOffDiode(diode);
			blinkCodeIndex++;
		} else {
			turnOnDiode(diode);
		}
		blinkCodeTimestamp = HAL_GetTick();
	}
}

void playLetterBlink() {
	uint32_t timeWithoutBlink = getTimeDifference(blinkLetterTimestamp);
	if (timeWithoutBlink >= blinkDelay){
		uint8_t buff = letterToBlink;
		uint8_t counter = blinkLetterLength - blinkLetterIndex;
		for(int i = 1; i < counter; i++){
			buff = buff / 10;
		}
		buff %= 10; // код 2 или 1
		uint8_t diode = buff == 1? 1: 2; // 1 - yellow, 2 - red, 0 - green
		if (getDiodeState(diode)){
			turnOffDiode(diode);
			blinkLetterIndex++;
		} else {
			turnOnDiode(diode);
		}
		blinkLetterTimestamp = HAL_GetTick();
	}
}

void handleTransmit(){ // вызывается только при выводе символа с кнопки
	if (transmitRequest){
		transmitRequest = false;
		transmitStarted = false;
		resetInputCode();
	}
}

void bufferPut(uint8_t letter){
	if (!isBufferFull){
		buffer[bufferIndex] = letter;
		bufferIndex++;
		bufferCounter++;
		if (bufferCounter == bufferLength) isBufferFull = true;
		if (bufferIndex == bufferCounter) bufferIndex = 0;
	}
}

bool isBufferEmpty(){
	return bufferCounter == 0;
}

uint8_t bufferGet(){
	if (!isBufferEmpty()){
		uint8_t letter = buffer[bufferGetIndex];
		bufferGetIndex++;
		bufferCounter--;
		if (bufferCounter < bufferLength) isBufferFull = false;
		if (bufferGetIndex == bufferLength) bufferGetIndex = 0;
		return letter;
	}
	return 0;
}

void putNewLetter(){
	if (!isBufferEmpty()){
		uint8_t letter = bufferGet();
		uint8_t code = letterToCode(letter);
		int len = 0;
		int buffer = code;
		while (buffer > 0) {
			buffer /= 10;
			len++;
		}
		blinkLetterLength = len;
		blinkLetterIndex = 0;
	}
}

void handleNewLetter(){
	if (receivedLetter >= 97 && receivedLetter <= 122){
		bufferPut(receivedLetter);
	} else if (receivedLetter == '+'){
		toggleInterrupt(isEnabledInterrupt);
		isEnabledInterrupt = !isEnabledInterrupt;
	}
}

void resetReceivedLetter(){
	receivedLetter = 0;
	receiveStarted = false;
}

void handleReceiveIT(){
	transmitIT(&huart6, &receivedLetter);
	handleNewLetter();
	resetReceivedLetter();
	newLetter = false;
}

void startReceiveTransmitIT(){
	if (!receivedLetter && !receiveStarted){
		receiveIT(&huart6, &receivedLetter);
		receiveStarted = true;
	}
	if (transmitRequest && !transmitStarted){
		transmitIT(&huart6, &transmitLetter);
		transmitStarted = true;
	}
}

void startReceiveTransmit(){
	if (transmitRequest){
		if (transmitBlocking(&huart6, &transmitLetter)){
			handleTransmit();
		}
	}
	receivedLetter = receiveBlocking(&huart6);
	if (receivedLetter){
		transmitBlocking(&huart6, &receivedLetter);
		handleNewLetter();
		resetReceivedLetter();
	}
}

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
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */
  blinkCodeTimestamp = HAL_GetTick();
  blinkLetterTimestamp = HAL_GetTick();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  handleButtonClick();
	  if (isEnabledInterrupt){
		  startReceiveTransmitIT();
		  if (newLetter) handleReceiveIT();
	  } else {
		  startReceiveTransmit();
	  }

	  if (blinkLetterIndex == blinkLetterLength){
		  putNewLetter();
	  }

	  if (blinkLetterIndex < blinkLetterLength){
		  playLetterBlink();
	  }

	  if (blinkCodeIndex < inputCodeLength){
		  playCodeBlink();
	  }
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 15;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart == &huart6) {
        newLetter = true;
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart == &huart6) {
        handleTransmit();
    }
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
