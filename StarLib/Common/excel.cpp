#include "common/header.h"
#include "common/excel.h"

namespace XGC
{
	namespace common
	{
		struct excel::impl
		{
			std::vector< std::vector< xgc_string > >	strTable;
			std::map< xgc_string, int >				ColumnMap;
		};

		excel::excel(void)
		: m_impl( new excel::impl )
		{
			m_nColMax	=	0;
			m_nLineMax	=	0;

			m_nCurCol		=	0;
			m_bReadString	=	FALSE;
			m_nCP			=	0;

			_tcscpy( m_szUnit, _T("") );
		}

		excel::~excel(void)
		{
			SAFE_DELETE(m_impl); 
		}

		bool excel::ReadExcelFile( xgc_lpctstr strFileName, int nTitleLine )
		{
			FILE* fp;
			if( ( fp = _tfopen( strFileName, _T("r") ) ) == NULL )
			{
				return FALSE;
			}
			xgc_tchar buffer[1024];
			int nLine = 0;
			while( !feof( fp ) )
			{
				buffer[0] = 0;
				_fgetts( buffer, sizeof( buffer ), fp );
				if( !AnalyzeLine( buffer, sizeof( buffer ) ) )
				{
					DestoryTable();
					return false;
				}

				if( nTitleLine == nLine )
				{
					--m_nLineMax;
					std::vector< xgc_string >& line = m_impl->strTable[m_nLineMax];
					for( size_t n = 0; n < line.size(); ++n )
					{
						xgc_string &idx = line[n];
						if( !idx.empty() )
						{
							m_impl->ColumnMap[idx] = n;
						}
					}
					// m_impl->strTable.erase( m_impl->strTable.begin() + m_nLineMax );
				}
				++nLine;
			}
			fclose( fp );
			return TRUE;
		}

		bool excel::AnalyzeLine( xgc_lpctstr pstrLine, int nSize )
		{
			if( !pstrLine || nSize == 0)	return false;
			int i = 0;

			int n = 0;
			do
			{
				ASSERT( i < nSize );
				if( i > nSize )
					return false;

				ASSERT( n < sizeof( m_szUnit ) );
				if( n >= sizeof( m_szUnit ) )	
					return false;

				switch( pstrLine[i] )
				{
					case 0:
					case _T('\t'):
					case _T('\n'):
					{
						if( !m_bReadString && i > 0 )
						{
							m_szUnit[m_nCP] = 0;
							m_nCP = 0;
							AddElement( m_nLineMax, m_nCurCol++, xgc_string( m_szUnit ) );
						}
						else
						{
							m_szUnit[m_nCP++] = pstrLine[i];
						}
					}
					break;
					case _T('"'):
					{
						m_bReadString = !m_bReadString;
					}
					break;
					default:
					{
						m_szUnit[m_nCP++] = pstrLine[i];
					}
				}
			}while( pstrLine[i++] != 0 );
			// 该行结束的时候，如果不是在读取字符串状态，开始新的一条记录。
			if( !m_bReadString && m_nCurCol != 0 )
			{
				m_nLineMax++;
				m_nCurCol = 0;
			}
			return true;
		}

		bool excel::AddElement( int nLine, int nCol, xgc_string& Value )
		{
			if( nLine > m_nLineMax )	m_nLineMax = nLine;
			if( nCol > m_nColMax )	m_nColMax = nCol;

			int nLineSizeMax = ( int )m_impl->strTable.size();
			if( nLine >= nLineSizeMax )	m_impl->strTable.resize( nLineSizeMax + 100, std::vector< xgc_string >( 64, _T("") ) );

			int nColSizeMax = ( int ) m_impl->strTable[nLine].size();
			if( nCol >= nColSizeMax )	m_impl->strTable[nLine].resize( nColSizeMax + 32, _T("") );

			try
			{
				m_impl->strTable[nLine][nCol] = Value;
			}
			catch( ... )
			{
				ASSERT( FALSE );
				return FALSE;
			}
			return TRUE;
		}

