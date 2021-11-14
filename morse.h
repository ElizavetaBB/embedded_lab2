/*
 * morse.h
 *
 *  Created on: Nov 12, 2021
 *      Author: 1
 */

#ifndef INC_MORSE_H_
#define INC_MORSE_H_

#include <stdint.h>

uint8_t letterToCode(char letter);

char codeToLetter(uint8_t code);

#endif /* INC_MORSE_H_ */
