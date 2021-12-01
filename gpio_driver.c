/*
 * gpio_driver.c
 *
 *  Created on: Nov 29, 2021
 *      Author: boris
 */


#include "gpio_driver.h"

bool getButtonState() {
    if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15) == GPIO_PIN_SET) return 0;
    else return 1;
}

uint32_t getCurrentTime() {
    return HAL_GetTick();
}

uint32_t getTimeDifference(uint32_t start) {
    return HAL_GetTick() - start;
}

uint16_t getPinByColor(int color) {
    switch (color) {
        case 0:
            return GPIO_PIN_13;
        case 1:
            return GPIO_PIN_14;
        case 2:
            return GPIO_PIN_15;
        default:
            return GPIO_PIN_13;
    }
}

GPIO_PinState getDiodeState(int color) {
    uint16_t pin = getPinByColor(color);
    return HAL_GPIO_ReadPin(GPIOD, pin);
}

void turnDiodeOn(int color) {
    uint16_t pin = getPinByColor(color);
    HAL_GPIO_WritePin(GPIOD, pin, GPIO_PIN_SET);
}

void turnDiodeOff(int color) {
    uint16_t pin = getPinByColor(color);
    HAL_GPIO_WritePin(GPIOD, pin, GPIO_PIN_RESET);
}
