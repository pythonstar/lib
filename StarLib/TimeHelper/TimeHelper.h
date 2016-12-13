// TimeHelper.h: interface for the TimeHelper class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TIMEHELPER_H__BA0C1E67_FAAA_4E65_8EF3_01B011ACFBBC__INCLUDED_)
#define AFX_TIMEHELPER_H__BA0C1E67_FAAA_4E65_8EF3_01B011ACFBBC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>

enum 
{
	THU_MINS	= 'I',
	THU_HOURS	= 'H',
	THU_DAYS	= 'D',
	THU_WEEKS	= 'W',
	THU_MONTHS	= 'M',
	THU_YEARS	= 'Y',
};

class CTimeHelper  
{
public:
	CTimeHelper(); // uses statically defined hours and days
	CTimeHelper(double dHoursInDay, double dDaysInWeek);

	double GetTime(double dTime, int nFromUnits, int nToUnits);
	CString FormatTimeHMS(double dTime, int nUnitsFrom, BOOL bDecPlaces = TRUE);
	CString FormatTime(double dTime, int nUnits, int nDecPlaces);
	CString FormatTime(double dTime, int nDecPlaces);

public:
	static BOOL SetHoursInOneDay(double dHours);
	static BOOL SetDaysInOneWeek(double dDays);
	static double GetHoursInOneDay() { return HOURS2DAYS; }
	static double GetDaysInOneWeek() { return DAYS2WEEKS; }
	static void SetUnits(int nUnits, LPCTSTR szUnits);
	static void SetUnits(int nUnits, TCHAR cUnits);
	static TCHAR GetUnits(int nUnits);

  static CString FormatISOTime(int nHour, int nMin, int nSec = 0, BOOL bIncSeconds = FALSE);
	static CString Format24HourTime(int nHour, int nMin, int nSec = 0, BOOL bIncSeconds = FALSE);

protected:
	double m_dHours2Days, m_dDaysToWeeks;

protected:
	static double HOURS2DAYS, DAYS2WEEKS; // user definable
	static CMap<int, int, TCHAR, TCHAR&> MAPUNIT2CH; // user definable

protected:
	static BOOL Compare(int nFromUnits, int nToUnits); // 0=same, -1=nFrom < nTo else 1
	static CString FormatTimeHMS(double dTime, int nUnits, int nLeftOverUnits, 
								   double dLeftOverMultiplier, BOOL bDecPlaces);
};

#endif // !defined(AFX_TIMEHELPER_H__BA0C1E67_FAAA_4E65_8EF3_01B011ACFBBC__INCLUDED_)
