
#include "stdafx.h"
#ifndef _TRIAL 

#include <iostream>
using namespace std;
#include "SqliteDB.h"

CSqliteDB::CSqliteDB():m_pDB(NULL)
{
}

CSqliteDB::~CSqliteDB()
{
}

bool CSqliteDB::Connect(const char * szFileName)
{
	if(m_pDB!=NULL)
		return true;

	char*errMsg = NULL;
	WCHAR buff1[1024] = {0};
	char buff2[1024] = {0};

	MultiByteToWideChar (CP_ACP, 0, szFileName, -1, buff1, sizeof(buff1));
	WideCharToMultiByte(CP_UTF8, 0, buff1, -1, buff2, sizeof(buff1), NULL, NULL);

	//打开数据库，如不存在则创建
	int rc = sqlite3_open(buff2,&m_pDB);
	if (rc)
	{
		TRACE( "Open the database: %s Error: %s\n",szFileName,sqlite3_errmsg(m_pDB) );
		m_pDB = NULL;
		return false;
	}

	//如果创建成功，添加表
	TRACE("Open the database successful!\n");
	return true;
}

bool CSqliteDB::Disconnect()
{
	if (m_pDB!=NULL)
	{
		if( sqlite3_close(m_pDB) != SQLITE_OK )
			return false;
		m_pDB = NULL;
	}
	return true;
}

bool CSqliteDB::ExecuteSQL(const char * szSql) const
{
	if ( m_pDB==NULL ){
		return false;
	}

	char*errMsg = NULL;
	int rc = sqlite3_exec(m_pDB,szSql,0,0,(char**)&errMsg);
	if (rc!= SQLITE_OK)
	{
		TRACE(errMsg);
		sqlite3_free(errMsg);
		return false;
	}
	return true;
}

int	CSqliteDB::IsExists(const char * szSql)
{
	CString strSQL;
	char*szErrorMsg = NULL;
	char**dbResult = NULL;
	int nRow = 0, nColumn = 0;

	int result = sqlite3_get_table( m_pDB, szSql, &dbResult, &nRow, &nColumn, &szErrorMsg );
	if( result==SQLITE_OK && nRow!=0 ){
	}

	if ( dbResult!=NULL ){
		sqlite3_free_table(dbResult);
	}

	return nRow;
}

bool CSqliteDB::BeginTransaction()
{
	if ( m_pDB==NULL ){
		return false;
	}

	int result;
	char*szErrorMsg = NULL;

	result = sqlite3_exec( m_pDB, "BEGIN;", 0, 0, &szErrorMsg ); //开始一个事务
	if(result!=SQLITE_OK)
	{
		TRACE(szErrorMsg);
		sqlite3_free(szErrorMsg);
		return false;
	}
	return true;
}

bool CSqliteDB::CommitTransaction()
{
	if ( m_pDB==NULL ){
		return false;
	}

	int result;
	char*szErrorMsg = NULL;

	result = sqlite3_exec( m_pDB, "COMMIT;", 0, 0, &szErrorMsg ); //提交事务
	if(result!=SQLITE_OK)
	{
		TRACE(szErrorMsg);
		sqlite3_free(szErrorMsg);
		return false;
	}
	return true;
}

bool CSqliteDB::RollBackTransaction()
{
	if ( m_pDB==NULL ){
		return false;
	}

	int result;
	char*szErrorMsg = NULL;

	result = sqlite3_exec( m_pDB, "rollback transaction", 0, 0, &szErrorMsg ); //回滚事务
	if(result!=SQLITE_OK)
	{
		TRACE(szErrorMsg);
		sqlite3_free(szErrorMsg);
		return false;
	}
	return true;
}

#endif
