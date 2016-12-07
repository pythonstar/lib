// testsqlite.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "../src/sqlite3.h"
#include "Database/Database.h"
#include <afx.h>

void test()
{
	sqlite3 * db = NULL; //声明sqlite关键结构指针

	int result;



	//打开数据库

	//需要传入 db 这个指针的指针，因为 sqlite3_open 函数要为这个指针分配内存，还要让db指针指向这个内存区

	result = sqlite3_open("E:\\VS工程\\sqlite\\Debug\\test.db", &db );

	if( result != SQLITE_OK )

	{

		//数据库打开失败

		return ;

	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	//test();

	CString strSql;
	CString strDBFile;
	char szDBFile[MAX_PATH];
	char szBuff[100]={"\0\0\0\0\0\0\01234\0\065789"};

	GetModuleFileName(NULL,szDBFile,MAX_PATH);
	strcpy(&strrchr(szDBFile,'\\')[1],"test.db");
	strDBFile=szDBFile;
	strDBFile="c:\\test.db";

	DeleteFile(strDBFile);

	CSQLiteDB db;
	db.Connect(strDBFile);
	
	strSql=
		"CREATE TABLE test("
		"id INTEGER PRIMARY KEY ASC,"
		"bin1 BLOB,"
		"bin2 BLOB"
		");";
	db.ExecSQL(strSql);

	strSql="INSERT INTO test(bin1,bin2)VALUES(?,?);";
	sqlite3_stmt*pstmt;
	sqlite3_prepare(db.m_pDB,strSql,-1,&pstmt,NULL);
	sqlite3_bind_blob(pstmt,1,szBuff,100,NULL);
	sqlite3_bind_blob(pstmt,2,szBuff,100,NULL);
	sqlite3_step(pstmt);
	sqlite3_finalize(pstmt);

	sqlite3_prepare(db.m_pDB,"SELECT * FROM test; WHERE id=1",-1,&pstmt,NULL);
	sqlite3_step(pstmt);
	const void* buff=sqlite3_column_blob(pstmt,1);
	int nlen=sqlite3_column_bytes(pstmt,1);
	sqlite3_finalize(pstmt);

	db.Disconnect();
	return 0;
}

