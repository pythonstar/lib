
#include "stdafx.h"
#include "SignMatch.h"
#include <TlHelp32.h>

CSignMatch::CSignMatch()
{
	memset(m_szOrigSign,0,CSignMatch::MaxSignLen);
	m_pPureSign=NULL;
	m_pMaskSign=NULL;
	m_nSignLen=0;
	m_dwSearchBegin=0;
	m_dwSearchEnd=0;
	m_dwMatchPos=0;
	m_bFuzzy=false;
}

CSignMatch::CSignMatch(PBYTE pBase,DWORD dwStartRva,DWORD dwEndRva,char*szOrigSign)
{
	memset(m_szOrigSign,0,CSignMatch::MaxSignLen);
	m_pPureSign=NULL;
	m_pMaskSign=NULL;
	m_nSignLen=0;
	m_dwSearchBegin=0;
	m_dwSearchEnd=0;
	m_dwMatchPos=0;
	m_bFuzzy=false;

	SetOrigSign(pBase,dwStartRva,dwEndRva,szOrigSign);
}

CSignMatch::CSignMatch(const CSignMatch&obj)
{
	memcpy(m_szOrigSign,obj.m_szOrigSign,CSignMatch::MaxSignLen);
	m_nSignLen=obj.m_nSignLen;
	m_pPureSign=new BYTE[m_nSignLen];
	m_pMaskSign=new BYTE[m_nSignLen];
	memcpy(m_pPureSign,obj.m_pPureSign,m_nSignLen);
	memcpy(m_pMaskSign,obj.m_pMaskSign,m_nSignLen);

	m_pBase=obj.m_pBase;
	m_dwSearchBegin=obj.m_dwSearchBegin;
	m_dwSearchEnd=obj.m_dwSearchEnd;
	m_nRange=obj.m_nRange;
	m_dwMatchPos=obj.m_dwMatchPos;
	m_bFuzzy=obj.m_bFuzzy;
}

CSignMatch::~CSignMatch()
{
	FreeMemory();
}

void CSignMatch::FreeMemory()
{
	if (m_pPureSign){
		delete[] m_pPureSign;
		m_pPureSign=NULL;
	}
	
	if (m_pMaskSign){
		delete[] m_pMaskSign;
		m_pMaskSign=NULL;
	}
}

BYTE CSignMatch::HexCharToBin(BYTE chHex)
{
	if ( chHex>='0' && chHex<='9' ){
		return chHex-'0';
	}else{
		return chHex-'a'+10;
	}
	return 0;
}

/*------------------------------------------------------------------------
说明:   接受16进制的特征码字符串,((orig or mask) xor pure) ==0
------------------------------------------------------------------------*/
BOOL CSignMatch::SetOrigSign(PBYTE pBase,DWORD dwStartRva,DWORD dwEndRva,char*szOrigSign)
{
	m_bFuzzy=false;			//默认不通配
	m_pBase=pBase;
	m_dwSearchBegin=dwStartRva;
	m_dwSearchEnd=dwEndRva;
	m_dwMatchPos=0;
	m_nRange=dwEndRva-dwStartRva;

	int nStrLen=(int)strlen(szOrigSign);
	//太短太长都不行
	if ( nStrLen<CSignMatch::MinSignLen || nStrLen>CSignMatch::MaxSignLen ){
		//MessageBox(NULL,"特征码设置不合理,长度限制在5~100字节.",NULL,MB_OK);
		return false;
	}

	if ( nStrLen%2!=0 ){
		//奇数长度,则尾补一个?构成一个字节
		memcpy(m_szOrigSign,szOrigSign,nStrLen);
		nStrLen++;
		if ( nStrLen<CSignMatch::MaxSignLen ){
			m_szOrigSign[nStrLen]='?';
		}
	}else{
		memcpy(m_szOrigSign,szOrigSign,nStrLen);
	}
	m_nSignLen=nStrLen/2;	//字节长度

	FreeMemory();

	m_pPureSign=new BYTE[m_nSignLen];
	m_pMaskSign=new BYTE[m_nSignLen];

	BYTE ch;
	for ( int i=0; i<m_nSignLen; ++i ){
		//一个字符只占4位(半个字节),处理第一个半字节
		ch=m_szOrigSign[i*2];
		if ( ch=='?' ){
			m_pPureSign[i]=0xf0;
			m_pMaskSign[i]=0xf0;
			m_bFuzzy=true;		//含有?表示通配
		}else{
			m_pPureSign[i]=(HexCharToBin(ch)<<4)&0xf0;
			m_pMaskSign[i]=0;
		}

		//处理第二个半字节
		ch=m_szOrigSign[i*2+1];
		if ( ch=='?' ){
			m_pPureSign[i]|=0x0f;
			m_pMaskSign[i]|=0x0f;
			m_bFuzzy=true;		//含有?表示通配
		}else{
			m_pPureSign[i]|= HexCharToBin(ch)&0x0f;
			m_pMaskSign[i]&=0xf0;
		}
	}//end for

	return true;
}

