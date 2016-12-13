
#pragma once
#include "../../tinyxml/tinyxml.h"

class CTinyXml
{
public:
	BOOL LoadFile(LPCTSTR lpszFilePath);
	BOOL SaveFile(LPCTSTR lpszFilePath);

public:
	TiXmlElement* GetRoot();
	//在节点pParentNode下获取子节点lpszChildName，如果pParentNode为NULL，则父节点认为是root，在root节点下获取子节点lpszChildName
	TiXmlNode* GetChildNode(TiXmlNode* pParentNode, const char* lpszChildName);

	//在节点pParentNode下新加子节点lpszChildName，如果pParentNode为NULL，则创建一个名为lpszChildName的根节点
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
	//获取节点pNode下子节点lpszChildName的值，如果lpszChildName为NULL，则获取节点pNode的值
	const char *GetXmlNodeValue(TiXmlNode* pNode, const char* lpszChildName/* = NULL*/);

private:
	TiXmlDocument m_doc;
};