/*
 * usart_driver.h
 *
 *  Created on: Nov 12, 2021
 *      Author: 1
 */

#ifndef INC_USART_DRIVER_H_
#define INC_USART_DRIVER_H_

#include "stm32f4xx_hal.h"
#include <stdbool.h>

uint8_t receiveBlocking(UART_HandleTypeDef *huart);

uint8_t transmitBlocking(UART_HandleTypeDef *huart, uint8_t *letters);

void receiveIT(UART_HandleTypeDef *huart, uint8_t *letters);

void transmitIT(UART_HandleTypeDef *huart, uint8_t *letters);

void enableInterrupt();

void disableInterrupt();

void toggleInterrupt(bool enabledInterrupt);

#endif /* INC_USART_DRIVER_H_ */
