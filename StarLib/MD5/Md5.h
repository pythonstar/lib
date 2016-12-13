/************************************************************
MD5校验和计算小程序（C）
  Author: butterinsect
  Email : yyongtai@gmail.com
  refer  : rsnn
 ************************************************************/
#pragma once

#include <afx.h>
#include<stdio.h>  

//MD5摘要值结构体
typedef struct MD5VAL_STRUCT
{
	unsigned int a;
	unsigned int b;
	unsigned int c;
	unsigned int d;
} MD5VAL;

//计算字符串的MD5值(若不指定长度则由函数计算)
MD5VAL md5(char * str, unsigned int size=0);

//MD5文件摘要
MD5VAL md5File(FILE * fpin);

//参数不可为：CString&file，因为会被改写，惨惨惨。
CString ValueMD5(CString file);

CString MD5Data(char * str, unsigned int size);
CString Md5File(const CString&strFileName);

