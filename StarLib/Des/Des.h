//////////////////////////////////////////////////////////////////////////
/*
    Provided by 王俊川, Northeastern University (www.neu.edu.cn)
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

			// Type—ENCRYPT:加密,DECRYPT:解密
			// 输出缓冲区(Out)的长度 >= ((datalen+7)/8)*8,即比datalen大的且是8的倍数的最小正整数
			// In 可以= Out,此时加/解密后将覆盖输入缓冲区(In)的内容
			// 当keylen>8时系统自动使用3次DES加/解密,否则使用标准DES加/解密.超过16字节后只取前16字节
			int Des_Go(char *Out,char *In,long datalen,const char *Key,int keylen,bool Type = ENCRYPT);
		}
	}
}


//////////////////////////////////////////////////////////////////////////

/*------------------------------------------------------------------------
/调用示例:
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