#ifndef _REGISTER_H__
#define _REGISTER_H__

#define KEYGEN_VERSION	0x2


// ������
typedef struct MachineCode {
	char buffer[100];
}
MachineCode;

// ע����
typedef struct LicenseCode {
	char buffer[100];
}
LicenseCode;


//////////////////////////////////////////////////////
// �õ�������
int get_machinecode(const char*prefix,MachineCode* mc);

//////////////////////////////////////////////////////
// ���ݻ����뼰�û������õ�ע����
int get_licensecode(const char* name, MachineCode* mc, LicenseCode* lc);

//////////////////////////////////////////////////////
// ���Ի����룬mc���ΪNULL���ڲ����ټ���
int test_licensecode(const char*prefix, MachineCode* mc, LicenseCode* lc);

//////////////////////////////////////////////////////
// �ַ���ת��������
// ����ֵ
int str_to_bin(char* str, void* bin, int bin_len);
int unicode_to_bin(wchar_t* str, void* bin, int bin_len);

//////////////////////////////////////////////////////
// �����Ƶ��ַ���ת��
// ����Ϊ�����ƣ������Ƴ��ȣ�Ŀ���ַ�����Ŀ���ַ�������
int bin_to_str(void* bin, int bin_len, char* str, int str_len);
int bin_to_unicode(void* bin, int bin_len, wchar_t* str, int str_len);


////////////////////////////////////////////////////////
//// ��luaԴ����ת���ɼ��ܵĴ����
//// ����buffer�ĳ��ȣ����御���ܵĴ��
//int encode_lua(const char* lua_string, char* buffer, int len);




#endif // _REGISTER_H__