#include "stdafx.h"
#include "tinyxml.h"
#include <shlwapi.h>
#include <StarLib/Common/common.h>

#ifdef _DEBUG
#pragma comment(lib,"tinyxmld.lib")
#else
#pragma comment(lib,"tinyxml.lib")
#endif


static const TCHAR seps[] = _T(" ,\t:/");


BOOL CTinyXml::LoadFile(LPCTSTR lpszFilePath)
{
	CStringA strFilePathA;
	strFilePathA = lpszFilePath;
	return m_doc.LoadFile(strFilePathA);
}

BOOL CTinyXml::SaveFile(LPCTSTR lpszFilePath)
{
	CStringA strFilePathA;
	strFilePathA = lpszFilePath;

	TiXmlDeclaration* pdecl = new TiXmlDeclaration("1.0", "UTF-8", "yes"); 
	m_doc.LinkEndChild(pdecl);		//<?xml version="1.0" encoding="UTF-8"?> 
	return m_doc.SaveFile(strFilePathA);
}

TiXmlElement* CTinyXml::GetRoot()
{
	return m_doc.RootElement();
}

TiXmlNode* CTinyXml::GetChildNode(TiXmlNode* pParentNode, const char* lpszChildName)
{
	TiXmlNode* pNode = NULL;

	if ( pParentNode ) {
		pNode = pParentNode->FirstChild(lpszChildName);
	}else{
		TiXmlElement* pRoot = m_doc.RootElement();
		if ( pRoot ) {
			pNode = pRoot->FirstChild(lpszChildName);
		}
	}

	return pNode;
}

const char *CTinyXml::GetXmlNodeValue(TiXmlNode* pNode, const char* lpszChildName = NULL)
{
	TiXmlNode* pChildNode = NULL;
	TiXmlElement* pElement = NULL;
	const char* pText = NULL;

	if ( pNode!=NULL ) {
		if ( lpszChildName!=NULL ) {
			pChildNode = pNode->FirstChild(lpszChildName);
			if ( pChildNode!=NULL ) {
				pElement = pChildNode->ToElement();
				if ( pElement!=NULL ) {
					pText = pElement->GetText();
				}
			}
		}else{
			pElement = pNode->ToElement();
			if ( pElement!=NULL ) {
				pText = pElement->GetText();
			}
		}
	}

	return pText;
}

//从节点pNode的子节点lpszChildName中读取其值
BOOL CTinyXml::GetNodeValue(TiXmlNode* pNode, const char* lpszChildName, IN OUT int&nValue)
{
	BOOL bOK = TRUE;
	TiXmlNode* pChildNode = NULL;
	TiXmlElement* pElement = NULL;
	const char* pText = GetXmlNodeValue(pNode, lpszChildName);
	if ( pText!=NULL ) {
		nValue = StrToIntA(pText);
	}

	return bOK;
}

BOOL CTinyXml::GetNodeValue(TiXmlNode* pNode, const char* lpszChildName, IN OUT RECT&rcValue)
{
	BOOL bOK = TRUE;
	TiXmlNode* pChildNode = NULL;
	TiXmlElement* pElement = NULL;
	const char* pText = GetXmlNodeValue(pNode, lpszChildName);
	if ( pText!=NULL ) {
		static const TCHAR seps[] = _T(" ,\t:/");	
		CString value;
		value = pText;
		RECT tmprt;
		int pos = -1;

		// left
		value.TrimLeft(seps);
		tmprt.left = _ttoi(value);

		// top
		pos = value.FindOneOf(seps);
		if (pos<0)
			return false;
		value = value.Mid(pos+1);
		value.TrimLeft(seps);
		tmprt.top = _ttoi(value);

		// right
		pos = value.FindOneOf(seps);
		if (pos<0)
			return false;
		value = value.Mid(pos+1);
		value.TrimLeft(seps);
		tmprt.right = _ttoi(value);

		// bottom
		pos = value.FindOneOf(seps);
		if (pos<0)
			return false;
		value = value.Mid(pos+1);
		value.TrimLeft(seps);
		tmprt.bottom = _ttoi(value);

		rcValue = tmprt;
	}

	return bOK;
}

BOOL CTinyXml::GetNodeValue(TiXmlNode* pNode, const char* lpszChildName, IN OUT CString&strValue)
{
	const char *pText = GetXmlNodeValue(pNode, lpszChildName);
	if ( pText ) {
		strValue = Star::Common::utf8s2ts(pText);
		return TRUE;
	}

	return FALSE;
}

BOOL CTinyXml::GetNodeValue(TiXmlNode* pNode, const char* lpszChildName, IN OUT bool&bValue)
{
	CString strValue;
	CTinyXml::GetNodeValue(pNode, lpszChildName, strValue);
	bValue = strValue.CompareNoCase(_T("true"))==0;

	return TRUE;
}

BOOL CTinyXml::GetNodeValue(TiXmlNode* pNode, const char* lpszChildName, IN OUT double&fValue)
{
	CString strValue;
	CTinyXml::GetNodeValue(pNode, lpszChildName, strValue);
	fValue = _tstof(strValue);

	return TRUE;
}

