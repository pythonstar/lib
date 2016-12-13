#pragma  once
#include "../../src/sqlite3.h"

class CSQLiteDB
{
public:
	CSQLiteDB();
	~CSQLiteDB();
public:
	bool Connect(const char * szFileName);
	bool Disconnect();
	bool ExecSQL(const char * szSql) const;
public:		
	sqlite3	*	m_pDB;
};