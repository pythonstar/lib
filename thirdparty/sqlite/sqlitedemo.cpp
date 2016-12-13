// sqlitedemo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../sqlite3_lib/sqlite3.h"

#pragma comment(lib, "../sqlite3_lib/sqlite3.lib")

static int _sql_callback(void * notused, int argc, char ** argv, char ** szColName)
{
    int i;
    for ( i=0; i < argc; i++ )
    {
        printf( "%s = %s\n", szColName[i], argv[i] == 0 ? "NUL" : argv[i] );
    }
	
    return 0;
}

int main(int argc, char * argv[])
{
	const char * sSQL1 = "create table users(userid varchar(20) PRIMARY KEY, age int, birthday datetime);";
	const char * sSQL2 = "insert into users values('wang',20,'1989-5-4');";
    const char * sSQL3 = "select * from users;";

    sqlite3 * db = 0;
    char * pErrMsg = 0;
    int ret = 0;
    
	// �������ݿ�
    ret = sqlite3_open("./test.db", &db);
	
    if ( ret != SQLITE_OK )
    {
        fprintf(stderr, "�޷������ݿ�: %s", sqlite3_errmsg(db));
        return(1);
    }
	
    printf("���ݿ����ӳɹ�!\n");
	
	// ִ�н���SQL

    sqlite3_exec( db, sSQL1, 0, 0, &pErrMsg );
    if ( ret != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", pErrMsg);
        sqlite3_free(pErrMsg);
    }
	
	// ִ�в����¼SQL
	sqlite3_exec( db, sSQL2, 0, 0, &pErrMsg);

	// ��ѯ���ݱ�
	sqlite3_exec( db, sSQL3, _sql_callback, 0, &pErrMsg);

	// �ر����ݿ�
    sqlite3_close(db);
    db = 0;
	
    return 0;
}
