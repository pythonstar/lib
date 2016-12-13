#include "common/header.h"
#include "common/utility.h"
namespace XGC
{
	namespace common
	{
		xgc_lpcstr gethost( xgc_lpcstr pszAny )
		{
			struct hostent *host;
			if ( pszAny == NULL) return NULL;

			size_t size = 0;
			const char* szAnyANSI = NULL;
			szAnyANSI = (const char* )pszAny;

			host = gethostbyname( szAnyANSI );
			if( host == NULL )
			{
				return NULL;
			}

			return (char *)inet_ntoa(*(struct in_addr *)(host->h_addr));
		}

		//--------------------------------------------------------------------------------------------------------//
		// 规格化路径，将路径规格化为绝对路径 类似于"E:\tools\Downloads\"
		// 规格化后的路径如果传入绝对路径则只判断路径是否合法，如果传入相对路径则以当前程序所在的目录为当前目录
		// 目录规格化后都以'\'结尾。
		// [in]lpszPath				:	需要规格化的路径
		// [out]lpszNormalizePath	:	规格化后的输出缓冲区
		// [in]dwSize				:	输出缓冲区大小
		//--------------------------------------------------------------------------------------------------------//
		bool NormalizePath( xgc_lpctstr lpszPath, xgc_lptstr lpszNormalizePath, DWORD dwSize )
		{
			// 该函数由于对szModulePath 的缓冲区判断不严格，可能导致一个栈溢出。需要更严格的缓冲区长度判断。
			if( lpszPath[0] == '.' )
			{
				TCHAR szModulePath[_MAX_PATH];
				::GetModuleFileName( NULL, szModulePath, sizeof( szModulePath )/sizeof( szModulePath[0] ) );

				// 去掉文件名
				TCHAR* p = _tcsrchr( szModulePath, _T('\\') );
				if( p ) *p = 0;

				int i = 0;
				int dot = 0;
				CONST TCHAR* pDirector = lpszPath;
				do
				{
					if( lpszPath[i] == '\\' || lpszPath[i] == 0 )
					{
						if( dot == 2 ) // 后退一级目录
						{
							TCHAR* p = _tcsrchr( szModulePath, _T('\\') );
							if( p ) *p = 0; 
							else 
							{
								return false;
							}
						}
						else if( dot == 0 ) // 相对目录的子目录
						{
							_tcsncat( szModulePath, pDirector, lpszPath + i - pDirector );
							ASSERT( _tcslen( lpszNormalizePath ) < dwSize );
						}
						dot = 0;
						pDirector = &lpszPath[i];
					}
					else if( lpszPath[i] == '.' )
					{
						++dot;
					}

					if( dot > 2 ) 
					{
						return false;
					}
				}while( lpszPath[i++] );

				_tcsncpy( lpszNormalizePath, szModulePath, dwSize );
				ASSERT( _tcslen( lpszNormalizePath ) < dwSize );
			}
			else
			{
				_tcsncpy( lpszNormalizePath, lpszPath, dwSize );
				ASSERT( _tcslen( lpszNormalizePath ) < dwSize );
			}
			// 格式化目录，最后的一个字符一定要是'\'
			xgc_tchar* right = _tcsrchr( lpszNormalizePath, _T('\\') );
			if( *(right+1) != 0 )
			{
				_tcsncat( lpszNormalizePath, _T("\\"), 1 );
				ASSERT( _tcslen( lpszNormalizePath ) < dwSize );
			}
			return true;
		}
		/* IsTextUTF8
		*
		* UTF-8 is the encoding of Unicode based on Internet Society RFC2279
		* ( See http://www.cis.ohio-state.edu/htbin/rfc/rfc2279.html )
		*
		* Basicly:
		* 0000 0000-0000 007F - 0xxxxxxx  (ascii converts to 1 octet!)
		* 0000 0080-0000 07FF - 110xxxxx 10xxxxxx    ( 2 octet format)
		* 0000 0800-0000 FFFF - 1110xxxx 10xxxxxx 10xxxxxx (3 octet format)
		* (this keeps going for 32 bit unicode) 
		* 
		*
		* Return value:  TRUE, if the text is in UTF-8 format.
		*                FALSE, if the text is not in UTF-8 format.
		*                We will also return FALSE is it is only 7-bit ascii, so the right code page
		*                will be used.
		*
		*                Actually for 7 bit ascii, it doesn't matter which code page we use, but
		*                notepad will remember that it is utf-8 and "save" or "save as" will store
		*                the file with a UTF-8 BOM.  Not cool.
		*/
		// 这段代码是有bug的，微软记事本很著名的“联通”的bug也是这段代码引起的，暂时还没找到更好的识别无BOM的UTF8文本的更好的办法
		xgc_int32 IsTextUTF8( xgc_lpstr lpstrInputStream, int iLen )
		{
			INT   i;
			DWORD cOctets;  // octets to go in this UTF-8 encoded character
			UCHAR chr;
			xgc_bool  bAllAscii= TRUE;

			cOctets= 0;
			for( i=0; i < iLen; i++ ) {
				chr= *(lpstrInputStream+i);

				if( (chr&0x80) != 0 ) bAllAscii= FALSE;

				if( cOctets == 0 )  {
					//
					// 7 bit ascii after 7 bit ascii is just fine.  Handle start of encoding case.
					//
					if( chr >= 0x80 ) {  
						//
						// count of the leading 1 bits is the number of characters encoded
						//
						do {
							chr <<= 1;
							cOctets++;
						}
						while( (chr&0x80) != 0 );

						cOctets--;                        // count includes this character
						if( cOctets == 0 ) return FALSE;  // must start with 11xxxxxx
					}
				}
				else {
					// non-leading bytes must start as 10xxxxxx
					if( (chr&0xC0) != 0x80 ) {
						return FALSE;
					}
					cOctets--;                           // processed another octet in encoding
				}
			}

			//
			// End of text.  Check for consistency.
			//

			if( cOctets > 0 ) {   // anything left over at the end is an error
				return FALSE;
			}

			if( bAllAscii ) {     // Not utf-8 if all ascii.  Forces caller to use code pages for conversion
				return FALSE;
			}

			return TRUE;
		}

