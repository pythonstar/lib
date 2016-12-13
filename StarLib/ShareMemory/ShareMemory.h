// ShareMemory.h : header file

#pragma once

namespace Star
{

	class CShareMemory
	{
	public:
		CShareMemory(const char * pszMapName, int nFileSize = 0, BOOL bServer = FALSE);
		~CShareMemory();

		LPVOID GetBuffer() const { return  m_pBuffer; }
	private:
		HANDLE	m_hFileMap;
		LPVOID	m_pBuffer;
	};


	//////////////////////////

	inline CShareMemory::CShareMemory(const char * pszMapName, 
				int nFileSize, BOOL bServer) : m_hFileMap(NULL), m_pBuffer(NULL)
	{
		if (bServer)
		{
			m_hFileMap = CreateFileMapping(
				INVALID_HANDLE_VALUE,
				NULL,
				PAGE_READWRITE,
				0,
				nFileSize,
				pszMapName
				);
		}
		else
		{
			m_hFileMap = OpenFileMapping(
				FILE_MAP_ALL_ACCESS,
				FALSE,
				pszMapName
				);
		}
		
		m_pBuffer = (LPBYTE)MapViewOfFile(
			m_hFileMap,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			0
			);
	}

	inline CShareMemory::~CShareMemory()
	{
		UnmapViewOfFile(m_pBuffer);
		CloseHandle(m_hFileMap);
		m_pBuffer  = NULL;
		m_hFileMap = NULL;
	}

}

