
#pragma once
#include <afx.h>

BYTE ByteSub(BYTE x);
void gentables(void);
void strtoHex(char *str,char *hex);
void hextoStr(char *hex,char *str);
void gkey(int nb,int nk,char *key);
void encrypt(char *buff);
void decrypt(char *buff);
