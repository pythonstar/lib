// TimeHelper.cpp: implementation of the TimeHelper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TimeHelper.h"

#include <locale.h>
#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const double MINS2HOURS = 60;
double CTimeHelper::HOURS2DAYS = 8; // user definable
double CTimeHelper::DAYS2WEEKS = 5; // user definable
const double WEEKS2MONTHS = 4.348;
const double MONTHS2YEARS = 12;
const double FUDGE = 1e-6;

CMap<int, int, TCHAR, TCHAR&> CTimeHelper::MAPUNIT2CH; // user definable

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTimeHelper::CTimeHelper() : m_dHours2Days(HOURS2DAYS), m_dDaysToWeeks(DAYS2WEEKS)
{
}

CTimeHelper::CTimeHelper(double dHoursInDay, double dDaysInWeek) 
	: m_dHours2Days(dHoursInDay), m_dDaysToWeeks(dDaysInWeek)
{

}


double CTimeHelper::GetTime(double dTime, int nFromUnits, int nToUnits)
{
	if (nFromUnits == nToUnits)
		return dTime;

	else if (Compare(nFromUnits, nToUnits) > 0)
	{
		while (Compare(nFromUnits, nToUnits) > 0)
		{
			switch (nFromUnits)
			{
			case THU_HOURS:
				dTime *= MINS2HOURS;
				nFromUnits = THU_MINS;
				break;
				
			case THU_DAYS:
				dTime *= m_dHours2Days;
				nFromUnits = THU_HOURS;
				break;
				
			case THU_WEEKS:
				dTime *= m_dDaysToWeeks;
				nFromUnits = THU_DAYS;
				break;
				
			case THU_MONTHS:
				dTime *= WEEKS2MONTHS;
				nFromUnits = THU_WEEKS;
				break;
				
			case THU_YEARS:
				dTime *= MONTHS2YEARS;
				nFromUnits = THU_MONTHS;
				break;
			}
		}
	}
	else // nFromUnits < nToUnits
	{
		while (Compare(nFromUnits, nToUnits) < 0)
		{
			switch (nFromUnits)
			{
			case THU_MINS:
				dTime /= MINS2HOURS;
				nFromUnits = THU_HOURS;
				break;

			case THU_HOURS:
				dTime /= m_dHours2Days;
				nFromUnits = THU_DAYS;
				break;

			case THU_DAYS:
				dTime /= m_dDaysToWeeks;
				nFromUnits = THU_WEEKS;
				break;
				
			case THU_WEEKS:
				dTime /= WEEKS2MONTHS;
				nFromUnits = THU_MONTHS;
				break;
				
			case THU_MONTHS:
				dTime /= MONTHS2YEARS;
				nFromUnits = THU_YEARS;
				break;
			}
		}
	}

	return dTime;
}

CString CTimeHelper::FormatISOTime(int nHour, int nMin, int nSec, BOOL bIncSeconds)
{
  CString sFormat = "HH:mm";

  if (bIncSeconds)
      sFormat += ":ss";

  CString sTime;
  SYSTEMTIME st = { 0, 0, 0, 0, (WORD)nHour, (WORD)nMin, (WORD)nSec, 0 };

  ::GetTimeFormat(0, bIncSeconds ? 0 : TIME_NOSECONDS, &st, sFormat, sTime.GetBuffer(50), 49);
  sTime.ReleaseBuffer();

  return sTime;
}

CString CTimeHelper::Format24HourTime(int nHour, int nMin, int nSec, BOOL bIncSeconds)
{
	CString sTime;
	SYSTEMTIME st = { 0, 0, 0, 0, (WORD)nHour, (WORD)nMin, (WORD)nSec, 0 };

	::GetTimeFormat(0, bIncSeconds ? 0 : TIME_NOSECONDS, &st, NULL, sTime.GetBuffer(50), 49);
	sTime.ReleaseBuffer();

	return sTime;
}

CString CTimeHelper::FormatTime(double dTime, int nDecPlaces)
{
	return FormatTime(dTime, 0, nDecPlaces);
}

CString CTimeHelper::FormatTime(double dTime, int nUnits, int nDecPlaces)
{
	// handle locale specific decimal separator
	setlocale(LC_NUMERIC, "");

	CString sTime;
	TCHAR cUnits;

	if (nUnits && MAPUNIT2CH.Lookup(nUnits, cUnits))
		sTime.Format(_T("%.*f %c"), nDecPlaces, dTime, cUnits);
	else
		sTime.Format(_T("%.*f"), nDecPlaces, dTime);

	// restore decimal separator to '.'
	setlocale(LC_NUMERIC, "English");

	return sTime;
}

