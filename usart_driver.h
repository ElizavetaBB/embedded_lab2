/*
 * usart_druver.h
 *
 *  Created on: Nov 29, 2021
 *      Author: boris
 */

#ifndef INC_USART_DRIVER_H_
#define INC_USART_DRIVER_H_

#include "stm32f4xx_hal.h"
#include <stdbool.h>

uint8_t receive(struct __UART_HandleTypeDef *huart);

char transmit(struct __UART_HandleTypeDef *huart, uint8_t *letter);

void receiveIT(struct __UART_HandleTypeDef *huart, uint8_t *letter);

void transmitIT(struct __UART_HandleTypeDef *huart, uint8_t *letter);

void enableInterrupt();

void disableInterrupt();

#endif /* INC_USART_DRIVER_H_ */
