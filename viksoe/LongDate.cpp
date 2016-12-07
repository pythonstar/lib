// Date.cpp: implementation of the CLongDate class.
// Begun: 5 feb 1998 v0.0 Bjarke Viksøe
//
// Description:
// Wrapper around the 'time_t' ANSI C structure. Implements important
// functions to convert from/to a character string/time_t.
// Primarily used back when dates were stored as "YYYYMMDD" strings in
// the AMO database.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LongDate.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CLongDate

IMPLEMENT_DYNCREATE(CLongDate, CObject)

CLongDate::CLongDate()
{
   m_time = 0L;
}

CLongDate::CLongDate( time_t Time )
{
   m_time = Time;
}

CLongDate::CLongDate( LPCTSTR Date, LPCTSTR Time/*=NULL*/ )
{
   SetTime( Date, Time );
}

CLongDate::~CLongDate()
{
}


/////////////////////////////////////////////////////////////////////////////
// CLongDate functions


RETCODE CLongDate::SetTime( time_t Time )
// Function name   : CLongDate::SetTime
// Description      : Sets the object's time value
// Return type      : RETCODE 
// Argument       : time_t Time
{
   m_time = Time;
   if( m_time<0L ) m_time=0L;
   return RET_OK;
};


RETCODE CLongDate::SetTime( LPCTSTR Date, LPCTSTR Time/*=NULL*/ )
// Function name    : CLongDate::SetTime
// Description       : Set the object's time value
// Return type       : RETCODE 
// Argument        : LPCTSTR Date
// Argument        : LPCTSTR Time
//
// Converts date string to internal time value.
// A broad range of date formats are supported, including
// YYYYMMDDHHNNSS and YYMMDDHHNNSS
// Any parameter can be NULL, forcing conversion of the
// specifed argument only.
// To specify time strings with both day/time components
// (such as YYYYMMDDHHNNSS), pass it to Date and leave Time 
// set to NULL
{
   m_time = 0L;
   long years,months,days,hours,minutes,seconds;
   years = months = days = hours = minutes = seconds = 0;

   if( Time==NULL ) {
      if( Date==NULL ) return RET_OK; //empty (0)
      ASSERT(AfxIsValidString(Date));
      if( *Date==_T('\0')) return RET_OK; //empty (0)
      switch( _tcslen( Date ) ) {
      case 6:
         // YYMMDD
         // (Years before 70 is 2000, after 70 is 1900)
         _stscanf( Date, _T("%02d%02d%02d"), &years,&months,&days );
         if( years<70 ) years+=2000; else years+=1900;
         break;
      case 8:
         // YYYYMMDD
         _stscanf( Date, _T("%04d%02d%02d"), &years,&months,&days );
         break;
      case 12:
         // YYMMDDHHNNSS
         _stscanf( Date, _T("%02d%02d%02d%02d%02d%02d"), &years,&months,&days,&hours,&minutes,&seconds );
         if( years<70 ) years+=2000; else years+=1900;
         break;
      case 14:
         // YYYYMMDDHHNNSS
         _stscanf( Date, _T("%04d%02d%02d%02d%02d%02d"), &years,&months,&days,&hours,&minutes,&seconds );
         break;
      default:
         // Unsupported format error
         ASSERT(FALSE);
         return RET_INVALIDARGS;
      };
   }
   else {
      // We know Time is not NULL
      ASSERT(AfxIsValidString(Time));
      switch( _tcslen( Time ) ) {
      case 4:
         // HHNN
         _stscanf( Time, _T("%02d%02d"), &hours,&minutes );
         break;
      case 6:
         // HHNNSS
         _stscanf( Time, _T("%02d%02d%02d"), &hours,&minutes,&seconds );
         break;
      default:
         return RET_ERROR;
      };
      if( Date!=NULL ) {
         switch( _tcslen( Date ) ) {
         case 6:
            // YYMMDD
            _stscanf( Date, _T("%02d%02d%02d"), &years,&months,&days );
            if( years<70 ) years+=2000; else years+=1900;
            break;
         case 8:
            // YYYYMMDD
            _stscanf( Date, _T("%04d%02d%02d"), &years,&months,&days );
            break;
         default:
            // Unsupported format error
            ASSERT(FALSE);
            return RET_INVALIDARGS;
         };
      }
      else {
         // No date! gotta return the result ourself
         // since mktime() does not support dates before 1970
         // It is itself 1900-based
         m_time = (time_t)seconds + (minutes*60L) + (hours*60L*60L);
         return RET_OK;
      };
   };

   ASSERT((days==0) || (days >= 1 && days <= 31));
   ASSERT((months==0) || (months >= 1 && months <= 12));
   ASSERT((years==0) || (years >= 1900));
   // Validate result
   if(!(days >= 1 && days <= 31)) return RET_ERROR;
   if(!(months >= 1 && months <= 12)) return RET_ERROR;
   if(years < 1900) return RET_ERROR;

   // Need to convert attributes to a time_t
   // structiure
   struct tm mtime;
   memset( &mtime, 0, sizeof(struct tm) );
   mtime.tm_year = years - 1900; // tm_year is 1900 based
   mtime.tm_mon = months - 1; // tm_mon is 0 based
   mtime.tm_mday = days;
   mtime.tm_hour = hours;
   mtime.tm_min = minutes;
   mtime.tm_sec = seconds;
   mtime.tm_isdst = -1; // we don't know daylight saving question
   m_time = ::mktime( &mtime );

   // If no error return ok
   if( m_time!=-1 ) return RET_OK;
   // Error! Unable to convert time! Reset time...
   m_time = 0;
   return RET_ERROR;
};


CString CLongDate::GetDateTimeStr() const
// Function name   : CLongDate::GetDateTimeStr
// Description       : Construct a string containing complete date/time value
// Return type      : CString 
// Argument         : void
{
   CString s;
   struct tm *mtime = ::localtime( &m_time );
   ASSERT(mtime);
   s.Format( _T("%04d%02d%02d%02d%02d%02d"),
      mtime->tm_year + 1900,
      mtime->tm_mon + 1,
      mtime->tm_mday,
      mtime->tm_hour,
      mtime->tm_min,
      mtime->tm_sec );
   return s;
};


CString CLongDate::GetDateStr() const
// Function name   : CLongDate::GetDateStr
// Description       : Construct a string containing the date part of 
//                   the time value
// Return type       : CString 
// Argument        : void
{
   CString s;
   struct tm *mtime = ::localtime( &m_time );
   ASSERT(mtime);
   s.Format( _T("%04d%02d%02d"),
      mtime->tm_year + 1900,
      mtime->tm_mon + 1,
      mtime->tm_mday );
   return s;
};


CString CLongDate::GetTimeStr() const
// Function name   : CLongDate::GetTimeStr
// Description       : Construct a string containing the time part of the 
//                   time value
// Return type       : CString 
// Argument        : void
{
   CString s;
   if( m_time <= SECS_PR_DAY ) {
      // In case we only have a time number...
      s.Format( _T("%02d%02d%02d"),
         m_time / (60L*60L),
         (m_time % (60L*60L)) / 60L ,
         m_time % 60);
   }
   else {
      // Both date and time is in number
      struct tm *mtime = ::localtime( &m_time );
      ASSERT(mtime);
      s.Format( _T("%02d%02d%02d"),
         mtime->tm_hour,
         mtime->tm_min ,
         mtime->tm_sec );
   };
   return s;
};

