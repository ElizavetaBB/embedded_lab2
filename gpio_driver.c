/*
 * driver.c
 *
 *  Created on: 28 сент. 2021 г.
 *      Author: 1
 */

#include <gpio_driver.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"

bool getButtonState(){
	if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15) == GPIO_PIN_SET) return 0;
	else return 1;
}

void resetButton(){
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_RESET);
}

void resetAllDiodes(){
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
}

uint32_t getTimeDifference(uint32_t time){
	return HAL_GetTick()-time;
}

void delay(uint32_t ms){
	HAL_Delay(ms);
}

uint16_t getDiode(uint8_t diode){
	if (diode == 0) {
		return GPIO_PIN_13;
	} else if (diode == 1){
		return GPIO_PIN_14;
	} else if (diode == 2){
		return GPIO_PIN_15;
	} else return GPIO_PIN_13;
}

void turnOnDiode(uint8_t diode){
	resetAllDiodes();
	uint16_t pin = getDiode(diode);
	HAL_GPIO_WritePin(GPIOD, pin, GPIO_PIN_SET);
}

void turnOffDiode(uint8_t diode){
	uint16_t pin = getDiode(diode);
	HAL_GPIO_WritePin(GPIOD, pin, GPIO_PIN_RESET);
}

GPIO_PinState getDiodeState(uint8_t diode) {
	uint16_t pin = getDiode(diode);
    return HAL_GPIO_ReadPin(GPIOD, pin);
}


