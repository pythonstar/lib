
#pragma once
#include <atlstr.h>
#include <WinInet.h>

//内部通过调用URLDownloadToFile来下载文件，该函数有缓存，超时参数无效果。
HRESULT DownloadFileCachely(const CString&strUrl,LPCTSTR lpszSaveAs,int nTimeOutSeconds=10);

//内部实现方法同GetHttpFileContentNoCache，不适合https，如果是https的链接，返回错误：E_INVALIDARG
//该函数没有1MB大小数据限制，且该函数没有缓存，每次下载必是最新文件。
HRESULT DownloadFileNoCache(const CString&strUrl,LPCTSTR lpszSaveAs,int nTimeOutSeconds=10);

/*
[推荐]
如果协议中指明了是utf8格式字符串，则该函数会自动转码。
适合场景：json字符串直接返回的是utf8格式字符串，则该函数会自行转换。
但是html中的源码好像还是需要外部使用者根据网页中的内容是否包含<meta charset="utf-8">来自行转码
经长期使用，该函数有效。并且也适合https
*/
HRESULT GetHttpFileContent(LPCTSTR lpszUrl,CString&strHtml,int nTimeOutSeconds=10);

/*
该函数没有缓存，每次下载必定是最新数据，在即时生效场合很有用。
但是不适合https，如果是https的链接，返回错误：E_INVALIDARG
*/
HRESULT GetHttpFileContentNoCache(const CString&strUrl, CString&strHtml,int nTimeOutSeconds=10);

/*
内部实现方法同GetHttpFileContentNoCache，不适合https，如果是https的链接，返回错误：E_INVALIDARG
另外存在大小限制，只能获取1MB以内的数据，总体推荐使用：GetHttpFileContentNoCache
*/
HRESULT GetHttpFileContentEx(const CString&strUrl,CString&strHtml,int nTimeOutSeconds=10);


//内部通过调用URLDownloadToCacheFile先把文件下载到临时文件，再读取内容后删除文件，超时参数无效果。
BOOL URLDownloadToString(const CString&strUrl,CString&strHtml,int nTimeOutSeconds=10);
