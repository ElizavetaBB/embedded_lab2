/*
 * gpio_driver.h
 *
 *  Created on: Nov 29, 2021
 *      Author: boris
 */

#ifndef INC_GPIO_DRIVER_H_
#define INC_GPIO_DRIVER_H_

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"

bool getButtonState();

uint32_t getCurrentTime();

uint32_t getTimeDifference(uint32_t start);

GPIO_PinState getDiodeState(int diode);

void turnDiodeOn(int color);

void turnDiodeOff(int color);

#endif /* INC_GPIO_DRIVER_H_ */
