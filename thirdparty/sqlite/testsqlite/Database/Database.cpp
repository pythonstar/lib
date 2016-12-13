#include "stdafx.h"
#include <iostream>
using namespace std;
#include "Database.h"



CSQLiteDB::CSQLiteDB():m_pDB(NULL)
{
	;
}


CSQLiteDB::~CSQLiteDB()
{
	;
}

bool CSQLiteDB::Connect(const char * szFileName)
{
	char *		errMsg = NULL;

	//打开数据库，如不存在则创建

	int rc = sqlite3_open(szFileName,&m_pDB);

	if (rc)
	{
		cout<<"Open the database "<<szFileName<<" failed!"<<endl;
		cout<<sqlite3_errmsg(m_pDB);
		//sqlite3_close(m_pDB);
		m_pDB = NULL;
		return false;
	}
	else			//如果创建成功，添加表
	{
		cout<<"Open the database successful!"<<endl;

	}
	return true;
}


bool CSQLiteDB::Disconnect()
{
	if (m_pDB)
	{
		if(sqlite3_close(m_pDB) != SQLITE_OK)
			return false;

		m_pDB = NULL;
	}

	return true;
}


bool CSQLiteDB::ExecSQL(const char * szSql) const
{
	char *		errMsg = NULL;
	int rc = sqlite3_exec(m_pDB,szSql,0,0,(char**)&errMsg);
	
	if (rc!= SQLITE_OK)
	{
		cout<<errMsg<<endl;
		sqlite3_free(errMsg);
		return false;
	}

	return true;
}