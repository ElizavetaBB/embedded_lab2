/*
 * usart_driver.c
 *
 *  Created on: Nov 12, 2021
 *      Author: 1
 */

#include <usart_driver.h>

uint8_t receiveBlocking(UART_HandleTypeDef *huart){
	uint8_t buffer;
	if (HAL_UART_Receive(huart, &buffer, 1, 20) == HAL_OK){
		return buffer;
	}
	return 0;
}

uint8_t transmitBlocking(UART_HandleTypeDef *huart, uint8_t *letters){
	if (HAL_UART_Transmit(huart, letters, 1, 20) == HAL_OK){
		return 1;
	}
	return 0;
}

void receiveIT(UART_HandleTypeDef *huart, uint8_t *letters){
	HAL_UART_Receive_IT(huart, letters, 1);
}

void transmitIT(UART_HandleTypeDef *huart, uint8_t *letters){
	HAL_UART_Transmit_IT(huart, letters, 1);
}

void enableInterrupt(){
    HAL_NVIC_EnableIRQ(USART6_IRQn);
}

void disableInterrupt(){
    HAL_NVIC_DisableIRQ(USART6_IRQn);
}

void toggleInterrupt(bool enabledInterrupt){
	if (enabledInterrupt) disableInterrupt();
	else enableInterrupt();
}

