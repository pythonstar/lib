// DateHelper.cpp: implementation of the CDateHelper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DateHelper.h"
#include "TimeHelper.h"
#include "misc.h"

#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

int CDateHelper::CalcDaysFromTo(const COleDateTime& dateFrom, const COleDateTime& dateTo, BOOL bInclusive)
{
	int nDiff = (int)(floor(dateTo) - floor(dateFrom));

	return nDiff + (bInclusive ? 1 : 0);
}

int CDateHelper::CalcDaysFromTo(const COleDateTime& dateFrom, DH_DATE nTo, BOOL bInclusive)
{
	return CalcDaysFromTo(dateFrom, GetDate(nTo), bInclusive);
}

int CDateHelper::CalcDaysFromTo(DH_DATE nFrom, DH_DATE nTo, BOOL bInclusive)
{
	ASSERT (nFrom <= nTo);

	if (nFrom > nTo)
		return 0;
	
	else if (nFrom == nTo)
		return bInclusive ? 1 : 0;

	// else
	return CalcDaysFromTo(GetDate(nFrom), GetDate(nTo), bInclusive);
}

BOOL CDateHelper::DecodeISODate(const CString& sDate, COleDateTime& date)
{
	int nYear = -1, nMonth = -1, nDay = -1;

#if _MSC_VER >= 1400
	if (sscanf_s(sDate, _T("%4d-%2d-%2d"), &nYear, &nMonth, &nDay) == 3)
#else
	if (_stscanf(sDate, _T("%4d-%2d-%2d"), &nYear, &nMonth, &nDay) == 3)
#endif
	{
		return (date.SetDate(nYear, nMonth, nDay) == 0);
	}
			
	return FALSE;
}

BOOL CDateHelper::DecodeISODate(const CString& sDate, time_t& date)
{
	COleDateTime dt;

	if (DecodeISODate(sDate, dt))
	{
		tm time = { 0, 
					0,
					0, 
					dt.GetDay(),
					dt.GetMonth() - 1, 
					dt.GetYear() - 1900, 
					dt.GetDayOfWeek() - 1, 
					dt.GetDayOfYear(), 
					-1 };

		date = mktime(&time);
		return (date != -1);
	}

	return FALSE;
}

BOOL CDateHelper::DecodeLocalShortDate(const CString& sDate, COleDateTime& date)
{
	if (!sDate.IsEmpty())
	{
		// split the string and format by the delimiter
		CString sFormat = Misc::GetShortDateFormat();
		CStringArray aDateParts, aFmtParts;

		Misc::Split(sDate, aDateParts, TRUE, Misc::GetDateSeparator());
		Misc::Split(sFormat, aFmtParts, TRUE, Misc::GetDateSeparator());

		ASSERT (aDateParts.GetSize() == aFmtParts.GetSize());

		// process the parts, deciphering the format
		int nYear = -1, nMonth = -1, nDay = -1;

		for (int nPart = 0; nPart < aFmtParts.GetSize(); nPart++)
		{
			if (aFmtParts[nPart].FindOneOf(_T("Yy")) != -1)
				nYear = _ttoi(aDateParts[nPart]);

			if (aFmtParts[nPart].FindOneOf(_T("Mm")) != -1)
				nMonth = _ttoi(aDateParts[nPart]);

			if (aFmtParts[nPart].FindOneOf(_T("Dd")) != -1)
				nDay = _ttoi(aDateParts[nPart]);
		}

		if (nYear != -1 && nMonth != -1 && nDay != -1)
			return (date.SetDate(nYear, nMonth, nDay) == 0);
	}

	// else
	date.m_dt = 0.0;
	return FALSE;
}

BOOL CDateHelper::DecodeLocalShortDate(const CString& sDate, time_t& date)
{
	COleDateTime dt;

	if (DecodeLocalShortDate(sDate, dt))
	{
		tm time = { 0, 
					0,
					0, 
					dt.GetDay(),
					dt.GetMonth() - 1, 
					dt.GetYear() - 1900, 
					dt.GetDayOfWeek() - 1, 
					dt.GetDayOfYear(), 
					-1 };

		date = mktime(&time);
		return (date != -1);
	}

	// else
	date = 0;
	return FALSE;
}

