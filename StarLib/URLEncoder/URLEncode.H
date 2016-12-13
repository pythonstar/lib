/*****************************************************************************
Module :     URLEncode.H
Notices:     Written 2002 by ChandraSekar Vuppalapati
Description: H URL Encoder
*****************************************************************************/
#pragma once
#include <afx.h>

class CURLEncode
{
private:
static CString csUnsafeString;
CString decToHex(char num, int radix);
bool isUnsafe(char compareChar);
CString convert(char val);

public:
CURLEncode() { };
virtual ~CURLEncode() { };
CString URLEncode(CString vData);
};
