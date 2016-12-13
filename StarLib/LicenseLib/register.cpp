#include "register.h"
#include <stdio.h>
#include <windows.h>
#include <assert.h>
#include "AntiDebug.h"

extern "C" {
	#include "md5.h"
	#include "rc4.h"
	#include "des.h"
	#include "libcpuid\libcpuid.h"
}

// ȡ�ùؼ�key
extern bool get_luacode(MachineCode* mc, unsigned int& code1, unsigned int& code2);
extern bool get_luacode_c(MachineCode* mc, unsigned int& code1, unsigned int& code2);
extern int get_licensecode_core(const char*prefix, MachineCode* mc, LicenseCode* lc, int flag);
void hex_to_bin(const char* in, char* out);
void hex_to_bin(const wchar_t* in, char* out);



// ȡ��Ӳ�̱��
extern int ReadPhysicalDriveInNTUsingSmart (char* diskid, int len);


struct machine_flag {
	short country;		// ����
	char diskid[18];	// Ӳ�̱�־
	char vendor;		
	char family;
	char model;
	char stepping;
};

struct license_flag {
	char ver;
	char v1;
	char v2;
	char v3;
	ULONG key[4];
	DWORD crc;
};

int get_machinecode(const char*prefix,MachineCode* mc)
{
	machine_flag mf;
	memset(mc->buffer, 0, sizeof(mc->buffer));
	memset(&mf, 0, sizeof(machine_flag));

	// �õ�����ID
	LCID lcid = GetSystemDefaultLCID();
	mf.country = LOWORD(lcid);
	
	// Ӳ�̱��
	// ע��ײ�д��20���ַ��������������ֻ����ǰ18������һ����\0;
	// ���2��byte������vendor��family����
	char diskid[1024] = {0};
	ReadPhysicalDriveInNTUsingSmart(diskid, 18);
	memcpy(mf.diskid, diskid, 18);

	// CPU���
	struct cpu_id_t data;
	int ret = cpu_identify(0, &data);
	if (ret<0)
	{
		mf.vendor = -1;
		mf.family = -1;
		mf.model = -1;
		mf.stepping = ret;
	}
	else
	{
		mf.vendor = (char)data.vendor;
		mf.family = (char)data.family;
		mf.model = (char)data.model;
		mf.stepping = (char)data.stepping;
	}

	// ���������ַ���rc4����һ��
	AVRC4 rc4;
	av_rc4_init(&rc4, (uint8_t*)prefix, 8*8, 0);
	av_rc4_crypt(&rc4, (uint8_t*)mc, (const uint8_t*)&mf, sizeof(mf), NULL, 0);

	//int len = strlen(ok_buf);
	//machine_flag mf2;
	//char ok2[1024];
	//av_rc4_init(&rc4, (uint8_t*)"xilisoft", 8*8, 1);
	//av_rc4_crypt(&rc4, (uint8_t*)&mf2, (const uint8_t*)ok_buf, sizeof(mf2), NULL, 1);

	return 0;
}


int str_to_bin(char* str, void* bin, int bin_len)
{
	int idx = 0;
	const char* p = str;
	while (*p) 
	{
		hex_to_bin(p, &((char*)bin)[idx]);
		if ((p-1)>str&&(*(p-1)==0)) {
			break;
		}
		p += 2;

		// ������嵽ͷ�ˣ�Ҳ��������
		if (++idx>=bin_len) {
			break;
		}
	}
	return idx;
}

int unicode_to_bin(wchar_t* str, void* bin, int bin_len)
{
	int idx = 0;
	const wchar_t* p = str;
	while (*p) 
	{
		hex_to_bin(p, &((char*)bin)[idx]);
		if ((p-1)>str&&(*(p-1)==0)) {
			break;
		}
		p += 2;

		// ������嵽ͷ�ˣ�Ҳ��������
		if (++idx>=bin_len) {
			break;
		}
	}
	return idx;
}

int bin_to_str(void* bin, int bin_len, char* str, int str_len)
{
	if (bin_len*2+1 >= str_len) {
		return -1;
	}

	// ��24��charת�����ַ���
	char* p = str;
	for (int i=0;i<bin_len;++i) {
		p += sprintf(p, "%02X", *(unsigned char*)((char*)bin+i));
	}
	return (p-str);
}