double CDateHelper::GetDate(DH_DATE nDate)
{
	COleDateTime date;

	switch (nDate)
	{
	case DHD_TODAY:
		date = COleDateTime::GetCurrentTime();
		break;

	case DHD_TOMORROW:
		date = GetDate(DHD_TODAY) + 1;
		break;

	case DHD_ENDTHISWEEK:
		{
			// we must get the locale info to find out when this 
			// user's week starts
			date = COleDateTime::GetCurrentTime();
			
			// increment the date until we hit the last day of the week
			// note: we could have kept checking date.GetDayOfWeek but
			// it's a lot of calculation that's just not necessary
			int nLastDOW = LastDayOfWeek();
			int nDOW = date.GetDayOfWeek();
			
			while (nDOW != nLastDOW) 
			{
				date += 1;
				nDOW = NextDayOfWeek(nDOW);
			}
		}
		break;

	case DHD_ENDNEXTWEEK:
		return GetDate(DHD_ENDTHISWEEK) + 7;

	case DHD_ENDTHISMONTH:
		{
			date = COleDateTime::GetCurrentTime();
			int nThisMonth = date.GetMonth();

			while (date.GetMonth() == nThisMonth)
				date += 20; // much quicker than doing it one day at a time

			date -= date.GetDay(); // because we went into next month
		}
		break;

	case DHD_ENDNEXTMONTH:
		{
			date = GetDate(DHD_ENDTHISMONTH) + 1; // first day of next month
			int nNextMonth = date.GetMonth();

			while (date.GetMonth() == nNextMonth)
				date += 20; // much quicker than doing it one day at a time

			date -= date.GetDay(); // because we went into next month + 1
		}
		break;

	case DHD_ENDTHISYEAR:
		date = COleDateTime::GetCurrentTime(); // for current year
		date = COleDateTime(date.GetYear(), 12, 31, 0, 0, 0);
		break;

	case DHD_ENDNEXTYEAR:
		date = COleDateTime::GetCurrentTime(); // for current year
		date = COleDateTime(date.GetYear() + 1, 12, 31, 0, 0, 0);
		break;

	default:
		ASSERT (0);
		date.m_dt = 0;
		break;
	}

	return floor(date);
}

int CDateHelper::CalcWeekdaysFromTo(const COleDateTime& dateFrom, const COleDateTime& dateTo, BOOL bInclusive)
{
	int nWeekdays = 0;
	
	COleDateTime dFrom(floor(dateFrom.m_dt));
	COleDateTime dTo(floor(dateTo.m_dt));

	if (bInclusive)
		dTo += 1;

	int nDiff = (int)(double)(dTo - dFrom);

	if (nDiff > 0)
	{
		while (dFrom < dTo)
		{
			int nDOW = dFrom.GetDayOfWeek();

			if (nDOW != 1 && nDOW != 7)
				nWeekdays++;

			dFrom += 1;
		}
	}

	return nWeekdays;
}

int CDateHelper::FirstDayOfWeek()
{
	TCHAR szFDW[3]; // 2 + NULL

	::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IFIRSTDAYOFWEEK, szFDW, 2);

	int nFirstDOW = _ttoi(szFDW);

	// we need days to have same order as COleDateTime::GetDayOfWeek()
	// which is 1 (sun) - 7 (sat)
	switch (nFirstDOW)
	{
	case 0: /* mon */ return 2;
	case 1: /* tue */ return 3;
	case 2: /* wed */ return 4;
	case 3: /* thu */ return 5;
	case 4: /* fri */ return 6;
	case 5: /* sat */ return 7;
	case 6: /* sun */ return 1;
	}

	ASSERT (0);
	return 1;
}

int CDateHelper::LastDayOfWeek()
{
	switch (FirstDayOfWeek())
	{
	case 2: /* mon */ return 1; // sun
	case 3: /* tue */ return 2; // mon
	case 4: /* wed */ return 3; // tue
	case 5: /* thu */ return 4; // wed
	case 6: /* fri */ return 5; // thu
	case 7: /* sat */ return 6; // fri
	case 1: /* sun */ return 7; // sat
	}

	ASSERT (0);
	return 1;
}

int CDateHelper::NextDayOfWeek(int nDOW)
{
	switch (nDOW)
	{
	case 2: /* mon */ return 3; // tue
	case 3: /* tue */ return 4; // wed
	case 4: /* wed */ return 5; // thu
	case 5: /* thu */ return 6; // fri
	case 6: /* fri */ return 7; // sat
	case 7: /* sat */ return 1; // sun
	case 1: /* sun */ return 2; // mon
	}

	ASSERT (0);
	return 1;
}