void CTimeHelper::SetUnits(int nUnits, TCHAR cUnits)
{
	MAPUNIT2CH[nUnits] = cUnits;
}

void CTimeHelper::SetUnits(int nUnits, LPCTSTR szUnits)
{
	if (szUnits && *szUnits)
		SetUnits(nUnits, szUnits[0]);
}

TCHAR CTimeHelper::GetUnits(int nUnits)
{
	TCHAR cUnits = 0;

	MAPUNIT2CH.Lookup(nUnits, cUnits);

	return cUnits;
}

CString CTimeHelper::FormatTimeHMS(double dTime, int nUnitsFrom, BOOL bDecPlaces)
{
	// handle negative times
	BOOL bNegative = (dTime < 0.0);

	if (bNegative)
		dTime = -dTime;

	// convert the time to minutes 
	double dMins = GetTime(dTime, nUnitsFrom, THU_MINS);
	
	// and all the others up to years
	double dHours = dMins / MINS2HOURS;
	double dDays = dHours / m_dHours2Days;
	double dWeeks = dDays / m_dDaysToWeeks;
	double dMonths = dWeeks / WEEKS2MONTHS;
	double dYears = dMonths / MONTHS2YEARS;
	
	CString sTime;
	
	if (dYears >= 1.0)
		sTime = FormatTimeHMS(dYears, THU_YEARS, THU_MONTHS, MONTHS2YEARS, bDecPlaces);
	
	else if (dMonths >= 1.0)
		sTime = FormatTimeHMS(dMonths, THU_MONTHS, THU_WEEKS, WEEKS2MONTHS, bDecPlaces);
	
	else if (dWeeks >= 1.0)
		sTime = FormatTimeHMS(dWeeks, THU_WEEKS, THU_DAYS, m_dDaysToWeeks, bDecPlaces);
	
	else if (dDays >= 1.0)
		sTime = FormatTimeHMS(dDays, THU_DAYS, THU_HOURS, m_dHours2Days, bDecPlaces);
	
	else if (dHours >= 1.0)
		sTime = FormatTimeHMS(dHours, THU_HOURS, THU_MINS, MINS2HOURS, bDecPlaces);
	
	else if (dMins >= 1.0)
		sTime = FormatTimeHMS(dMins, THU_MINS, THU_MINS, 0, FALSE);
	
	sTime.MakeLower();

	// handle negative times
	if (bNegative)
		sTime = "-" + sTime;
	
	return sTime;
	
}

CString CTimeHelper::FormatTimeHMS(double dTime, int nUnits, int nLeftOverUnits, 
								   double dLeftOverMultiplier, BOOL bDecPlaces)
{
	CString sTime;
	
	if (bDecPlaces)
	{
		double dLeftOver = (dTime - (int)dTime) * dLeftOverMultiplier + FUDGE;

		// omit second element if zero
		if (((int)dLeftOver) == 0)
			sTime.Format(_T("%d%c"), (int)dTime, GetUnits(nUnits));
		else
			sTime.Format(_T("%d%c%d%c"), (int)dTime, GetUnits(nUnits), 
								(int)dLeftOver, GetUnits(nLeftOverUnits));
	}
	else
		sTime.Format(_T("%d%c"), (int)(dTime + 0.5), GetUnits(nUnits));
	
	return sTime;
}

int CTimeHelper::Compare(int nFromUnits, int nToUnits)
{
	if (nFromUnits == nToUnits)
		return 0;

	switch (nFromUnits)
	{
	case THU_MINS:
		return -1; // less than everything else
	
	case THU_HOURS:
		return (nToUnits == THU_MINS) ? 1 : -1;
	
	case THU_DAYS:
		return (nToUnits == THU_HOURS || nToUnits == THU_MINS) ? 1 : -1;
	
	case THU_WEEKS:
		return (nToUnits == THU_YEARS || nToUnits == THU_MONTHS) ? -1 : 1;
	
	case THU_MONTHS:
		return (nToUnits == THU_YEARS) ? -1 : 1;
	
	case THU_YEARS:
		return 1; // greater than everything else
	}

	// else
	return 0;
}

BOOL CTimeHelper::SetHoursInOneDay(double dHours)
{
	if (dHours <= 0 || dHours > 24)
		return FALSE;

	HOURS2DAYS = dHours;
	return TRUE;
}

BOOL CTimeHelper::SetDaysInOneWeek(double dDays)
{
	if (dDays <= 0 || dDays > 7)
		return FALSE;

	DAYS2WEEKS = dDays;
	return TRUE;
}