int bin_to_unicode(void* bin, int bin_len, wchar_t* str, int str_len)
{
	if (bin_len*2+1 >= str_len) {
		return -1;
	}

	// ��24��charת�����ַ���
	wchar_t* p = str;
	for (int i=0;i<bin_len&&*((char*)bin+i);++i) {
		p += wsprintfW(p, L"%02X", *(unsigned char*)((char*)bin+i));
	}
	return (p-str);
}


int get_licensecode(const char* name, MachineCode* mc, LicenseCode* lc)
{
	return get_licensecode_core(name, mc, lc, 0);
}


int test_licensecode(const char*prefix, MachineCode* mc, LicenseCode* lc)
{
	MachineCode tempcode;

	// ע�����Ʋ��ܴ���24�ַ�
	int name_len = strlen(prefix);

	JUNK_CODE_ONE
	JUNK_CODE_TWO2
	JUNK_CODE_ONE
	if (name_len>24) {
		return -1;
	}
	JUNK_CODE_TWO2
	JUNK_CODE_ONE
	JUNK_CODE_TWO2
	JUNK_CODE_ONE
	if (name_len<=0) {
		return -1;
	}
	JUNK_CODE_TWO2
	JUNK_CODE_ONE
	if (mc==0) {
		get_machinecode(prefix,&tempcode);
	}
	JUNK_CODE_ONE

	// �����û���
	char test_key[24] = {0};
	strncpy(test_key, prefix, 24);
	for ( int i=0; i<name_len; ++i ){
		test_key[i]^=0x85;
	}

	JUNK_CODE_TWO2

	license_flag lf;
	memcpy(&lf, lc, sizeof(license_flag));

	// ���ſ�-1�����ã�
	license_flag lf2;
	memcpy(&lf2, lc, sizeof(license_flag));
	AVRC4 rc4_2;
	const char* GHOFFICE = "soft";	//�����õģ����ȡ���ַ���
	av_rc4_init(&rc4_2, (uint8_t*)GHOFFICE, strlen(GHOFFICE)*8, 1);
	JUNK_CODE_TWO2
	av_rc4_crypt(&rc4_2, (uint8_t*)&lf2, (const uint8_t*)lc, sizeof(LicenseCode), NULL, 1);

	JUNK_CODE_TWO2
	JUNK_CODE_TWO2

	// ���ſ�-2�����ã�
	license_flag lf3;
	AVRC4 rc4_3;
	av_rc4_init(&rc4_3, (uint8_t*)&lf.crc, 4*8, 1);
	av_rc4_crypt(&rc4_3, (uint8_t*)&lf3, (const uint8_t*)lc, sizeof(LicenseCode), NULL, 1);
	JUNK_CODE_FOUR

	// ����������Ϊ0��������һ����������
	lf.crc = lf2.crc + lf3.crc;
	if (lf.crc==0) {
		return -1;
	}
	
	// ���Ƚ���ԭʼ��������
	AVDES des;
    av_des_init(&des, (uint8_t*)test_key, 192, 1);
    av_des_crypt(&des, (uint8_t*)&lf, (const uint8_t*)lc, 3, NULL, 1);

	JUNK_CODE_TWO
	// Ŀǰֻ���ܵ�һ�汾��ע��
	if (lf.ver!=KEYGEN_VERSION) {
		return -1;
	}

	JUNK_CODE_TWO
	// ���ſ�-3�����ã�
	if (rc4_2.x==0) {
		return -1;
	}

	JUNK_CODE_TWO2
	if (rc4_3.x==0) {
		return -1;
	}

	// ���ſ�-4�����ã�
	JUNK_CODE_TWO3
	unsigned int x = lf.crc;
	x = (x & 0x55555555UL) + ((x >> 1) & 0x55555555UL);
	JUNK_CODE_TWO3
	x = (x & 0x33333333UL) + ((x >> 2) & 0x33333333UL);
	JUNK_CODE_TWO3
	x = (x & 0x0f0f0f0fUL) + ((x >> 4) & 0x0f0f0f0fUL);
	if (mc==0) {
		mc = &tempcode;
	}
	JUNK_CODE_TWO3
	x = (x & 0x00ff00ffUL) + ((x >> 8) & 0x00ff00ffUL);
	JUNK_CODE_TWO3
	x = (x & 0x0000ffffUL) + ((x >> 16) & 0x0000ffffUL);
	JUNK_CODE_TWO3

	// v1�����λ����Ϊ1��v2�����λ����Ϊ1��v3����Ҫ�ܱ�7������С��500
	if (lf.v1&0x80||lf.v2&0x01||lf.v3%7!=0&&lf.v3>500) {
		return -1;
	}

	JUNK_CODE_ONE
	JUNK_CODE_ONE
	JUNK_CODE_ONE
	// ���ţ�Ҳ�ǲ��������
	if (x==lf.key[2]||x==0) {
		return -1;
	}

	// �����Щ����ȷ�ˣ���û�취�ˣ�ֻ������һ��ԭʼע����
	// ��õķ�������rar���ܵķ���������Ŀǰ��ô�������о�
	// Ŀǰ�ķ�����������һ������Է�ֻҪget_licensecode����
	LicenseCode new_code = {0};
	get_licensecode_core(prefix, mc, &new_code, lf.v3);

	////////////////////////////////////
	// junk code -  �����Ⱦ��˳�
	LicenseCode new_code2 = {0};
	get_licensecode_core(prefix, (MachineCode*)&lf3, &new_code2, lf.crc&0xe8);
	if (!_memicmp(&new_code, &new_code2, sizeof(LicenseCode))) {
		return -1;
	}
	////////////////////////////////////

	JUNK_CODE_TWO3
	JUNK_CODE_ONE
	JUNK_CODE_ONE
	JUNK_CODE_ONE
	if (_memicmp(&new_code, lc, sizeof(LicenseCode))) {
		JUNK_CODE_ONE
		JUNK_CODE_ONE
		return -1;
	}

	JUNK_CODE_TWO2

	return 0;
}