		#define R2VA(modulebase, rva)  (((PBYTE)modulebase) + rva) // Relative Virtual Address to Virtual Address conversion.
		//////////////////////////////////////////////////////////////////////////
		//
		bool patchimport( HMODULE importmodule, xgc_lpcstr exportmodulename, xgc_lpcstr exportmodulepath, xgc_lpcstr importname, xgc_lpcvoid replacement )
		{
			HMODULE                  exportmodule;
			IMAGE_THUNK_DATA        *iate;
			IMAGE_IMPORT_DESCRIPTOR *idte;
			FARPROC                  import;
			DWORD                    protect;
			IMAGE_SECTION_HEADER    *section;
			ULONG                    size;

			ASSERT(exportmodulename != NULL);

			// Locate the importing module's Import Directory Table (IDT) entry for the
			// exporting module. The importing module actually can have several IATs --
			// one for each export module that it imports something from. The IDT entry
			// gives us the offset of the IAT for the module we are interested in.
			idte = (IMAGE_IMPORT_DESCRIPTOR*)ImageDirectoryEntryToDataEx((PVOID)importmodule, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &size, &section);
			if (idte == NULL) 
			{
				// This module has no IDT (i.e. it imports nothing).
				return FALSE;
			}
			while (idte->OriginalFirstThunk != 0x0) {
				if (_stricmp((PCHAR)R2VA(importmodule, idte->Name), exportmodulename) == 0) 
				{
					// Found the IDT entry for the exporting module.
					break;
				}
				idte++;
			}
			if (idte->OriginalFirstThunk == 0x0) 
			{
				// The importing module does not import anything from the exporting
				// module.
				return FALSE;
			}

			// Get the *real* address of the import. If we find this address in the IAT,
			// then we've found the entry that needs to be patched.
			if (exportmodulepath != NULL) 
			{
				// Always try to use the full path if available. There seems to be a bug
				// (or is this a new feature of the "side-by-side" kruft?) where
				// GetModuleHandle sometimes fails if the full path is not supplied for
				// DLLs in the side-by-side cache.
				exportmodule = GetModuleHandleA(exportmodulepath);
			}
			else 
			{
				// No full path available. Try using just the module name by itself.
				exportmodule = GetModuleHandleA(exportmodulename);
			}
			ASSERT(exportmodule != NULL);
			import = GetProcAddress(exportmodule, importname);
			ASSERT(import != NULL); // Perhaps the named export module does not actually export the named import?

			// Locate the import's IAT entry.
			iate = (IMAGE_THUNK_DATA*)R2VA(importmodule, idte->FirstThunk);
			while (iate->u1.Function != 0x0) {
				if (iate->u1.Function == (DWORD_PTR)import) {
					// Found the IAT entry. Overwrite the address stored in the IAT
					// entry with the address of the replacement. Note that the IAT
					// entry may be write-protected, so we must first ensure that it is
					// writable.
					VirtualProtect(&iate->u1.Function, sizeof(iate->u1.Function), PAGE_READWRITE, &protect);
					iate->u1.Function = (DWORD)(DWORD_PTR)replacement;
					VirtualProtect(&iate->u1.Function, sizeof(iate->u1.Function), protect, &protect);

					// The patch has been installed in the import module.
					return TRUE;
				}
				iate++;
			}

			// The import's IAT entry was not found. The importing module does not
			// import the specified import.
			return FALSE;
		}