		bool excel::GetElement( int nLine, int nCol, xgc_lpctstr* pstrValue )
		{
			xgc_string const* pstring = NULL;
			if( !GetElement( nLine, nCol, &pstring ) )	return FALSE;

			*pstrValue = pstring->c_str();
			return true;
		}

		bool excel::GetElement( int nLine, int nCol, const xgc_string **pstrValue )
		{
			if( m_impl->strTable.size() <= ( size_t )nLine )
				return false;

			if( m_impl->strTable[nLine].size() <= ( size_t )nCol )	
				return false;

			*pstrValue = &m_impl->strTable[nLine][nCol];
			if( (*pstrValue)->empty() )
				return false;
			return true;
		}

		bool excel::GetElement( int nLine, int nCol, xgc_string &strValue )
		{
			if( m_impl->strTable.size() <= ( size_t )nLine )
				return false;

			if( m_impl->strTable[nLine].size() <= ( size_t )nCol )	
				return false;

			strValue = m_impl->strTable[nLine][nCol];
			if( strValue.empty() )
				return false;
			return true;
		}

		bool excel::GetElement( int nLine, xgc_lpctstr lpszTitle, const xgc_string **pstrValue )
		{
			if( lpszTitle == NULL )
				return false;

			std::map< xgc_string, int >::iterator idx = m_impl->ColumnMap.find( lpszTitle );
			if( idx != m_impl->ColumnMap.end() )
			{
				return GetElement( nLine, idx->second, pstrValue );
			}

			return false;
		}

		bool excel::GetElement( int nLine, xgc_lpctstr lpszTitle, xgc_lpctstr *pstrValue )
		{
			if( lpszTitle == NULL )
				return false;

			std::map< xgc_string, int >::iterator idx = m_impl->ColumnMap.find( lpszTitle );
			if( idx != m_impl->ColumnMap.end() )
			{
				return GetElement( nLine, idx->second, pstrValue );
			}

			return false;
		}

		bool excel::GetElement( int nLine, xgc_lpctstr lpszTitle, xgc_string &strValue )
		{
			if( lpszTitle == NULL )
				return false;

			std::map< xgc_string, int >::iterator idx = m_impl->ColumnMap.find( lpszTitle );
			if( idx != m_impl->ColumnMap.end() )
			{
				return GetElement( nLine, idx->second, strValue );
			}

			return false;
		}

		float excel::GetElement( int nLine, xgc_lpctstr lpszTitle, float *pFloatVal, float fDefault )
		{
			xgc_lpctstr pValue = NULL;
			float fValue;
			if( GetElement( nLine, lpszTitle, &pValue ) )
			{
				_stscanf( pValue, _T("%f"), &fValue );
				if( pFloatVal )
					*pFloatVal = fValue;
			}
			else if( pFloatVal )
			{
				fValue = *pFloatVal = fDefault;
			}
			return fValue;
		}

		int excel::GetElement( int nLine, xgc_lpctstr lpszTitle, int *pIntVal, int nDefault )
		{
			xgc_lpctstr pValue = NULL;
			int nValue;
			if( GetElement( nLine, lpszTitle, &pValue ) )
			{
				_stscanf( pValue, _T("%d"), &nValue );
				if( pIntVal )
					*pIntVal = nValue;
			}
			else if( pIntVal )
			{
				nValue = *pIntVal = nDefault;
			}
			return nValue;
		}

		short excel::GetElement( int nLine, xgc_lpctstr lpszTitle, short *pShortVal, short nDefault )
		{
			xgc_lpctstr pValue = NULL;
			int nValue;
			if( GetElement( nLine, lpszTitle, &pValue ) )
			{
				_stscanf( pValue, _T("%d"), &nValue );
				if( pShortVal )
					*pShortVal = (short)nValue;
			}
			else if( pShortVal )
			{
				nValue = *pShortVal = nDefault;
			}
			return (short)nValue;
		}

		int excel::GetLineElementCount( int nLine )
		{	
			return ( int )m_impl->strTable[nLine].size(); 
		}

		void excel::DestoryTable()
		{
			m_impl->strTable.clear();
			m_nLineMax = 0;
			m_nColMax = 0;
		}
	}
}