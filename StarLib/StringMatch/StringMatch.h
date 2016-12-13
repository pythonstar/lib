
#pragma once
#include <afx.h>

namespace Star
{
	namespace StringMatch
	{
		//�����ַ���
		int  FindingString(const char* lpszSour, const char* lpszFind, int nStart = 0);
		//��ͨ������ַ���ƥ��
		bool MatchingString(const char* lpszSour, const char* lpszMatch, bool bMatchCase = true);
		//����ƥ��
		bool MultiMatching(const char* lpszSour, const char* lpszMatch, int nMatchLogic = 0, bool bRetReversed = 0, bool bMatchCase = true);

	}
}