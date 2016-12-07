#if !defined(AFX_DATE_H__A0DE4D81_A07B_11D1_93C1_84CAF259627D__INCLUDED_)
#define AFX_DATE_H__A0DE4D81_A07B_11D1_93C1_84CAF259627D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CLongDate class
//
// Written by Bjarke Viksoe (bjarke@viksoe.dk)
// Copyright (c) 2000.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to you or your
// computer whatsoever. It's free, so don't hassle me about it.
//
// Beware of bugs.
//
#define SECS_PR_DAY (60L*60L*24L)

// Wraps the time_t structure
class CLongDate : public CObject
{
   DECLARE_DYNCREATE(CLongDate)

// Construction
public:
   CLongDate();
   CLongDate( time_t Time );
   CLongDate( LPCTSTR Date, LPCTSTR Time=NULL );
   virtual ~CLongDate();

// Attributes
private:
   time_t m_time;  // The current time.

// Operations
public:
   operator time_t(void) const { return m_time; };
   operator CString(void) { return GetDateTimeStr(); };
   const CLongDate& CLongDate::operator=(time_t t) { m_time = t; return *this; };
   const CLongDate& CLongDate::operator=(const CLongDate& date) { m_time = date.m_time; return *this; };
   time_t  GetTime(void) const { return m_time; };
   //
   void    SetCurrentTime(void) { m_time = ::time(NULL); };
   RETCODE SetTime( LPCTSTR Date, LPCTSTR Time=NULL );
   RETCODE SetTime( time_t Time );
   //
   CString GetDateTimeStr(void) const;
   CString GetDateStr(void) const;
   CString GetTimeStr(void) const;
};

#endif // !defined(AFX_DATE_H__A0DE4D81_A07B_11D1_93C1_84CAF259627D__INCLUDED_)