CString CDateHelper::FormatDate(const COleDateTime& date, DWORD dwFlags)
{
	SYSTEMTIME st;

	if (!date.GetAsSystemTime(st))
		return "";

	CString sFormat;

	if (dwFlags & DHFD_ISO)
	{
		sFormat = "yyyy-MM-dd";

		if (dwFlags & DHFD_DOW)
			sFormat = "ddd " + sFormat;
	}
	else
		sFormat = Misc::GetShortDateFormat(dwFlags & DHFD_DOW);

	CString sDate;
	::GetDateFormat(0, 0, &st, sFormat, sDate.GetBuffer(50), 49);
	sDate.ReleaseBuffer();

	// want time?
	if (dwFlags & DHFD_TIME)
	{
		if (dwFlags & DHFD_ISO)
		{
			sDate += 'T'; // ISO delimiter
			sDate += CTimeHelper::FormatISOTime(st.wHour, st.wMinute, st.wSecond, !(dwFlags & DHFD_NOSEC));
		}
		else 
		{
			sDate += ' ';
			sDate += CTimeHelper::Format24HourTime(st.wHour, st.wMinute, st.wSecond, !(dwFlags & DHFD_NOSEC));
		}
	}
	
	return sDate;
}

CString CDateHelper::FormatCurrentDate(DWORD dwFlags)
{
	return FormatDate(COleDateTime::GetCurrentTime(), dwFlags);
}

CString CDateHelper::GetWeekday(int nWeekday, BOOL bShort)
{
	LCTYPE lct = bShort ? LOCALE_SABBREVDAYNAME1 : LOCALE_SDAYNAME1;
	CString sWeekday;

	// data check
	if (nWeekday < 1 || nWeekday> 7)
		return "";

	switch (nWeekday)
	{
	case 1: lct = bShort ? LOCALE_SABBREVDAYNAME7 : LOCALE_SDAYNAME7; break; // sun
	case 2:	lct = bShort ? LOCALE_SABBREVDAYNAME1 : LOCALE_SDAYNAME1; break; // mon
	case 3:	lct = bShort ? LOCALE_SABBREVDAYNAME2 : LOCALE_SDAYNAME2; break; // tue
	case 4:	lct = bShort ? LOCALE_SABBREVDAYNAME3 : LOCALE_SDAYNAME3; break; // wed
	case 5:	lct = bShort ? LOCALE_SABBREVDAYNAME4 : LOCALE_SDAYNAME4; break; // thu
	case 6:	lct = bShort ? LOCALE_SABBREVDAYNAME5 : LOCALE_SDAYNAME5; break; // fri
	case 7:	lct = bShort ? LOCALE_SABBREVDAYNAME6 : LOCALE_SDAYNAME6; break; // sat
	}
	
	GetLocaleInfo(LOCALE_USER_DEFAULT, lct, sWeekday.GetBuffer(30),	29);
	sWeekday.ReleaseBuffer();

	return sWeekday;
}

int CDateHelper::CalcLongestWeekday(CDC* pDC, BOOL bShort)
{
	int nLongestWDWidth = 0;
		
	// figure out the longest day in pixels
	for (int nWD = 1; nWD <= 7; nWD++)
	{
		int nWDWidth = pDC->GetTextExtent(GetWeekday(nWD, bShort)).cx;
		nLongestWDWidth = max(nLongestWDWidth, nWDWidth);
	}
	
	return nLongestWDWidth;
}

void CDateHelper::OffsetDate(COleDateTime& date, int nAmount, DH_UNITS nUnits)
{
	if (date.m_dt > 0)
	{
		switch (nUnits)
		{
		case DHU_DAYS:
			date += (double)nAmount;
			break;

		case DHU_WEEKS:
			date += nAmount * 7.0;
			break;

		case DHU_MONTHS:
			{
				SYSTEMTIME st;
				date.GetAsSystemTime(st);

				// convert amount to years and months
				st.wYear = (WORD)((int)st.wYear + (nAmount / 12));
				st.wMonth = (WORD)((int)st.wMonth + (nAmount % 12));

				// handle overflow
				if (st.wMonth > 12)
				{
					st.wYear++;
					st.wMonth -= 12;
				}
				else if (st.wMonth < 1)
				{
					st.wYear--;
					st.wMonth += 12;
				}

				// clip dates to the end of the month
				st.wDay = min(st.wDay, (WORD)GetDaysInMonth(st.wMonth, st.wYear));

				// update time
				date = COleDateTime(st);
			}
			break;

		case DHU_YEARS:
			{
				SYSTEMTIME st;
				date.GetAsSystemTime(st);

				// update year
				st.wYear = (WORD)((int)st.wYear + nAmount);

				// update time
				date = COleDateTime(st);
			}
			break;
		}
	}
}

