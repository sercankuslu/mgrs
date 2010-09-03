#ifndef __STRING2EI_H
#define __STRING2EI_H

#include "Compiler.h"
#include "HardwareProfile.h"

void str2ram(static char *dest, static char rom *src);
signed int strcmp1(char *s1, char *s2);

//Transfoma um número hexadecimalem uma stringo no formato inteiro.
void STRtoINT(unsigned char e, char* g);
// Transforma um número hexadecimal(ab) para uma string no formato inteiro (cde) dividido por 2.
void STRtoINT2(unsigned char e, char* g);
// Transforma um número hexadecimal(ef) em uma string inteira dividida por 2.
void STRtoINT4(unsigned char e,unsigned char f, char* i);
//Transfoma um número hexadecimal em uma string no formato decimal multiplicada por 2
void STRtoINT2X(unsigned char e, char* g);
unsigned char STRING_to_int (unsigned char *str);

#endif