
	 /*µ÷ÓÃÊ¾Àý:
char szBuff[]="123456789";

Blowfish::InitKey((BYTE*)"12345",5);
Blowfish::BlowFishEncrypt((BYTE*)szBuff,9,true);
Blowfish::BlowFishEncrypt((BYTE*)szBuff,9,false);
*/

#include "stdafx.h"
#include "Blowfish.h"
using namespace Star::Encrypt;

int Blowfish::InitKey (unsigned char *const key, int keyLen)
{
	// return error code
	if(key == NULL) return BLOWFISH_NULL;
	if(keyLen < MINKEYBYTES) return BLOWFISH_INPUTTOOSHORT;
	if(keyLen > MAXKEYBYTES) return BLOWFISH_INPUTTOOLONG;

	// fill key_sbox from sbox
	for(int i = 0; i < 4; i++)
		for(int j = 0; j < 256; j++) key_sbox[i][j] = sbox[i][j];

	// fill key_pbox
	int				nIndex = 0;
	unsigned long	data = 0x00000000;
	for(int i = 0; i < N + 2; i++)
	{
		data = 0x00000000;
		for(int j = 0; j < 4; j++)
		{
			data = (data << 8) | key[nIndex];
			nIndex++;
			if(nIndex >= keyLen) nIndex = 0;
		}
		key_pbox[i] = pbox[i] ^ data;
	}
	unsigned long	xl = 0x00000000;
	unsigned long	xr = 0x00000000;

	for(int i = 0; i < N + 2; i += 2)
	{
		Encrypt(&xl, &xr);
		key_pbox[i] = xl;
		key_pbox[i + 1] = xr;
	}
	for(int i = 0; i < 4; ++i)
	{
		for(int j = 0; j < 256; j += 2)
		{
			Encrypt(&xl, &xr);
			key_sbox[i][j] = xl;
			key_sbox[i][j + 1] = xr;
		}
	}
	return BLOWFISH_SUCCESS;
}
unsigned long Blowfish::F(unsigned long x)
{
	return((key_sbox[0][(x >> 24) & 0xFF] + key_sbox[1][(x >> 16) & 0xFF]) ^ key_sbox[2][(x >> 8) & 0xFF]) + key_sbox[3][(x) & 0xFF];
}

int Blowfish::Encrypt(unsigned long *xl, unsigned long *xr)
{
	// return error code
	if(xl == NULL || xr == NULL) return BLOWFISH_NULL;

	unsigned long	Xl;
	unsigned long	Xr;
	unsigned long	temp;

	Xl = *xl;
	Xr = *xr;

	for(int i = 0; i < N; ++i)
	{
		Xl = Xl ^ key_pbox[i];
		Xr = F(Xl) ^ Xr;

		temp = Xl;
		Xl = Xr;
		Xr = temp;
	}

	temp = Xl;
	Xl = Xr;
	Xr = temp;

	Xr = Xr ^ key_pbox[N];
	Xl = Xl ^ key_pbox[N + 1];

	*xl = Xl;
	*xr = Xr;

	return BLOWFISH_SUCCESS;
}

int Blowfish::Decrypt(unsigned long *xl, unsigned long *xr)
{
	// return error code
	if(xl == NULL || xr == NULL) return BLOWFISH_NULL;

	unsigned long	Xl;
	unsigned long	Xr;
	unsigned long	temp;

	Xl = *xl;
	Xr = *xr;

	for(int i = N + 1; i > 1; --i)
	{
		Xl = Xl ^ key_pbox[i];
		Xr = F(Xl) ^ Xr;

		/* Exchange Xl and Xr */
		temp = Xl;
		Xl = Xr;
		Xr = temp;
	}

	/* Exchange Xl and Xr */
	temp = Xl;
	Xl = Xr;
	Xr = temp;

	Xr = Xr ^ key_pbox[1];
	Xl = Xl ^ key_pbox[0];

	*xl = Xl;
	*xr = Xr;

	return BLOWFISH_SUCCESS;
}

void Blowfish::BlowFishEncrypt(unsigned char *const pData, int DataLen, bool IsEncrypt)
{
	if(pData == NULL) return;

	unsigned long	*pLeft = NULL, *pRight = NULL;

	for(int i = 0; i < (int) (DataLen / 8); i++)
	{
		pLeft = (unsigned long *) (pData + 4 * (2 * i));
		pRight = (unsigned long *) (pData + 4 * (2 * i + 1));

		if(IsEncrypt)
			Blowfish::Encrypt(pLeft, pRight);
		else
			Blowfish::Decrypt(pLeft, pRight);
	}
}