int get_licensecode_core(const char*prefix, MachineCode* mc, LicenseCode* lc, int flag)
{
	// �û������ܴ���24���ַ�����192bit��des��֧��64��192bit����
	int name_len = strlen(prefix);
	if (name_len>24) {
		return -1;
	}

	license_flag lf;
	memset(&lf, 0, sizeof(license_flag));
	assert(sizeof(LicenseCode)==sizeof(license_flag));

	JUNK_CODE_TWO
	__asm { mov lf.ver, 8 }
	JUNK_CODE_TWO

	// Ŀǰ�汾�̶�Ϊ1
	lf.ver = KEYGEN_VERSION;

	// C++ ���б���MD5����õ����õ�ֵ
	av_md5_sum((uint8_t*)lf.key, (uint8_t*)&mc->buffer, sizeof(mc->buffer));

	JUNK_CODE_ONE
	JUNK_CODE_ONE

	// ��lua�����еõ�����ֵ
	unsigned int code1 = 0;
	unsigned int code2 = 0;
	get_luacode(mc, code1, code2);

#ifdef _DEBUG
	unsigned int code1c = 0;
	unsigned int code2c = 0;
	get_luacode_c(mc, code1c, code2c);
	assert(code1==code1c);
	assert(code2==code2c);
#endif

	lf.crc = code1;

	// ���һ���ܱ�7~500֮��������7����������ÿ�ν������䶯�ģ�Ȼ�����
	if (flag==0) {
		JUNK_CODE_THREE
		JUNK_CODE_FIVE
		lf.v3 = (rand() % 493 + 7) / 7;
		JUNK_CODE_ONE
	} else {
		JUNK_CODE_FOUR
		JUNK_CODE_FIVE
		lf.v3 = flag;
		JUNK_CODE_ONE
	}

	JUNK_CODE_THREE
	JUNK_CODE_ONE
	lf.v2 = code2 & 0xfe;
	lf.v1 = (code2 >> 16) & 0x7f;

	JUNK_CODE_ONE
	JUNK_CODE_THREE

	// �������������ַ������û�������һ��
	//AVRC4 rc4;
	//av_rc4_init(&rc4, (uint8_t*)name, strlen(name)*8, 0);
	//av_rc4_crypt(&rc4, (uint8_t*)lc, (const uint8_t*)&lf, sizeof(lf), NULL, 0);

	// ʹ��DES���м��ܼ���
	char test_key[24] = {0};
	strncpy(test_key, prefix, 24);
	for ( int i=0; i<name_len; ++i ){
		test_key[i]^=0x85;
	}


	AVDES des;
    av_des_init(&des, (uint8_t*)test_key, 192, 0);
    av_des_crypt(&des, (uint8_t*)lc, (const uint8_t*)&lf, 3, NULL, 0);

	return 0;
}

