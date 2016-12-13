
#pragma once
#include <afx.h>

namespace Star
{
	namespace StringMatch
	{
		//查找字符串
		int  FindingString(const char* lpszSour, const char* lpszFind, int nStart = 0);
		//带通配符的字符串匹配
		bool MatchingString(const char* lpszSour, const char* lpszMatch, bool bMatchCase = true);
		//多重匹配
		bool MultiMatching(const char* lpszSour, const char* lpszMatch, int nMatchLogic = 0, bool bRetReversed = 0, bool bMatchCase = true);

	}
}