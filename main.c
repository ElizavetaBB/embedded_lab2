/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "gpio_driver.h"
#include "morse.h"
#include "usart_driver.h"
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
bool previousButtonState = false;
uint32_t pressTimestamp;
const uint32_t longPressTime = 500;
int buttonCode = 0; // последовательность, набранная кнопкой
char buttonCodeLength = 0; // длина последовательности
const uint32_t endTimeDuration = 5000; // время, спустя которое выведется последовательность с кнопки

char blinkCodeIndex = 0; // текущий индекс на моргание последовательности с кнопки
uint32_t blinkCodeTimestamp;
const uint32_t shortBlinkDelay = 500;
const uint32_t longBlinkDelay = 1000;

uint8_t transmitLetter = 0; // символ для передачи
bool transmitRequest = false; // запрос на передачу

bool interruptsEnabled = false; // флаг разрешения прерывания

uint8_t buffer[8]; // буфер символов, введенных с клавиатуры
char putIndex = 0; // индекс добавления символа в буфер
bool isBufferFull = false;
bool isBufferEmpty = true;
char bufferCounter = 0;

bool receiveStarted = false;
uint8_t receivedLetter = 0;
bool newLetterReceived = false;
bool receiveITInitialized = false;

char blinkLetterIndex; // текущий индекс на моргание символа с клавиатуры
int blinkLetter; // символ на моргание
uint8_t blinkLetterLength; // длина текущего символа на моргание
uint32_t blinkLetterTimestamp;
bool bufferLetterAvailable = true; // флаг доступности вывода следующего символа с клавиатуры на светодиоды

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void handleButtonClick() {
	bool buttonState = getButtonState();
	    if (buttonState) { // если нажата
	        if (!previousButtonState) { // если до этого не была нажата
	            pressTimestamp = getCurrentTime(); // фиксируем время начала нажатия
	        }
	    } else { // не нажата
	    	if (previousButtonState){ // если до этого была нажата
	    		if (buttonCodeLength >= 4){ // максимальная длина кода морзе равна 4
	    			transmitLetter = '?';
	    			transmitRequest = true;
	    			buttonCode = 0;
	    			buttonCodeLength = 0;
	    			blinkCodeIndex = 0;
	    		} else {
	    			uint32_t pressDuration = getTimeDifference(pressTimestamp); // длительность нажатия
	    			if (pressDuration >= longPressTime){
	    				buttonCode = buttonCode * 10 + 2;
	    				buttonCodeLength++; // длина последовательности
	    			} else if (pressDuration > 0){
	    				buttonCode = buttonCode * 10 + 1;
	    				buttonCodeLength++;
	    			}
	    		}
	    		pressTimestamp = getCurrentTime();
	    	} else {
	    		uint32_t noPressDuration = getTimeDifference(pressTimestamp); // время без нажатия
	    		if (noPressDuration >= endTimeDuration && buttonCodeLength > 0){
	    			// длина последовательности для вывода обязана быть ненулевой
	    			char letter = codeToLetter(buttonCode);
	    			if (letter){ // если последовательность является символом
	    				transmitLetter = letter;
	    			} else {
	    				transmitLetter = '?';
	    			}
    				transmitRequest = true;
	    			buttonCode = 0;
	    			buttonCodeLength = 0;
	    			blinkCodeIndex = 0;
	    		}
	    	}
	    }
	    previousButtonState = buttonState;
}

// вывод последовательности с кнопки
void blinkButtonCode(){
	uint32_t duration = getTimeDifference(blinkCodeTimestamp);
	if (duration >= shortBlinkDelay){
		int code = buttonCode;
		int pow = buttonCodeLength - blinkCodeIndex - 1;
		for (int i = 0; i < pow; i++){
			code /= 10;
		}
		code %= 10;
		int diodeColor = code;
		if (getDiodeState(diodeColor)){ // если диод уже включен, отключаем и считаем его выведенным
			turnDiodeOff(diodeColor);
			blinkCodeIndex++;
		} else {
			turnDiodeOn(diodeColor);
		}
		blinkCodeTimestamp = getCurrentTime();
	}
}

// осуществляется блокирующая передача по запросу
void transmitBlocking(){
	if (transmit(&huart6, &transmitLetter)){
		transmitRequest = false;
		transmitLetter = 0;
	}
}

