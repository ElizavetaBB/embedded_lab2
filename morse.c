/*
 * morse.c
 *
 *  Created on: Nov 29, 2021
 *      Author: boris
 */

#include "morse.h"

//a = 97 ... z = 122
char letters[26] = {'a', 'b', 'c', 'd', 'e', 'f',
					'g', 'h', 'i', 'j', 'k', 'l',
					'm', 'n', 'o', 'p', 'q', 'r',
					's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};

//точка - 1, тире -2
int codes[26] ={12, 2111, 2121, 211, 1, 1121,
				221, 1111, 11, 1222, 212, 1211,
				22, 21, 222, 1221, 2212, 121,
				111, 2, 112, 1112, 122, 2112, 2122, 2211};

char codeToLetter(int code) {
    for (int i = 0; i < 26; i++) {
        if (codes[i] == code) {
            return letters[i];
        }
    }
    return 0;
}

int letterToCode(char letter) {
	if (letter < 97 || letter > 122) return 0;
	return codes[letter - 97];
}
