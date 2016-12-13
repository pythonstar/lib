#pragma once
#ifndef _COMMON_H
#define _COMMON_H

#ifdef EXPORT_API
#define EXPORT_LIB __declspec(dllexport)
#else
#define EXPORT_LIB __declspec(dllimport)
#endif
namespace XGC
{
	namespace common
	{
		//--------------------------------------------------------------------------------------------------------//
		// 规格化路径，将路径规格化为绝对路径 类似于"E:\tools\Downloads\"
		// 规格化后的路径如果传入绝对路径则只判断路径是否合法，如果传入相对路径则以当前程序所在的目录为当前目录
		// 目录规格化后都以'\'结尾。
		// [in]lpszPath				:	需要规格化的路径
		// [out]lpszNormalizePath	:	规格化后的输出缓冲区
		// [in]dwSize				:	输出缓冲区大小
		//--------------------------------------------------------------------------------------------------------//
		EXPORT_LIB bool NormalizePath( xgc_lpctstr lpszPath, xgc_lptstr lpszNormalizePath, xgc_uint32 dwSize );

		// 判断是否UTF8文本
		EXPORT_LIB xgc_int32 IsTextUTF8( xgc_lpstr lpstrInputStream, int iLen );

		// 为API打补丁（IAT）
		EXPORT_LIB bool patchimport( HMODULE importmodule, xgc_lpcstr exportmodulename, xgc_lpcstr exportmodulepath, xgc_lpcstr importname, xgc_lpcvoid replacement );

		// 清除补丁
		EXPORT_LIB void restoreimport (HMODULE importmodule, xgc_lpcstr exportmodulename, xgc_lpcstr exportmodulepath, xgc_lpcstr importname, xgc_lpcvoid replacement);

		//--------------------------------------------------------//
		//	created:	3:12:2009   16:04
		//	filename: 	utility
		//	author:		Albert.xu
		//
		//	purpose:	获取最后一次系统错误的描述
		//--------------------------------------------------------//
		EXPORT_LIB xgc_lpctstr GetErrorDescription();

		// 串转换
		EXPORT_LIB xgc_lpcstr ws2s(const std::wstring& ws);
		EXPORT_LIB xgc_lpcstr ws2s(const wchar_t* ws);

		EXPORT_LIB xgc_lpcwstr s2ws(const std::string& s);
		EXPORT_LIB xgc_lpcwstr s2ws(const char* s);

		#ifdef _UNICODE
			#define XA2T( a )	XGC::common::s2ws( a )
			#define XT2A( t )	XGC::common::ws2s( t )
			#define XW2T( w )	w
			#define XT2W( t )	t

		#else
			#define XA2T( a )	a
			#define XT2A( t )	t
			#define XW2T( w )	XGC::common::ws2s( a )
			#define XT2W( t )	XGC::common::s2ws( t )

		#endif
			#define XA2W( a )	XGC::common::s2ws( a )
			#define XW2A( w )	XGC::common::ws2s( w )

		EXPORT_LIB xgc_lpcstr gethost( xgc_lpcstr pszAny );

		//--------------------------------------------------------------------------------------------------------//
		// D2Hex
		// pData		:	数据首址
		// dwDataSize	:	数据长度（按字符计）
		// pOut			:	输出缓冲首址
		// dwOutSize	:	缓冲区长度（按字符计）
		// dwFlags		:	标志 （未定义）
		// return		:	返回转换了多少字符
		//--------------------------------------------------------------------------------------------------------//
		#define HEX_WIDTH_8		0x00000000
		#define HEX_WIDTH_16	0x00000001
		#define HEX_WIDTH_32	0x00000002
		#define HEX_WIDTH_64	0x00000003

		EXPORT_LIB xgc_uint32 data2hex( xgc_lpcstr pData, xgc_uint32 nDataSize, xgc_lptstr pOut, xgc_uint32 nOutSize, xgc_uint32 nFlags = 0 );

		EXPORT_LIB char* strntok( char *string, const char *end, const char *control, char **next );

		EXPORT_LIB bool compare_wildcard(const char* str1, const char* comaprestr);
		//--------------------------------------------------------------------------------------------------------//
		#define INET_SERVICE_UNK        0x1000
		//--------------------------------------------------------------------------------------------------------//
		// inet 辅助函数
		#if defined( _USE_WININET )
		xgc_bool EXPORT_LIB ParseURLEx(xgc_lpctstr pstrURL, xgc_uint32& dwServiceType, xgc_string& strServer, xgc_string& strObject, INTERNET_PORT& nPort, xgc_string& strUsername, xgc_string& strPassword, xgc_uint32 dwFlags/* = 0*/);
		xgc_bool EXPORT_LIB ParseURL(xgc_lpctstr pstrURL, xgc_uint32& dwServiceType, xgc_string& strServer, xgc_string& strObject, INTERNET_PORT& nPort);
		#endif
	}
}
#endif // _COMMON_H