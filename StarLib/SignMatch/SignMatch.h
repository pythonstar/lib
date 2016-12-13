#pragma once
#include <Psapi.h>
#pragma comment(lib,"Psapi.lib")


class CSignMatch
{
public:
	enum{MinSignLen=1,MaxSignLen=0x100,MaxScanRange=0x10000};
	//用户指定的特征码字符串
	char m_szOrigSign[MaxSignLen];	
	//m_strOrigSign一次处理后的特征码,主要将?变换为0
	BYTE*	m_pPureSign;	
	//特征码的掩码,用以完成模糊匹配
	BYTE*	m_pMaskSign;
	//特征码长度
	int		m_nSignLen;

	//搜索基地址
	PBYTE	m_pBase;

	//搜索的起始rva
	DWORD	m_dwSearchBegin;

	//搜索的结束rva
	DWORD	m_dwSearchEnd;

	int		m_nRange;
	DWORD	m_dwMatchPos;

private:
	void	FreeMemory();
	BYTE	HexCharToBin(BYTE chHex);
	bool	m_bFuzzy;	//是否是模糊匹配

public:
	CSignMatch();
	CSignMatch(const CSignMatch&obj);
	CSignMatch(PBYTE pBase,DWORD dwStartRva,DWORD dwEndRva,char*szOrigSign);
	virtual ~CSignMatch();

	BOOL	SetOrigSign(PBYTE pBase,DWORD dwStartRva,DWORD dwEndRva,char*szOrigSign);

	//如果pBase为NULL，则使用SetOrigSign设置的pBase，否则使用参数pBase
	PBYTE	SearchSign(PBYTE pBase=NULL);
	PBYTE	SearchProcessSign(HANDLE hProcess,PBYTE pImageBase,DWORD dwImageSize);
};