		void restoreimport (HMODULE importmodule, xgc_lpcstr exportmodulename, xgc_lpcstr exportmodulepath, xgc_lpcstr importname, xgc_lpcvoid replacement)
		{
			HMODULE                  exportmodule;
			IMAGE_THUNK_DATA        *iate;
			IMAGE_IMPORT_DESCRIPTOR *idte;
			FARPROC                  import;
			DWORD                    protect;
			IMAGE_SECTION_HEADER    *section;
			ULONG                    size;

			ASSERT(exportmodulename != NULL);

			// Locate the importing module's Import Directory Table (IDT) entry for the
			// exporting module. The importing module actually can have several IATs --
			// one for each export module that it imports something from. The IDT entry
			// gives us the offset of the IAT for the module we are interested in.
			idte = (IMAGE_IMPORT_DESCRIPTOR*)ImageDirectoryEntryToDataEx((PVOID)importmodule, TRUE,
				IMAGE_DIRECTORY_ENTRY_IMPORT, &size, &section);
			if (idte == NULL) {
				// This module has no IDT (i.e. it imports nothing).
				return;
			}
			while (idte->OriginalFirstThunk != 0x0) {
				if (_stricmp((PCHAR)R2VA(importmodule, idte->Name), exportmodulename) == 0) {
					// Found the IDT entry for the exporting module.
					break;
				}
				idte++;
			}
			if (idte->OriginalFirstThunk == 0x0) {
				// The importing module does not import anything from the exporting
				// module.
				return;
			}

			// Get the *real* address of the import.
			if (exportmodulepath != NULL) {
				// Always try to use the full path if it'savailable. There seems to be a
				// bug (or is this a new feature of the "side-by-side" kruft?) where
				// GetModuleHandle sometimes fails if the full path is not supplied for
				// DLLs in the side-by-side cache.
				exportmodule = GetModuleHandleA(exportmodulepath);
			}
			else {
				// No full path available. Try using just the module name by itself.
				exportmodule = GetModuleHandleA(exportmodulename);
			}
			ASSERT(exportmodule != NULL);
			import = GetProcAddress(exportmodule, importname);
			ASSERT(import != NULL); // Perhaps the named export module does not actually export the named import?

			// Locate the import's original IAT entry (it currently has the replacement
			// address in it).
			iate = (IMAGE_THUNK_DATA*)R2VA(importmodule, idte->FirstThunk);
			while (iate->u1.Function != 0x0) {
				if (iate->u1.Function == (DWORD_PTR)replacement) {
					// Found the IAT entry. Overwrite the address stored in the IAT
					// entry with the import's real address. Note that the IAT entry may
					// be write-protected, so we must first ensure that it is writable.
					VirtualProtect(&iate->u1.Function, sizeof(iate->u1.Function), PAGE_READWRITE, &protect);
					iate->u1.Function = (DWORD)(DWORD_PTR)import;
					VirtualProtect(&iate->u1.Function, sizeof(iate->u1.Function), protect, &protect);
					break;
				}
				iate++;
			}
		}
		//////////////////////////////////////////////////////////////////////////

