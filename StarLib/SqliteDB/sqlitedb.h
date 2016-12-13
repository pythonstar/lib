
#pragma  once
#ifndef _TRIAL 

#include <afx.h>
#include <sqlite/src/sqlite3.h>

class CSqliteDB
{
public:
	CSqliteDB();
	~CSqliteDB();
public:
	bool Connect(const char * szFileName);
	bool Disconnect();
	bool ExecuteSQL(const char * szSql) const;
	bool BeginTransaction();
	bool CommitTransaction();
	bool RollBackTransaction();
	int	IsExists(const char * szSql);

public:		
	sqlite3*m_pDB;
};

#endif