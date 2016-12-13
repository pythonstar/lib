//////////////////////////////////////////////////////////////////////////
/*
    Provided by ������, Northeastern University (www.neu.edu.cn)
    Email: blackdrn@sohu.com
	This product is free for use.
*/
//////////////////////////////////////////////////////////////////////////

#pragma once

namespace Star
{
	namespace Encrypt
	{
		namespace Des
		{
			enum	{ENCRYPT,DECRYPT};
			//enum    bool{false,true}; // if bool is not supported,use this or just replace with char
										// and use 1 for true,0 for false;
			//////////////////////////////////////////////////////////////////////////

			// Type��ENCRYPT:����,DECRYPT:����
			// ���������(Out)�ĳ��� >= ((datalen+7)/8)*8,����datalen�������8�ı�������С������
			// In ����= Out,��ʱ��/���ܺ󽫸������뻺����(In)������
			// ��keylen>8ʱϵͳ�Զ�ʹ��3��DES��/����,����ʹ�ñ�׼DES��/����.����16�ֽں�ֻȡǰ16�ֽ�
			int Des_Go(char *Out,char *In,long datalen,const char *Key,int keylen,bool Type = ENCRYPT);
		}
	}
}


//////////////////////////////////////////////////////////////////////////

/*------------------------------------------------------------------------
/����ʾ��:
char key[]={0,2,0,0,9,3,5,1,9,8,0,0,9,1,7},buf[255];
char str[]="W\0elcome to My 3-DES Test! --WangJunchuan\n"
"Northeastern University (www.neu.edu.cn)\n"
"Email: blackdrn@sohu.com";

memset(buf, 0, sizeof(buf));
strcpy(buf, str);
puts("\nBefore encrypting");
puts(buf);

Des_Go(buf, buf, sizeof(str), key, sizeof(key), ENCRYPT);
puts("\nAfter encrypting");
puts(buf);

Des_Go(buf, buf, sizeof(str), key, sizeof(key), DECRYPT);
puts("\nAfter decrypting");
puts(buf);
------------------------------------------------------------------------*/