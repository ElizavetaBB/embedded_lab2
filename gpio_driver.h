/*
 * driver.h
 *
 *  Created on: 2 окт. 2021 г.
 *      Author: 1
 */

#ifndef INC_GPIO_DRIVER_H_
#define INC_GPIO_DRIVER_H_

/*
 * driver.c
 *
 *  Created on: 28 сент. 2021 г.
 *      Author: 1
 */

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"

bool getButtonState();

void resetButton();

void resetAllDiodes();

uint32_t getTimeDifference(uint32_t time);

void delay(uint32_t ms);

uint16_t getDiode(uint8_t diode);

void turnOnDiode(uint8_t diode);

void turnOffDiode(uint8_t diode);

GPIO_PinState getDiodeState(uint8_t diode);

#endif /* INC_GPIO_DRIVER_H_ */
