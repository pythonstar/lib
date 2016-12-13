
#pragma once

#include <afx.h>

class CTask
{
public:
	static BOOL CreateAutoRunTask(LPCTSTR lpszTaskName, LPCTSTR lpszAppName,
		LPCTSTR lpszParameters, LPCTSTR lpszWorkingDirectory);
	static BOOL DeleteTask(LPCTSTR lpszTaskName);
protected:
private:
};