//extern "C" int luac_main(int argc, char* argv[]);
//int encode_lua(const char* lua_string, char* buffer, int len)
//{
//	int argc=3;
//	char* argv[] = {
//		"luac",
//		"-s",
//		"c:\\test.lua",
//	};
//
//
//	luac_main(argc, argv);
//
//
//
//	return 0;
//}



void hex_to_bin(const char* in, char* out)
{
  int val;

  val = 0;
  switch (in[0])
  {
    case '1': val = 16; break;
    case '2': val = 16 * 2; break;
    case '3': val = 16 * 3; break;
    case '4': val = 16 * 4; break;
    case '5': val = 16 * 5; break;
    case '6': val = 16 * 6; break;
    case '7': val = 16 * 7; break;
    case '8': val = 16 * 8; break;
    case '9': val = 16 * 9; break;
    case 'a': val = 16 * 10; break;
    case 'A': val = 16 * 10; break;
    case 'b': val = 16 * 11; break;
    case 'B': val = 16 * 11; break;
    case 'c': val = 16 * 12; break;
    case 'C': val = 16 * 12; break;
    case 'd': val = 16 * 13; break;
    case 'D': val = 16 * 13; break;
    case 'e': val = 16 * 14; break;
    case 'E': val = 16 * 14; break;
    case 'f': val = 16 * 15; break;
    case 'F': val = 16 * 15; break;
  }
  switch (in[1])
  {
    case '1': val += 1; break;
    case '2': val += 2; break;
    case '3': val += 3; break;
    case '4': val += 4; break;
    case '5': val += 5; break;
    case '6': val += 6; break;
    case '7': val += 7; break;
    case '8': val += 8; break;
    case '9': val += 9; break;
    case 'a': val += 10; break;
    case 'A': val += 10; break;
    case 'b': val += 11; break;
    case 'B': val += 11; break;
    case 'c': val += 12; break;
    case 'C': val += 12; break;
    case 'd': val += 13; break;
    case 'D': val += 13; break;
    case 'e': val += 14; break;
    case 'E': val += 14; break;
    case 'f': val += 15; break;
    case 'F': val += 15; break;
  }
  *out = val;
}

void hex_to_bin(const wchar_t* in, char* out)
{
  int val;

  val = 0;
  switch (in[0])
  {
    case L'1': val = 16; break;
    case L'2': val = 16 * 2; break;
    case L'3': val = 16 * 3; break;
    case L'4': val = 16 * 4; break;
    case L'5': val = 16 * 5; break;
    case L'6': val = 16 * 6; break;
    case L'7': val = 16 * 7; break;
    case L'8': val = 16 * 8; break;
    case L'9': val = 16 * 9; break;
    case L'a': val = 16 * 10; break;
    case L'A': val = 16 * 10; break;
    case L'b': val = 16 * 11; break;
    case L'B': val = 16 * 11; break;
    case L'c': val = 16 * 12; break;
    case L'C': val = 16 * 12; break;
    case L'd': val = 16 * 13; break;
    case L'D': val = 16 * 13; break;
    case L'e': val = 16 * 14; break;
    case L'E': val = 16 * 14; break;
    case L'f': val = 16 * 15; break;
    case L'F': val = 16 * 15; break;
  }
  switch (in[1])
  {
    case L'1': val += 1; break;
    case L'2': val += 2; break;
    case L'3': val += 3; break;
    case L'4': val += 4; break;
    case L'5': val += 5; break;
    case L'6': val += 6; break;
    case L'7': val += 7; break;
    case L'8': val += 8; break;
    case L'9': val += 9; break;
    case L'a': val += 10; break;
    case L'A': val += 10; break;
    case L'b': val += 11; break;
    case L'B': val += 11; break;
    case L'c': val += 12; break;
    case L'C': val += 12; break;
    case L'd': val += 13; break;
    case L'D': val += 13; break;
    case L'e': val += 14; break;
    case L'E': val += 14; break;
    case L'f': val += 15; break;
    case L'F': val += 15; break;
  }
  *out = val;
}

