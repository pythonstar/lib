#pragma once
#ifdef EXPORT_API
#define EXPORT_LIB __declspec(dllexport)
#else
#define EXPORT_LIB __declspec(dllimport)
#endif

namespace XGC
{
	namespace common
	{

		enum BasicType  // Stolen from CVCONST.H in the DIA 2.0 SDK
		{
			btNoType = 0,
			btVoid = 1,
			btChar = 2,
			btWChar = 3,
			btInt = 6,
			btUInt = 7,
			btFloat = 8,
			btBCD = 9,
			btBool = 10,
			btLong = 13,
			btULong = 14,
			btCurrency = 25,
			btDate = 26,
			btVariant = 27,
			btComplex = 28,
			btBit = 29,
			btBSTR = 30,
			btHresult = 31
		};

		//extern WheatyExceptionReport g_WheatyExceptionReport; //  global instance of class
		void EXPORT_LIB InstallCrashHandle();
	}
}