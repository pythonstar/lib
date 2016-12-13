#pragma once
#ifdef EXPORT_API
#define EXPORT_LIB __declspec(dllexport)
#else
#define EXPORT_LIB __declspec(dllimport)
#endif

#include <vector>

namespace XGC
{
	namespace common
	{

		#define EXCEL_BUFFER_SIZE	512
		class EXPORT_LIB excel
		{
		public:
			excel(void);
			~excel(void);
		public:
			//	读取指定的表格文件
			//	strFileName	:	文件名
			bool ReadExcelFile( xgc_lpctstr strFileName, int nTitleLine = 0 );

			bool GetElement( int nLine, int nCol, const xgc_string **pstrValue );
			bool GetElement( int nLine, int nCol, xgc_lpctstr *pstrValue );
			bool GetElement( int nLine, int nCol, xgc_string &strValue );

			bool GetElement( int nLine, xgc_lpctstr lpszTitle, const xgc_string **pstrValue );
			bool GetElement( int nLine, xgc_lpctstr lpszTitle, xgc_lpctstr *pstrValue );
			bool GetElement( int nLine, xgc_lpctstr lpszTitle, xgc_string &strValue );

			float	GetElement( int nLine, xgc_lpctstr lpszTitle, float *pFloatVal, float fDefault );
			int		GetElement( int nLine, xgc_lpctstr lpszTitle, int *pIntVal, int nDefault );
			short	GetElement( int nLine, xgc_lpctstr lpszTitle, short *pShortVal, short nDefault );


			int	 GetLineElementCount( int nLine );

			int	 GetColMax(){	return m_nColMax; }
			int	 GetLineMax(){	return m_nLineMax; }
		protected:
			void DestoryTable();

			//	分析读取的一行数据
			bool AnalyzeLine( xgc_lpctstr pstrLine, int nSize );

			//	添加一个元素到指定位置。
			bool AddElement( int nLine, int nCol, xgc_string& Value );

			struct impl;
			impl* m_impl;

			int		m_nColMax;
			int		m_nLineMax;

			int		m_nCurCol;
			bool	m_bReadString;
			xgc_tchar	m_szUnit[EXCEL_BUFFER_SIZE];
			int		m_nCP;

		};
	}
}