		__declspec( thread ) xgc_tchar g_szDescription[1024*32];
		EXPORT_LIB xgc_lpctstr GetErrorDescription()
		{
			DWORD dwWrite = FormatMessage( 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(xgc_lptstr)g_szDescription,
				_countof(g_szDescription),
				NULL );

			g_szDescription[dwWrite] = 0;
			return g_szDescription;
			// Process any inserts in lpMsgBuf.
			// ...
		}

		__declspec( thread ) char __cover_buffer__[4096];
		__declspec( thread ) wchar_t __cover_wbuffer__[4096];
		xgc_lpcstr ws2s(const std::wstring& ws)
		{
			const wchar_t* _Source = ws.c_str();
			size_t _Dsize = _countof(__cover_buffer__);
			if( ws.size() + 1 >= _Dsize )
				return NULL;

			char *_Dest = __cover_buffer__;
			memset(_Dest,0,_Dsize);
			wcstombs(_Dest,_Source,_Dsize);
			return _Dest;
		}

		xgc_lpcstr ws2s(const wchar_t* ws)
		{
			const wchar_t* _Source = ws;
			size_t _Dsize = _countof(__cover_buffer__);
			char *_Dest = __cover_buffer__;
			memset(_Dest,0,_Dsize);
			wcstombs(_Dest,_Source,_Dsize);
			return _Dest;
		}

		xgc_lpcwstr s2ws(const std::string& s)
		{
			const char* _Source = s.c_str();
			size_t _Dsize = _countof(__cover_wbuffer__);
			wchar_t *_Dest = __cover_wbuffer__;
			wmemset(_Dest, 0, _Dsize);
			mbstowcs(_Dest,_Source,_Dsize);
			return _Dest;
		}

		xgc_lpcwstr s2ws(const char* s)
		{
			const char* _Source = s;
			size_t _Dsize = _countof(__cover_wbuffer__);
			wchar_t *_Dest = __cover_wbuffer__;
			wmemset(_Dest, 0, _Dsize);
			mbstowcs(_Dest,_Source,_Dsize);
			return _Dest;
		}


		//--------------------------------------------------------------------------------------------------------//
		// D2Hex
		// pData		:	数据首址
		// nDataSize	:	数据长度（按字符计）
		// pOut			:	输出缓冲首址
		// nOutSize		:	缓冲区长度（按字符计）
		// nFlags		:	标志 （未定义）
		// return		:	返回转换了多少字符
		//--------------------------------------------------------------------------------------------------------//
		xgc_uint32 data2hex( xgc_lpcstr data, xgc_uint32 size, xgc_lptstr out, xgc_uint32 osize, xgc_uint32 flags )
		{
			ASSERT( data );
			ASSERT( out );
			ASSERT( (void*)data != (void*)out );

			int width = ((flags&0x00000003)+1)*8;
			xgc_tchar* ascii = (xgc_tchar*)_alloca( (width + 1)*sizeof( xgc_tchar ) );
			xgc_tchar* p = out;
			xgc_tchar* end = p + osize;

			int n = _sntprintf( p, (size_t)(end - p), _T("\r\n") );
			p += n;
			xgc_uint32 i = 0;
			for( ; i < size; ++i )
			{
				if( p >= end ) break;
				unsigned char uch = *(data + i);
				n = _sntprintf( p, (size_t)(end - p), _T("%02x "), uch );
				if( n < 0 )
				{
					*(p-1) = 0;
					break;
				}
				if( _istprint( uch ) )
				{
					*ascii++ = uch;
				}
				else
				{
					*ascii++ = '.';
				}
				p += n;

				if( (i+1)%width == 0 )
				{
					*ascii++ = 0;
					ascii -= width+1;
					n = _sntprintf( p, (size_t)(end - p), _T("%s\r\n"), ascii );
					if( n < 0 )
					{
						*(p-1) = 0;
						break;
					}
					p += n;
				}
			}

			xgc_tchar szFmt[64];
			_sntprintf( szFmt, _countof( szFmt ), _T("%%%ds"), ( width - i%width ) * 3 );
			n = _sntprintf( p, (size_t)(end - p), szFmt, _T("") );
			if( n < 0 )
			{
				*(p-1) = 0;
			}
			else
			{
				p += n;
				*ascii++ = 0;
				ascii -= (i+1)%width;
				n = _sntprintf( p, (size_t)(end - p), _T("%s\r\n"), ascii );
				if( n < 0 )
				{
					*(p-1) = 0;
				}
				else
				{
					p += n;
				}
			}
			return xgc_uint32(p - out);
		}