// добавление символа с клавиатуры в незаполенный буфер
void bufferPut(uint8_t letter){
	if (!isBufferFull){
		buffer[putIndex] = letter;
		putIndex = (putIndex + 1) % 8;
		bufferCounter++;
		if (bufferCounter == 8) isBufferFull = true;
		isBufferEmpty = false;
	}
}

// получение символа из буфера только в том случае, если он не пустой
uint8_t bufferGet(){
	if (!isBufferEmpty){
		uint8_t letter = buffer[(8 - bufferCounter + putIndex) % 8];
		bufferCounter--;
		isBufferFull = false;
		if (bufferCounter == 0) isBufferEmpty = true;
		return letter;
	}
	return 0;
}

// определение введенного с клавиатуры символа и соответствующей ему функции
void serviceNewLetter(uint8_t letter){
	if (letter >= 'a' && letter <= 'z'){
		bufferPut(letter);
	} else if (letter == '+') { // переключение режима
		interruptsEnabled = !interruptsEnabled;
		receiveITInitialized = false;
		if (interruptsEnabled) {
			enableInterrupt();
		}
		else {
			disableInterrupt();
		}
	}
}

// обработка нового символа с клавиатуры для неблокирующего режима
void finishReceiveIT(){
	transmitIT(&huart6, &receivedLetter);
	serviceNewLetter(receivedLetter);
	receivedLetter = 0;
	receiveStarted = false;
	newLetterReceived = false;
}

// блокирующее чтение символа
void receiveBlocking(){
	uint8_t receivedLetter = receive(&huart6);
	if (receivedLetter){
		transmit(&huart6, &receivedLetter);
		serviceNewLetter(receivedLetter);
	}
}

// вывод на зеленый светодиод символа с клавиатуры
void blinkBufferedLetter(){
	int letter = blinkLetter;
	int pow = blinkLetterLength - blinkLetterIndex - 1;
	for (int i = 0; i < pow; i++){
		letter /= 10;
	}
	letter %= 10;
	int diodeColor = 0;
	uint32_t delay = letter == 1? shortBlinkDelay: longBlinkDelay;
	uint32_t duration = getTimeDifference(blinkLetterTimestamp);
	if (duration >= delay){
		if (getDiodeState(diodeColor)){
			turnDiodeOff(diodeColor);
			blinkLetterIndex++;
			if (blinkLetterIndex == blinkLetterLength) {
				blinkLetter = 0;
				blinkLetterLength = 0;
				blinkLetterIndex = 0;
				bufferLetterAvailable = true; // разрешение моргания следующего символа
			}
		} else {
			turnDiodeOn(diodeColor);
		}
		blinkLetterTimestamp = getCurrentTime();
	}
}

// добавление символа с клавиатуры в буфер моргания
void putLetterToBlinkBuffer(){
	if (!isBufferEmpty){
		uint8_t letter = bufferGet();
		blinkLetter = letterToCode(letter);
		int buffer = blinkLetter;
		while (buffer > 0){
			buffer /= 10;
			blinkLetterLength++;
		}
		blinkLetterIndex = 0;
		bufferLetterAvailable = false;
	}
}

// неблокирующее чтение
void receiveNonBlocking(){
	if (!receiveStarted && !receivedLetter){
		receiveIT(&huart6, &receivedLetter);
		receiveStarted = true;
	}
}

// неблокирующая передача
void transmitNonBlocking(){
	transmitIT(&huart6, &transmitLetter);
	transmitRequest = false;
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
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1)
      {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

        handleButtonClick(); //получение последовательности с кнопки
        if (blinkCodeIndex < buttonCodeLength){ // если не все символы с кнопки были отображены на светодиодах
        	blinkButtonCode(); // вывод следующего нажатия
        }
        if (!interruptsEnabled){ // прерывания запрещены
        	if (transmitRequest) transmitBlocking();
        	receiveBlocking();
        } else {
        	receiveNonBlocking();
        	if (transmitRequest) transmitNonBlocking();
        	if (newLetterReceived) finishReceiveIT(); // вызов обработки принятого символа
        }
        if (bufferLetterAvailable){ // если буфер символа с клавиатуры свободен
        	putLetterToBlinkBuffer(); // если есть новый символ с клавиатуры, добавляем его в буфер
        } else {
        	blinkBufferedLetter(); // выводим символ из буфера на светодиод
        }
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
    	newLetterReceived = true;
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart == &huart6) {
    	transmitRequest = false;
    	transmitLetter = 0;
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

