#ifndef _REGISTER_H__
#define _REGISTER_H__

#define KEYGEN_VERSION	0x2


// 机器码
typedef struct MachineCode {
	char buffer[100];
}
MachineCode;

// 注册码
typedef struct LicenseCode {
	char buffer[100];
}
LicenseCode;


//////////////////////////////////////////////////////
// 得到机器码
int get_machinecode(const char*prefix,MachineCode* mc);

//////////////////////////////////////////////////////
// 根据机器码及用户名，得到注册码
int get_licensecode(const char* name, MachineCode* mc, LicenseCode* lc);

//////////////////////////////////////////////////////
// 测试机器码，mc如果为NULL，内部会再计算
int test_licensecode(const char*prefix, MachineCode* mc, LicenseCode* lc);

//////////////////////////////////////////////////////
// 字符串转到二进制
// 返回值
int str_to_bin(char* str, void* bin, int bin_len);
int unicode_to_bin(wchar_t* str, void* bin, int bin_len);

//////////////////////////////////////////////////////
// 二进制到字符串转换
// 参数为二进制，二进制长度，目标字符串，目标字符串长度
int bin_to_str(void* bin, int bin_len, char* str, int str_len);
int bin_to_unicode(void* bin, int bin_len, wchar_t* str, int str_len);


////////////////////////////////////////////////////////
//// 将lua源代码转换成加密的代码块
//// 返回buffer的长度，缓冲尽可能的大吧
//int encode_lua(const char* lua_string, char* buffer, int len);




#endif // _REGISTER_H__