PBYTE CSignMatch::SearchSign(PBYTE pBase/*=NULL*/)
{
	if ( pBase!=NULL ){
		m_pBase=pBase;
	}

	PBYTE pMatchAddr=NULL;
	DWORD dwSearchBegin = 0;
	PBYTE pStart=m_pBase+dwSearchBegin;
	if (m_nRange<m_nSignLen){
		return NULL;
	}

	//TRACE("m_nSignLen:%d m_nRange:%d\n",m_nSignLen,m_nRange);
	//TRACE("pStart:%d\n",(DWORD)pStart);

	__try{
		if ( m_bFuzzy ){
			for ( int i=0, j=0; i<(int)m_nRange; ++i ){
				if( m_pPureSign[j]!=(pStart[i]|m_pMaskSign[j]) ){
					j=0;
				}else{
					++j;
					if ( j==m_nSignLen ){
						pMatchAddr=m_pBase+dwSearchBegin+i-m_nSignLen+1;
						m_dwMatchPos=dwSearchBegin+i-m_nSignLen+1;
						break;
					}
				}
			}//end for
		}else{
			for ( int i=0, j=0; i<(int)m_nRange; ++i ){
				if( m_pPureSign[j]!=pStart[i] ){
					j=0;
				}else{
					++j;
					if ( j==m_nSignLen ){
						pMatchAddr=m_pBase+dwSearchBegin+i-m_nSignLen+1;
						m_dwMatchPos=dwSearchBegin+i-m_nSignLen+1;
						break;
					}
				}

			}//end for
		}

	}__except(1){
		pMatchAddr=NULL;
	}

	return pMatchAddr;
}

//win7下调用GetModuleInformation有问题，这里让外部传入基址和大小
PBYTE CSignMatch::SearchProcessSign(HANDLE hProcess,PBYTE pImageBase,DWORD dwImageSize)
{
	PBYTE pMatchAddr=NULL;
	DWORD dwReaded=0;
	MODULEINFO mi={0};
	BYTE*pBuffer=NULL;
	PBYTE pRealBase = pImageBase;

	if ( pRealBase==NULL ){
		return NULL;
	}

	//不能越界
	if ( m_dwSearchBegin>=dwImageSize || m_dwSearchEnd>dwImageSize || m_nRange<m_nSignLen ){
		return NULL;
	}

	//开辟内存
	pBuffer=new BYTE[m_nRange];
	if ( pBuffer==NULL ){
		return NULL;
	}

	if( ReadProcessMemory(hProcess,(LPCVOID)(pRealBase+m_dwSearchBegin),pBuffer,m_nRange,&dwReaded) ){
		PBYTE pStart=pBuffer;
		
		__try{
			if ( m_bFuzzy ){
				for ( int i=0, j=0; i<(int)m_nRange; ++i ){
					if( m_pPureSign[j]!=(pStart[i]|m_pMaskSign[j]) ){
						j=0;
					}else{
						++j;
						if ( j==m_nSignLen ){
							pMatchAddr=pRealBase+m_dwSearchBegin+i-m_nSignLen+1;
							m_dwMatchPos=m_dwSearchBegin+i-m_nSignLen+1;
							break;
						}
					}
				}//end for
			}else{
				for ( int i=0, j=0; i<(int)m_nRange; ++i ){
					if( m_pPureSign[j]!=pStart[i] ){
						j=0;
					}else{
						++j;
						if ( j==m_nSignLen ){
							pMatchAddr=pRealBase+m_dwSearchBegin+i-m_nSignLen+1;
							m_dwMatchPos=m_dwSearchBegin+i-m_nSignLen+1;
							break;
						}
					}
					
				}//end for
			}

		}__except(1){
			pMatchAddr=NULL;
		}
	}

	delete[] pBuffer;

	return pMatchAddr;
}