		char* strntok( char *string, const char *end, const char *control, char **next )
		{
			unsigned char *str;
			const unsigned char *ctl = (const unsigned char*)control;
			unsigned char map[32];
			int count;

			if( *next == end ) 
				return NULL;

			/* Clear control map */
			for (count = 0; count < 32; count++)
			{
				map[count] = 0;
			}

			/* Set bits in delimiter table */
			do {
				map[*ctl >> 3] |= (1 << (*ctl & 7));
			} while (*ctl++);

			/* If string is NULL, set str to the saved
			* pointer (i.e., continue breaking tokens out of the string
			* from the last strtok call) */
			if (string != NULL )
			{
				str = (unsigned char*)string;
			}
			else
			{
				str = (unsigned char*)(*next+1);
			}

			/* Find beginning of token (skip over leading delimiters). Note that
			* there is no token iff this loop sets str to point to the terminal
			* null (*str == 0) */
			while ((map[*str >> 3] & (1 << (*str & 7))) && *str != 0 && (char*)str != end )
			{
				str++;
			}

			string = (char*)str;

			/* Find the end of the token. If it is not the end of the string,
			* put a null there. */
			for ( ; *str != 0 ; str++ )
			{
				if ( ( (char*)str == end ) || (map[*str >> 3] & (1 << (*str & 7))) )
				{
					//*str++ = 0;
					break;
				}
			}

			/* Update context */
			*next = (char*)str;

			/* Determine if a token has been found. */
			if (string == (char*)str)
			{
				return NULL;
			}
			else
			{
				return string;
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// 字符串过滤 [4/1/2010 Administrator]
		// 支持通配符
		//////////////////////////////////////////////////////////////////////////
		bool compare_wildcard( xgc_lpctstr str1, xgc_lpctstr comaprestr)
		{
			xgc_lpctstr dstr = str1;
			xgc_lpctstr cstr = comaprestr;

			while (*dstr != 0 && *cstr != 0)
			{
				if( *cstr == _T('*') )
				{
					while( *cstr == _T('*') ) 
						cstr = _tcsinc(cstr);

					if( *cstr == _T('?') ) 
						cstr = _tcsinc(cstr);

					while( _tccmp( cstr, dstr ) != 0 )
						dstr = _tcsinc(dstr);
				}
				else if( *cstr == '?' )
				{
					dstr = _tcsinc(dstr); 
					cstr = _tcsinc(cstr); 
				}
				else if( _tccmp( dstr, cstr ) == 0 )
				{
					dstr = _tcsinc(dstr);
					cstr = comaprestr;
				}
			}

			return (*cstr!=0);
		}

		//--------------------------------------------------------------------------------------------------------//
		#if defined( _USE_WININET )
		#pragma comment( lib, "shlwapi.lib" )
		#pragma comment( lib, "Wininet.lib" )

		xgc_bool ParseURLWorker(xgc_lpctstr pstrURL, LPURL_COMPONENTS lpComponents, DWORD& dwServiceType, WORD& nPort, DWORD nFlags)
		{

			// this function will return bogus stuff if lpComponents
			// isn't set up to copy the components

			ASSERT(lpComponents != NULL && pstrURL != NULL);
			if (lpComponents == NULL || pstrURL == NULL)
				return FALSE;
			ASSERT(lpComponents->dwHostNameLength == 0 ||
				lpComponents->lpszHostName != NULL);
			ASSERT(lpComponents->dwUrlPathLength == 0 ||
				lpComponents->lpszUrlPath != NULL);
			ASSERT(lpComponents->dwUserNameLength == 0 ||
				lpComponents->lpszUserName != NULL);
			ASSERT(lpComponents->dwPasswordLength == 0 ||
				lpComponents->lpszPassword != NULL);

			LPTSTR pstrCanonicalizedURL;
			TCHAR szCanonicalizedURL[INTERNET_MAX_URL_LENGTH];
			DWORD dwNeededLength = INTERNET_MAX_URL_LENGTH;
			xgc_bool bRetVal;
			xgc_bool bMustFree = FALSE;

			// Decoding is done in InternetCrackUrl/UrlUnescape 
			// so we don't need the ICU_DECODE flag here.

			DWORD dwCanonicalizeFlags = nFlags &
				(ICU_NO_ENCODE | ICU_NO_META |
				ICU_ENCODE_SPACES_ONLY | ICU_BROWSER_MODE);

			DWORD dwCrackFlags = nFlags & ICU_USERNAME;

			xgc_bool bUnescape = FALSE;

			if((nFlags & (ICU_ESCAPE | ICU_DECODE)) && (lpComponents->dwUrlPathLength != 0) )
			{

				// We use only the ICU_ESCAPE flag for decoding even if
				// ICU_DECODE is passed.

				// Also, if ICU_BROWSER_MODE is passed we do the unescaping
				// manually because InternetCrackUrl doesn't do
				// Browser mode unescaping

				if (nFlags & ICU_BROWSER_MODE)
					bUnescape = TRUE;
				else
					dwCrackFlags |= ICU_ESCAPE;
			}

			bRetVal = InternetCanonicalizeUrl(pstrURL, szCanonicalizedURL,
				&dwNeededLength, dwCanonicalizeFlags);

			if (!bRetVal)
			{
				if (::GetLastError() != ERROR_INSUFFICIENT_BUFFER)
					return FALSE;

				pstrCanonicalizedURL = new TCHAR[dwNeededLength];
				if (pstrCanonicalizedURL == NULL)
					return FALSE;

				bMustFree = TRUE;
				bRetVal = InternetCanonicalizeUrl(pstrURL, pstrCanonicalizedURL,
					&dwNeededLength, dwCanonicalizeFlags);
				if (!bRetVal)
				{
					delete [] pstrCanonicalizedURL;
					return FALSE;
				}
			}
			else
				pstrCanonicalizedURL = szCanonicalizedURL;

			// now that it's safely canonicalized, crack it

			bRetVal = InternetCrackUrl(pstrCanonicalizedURL, 0,
				dwCrackFlags, lpComponents);

			if(bUnescape)
			{
				if(FAILED(UrlUnescape(lpComponents->lpszUrlPath,NULL,NULL,URL_UNESCAPE_INPLACE | URL_DONT_UNESCAPE_EXTRA_INFO)))
				{
					if (bMustFree)
						delete [] pstrCanonicalizedURL;

					return FALSE;
				}

				lpComponents->dwUrlPathLength = lstrlen(lpComponents->lpszUrlPath);
			}

			if (bMustFree)
				delete [] pstrCanonicalizedURL;

			// convert to MFC-style service ID

			if (!bRetVal)
				dwServiceType = INET_SERVICE_UNK;
			else
			{
				nPort = lpComponents->nPort;
				dwServiceType = lpComponents->nScheme;
				//switch (lpComponents->nScheme)
				//{
				//case INTERNET_SCHEME_FTP:
				//	dwServiceType = AFX_INET_SERVICE_FTP;
				//	break;

				//case INTERNET_SCHEME_GOPHER:
				//	dwServiceType = AFX_INET_SERVICE_GOPHER;
				//	break;

				//case INTERNET_SCHEME_HTTP:
				//	dwServiceType = AFX_INET_SERVICE_HTTP;
				//	break;

				//case INTERNET_SCHEME_HTTPS:
				//	dwServiceType = AFX_INET_SERVICE_HTTPS;
				//	break;

				//case INTERNET_SCHEME_FILE:
				//	dwServiceType = AFX_INET_SERVICE_FILE;
				//	break;

				//case INTERNET_SCHEME_NEWS:
				//	dwServiceType = AFX_INET_SERVICE_NNTP;
				//	break;

				//case INTERNET_SCHEME_MAILTO:
				//	dwServiceType = AFX_INET_SERVICE_MAILTO;
				//	break;

				//default:
				//	dwServiceType = AFX_INET_SERVICE_UNK;
				//}
			}

			return bRetVal;
		}

		xgc_bool ParseURLEx(xgc_lpctstr pstrURL, DWORD& dwServiceType,
						xgc_string& strServer, xgc_string& strObject, INTERNET_PORT& nPort,
						xgc_string& strUsername, xgc_string& strPassword, DWORD nFlags/* = 0*/)
		{
			dwServiceType = INET_SERVICE_UNK;

			ASSERT(pstrURL != NULL);
			if (pstrURL == NULL)
				return FALSE;

			URL_COMPONENTS urlComponents;
			memset(&urlComponents, 0, sizeof(URL_COMPONENTS));
			urlComponents.dwStructSize = sizeof(URL_COMPONENTS);

			TCHAR szServer[INTERNET_MAX_HOST_NAME_LENGTH+1];
			TCHAR szObject[INTERNET_MAX_PATH_LENGTH+1];
			TCHAR szUsername[INTERNET_MAX_USER_NAME_LENGTH+1];
			TCHAR szPassword[INTERNET_MAX_PASSWORD_LENGTH+1];
			_tcsncpy( szServer, strServer.c_str(), _countof( szServer ) );
			_tcsncpy( szObject, strObject.c_str(), _countof( szObject ) );

			urlComponents.dwHostNameLength = INTERNET_MAX_HOST_NAME_LENGTH;
			urlComponents.lpszHostName = szServer;
			urlComponents.dwUrlPathLength = INTERNET_MAX_PATH_LENGTH;
			urlComponents.lpszUrlPath = szObject;
			urlComponents.dwUserNameLength = INTERNET_MAX_USER_NAME_LENGTH;
			urlComponents.lpszUserName = szUsername;
			urlComponents.dwPasswordLength = INTERNET_MAX_PASSWORD_LENGTH;
			urlComponents.lpszPassword = szPassword;

			xgc_bool bRetVal = ParseURLWorker(pstrURL, &urlComponents,
				dwServiceType, nPort, nFlags);

			strServer = szServer;
			strObject = szObject;
			strUsername = szUsername;
			strPassword = szPassword;
			return bRetVal;
		}

		xgc_bool ParseURL(xgc_lpctstr pstrURL, DWORD& dwServiceType, xgc_string& strServer, xgc_string& strObject, INTERNET_PORT& nPort)
		{
			dwServiceType = INET_SERVICE_UNK;

			ASSERT(pstrURL != NULL);
			if (pstrURL == NULL)
				return FALSE;

			URL_COMPONENTS urlComponents;
			memset(&urlComponents, 0, sizeof(URL_COMPONENTS));
			urlComponents.dwStructSize = sizeof(URL_COMPONENTS);

			TCHAR szServer[INTERNET_MAX_HOST_NAME_LENGTH+1];
			TCHAR szObject[INTERNET_MAX_PATH_LENGTH+1];
			_tcsncpy( szServer, strServer.c_str(), _countof( szServer ) );
			_tcsncpy( szObject, strObject.c_str(), _countof( szObject ) );

			urlComponents.dwHostNameLength = INTERNET_MAX_URL_LENGTH;
			urlComponents.lpszHostName = szServer;
			urlComponents.dwUrlPathLength = INTERNET_MAX_URL_LENGTH;
			urlComponents.lpszUrlPath = szObject;

			xgc_bool bRetVal = ParseURLWorker( pstrURL, &urlComponents, dwServiceType, nPort, ICU_BROWSER_MODE );

			strServer = szServer;
			strObject = szObject;
			return bRetVal;
		}
		#endif
	}
}