
#pragma once
#include "../../tinyxml/tinyxml.h"

class CTinyXml
{
public:
	BOOL LoadFile(LPCTSTR lpszFilePath);
	BOOL SaveFile(LPCTSTR lpszFilePath);

public:
	TiXmlElement* GetRoot();
	//�ڽڵ�pParentNode�»�ȡ�ӽڵ�lpszChildName�����pParentNodeΪNULL���򸸽ڵ���Ϊ��root����root�ڵ��»�ȡ�ӽڵ�lpszChildName
	TiXmlNode* GetChildNode(TiXmlNode* pParentNode, const char* lpszChildName);

	//�ڽڵ�pParentNode���¼��ӽڵ�lpszChildName�����pParentNodeΪNULL���򴴽�һ����ΪlpszChildName�ĸ��ڵ�
	TiXmlElement* AddNode(TiXmlNode* pParentNode, const char* lpszChildName);

public:
	BOOL GetNodeValue(TiXmlNode* pNode, const char* lpszChildName, IN OUT int&nValue);
	BOOL GetNodeValue(TiXmlNode* pNode, const char* lpszChildName, IN OUT CString&strValue);
	BOOL GetNodeValue(TiXmlNode* pNode, const char* lpszChildName, IN OUT bool&bValue);
	BOOL GetNodeValue(TiXmlNode* pNode, const char* lpszChildName, IN OUT double&fValue);
	BOOL GetNodeValue(TiXmlNode* pNode, const char* lpszChildName, IN OUT RECT&rcValue);
	BOOL GetNodeValue(TiXmlNode* pNode, const char* lpszChildName, IN OUT SIZE&szValue);
	BOOL GetNodeValue(TiXmlNode* pNode, const char* lpszChildName, IN OUT POINT&ptValue);
	BOOL GetNodeValue(TiXmlNode* pNode, const char* lpszChildName, IN OUT COLORREF&clrValue);

	BOOL SetNodeValue(TiXmlNode* pNode, const char* lpszChildName, IN const int&nValue);
	BOOL SetNodeValue(TiXmlNode* pNode, const char* lpszChildName, IN const CString&strValue);
	BOOL SetNodeValue(TiXmlNode* pNode, const char* lpszChildName, IN const bool&bValue);
	BOOL SetNodeValue(TiXmlNode* pNode, const char* lpszChildName, IN const double&fValue);
	BOOL SetNodeValue(TiXmlNode* pNode, const char* lpszChildName, IN const RECT&rcValue);
	BOOL SetNodeValue(TiXmlNode* pNode, const char* lpszChildName, IN const SIZE&szValue);
	BOOL SetNodeValue(TiXmlNode* pNode, const char* lpszChildName, IN const POINT&ptValue);
	BOOL SetNodeValue(TiXmlNode* pNode, const char* lpszChildName, IN const COLORREF&clrValue);

protected:
private:
	//��ȡ�ڵ�pNode���ӽڵ�lpszChildName��ֵ�����lpszChildNameΪNULL�����ȡ�ڵ�pNode��ֵ
	const char *GetXmlNodeValue(TiXmlNode* pNode, const char* lpszChildName/* = NULL*/);

private:
	TiXmlDocument m_doc;
};