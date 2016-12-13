// ChineseCodeLib.h: interface for the CChineseCodeLib class.
//
//////////////////////////////////////////////////////////////////////
#include<string>
#include <afx.h>
using namespace std;

/*
功能：汉字GB2312与UTF-8编码互转
作者：litz
Email:mycro@163.com
参考：吴康彬先生的文章《UTF-8与GB2312之间的互换》
http://www.vckbase.com/document/viewdoc/?id=1397
*/


#if !defined(__CCHINESECODELIB_H_)
#define __CCHINESECODELIB_H_

class CChineseCodeLib  
{
public:
	static void UTF_8ToGB2312(string& pOut,char *pText, int pLen);
	static void GB2312ToUTF_8(string& pOut,char *pText, int pLen);
	// Unicode 转换成UTF-8
	static void UnicodeToUTF_8(char* pOut,WCHAR* pText);
	// GB2312 转换成　Unicode
	static void Gb2312ToUnicode(WCHAR* pOut,char *gbBuffer);
	// 把Unicode 转换成 GB2312
	static void UnicodeToGB2312(char* pOut,unsigned short uData);
	// 把UTF-8转换成Unicode
	static void UTF_8ToUnicode(WCHAR* pOut,char* pText);

	static void ConvertUtf8ToGBK(CString& strUtf8);
	static void ConvertGBKToUtf8(CString& strGBK);

	CChineseCodeLib();
	virtual ~CChineseCodeLib();
};

#endif // !defined(__CCHINESECODELIB_H_)