BOOL CTinyXml::GetNodeValue(TiXmlNode* pNode, const char* lpszChildName, IN OUT SIZE&szValue)
{
	CString strValue;
	CTinyXml::GetNodeValue(pNode, lpszChildName, strValue);
	SIZE tmpsize;
	int pos = -1;

	// cx
	strValue.TrimLeft(seps);
	tmpsize.cx = _ttoi(strValue);

	// cy
	pos = strValue.FindOneOf(seps);
	if (pos<0)
		return FALSE;
	strValue = strValue.Mid(pos+1);
	strValue.TrimLeft(seps);
	tmpsize.cy = _ttoi(strValue);

	szValue = tmpsize;
	return TRUE;
}

BOOL CTinyXml::GetNodeValue(TiXmlNode* pNode, const char* lpszChildName, IN OUT POINT&ptValue)
{
	CString strValue;
	CTinyXml::GetNodeValue(pNode, lpszChildName, strValue);
	POINT tmppt;
	int pos = -1;

	// x
	strValue.TrimLeft(seps);
	tmppt.x = _ttoi(strValue);

	// y
	pos = strValue.FindOneOf(seps);
	if (pos<0)
		return FALSE;
	strValue = strValue.Mid(pos+1);
	strValue.TrimLeft(seps);
	tmppt.y = _ttoi(strValue);

	ptValue = tmppt;
	return TRUE;
}

BOOL CTinyXml::GetNodeValue(TiXmlNode* pNode, const char* lpszChildName, IN OUT COLORREF&clrValue)
{
	CString strValue;
	CTinyXml::GetNodeValue(pNode, lpszChildName, strValue);
	BYTE r, g, b;
	int pos = -1;

	// r - 如果为-1的话，表示是一个无效的颜色
	strValue.TrimLeft(seps);
	r = _ttoi(strValue);
	if (r==-1) {		
		clrValue = 0xFFFFFFFF;
		return TRUE;
	}

	// g
	pos = strValue.FindOneOf(seps);
	if (pos<0)
		return false;
	strValue = strValue.Mid(pos+1);
	strValue.TrimLeft(seps);
	g = _ttoi(strValue);

	// b
	pos = strValue.FindOneOf(seps);
	if (pos<0)
		return false;
	strValue = strValue.Mid(pos+1);
	strValue.TrimLeft(seps);
	b = _ttoi(strValue);

	clrValue = RGB(r, g, b);
	return TRUE;
}

TiXmlElement* CTinyXml::AddNode(TiXmlNode* pParentNode, const char* lpszChildName)
{
	TiXmlElement* pResult = NULL;
	if ( lpszChildName==NULL ) {
		return NULL;
	}

	pResult = new TiXmlElement(lpszChildName);
	if ( pParentNode==NULL ) {
		m_doc.LinkEndChild(pResult);
	}else{
		pParentNode->LinkEndChild(pResult);
	}

	return pResult;
}

BOOL CTinyXml::SetNodeValue(TiXmlNode* pNode, const char* lpszChildName, IN const int&nValue)
{
	CString strValue;
	strValue.Format(_T("%d"), nValue);
	return SetNodeValue(pNode, lpszChildName, strValue);
}

BOOL CTinyXml::SetNodeValue(TiXmlNode* pNode, const char* lpszChildName, IN const CString&strValue)
{
	if ( pNode!=NULL && lpszChildName!=NULL ) {
		CStringA strTextA = Star::Common::ts2utf8s(strValue);

		TiXmlElement* pChildNode = new TiXmlElement(lpszChildName);
		TiXmlText* pText = new TiXmlText(strTextA);  //文本
		pChildNode->LinkEndChild(pText);
		pNode->LinkEndChild(pChildNode);
	}

	return TRUE;
}

BOOL CTinyXml::SetNodeValue(TiXmlNode* pNode, const char* lpszChildName, IN const RECT&rcValue)
{
	CString strValue;
	strValue.Format(_T("%d, %d, %d, %d"), rcValue.left, rcValue.top, rcValue.right, rcValue.bottom);
	return SetNodeValue(pNode, lpszChildName, strValue);
}

BOOL CTinyXml::SetNodeValue(TiXmlNode* pNode, const char* lpszChildName, IN const bool&bValue)
{
	CString strValue = (bValue ? _T("true") : _T("false"));
	return SetNodeValue(pNode, lpszChildName, strValue);
}

BOOL CTinyXml::SetNodeValue(TiXmlNode* pNode, const char* lpszChildName, IN const double&fValue)
{
	CString strValue;
	strValue.Format(_T("%f"), fValue);
	return SetNodeValue(pNode, lpszChildName, strValue);
}

BOOL CTinyXml::SetNodeValue(TiXmlNode* pNode, const char* lpszChildName, IN const SIZE&szValue)
{
	CString strValue;
	strValue.Format(_T("%d, %d"), szValue.cx, szValue.cy);
	return SetNodeValue(pNode, lpszChildName, strValue);
}

BOOL CTinyXml::SetNodeValue(TiXmlNode* pNode, const char* lpszChildName, IN const POINT&ptValue)
{
	CString strValue;
	strValue.Format(_T("%d, %d"), ptValue.x, ptValue.y);
	return SetNodeValue(pNode, lpszChildName, strValue);
}

BOOL CTinyXml::SetNodeValue(TiXmlNode* pNode, const char* lpszChildName, IN const COLORREF&clrValue)
{
	CString strValue;
	if (clrValue&0xFF000000) {
		strValue.Format(_T("%d"), -1);
	} else {
		strValue.Format( _T("%d, %d, %d"),  GetRValue(clrValue), GetGValue(clrValue), GetBValue(clrValue));
	}
	return SetNodeValue(pNode, lpszChildName, strValue);
}
