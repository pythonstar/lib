/************************************************************
MD5У��ͼ���С����C��
  Author: butterinsect
  Email : yyongtai@gmail.com
  refer  : rsnn
 ************************************************************/
#pragma once

#include <afx.h>
#include<stdio.h>  

//MD5ժҪֵ�ṹ��
typedef struct MD5VAL_STRUCT
{
	unsigned int a;
	unsigned int b;
	unsigned int c;
	unsigned int d;
} MD5VAL;

//�����ַ�����MD5ֵ(����ָ���������ɺ�������)
MD5VAL md5(char * str, unsigned int size=0);

//MD5�ļ�ժҪ
MD5VAL md5File(FILE * fpin);

//��������Ϊ��CString&file����Ϊ�ᱻ��д���ҲҲҡ�
CString ValueMD5(CString file);

CString MD5Data(char * str, unsigned int size);
CString Md5File(const CString&strFileName);

