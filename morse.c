/*
 * morse.c
 *
 *  Created on: Nov 12, 2021
 *      Author: 1
 */

#include <morse.h>

char letters[26] = {'a', 'b', 'c', 'd', 'e', 'f', 'g',
					'h', 'i', 'j', 'k', 'l', 'm', 'n',
					'o', 'p', 'q', 'r', 's', 't', 'u',
					'v', 'w', 'x', 'y', 'z'};

uint8_t codes[26] = {12, 2111, 2121, 211, 1, 1121, 221,
						1111, 11, 1222, 212, 1211, 22, 21,
						222, 1221, 2212, 121, 111, 2, 112,
						1112, 122, 2112, 2122, 2211};

uint8_t letterToCode(char letter){
	if (letter < 97 || letter > 122) return -1;
	return letters[letter-97];
}

char codeToLetter(uint8_t code){
	for (int i = 0; i < 26; i++){
		if (codes[i] == code) return letters[i];
	}
	return -1;
}

