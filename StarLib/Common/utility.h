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
		// ���·������·�����Ϊ����·�� ������"E:\tools\Downloads\"
		// ��񻯺��·������������·����ֻ�ж�·���Ƿ�Ϸ�������������·�����Ե�ǰ�������ڵ�Ŀ¼Ϊ��ǰĿ¼
		// Ŀ¼��񻯺���'\'��β��
		// [in]lpszPath				:	��Ҫ��񻯵�·��
		// [out]lpszNormalizePath	:	��񻯺�����������
		// [in]dwSize				:	�����������С
		//--------------------------------------------------------------------------------------------------------//
		EXPORT_LIB bool NormalizePath( xgc_lpctstr lpszPath, xgc_lptstr lpszNormalizePath, xgc_uint32 dwSize );

		// �ж��Ƿ�UTF8�ı�
		EXPORT_LIB xgc_int32 IsTextUTF8( xgc_lpstr lpstrInputStream, int iLen );

		// ΪAPI�򲹶���IAT��
		EXPORT_LIB bool patchimport( HMODULE importmodule, xgc_lpcstr exportmodulename, xgc_lpcstr exportmodulepath, xgc_lpcstr importname, xgc_lpcvoid replacement );

		// �������
		EXPORT_LIB void restoreimport (HMODULE importmodule, xgc_lpcstr exportmodulename, xgc_lpcstr exportmodulepath, xgc_lpcstr importname, xgc_lpcvoid replacement);

		//--------------------------------------------------------//
		//	created:	3:12:2009   16:04
		//	filename: 	utility
		//	author:		Albert.xu
		//
		//	purpose:	��ȡ���һ��ϵͳ���������
		//--------------------------------------------------------//
		EXPORT_LIB xgc_lpctstr GetErrorDescription();

		// ��ת��
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
		// pData		:	������ַ
		// dwDataSize	:	���ݳ��ȣ����ַ��ƣ�
		// pOut			:	���������ַ
		// dwOutSize	:	���������ȣ����ַ��ƣ�
		// dwFlags		:	��־ ��δ���壩
		// return		:	����ת���˶����ַ�
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
		// inet ��������
		#if defined( _USE_WININET )
		xgc_bool EXPORT_LIB ParseURLEx(xgc_lpctstr pstrURL, xgc_uint32& dwServiceType, xgc_string& strServer, xgc_string& strObject, INTERNET_PORT& nPort, xgc_string& strUsername, xgc_string& strPassword, xgc_uint32 dwFlags/* = 0*/);
		xgc_bool EXPORT_LIB ParseURL(xgc_lpctstr pstrURL, xgc_uint32& dwServiceType, xgc_string& strServer, xgc_string& strObject, INTERNET_PORT& nPort);
		#endif
	}
}
#endif // _COMMON_H