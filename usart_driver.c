/*
 * usart_driver.c
 *
 *  Created on: Nov 29, 2021
 *      Author: boris
 */

#include "usart_driver.h"

uint8_t receive(struct __UART_HandleTypeDef *huart) {
    uint8_t received_char;
    if (HAL_UART_Receive(huart, &received_char, 1, 20) == HAL_OK) {
        return received_char;
    }
    return 0;
}

char transmit(struct __UART_HandleTypeDef *huart, uint8_t *letter) {
    if (HAL_UART_Transmit(huart, letter, 1, 20) == HAL_OK) {
        return 1;
    }
    return 0;
}

void receiveIT(struct __UART_HandleTypeDef *huart, uint8_t *letter) {
    HAL_UART_Receive_IT(huart, letter, 1);
}

void transmitIT(struct __UART_HandleTypeDef *huart, uint8_t *letter) {
    HAL_UART_Transmit_IT(huart, letter, 1);
}

void enableInterrupt() {
    HAL_NVIC_EnableIRQ(USART6_IRQn);
}

void disableInterrupt() {
    HAL_NVIC_DisableIRQ(USART6_IRQn);
}

