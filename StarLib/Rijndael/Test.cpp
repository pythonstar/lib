
//Test.cpp

#include "Rijndael.h"
#include <iostream>

using namespace std;

//Function to convert unsigned char to string of length 2
void Char2Hex(unsigned char ch, char* szHex)
{
	unsigned char byte[2];
	byte[0] = ch/16;
	byte[1] = ch%16;
	for(int i=0; i<2; i++)
	{
		if(byte[i] >= 0 && byte[i] <= 9)
			szHex[i] = '0' + byte[i];
		else
			szHex[i] = 'A' + byte[i] - 10;
	}
	szHex[2] = 0;
}

//Function to convert string of length 2 to unsigned char
void Hex2Char(char const* szHex, unsigned char& rch)
{
	rch = 0;
	for(int i=0; i<2; i++)
	{
		if(*(szHex + i) >='0' && *(szHex + i) <= '9')
			rch = (rch << 4) + (*(szHex + i) - '0');
		else if(*(szHex + i) >='A' && *(szHex + i) <= 'F')
			rch = (rch << 4) + (*(szHex + i) - 'A' + 10);
		else
			break;
	}
}    

//Function to convert string of unsigned chars to string of chars
void CharStr2HexStr(unsigned char const* pucCharStr, char* pszHexStr, int iSize)
{
	int i;
	char szHex[3];
	pszHexStr[0] = 0;
	for(i=0; i<iSize; i++)
	{
		Char2Hex(pucCharStr[i], szHex);
		strcat(pszHexStr, szHex);
	}
}

//Function to convert string of chars to string of unsigned chars
void HexStr2CharStr(char const* pszHexStr, unsigned char* pucCharStr, int iSize)
{
	int i;
	unsigned char ch;
	for(i=0; i<iSize; i++)
	{
		Hex2Char(pszHexStr+2*i, ch);
		pucCharStr[i] = ch;
	}
}

void test1();
void test2();

void main()
{
	test1();
	test2();
	getchar();
}

/*------------------------------------------------------------------------
[7/29/2009 zhuxingxing]
说明:测试EncryptBlock,DecryptBlock.加解密的数据长度为MakeKey中设置的块大小.
------------------------------------------------------------------------*/
void test1()
{
	try
	{
		char szHex[33];

		//Initialization

		CRijndael oRijndael;
		oRijndael.MakeKey("abcdefghabcdefgh", CRijndael::sm_chain0, 16, 16);

		char szDataIn[] = "aaaaaaaabbbbbbbb";
		char szDataOut[17] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

		//Encryption

		oRijndael.EncryptBlock(szDataIn, szDataOut);

		CharStr2HexStr((unsigned char*)szDataIn, szHex, 16);
		cout << szHex << endl;
		CharStr2HexStr((unsigned char*)szDataOut, szHex, 16);
		cout << szHex << endl;
		memset(szDataIn, 0, 16);

		//Decryption

		oRijndael.DecryptBlock(szDataOut, szDataIn);

		CharStr2HexStr((unsigned char*)szDataIn, szHex, 16);
		cout << szHex << endl;
	}
	catch(exception& roException)
	{
		cout << roException.what() << endl;
	}
}

/*------------------------------------------------------------------------
[7/29/2009 zhuxingxing]
说明:测试Encrypt,Decrypt.数据块长度应是MakeKey中设置的块大小的整数倍,如块大小设置的是
16,则加解密数据的长度应是16的整数倍.

注意:使用CRijndael::CBC模式和CRijndael::CFB模式需要调用ResetChain,
而CRijndael::ECB模式则不需要.
------------------------------------------------------------------------*/
void test2()
{
	try
	{
		CRijndael oRijndael;
		oRijndael.MakeKey("1234567890123456", CRijndael::sm_chain0, 16, 16);
		char szDataIn1[49] = "ababababccccccccababababccccccccababababcccccccc";
		char szDataIn[49];
		char szDataOut[49];
		memset(szDataIn, 0, 49);
		memset(szDataOut, 0, 49);

		//Test ECB

		strcpy(szDataIn, szDataIn1);
		memset(szDataOut, 0, 49);
		oRijndael.Encrypt(szDataIn, szDataOut, 48, CRijndael::ECB);
		memset(szDataIn, 0, 49);
		oRijndael.Decrypt(szDataOut, szDataIn, 48, CRijndael::ECB);

		//Test CBC

		oRijndael.ResetChain();
		strcpy(szDataIn, szDataIn1);
		memset(szDataOut, 0, 49);
		oRijndael.Encrypt(szDataIn, szDataOut, 48, CRijndael::CBC);
		memset(szDataIn, 0, 49);
		oRijndael.ResetChain();
		oRijndael.Decrypt(szDataOut, szDataIn, 48, CRijndael::CBC);

		//Test CFB

		oRijndael.ResetChain();
		strcpy(szDataIn, szDataIn1);
		memset(szDataOut, 0, 49);
		oRijndael.Encrypt(szDataIn, szDataOut, 48, CRijndael::CFB);
		memset(szDataIn, 0, 49);
		oRijndael.ResetChain();
		oRijndael.Decrypt(szDataOut, szDataIn, 48, CRijndael::CFB);
	}
	catch(exception& roException)
	{
		cout << "Exception: " << roException.what() << endl;
	}
}
