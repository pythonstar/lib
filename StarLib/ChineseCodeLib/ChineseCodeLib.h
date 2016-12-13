// ChineseCodeLib.h: interface for the CChineseCodeLib class.
//
//////////////////////////////////////////////////////////////////////
#include<string>
#include <afx.h>
using namespace std;

/*
���ܣ�����GB2312��UTF-8���뻥ת
���ߣ�litz
Email:mycro@163.com
�ο����⿵�����������¡�UTF-8��GB2312֮��Ļ�����
http://www.vckbase.com/document/viewdoc/?id=1397
*/


#if !defined(__CCHINESECODELIB_H_)
#define __CCHINESECODELIB_H_

class CChineseCodeLib  
{
public:
	static void UTF_8ToGB2312(string& pOut,char *pText, int pLen);
	static void GB2312ToUTF_8(string& pOut,char *pText, int pLen);
	// Unicode ת����UTF-8
	static void UnicodeToUTF_8(char* pOut,WCHAR* pText);
	// GB2312 ת���ɡ�Unicode
	static void Gb2312ToUnicode(WCHAR* pOut,char *gbBuffer);
	// ��Unicode ת���� GB2312
	static void UnicodeToGB2312(char* pOut,unsigned short uData);
	// ��UTF-8ת����Unicode
	static void UTF_8ToUnicode(WCHAR* pOut,char* pText);

	static void ConvertUtf8ToGBK(CString& strUtf8);
	static void ConvertGBKToUtf8(CString& strGBK);

	CChineseCodeLib();
	virtual ~CChineseCodeLib();
};

#endif // !defined(__CCHINESECODELIB_H_)