void CDateHelper::GetWeekdays(BOOL bShort, CStringArray& aWeekDays)
{
	aWeekDays.RemoveAll();

	for (int nDay = 1; nDay <= 7; nDay++)
		aWeekDays.Add(GetWeekday(nDay, bShort));
}

int CDateHelper::GetDaysInMonth(int nMonth, int nYear)
{
	// data check
	if (nMonth < 1 || nMonth> 12)
		return 0;

	switch (nMonth)
	{
	case 1:  return 31; // jan
	case 2:  return IsLeapYear(nYear) ? 29 : 28; // feb
	case 3:  return 31; // mar
	case 4:  return 30; // apr
	case 5:  return 31; // may
	case 6:  return 30; // jun
	case 7:  return 31; // jul
	case 8:  return 31; // aug
	case 9:  return 30; // sep
	case 10: return 31; // oct
	case 11: return 30; // nov
	case 12: return 31; // dec
	}

	ASSERT(0);
	return 0;
}

BOOL CDateHelper::IsLeapYear(const COleDateTime& date)
{
	return IsLeapYear(date.GetYear());
}

BOOL CDateHelper::IsLeapYear(int nYear)
{
	return ((nYear % 4 == 0) && ((nYear % 100 != 0) || (nYear % 400 == 0)));
}

CString CDateHelper::GetMonth(int nMonth, BOOL bShort)
{
	LCTYPE lct = LOCALE_SABBREVMONTHNAME1;
	CString sMonth;

	// data check
	if (nMonth < 1 || nMonth> 12)
		return "";

	switch (nMonth)
	{
	case 1:  lct = bShort ? LOCALE_SABBREVMONTHNAME1  : LOCALE_SMONTHNAME1;  break; // jan
	case 2:  lct = bShort ? LOCALE_SABBREVMONTHNAME2  : LOCALE_SMONTHNAME2;  break; // feb
	case 3:  lct = bShort ? LOCALE_SABBREVMONTHNAME3  : LOCALE_SMONTHNAME3;  break; // mar
	case 4:  lct = bShort ? LOCALE_SABBREVMONTHNAME4  : LOCALE_SMONTHNAME4;  break; // apr
	case 5:  lct = bShort ? LOCALE_SABBREVMONTHNAME5  : LOCALE_SMONTHNAME5;  break; // may
	case 6:  lct = bShort ? LOCALE_SABBREVMONTHNAME6  : LOCALE_SMONTHNAME6;  break; // jun
	case 7:  lct = bShort ? LOCALE_SABBREVMONTHNAME7  : LOCALE_SMONTHNAME7;  break; // jul
	case 8:  lct = bShort ? LOCALE_SABBREVMONTHNAME8  : LOCALE_SMONTHNAME8;  break; // aug
	case 9:  lct = bShort ? LOCALE_SABBREVMONTHNAME9  : LOCALE_SMONTHNAME9;  break; // sep
	case 10: lct = bShort ? LOCALE_SABBREVMONTHNAME10 : LOCALE_SMONTHNAME10; break; // oct
	case 11: lct = bShort ? LOCALE_SABBREVMONTHNAME11 : LOCALE_SMONTHNAME11; break; // nov
	case 12: lct = bShort ? LOCALE_SABBREVMONTHNAME12 : LOCALE_SMONTHNAME12; break; // dec
	}

	GetLocaleInfo(LOCALE_USER_DEFAULT, lct, sMonth.GetBuffer(30),	29);
	sMonth.ReleaseBuffer();

	return sMonth;
}

void CDateHelper::GetMonths(BOOL bShort, CStringArray& aMonths)
{
	aMonths.RemoveAll();

	for (int nMonth = 1; nMonth <= 12; nMonth++)
		aMonths.Add(GetMonth(nMonth, bShort));
}

double CDateHelper::GetTimeOnly(const COleDateTime& date)
{
	return (date.m_dt - GetDateOnly(date));
}

BOOL CDateHelper::DateHasTime(const COleDateTime& date)
{
	return (GetTimeOnly(date) > 0);
}

double CDateHelper::GetDateOnly(const COleDateTime& date)
{
	return floor(date.m_dt);
}

void CDateHelper::SplitDate(const COleDateTime& date, double& dDateOnly, double& dTimeOnly)
{
	dDateOnly = GetDateOnly(date);
	dTimeOnly = GetTimeOnly(date);
}

double CDateHelper::MakeDate(const COleDateTime& dtTimeOnly, const COleDateTime& dtDateOnly)
{
	double dDateOnly = GetDateOnly(dtDateOnly);
	double dTimeOnly = GetTimeOnly(dtTimeOnly);
	
	return dDateOnly + dTimeOnly;
}
