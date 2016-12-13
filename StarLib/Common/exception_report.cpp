//==========================================
// Matt Pietrek
// MSDN Magazine, 2002
// FILE: WheatyExceptionReport.CPP
//==========================================
#include "common/header.h"
#include "common/exception_report.h"
#include "common/version.h"
#include "common/utility.h"

#define	ONEK			1024
#define	SIXTYFOURK		(64*ONEK)
#define	ONEM			(ONEK*ONEK)
#define	ONEG			(ONEK*ONEK*ONEK)

namespace XGC
{
	namespace common
	{

		class EXPORT_LIB WheatyExceptionReport
		{
		public:

			WheatyExceptionReport( );
			~WheatyExceptionReport( );

			void SetLogFileName( PTSTR pszLogFileName );


			// entry point where control comes on an unhandled exception
			static LONG WINAPI WheatyUnhandledExceptionFilter(
				PEXCEPTION_POINTERS pExceptionInfo );

		private:

			static LPWSTR buildsymbolsearchpath ();

			static bool SymInit( PCONTEXT pCtx );

			// where report info is extracted and generated 
			static void GenerateExceptionReport( PEXCEPTION_POINTERS pExceptionInfo );

			// Helper functions
			static LPTSTR GetExceptionString( DWORD dwCode );
			static BOOL GetLogicalAddress(  PVOID addr, PTSTR szModule, DWORD len,
				DWORD& section, DWORD& offset );

			static void WriteStackDetails( PCONTEXT pContext, bool bWriteVariables );

			static BOOL CALLBACK EnumerateSymbolsCallback(PSYMBOL_INFO,ULONG, PVOID);

			static bool FormatSymbolValue( PSYMBOL_INFO, STACKFRAME * );

			static void DumpTypeIndex( DWORD64, DWORD, unsigned, DWORD_PTR, bool & );

			static void FormatOutputValue( BasicType basicType, DWORD64 length, PVOID pAddress );

			static BasicType GetBasicType( DWORD typeIndex, DWORD64 modBase );

			static int __cdecl _tprintf(const TCHAR * format, ...);

			static int __cdecl _printf(const CHAR * format, ...);

			static void DumpMiniDump( HANDLE hFile, PEXCEPTION_POINTERS excpInfo);

			static bool DumpModuleInfo( HINSTANCE ModuleHandle, int nModuleNo);

			static void DumpModuleList();

			static void DumpSystemInformation();

			// Variables used by the class
			static TCHAR m_szLogFileName[MAX_PATH];
			static LPTOP_LEVEL_EXCEPTION_FILTER m_previousFilter;
			static HANDLE m_hReportFile;
			static HANDLE m_hProcess;
		};

		//============================== Global Variables =============================

		//
		// Declare the static variables of the WheatyExceptionReport class
		//
		TCHAR EXPORT_LIB WheatyExceptionReport::m_szLogFileName[MAX_PATH];
		LPTOP_LEVEL_EXCEPTION_FILTER EXPORT_LIB WheatyExceptionReport::m_previousFilter;
		HANDLE EXPORT_LIB WheatyExceptionReport::m_hReportFile;
		HANDLE EXPORT_LIB WheatyExceptionReport::m_hProcess;

		// Declare global instance of class
		//WheatyExceptionReport g_WheatyExceptionReport;

		void InstallCrashHandle()
		{
			static WheatyExceptionReport _exception;
		}

		///////////////////////////////////////////////////////////////////////////////
		// FormatTime
		//
		// Format the specified FILETIME to output in a human readable format,
		// without using the C run time.
		static void FormatTime(LPTSTR output, FILETIME TimeToPrint)
		{
			output[0] = _T('\0');
			WORD Date, Time;
			if (FileTimeToLocalFileTime(&TimeToPrint, &TimeToPrint) &&
				FileTimeToDosDateTime(&TimeToPrint, &Date, &Time))
			{
				wsprintf(output, _T("%d/%d/%d %02d:%02d:%02d"),
					(Date / 32) & 15, Date & 31, (Date / 512) + 1980,
					(Time >> 11), (Time >> 5) & 0x3F, (Time & 0x1F) * 2);
			}
		}

		///////////////////////////////////////////////////////////////////////////////
		// DumpMiniDump
		void WheatyExceptionReport::DumpMiniDump( HANDLE hFile, PEXCEPTION_POINTERS excpInfo)
		{
			if (excpInfo == NULL) 
			{
				// Generate exception to get proper context in dump
				__try 
				{
					OutputDebugString(_T("raising exception\r\n"));
					RaiseException(EXCEPTION_BREAKPOINT, 0, 0, NULL);
				} 
				__except(DumpMiniDump(hFile, GetExceptionInformation()),
					EXCEPTION_CONTINUE_EXECUTION) 
				{
				}
			} 
			else
			{
				OutputDebugString(_T("writing minidump\r\n"));
				MINIDUMP_EXCEPTION_INFORMATION eInfo;
				eInfo.ThreadId = GetCurrentThreadId();
				eInfo.ExceptionPointers = excpInfo;
				eInfo.ClientPointers = FALSE;

				// note:  MiniDumpWithIndirectlyReferencedMemory does not work on Win98
				MiniDumpWriteDump(
					GetCurrentProcess(),
					GetCurrentProcessId(),
					hFile,
					MiniDumpNormal,
					excpInfo ? &eInfo : NULL,
					NULL,
					NULL);
			}
		}

		///////////////////////////////////////////////////////////////////////////////
		// DumpModuleInfo
		//
		// Print information about a code module (DLL or EXE) such as its size,
		// location, time stamp, etc.
		bool WheatyExceptionReport::DumpModuleInfo( HINSTANCE ModuleHandle, int nModuleNo)
		{
			bool rc = false;
			TCHAR szModName[MAX_PATH*2];
			ZeroMemory(szModName, sizeof(szModName));

			__try
			{
				if (GetModuleFileName(ModuleHandle, szModName, sizeof(szModName)-2) > 0)
				{
					// If GetModuleFileName returns greater than zero then this must
					// be a valid code module address. Therefore we can try to walk
					// our way through its structures to find the link time stamp.
					IMAGE_DOS_HEADER *DosHeader = (IMAGE_DOS_HEADER*)ModuleHandle;
					if (IMAGE_DOS_SIGNATURE != DosHeader->e_magic)
						return false;

					IMAGE_NT_HEADERS *NTHeader = (IMAGE_NT_HEADERS*)((CHAR *)DosHeader
						+ DosHeader->e_lfanew);
					if (IMAGE_NT_SIGNATURE != NTHeader->Signature)
						return false;

					// open the code module file so that we can get its file date and size
					HANDLE ModuleFile = CreateFile(szModName, GENERIC_READ,
						FILE_SHARE_READ, 0, OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL, 0);

					TCHAR TimeBuffer[100];
					TimeBuffer[0] = _T('\0');

					DWORD FileSize = 0;
					if (ModuleFile != INVALID_HANDLE_VALUE)
					{
						FileSize = GetFileSize(ModuleFile, 0);
						FILETIME LastWriteTime;
						if (GetFileTime(ModuleFile, 0, 0, &LastWriteTime))
						{
							FormatTime(TimeBuffer, LastWriteTime);
						}
						CloseHandle(ModuleFile);
					}
					_tprintf( _T("Module %d\r\n"), nModuleNo);
					_tprintf( _T("%s\r\n"), szModName);
					_tprintf( _T("Image Base: 0x%08x  Image Size: 0x%08x\r\n"), 
						NTHeader->OptionalHeader.ImageBase, 
						NTHeader->OptionalHeader.SizeOfImage), 

						_tprintf( _T("Checksum:   0x%08x  Time Stamp: 0x%08x\r\n"), 
						NTHeader->OptionalHeader.CheckSum,
						NTHeader->FileHeader.TimeDateStamp);

					_tprintf( _T("File Size:  %-10d  File Time:  %s\r\n"),
						FileSize, TimeBuffer);

					_tprintf( _T("Version Information:\r\n"));

					CMiniVersion ver(szModName);
					TCHAR szBuf[200];
					WORD dwBuf[4];

					ver.GetCompanyName(szBuf, sizeof(szBuf)-1);
					_tprintf( _T("   Company:    %s\r\n"), szBuf);

					ver.GetProductName(szBuf, sizeof(szBuf)-1);
					_tprintf( _T("   Product:    %s\r\n"), szBuf);

					ver.GetFileDescription(szBuf, sizeof(szBuf)-1);
					_tprintf( _T("   FileDesc:   %s\r\n"), szBuf);

					ver.GetFileVersion(dwBuf);
					_tprintf( _T("   FileVer:    %d.%d.%d.%d\r\n"), 
						dwBuf[0], dwBuf[1], dwBuf[2], dwBuf[3]);

					ver.GetProductVersion(dwBuf);
					_tprintf( _T("   ProdVer:    %d.%d.%d.%d\r\n"), 
						dwBuf[0], dwBuf[1], dwBuf[2], dwBuf[3]);

					ver.Release();

					_tprintf( _T("\r\n"));

					rc = true;
				}
			}
			// Handle any exceptions by continuing from this point.
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
			}
			return rc;
		}

		///////////////////////////////////////////////////////////////////////////////
		// DumpModuleList
		//
		// Scan memory looking for code modules (DLLs or EXEs). VirtualQuery is used
		// to find all the blocks of address space that were reserved or committed,
		// and ShowModuleInfo will display module information if they are code
		// modules.
		void WheatyExceptionReport::DumpModuleList()
		{
			SYSTEM_INFO	SystemInfo;
			GetSystemInfo(&SystemInfo);

			const size_t PageSize = SystemInfo.dwPageSize;

			// Set NumPages to the number of pages in the 4GByte address space,
			// while being careful to avoid overflowing ints
			const size_t NumPages = 4 * size_t(ONEG / PageSize);
			size_t pageNum = 0;
			void *LastAllocationBase = 0;

			int nModuleNo = 1;

			while (pageNum < NumPages)
			{
				MEMORY_BASIC_INFORMATION MemInfo;
				if (VirtualQuery((void *)(pageNum * PageSize), &MemInfo,
					sizeof(MemInfo)))
				{
					if (MemInfo.RegionSize > 0)
					{
						// Adjust the page number to skip over this block of memory
						pageNum += MemInfo.RegionSize / PageSize;
						if (MemInfo.State == MEM_COMMIT && MemInfo.AllocationBase >
							LastAllocationBase)
						{
							// Look for new blocks of committed memory, and try
							// recording their module names - this will fail
							// gracefully if they aren't code modules
							LastAllocationBase = MemInfo.AllocationBase;
							if (DumpModuleInfo( 
								(HINSTANCE)LastAllocationBase, 
								nModuleNo))
							{
								nModuleNo++;
							}
						}
					}
					else
						pageNum += SIXTYFOURK / PageSize;
				}
				else
					pageNum += SIXTYFOURK / PageSize;

				// If VirtualQuery fails we advance by 64K because that is the
				// granularity of address space doled out by VirtualAlloc()
			}
		}

		///////////////////////////////////////////////////////////////////////////////
		// DumpSystemInformation
		//
		// Record information about the user's system, such as processor type, amount
		// of memory, etc.
		void WheatyExceptionReport::DumpSystemInformation()
		{
			FILETIME CurrentTime;
			GetSystemTimeAsFileTime(&CurrentTime);
			TCHAR szTimeBuffer[100];
			FormatTime(szTimeBuffer, CurrentTime);

			_tprintf( _T("Error occurred at %s.\r\n"), szTimeBuffer);

			TCHAR szModuleName[MAX_PATH*2];
			ZeroMemory(szModuleName, sizeof(szModuleName));
			if (GetModuleFileName(0, szModuleName, _countof(szModuleName)-2) <= 0)
				lstrcpy(szModuleName, _T("Unknown"));

			TCHAR szUserName[200];
			ZeroMemory(szUserName, sizeof(szUserName));
			DWORD UserNameSize = _countof(szUserName)-2;
			if (!GetUserName(szUserName, &UserNameSize))
				lstrcpy(szUserName, _T("Unknown"));

			_tprintf( _T("%s, run by %s.\r\n"), szModuleName, szUserName);

			// print out operating system
			TCHAR szWinVer[50], szMajorMinorBuild[50];
			int nWinVer;
			GetWinVer(szWinVer, &nWinVer, szMajorMinorBuild);
			_tprintf( _T("Operating system:  %s (%s).\r\n"), 
				szWinVer, szMajorMinorBuild);

			SYSTEM_INFO	SystemInfo;
			GetSystemInfo(&SystemInfo);
			_tprintf( _T("%d processor(s), type %d.\r\n"),
				SystemInfo.dwNumberOfProcessors, SystemInfo.dwProcessorType);

			MEMORYSTATUS MemInfo;
			MemInfo.dwLength = sizeof(MemInfo);
			GlobalMemoryStatus(&MemInfo);

			// Print out info on memory, rounded up.
			_tprintf( _T("%d%% memory in use.\r\n"), MemInfo.dwMemoryLoad);
			_tprintf( _T("%d MBytes physical memory.\r\n"), (MemInfo.dwTotalPhys +
				ONEM - 1) / ONEM);
			_tprintf( _T("%d MBytes physical memory free.\r\n"), 
				(MemInfo.dwAvailPhys + ONEM - 1) / ONEM);
			_tprintf( _T("%d MBytes paging file.\r\n"), (MemInfo.dwTotalPageFile +
				ONEM - 1) / ONEM);
			_tprintf( _T("%d MBytes paging file free.\r\n"), 
				(MemInfo.dwAvailPageFile + ONEM - 1) / ONEM);
			_tprintf( _T("%d MBytes user address space.\r\n"), 
				(MemInfo.dwTotalVirtual + ONEM - 1) / ONEM);
			_tprintf( _T("%d MBytes user address space free.\r\n"), 
				(MemInfo.dwAvailVirtual + ONEM - 1) / ONEM);
		}

		//============================== Class Methods =============================

		WheatyExceptionReport::WheatyExceptionReport( )   // Constructor
		{
			// Install the unhandled exception filter function
			m_previousFilter =
				SetUnhandledExceptionFilter(WheatyUnhandledExceptionFilter);

			// Create a filename to record the error information to.
			// Storing it in the executable directory works well.

			xgc_tchar szTime[128];
			xgc_tchar szDate[128];
			_tstrtime( szTime );
			int n = 0;
			while( szTime[n] != 0 )
			{
				if( szTime[n] == ':' ) szTime[n] = '_';
				++n;
			}

			_tstrdate( szDate );
			n = 0;
			while( szDate[n] != 0 )
			{
				if( szDate[n] == '/' ) szDate[n] = '_';
				++n;
			}

			TCHAR szModule[_MAX_PATH];
			::GetModuleFileName( NULL, szModule, _countof( szModule ) );
			TCHAR* pModuleEnd = _tcsrchr( szModule, '\\' );
			if( pModuleEnd ) *pModuleEnd = 0;

			TCHAR* pModuleName = ++pModuleEnd;
			pModuleEnd = _tcschr( pModuleName, '.' );
			if( pModuleEnd ) *pModuleEnd = 0;

			::CreateDirectory( _T(".\\CrashLog"), NULL );

			_sntprintf( m_szLogFileName, _countof( m_szLogFileName ), _T("%s\\CrashLog\\%s-%s-%s%s"), szModule, pModuleName, szDate, szTime, _T(".txt") );

			m_hProcess = GetCurrentProcess();
		}

		//============
		// Destructor 
		//============
		WheatyExceptionReport::~WheatyExceptionReport( )
		{
			SetUnhandledExceptionFilter( m_previousFilter );
		}

		//==============================================================
		// Lets user change the name of the report file to be generated 
		//==============================================================
		void WheatyExceptionReport::SetLogFileName( PTSTR pszLogFileName )
		{
			_tcscpy( m_szLogFileName, pszLogFileName );
		}

		//===========================================================
		// Entry point where control comes on an unhandled exception 
		//===========================================================
		LONG WINAPI WheatyExceptionReport::WheatyUnhandledExceptionFilter(
											PEXCEPTION_POINTERS pExceptionInfo )
		{
			m_hReportFile = CreateFile( m_szLogFileName,
										GENERIC_WRITE,
										0,
										0,
										OPEN_ALWAYS,
										FILE_FLAG_WRITE_THROUGH,
										0 );

			if ( m_hReportFile )
			{
				SetFilePointer( m_hReportFile, 0, 0, FILE_END );

				//ASSERT(FALSE);
				GenerateExceptionReport( pExceptionInfo );

				CloseHandle( m_hReportFile );
				m_hReportFile = 0;
			}

			if ( m_previousFilter )
				return m_previousFilter( pExceptionInfo );
			else
				return EXCEPTION_CONTINUE_SEARCH;
		}

		// strapp - Appends the specified source string to the specified destination
		//   string. Allocates additional space so that the destination string "grows"
		//   as new strings are appended to it. This function is fairly infrequently
		//   used so efficiency is not a major concern.
		//
		//  - dest (IN/OUT): Address of the destination string. Receives the resulting
		//      combined string after the append operation.
		//
		//  - source (IN): Source string to be appended to the destination string.
		//
		//  Return Value:
		//
		//    None.
		//
		VOID strapp (LPWSTR *dest, LPCWSTR source)
		{
			SIZE_T length;
			LPWSTR temp;

			temp = *dest;
			length = wcslen(*dest) + wcslen(source);
			*dest = new WCHAR [length + 1];
			wcsncpy_s(*dest, length + 1, temp, _TRUNCATE);
			wcsncat_s(*dest, length + 1, source, _TRUNCATE);
			delete [] temp;
		}

		LPWSTR WheatyExceptionReport::buildsymbolsearchpath ()
		{
			WCHAR   directory [_MAX_DIR];
			WCHAR   drive [_MAX_DRIVE];
			LPWSTR  env;
			DWORD   envlen;
			SIZE_T  index;
			SIZE_T  length;
			HMODULE module;
			LPWSTR  path = new WCHAR [MAX_PATH];
			SIZE_T  pos = 0;
			WCHAR   system [MAX_PATH];
			WCHAR   windows [MAX_PATH];

			// Oddly, the symbol handler ignores the link to the PDB embedded in the
			// executable image. So, we'll manually add the location of the executable
			// to the search path since that is often where the PDB will be located.
			path[0] = L'\0';
			module = GetModuleHandle(NULL);
			GetModuleFileNameW(module, path, MAX_PATH);
			_wsplitpath_s(path, drive, _MAX_DRIVE, directory, _MAX_DIR, NULL, 0, NULL, 0);
			wcsncpy_s(path, MAX_PATH, drive, _TRUNCATE);
			strapp(&path, directory);

			// When the symbol handler is given a custom symbol search path, it will no
			// longer search the default directories (working directory, system root,
			// etc). But we'd like it to still search those directories, so we'll add
			// them to our custom search path.
			//
			// Append the working directory.
			strapp(&path, L";.\\");

			// Append the Windows directory.
			if (GetWindowsDirectoryW(windows, MAX_PATH) != 0) {
				strapp(&path, L";");
				strapp(&path, windows);
			}

			// Append the system directory.
			if (GetSystemDirectoryW(system, MAX_PATH) != 0) {
				strapp(&path, L";");
				strapp(&path, system);
			}

			// Append %_NT_SYMBOL_PATH%.
			envlen = GetEnvironmentVariableW(L"_NT_SYMBOL_PATH", NULL, 0);
			if (envlen != 0) {
				env = new WCHAR [envlen];
				if (GetEnvironmentVariableW(L"_NT_SYMBOL_PATH", env, envlen) != 0) {
					strapp(&path, L";");
					strapp(&path, env);
				}
				delete [] env;
			}

			//  Append %_NT_ALT_SYMBOL_PATH%.
			envlen = GetEnvironmentVariableW(L"_NT_ALT_SYMBOL_PATH", NULL, 0);
			if (envlen != 0) {
				env = new WCHAR [envlen];
				if (GetEnvironmentVariableW(L"_NT_ALT_SYMBOL_PATH", env, envlen) != 0) {
					strapp(&path, L";");
					strapp(&path, env);
				}
				delete [] env;
			}

			// Remove any quotes from the path. The symbol handler doesn't like them.
			pos = 0;
			length = wcslen(path);
			while (pos < length) {
				if (path[pos] == L'\"') {
					for (index = pos; index < length; index++) {
						path[index] = path[index + 1];
					}
				}
				pos++;
			}

			return path;
		}

		//////////////////////////////////////////////////////////////////////////
		// initialize symbol file;
		bool WheatyExceptionReport::SymInit( PCONTEXT pCtx )
		{
			LPWSTR symbolpath = buildsymbolsearchpath();
			SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
			if (!SymInitializeW( GetCurrentProcess(), symbolpath, FALSE ) )
			{
				// 初始化失败则打印堆栈.
				__try
				{
					DWORD* pStackTop;
					DWORD* pStack = (DWORD*)DWORD_PTR( pCtx->Esp );

					const int NumCodeBytes = 16;	// Number of code bytes to record.
					const int MaxStackDump = 3072;	// Maximum number of DWORDS in stack dumps.
					const int StackColumns = 4;		// Number of columns in stack dump.

					__asm
					{
						// Load the top (highest address) of the stack from the
						// thread information block. It will be found there in
						// Win9x and Windows NT.
						mov	eax, fs:[4]
						mov pStackTop, eax
					}

					if (pStackTop > pStack + MaxStackDump)
						pStackTop = pStack + MaxStackDump;

					int Count = 0;

					DWORD* pStackStart = pStack;

					int nDwordsPrinted = 0;

					while (pStack + 1 <= pStackTop)
					{
						if ((Count % StackColumns) == 0)
						{
							pStackStart = pStack;
							nDwordsPrinted = 0;
							_tprintf( _T("0x%08x: "), (DWORD_PTR)pStack);
						}

						if ((++Count % StackColumns) == 0 || pStack + 2 > pStackTop)
						{
							_tprintf( _T("%08x "), *pStack);
							nDwordsPrinted++;

							int n = nDwordsPrinted;
							while (n < 4)
							{
								_tprintf( _T("         "));
								n++;
							}

							for (int i = 0; i < nDwordsPrinted; i++)
							{
								DWORD dwStack = *pStackStart;
								for (int j = 0; j < 4; j++)
								{
									char c = (char)(dwStack & 0xFF);
									if (c < 0x20 || c > 0x7E)
										c = '.';
		#ifdef _UNICODE
									WCHAR w = (WCHAR)c;
									_tprintf( _T("%c"), w);
		#else
									_tprintf( _T("%c"), c);
		#endif
									dwStack = dwStack >> 8;
								}
								pStackStart++;
							}

							_tprintf( _T("\r\n"));
						}
						else
						{
							_tprintf( _T("%08x "), *pStack);
							nDwordsPrinted++;
						}
						pStack++;
					}
					_tprintf( _T("\r\n"));

				}
				__except(EXCEPTION_EXECUTE_HANDLER)
				{
					_tprintf( _T("Exception encountered during stack dump.\r\n") );
				}
			}

			MODULEENTRY32 stME32              ;
			memset ( &stME32 , NULL , sizeof ( MODULEENTRY32 ) ) ;
			stME32.dwSize = sizeof ( MODULEENTRY32 ) ;

			HANDLE hShot = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, GetCurrentProcessId() );
			if( Module32First( hShot, &stME32 ) == TRUE )
			{
				CHAR szModuleName[MAX_PATH];
				do
				{
					HANDLE hFile = CreateFile ( stME32.szExePath
						,GENERIC_READ  
						,FILE_SHARE_READ
						,NULL
						,OPEN_EXISTING
						,0
						,0
						);

					// For whatever reason, SymLoadModule can return zero, but
					// it still loads the modules.  Sheez.

					GetModuleFileNameA( stME32.hModule, szModuleName, _countof(szModuleName) );
					SymLoadModule64( GetCurrentProcess(), hFile, szModuleName, NULL, (DWORD64)stME32.modBaseAddr, 0 );
					CloseHandle( hFile );
				}while( Module32Next( hShot, &stME32 ) );
			}
			return true;
		}

		//===========================================================================
		// Open the report file, and write the desired information to it.  Called by 
		// WheatyUnhandledExceptionFilter                                               
		//===========================================================================
		void WheatyExceptionReport::GenerateExceptionReport(
			PEXCEPTION_POINTERS pExceptionInfo )
		{
			//ASSERT(false);
			// Start out with a banner

			// 打印系统信息
			DumpSystemInformation();

			_tprintf(_T("//=====================================================\r\n"));

			PEXCEPTION_RECORD pExceptionRecord = pExceptionInfo->ExceptionRecord;

			// First print information about the type of fault
			_tprintf(   _T("Exception code: %08X %s\r\n"),
						pExceptionRecord->ExceptionCode,
						GetExceptionString(pExceptionRecord->ExceptionCode) );

			// Now print information about where the fault occured
			TCHAR szFaultingModule[MAX_PATH];
			DWORD section, offset;
			GetLogicalAddress(  pExceptionRecord->ExceptionAddress,
								szFaultingModule,
								sizeof( szFaultingModule ),
								section, offset );

			_tprintf( _T("Fault address:  %08X %02X:%08X %s\r\n"),
						pExceptionRecord->ExceptionAddress,
						section, offset, szFaultingModule );

			PCONTEXT pCtx = pExceptionInfo->ContextRecord;

			// Show the registers
			#ifdef _M_IX86  // X86 Only!
			_tprintf( _T("\r\nRegisters:\r\n") );

			_tprintf(_T("EAX:%08X\r\nEBX:%08X\r\nECX:%08X\r\nEDX:%08X\r\nESI:%08X\r\nEDI:%08X\r\n")
					,pCtx->Eax, pCtx->Ebx, pCtx->Ecx, pCtx->Edx,
					pCtx->Esi, pCtx->Edi );

			_tprintf( _T("CS:EIP:%04X:%08X\r\n"), pCtx->SegCs, pCtx->Eip );
			_tprintf( _T("SS:ESP:%04X:%08X  EBP:%08X\r\n"),
						pCtx->SegSs, pCtx->Esp, pCtx->Ebp );
			_tprintf( _T("DS:%04X  ES:%04X  FS:%04X  GS:%04X\r\n"),
						pCtx->SegDs, pCtx->SegEs, pCtx->SegFs, pCtx->SegGs );
			_tprintf( _T("Flags:%08X\r\n"), pCtx->EFlags );

			#endif

			SymInit( pCtx );

			CONTEXT trashableContext = *pCtx;

			#ifdef _M_IX86  // X86 Only!

			_tprintf( _T("========================\r\n") );
			_tprintf( _T("Local Variables And Parameters\r\n") );

			trashableContext = *pCtx;
			WriteStackDetails( &trashableContext, true );

			//_tprintf( _T("========================\r\n") );
			//_tprintf( _T("Global Variables\r\n") );

			//SymEnumSymbols( GetCurrentProcess(),
			//                (DWORD64)GetModuleHandle(szFaultingModule),
			//                0, EnumerateSymbolsCallback, 0 );
			#else
			WriteStackDetails( &trashableContext, false );
    
			#endif      // X86 Only!

			DumpModuleList();
			SymCleanup( GetCurrentProcess() );

			// MiniDump File
			TCHAR szDumpname[1024];
			_sntprintf( szDumpname, _countof(szDumpname), _T("%s.dmp"), m_szLogFileName );

			// Create the file
			HANDLE hMiniDumpFile = CreateFile(
				szDumpname,
				GENERIC_WRITE,
				0,
				NULL,
				CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
				NULL);

			// Write the minidump to the file
			if (hMiniDumpFile != INVALID_HANDLE_VALUE)
			{
				DumpMiniDump(hMiniDumpFile, pExceptionInfo );

				// Close file
				CloseHandle(hMiniDumpFile);
			}

			_tprintf( _T("\r\n") );
		}

		//======================================================================
		// Given an exception code, returns a pointer to a static string with a 
		// description of the exception                                         
		//======================================================================
		LPTSTR WheatyExceptionReport::GetExceptionString( DWORD dwCode )
		{
			#define EXCEPTION( x ) case EXCEPTION_##x: return _T(#x);

			switch ( dwCode )
			{
				EXCEPTION( ACCESS_VIOLATION )
				EXCEPTION( DATATYPE_MISALIGNMENT )
				EXCEPTION( BREAKPOINT )
				EXCEPTION( SINGLE_STEP )
				EXCEPTION( ARRAY_BOUNDS_EXCEEDED )
				EXCEPTION( FLT_DENORMAL_OPERAND )
				EXCEPTION( FLT_DIVIDE_BY_ZERO )
				EXCEPTION( FLT_INEXACT_RESULT )
				EXCEPTION( FLT_INVALID_OPERATION )
				EXCEPTION( FLT_OVERFLOW )
				EXCEPTION( FLT_STACK_CHECK )
				EXCEPTION( FLT_UNDERFLOW )
				EXCEPTION( INT_DIVIDE_BY_ZERO )
				EXCEPTION( INT_OVERFLOW )
				EXCEPTION( PRIV_INSTRUCTION )
				EXCEPTION( IN_PAGE_ERROR )
				EXCEPTION( ILLEGAL_INSTRUCTION )
				EXCEPTION( NONCONTINUABLE_EXCEPTION )
				EXCEPTION( STACK_OVERFLOW )
				EXCEPTION( INVALID_DISPOSITION )
				EXCEPTION( GUARD_PAGE )
				EXCEPTION( INVALID_HANDLE )
			}

			// If not one of the "known" exceptions, try to get the string
			// from NTDLL.DLL's message table.

			static TCHAR szBuffer[512] = { 0 };

			FormatMessage( FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
						   GetModuleHandle( _T("NTDLL.DLL") ),
						   dwCode, 0, szBuffer, sizeof( szBuffer ), 0 );

			return szBuffer;
		}

		//=============================================================================
		// Given a linear address, locates the module, section, and offset containing  
		// that address.                                                               
		//                                                                             
		// Note: the szModule paramater buffer is an output buffer of length specified 
		// by the len parameter (in characters!)                                       
		//=============================================================================
		BOOL WheatyExceptionReport::GetLogicalAddress(
				PVOID addr, PTSTR szModule, DWORD len, DWORD& section, DWORD& offset )
		{
			MEMORY_BASIC_INFORMATION mbi;

			if ( !VirtualQuery( addr, &mbi, sizeof(mbi) ) )
				return FALSE;

			DWORD_PTR hMod = (DWORD_PTR)mbi.AllocationBase;

			if ( !GetModuleFileName( (HMODULE)hMod, szModule, len ) )
				return FALSE;

			// Point to the DOS header in memory
			PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)hMod;

			// From the DOS header, find the NT (PE) header
			PIMAGE_NT_HEADERS pNtHdr = (PIMAGE_NT_HEADERS)(hMod + pDosHdr->e_lfanew);

			PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION( pNtHdr );

			DWORD_PTR rva = (DWORD_PTR)addr - hMod; // RVA is offset from module load address

			// Iterate through the section table, looking for the one that encompasses
			// the linear address.
			for (   unsigned i = 0;
					i < pNtHdr->FileHeader.NumberOfSections;
					i++, pSection++ )
			{
				DWORD sectionStart = pSection->VirtualAddress;
				DWORD sectionEnd = sectionStart
							+ max(pSection->SizeOfRawData, pSection->Misc.VirtualSize);

				// Is the address in this section???
				if ( (rva >= sectionStart) && (rva <= sectionEnd) )
				{
					// Yes, address is in the section.  Calculate section and offset,
					// and store in the "section" & "offset" params, which were
					// passed by reference.
					section = i+1;
					offset = (DWORD)( rva - sectionStart );
					return TRUE;
				}
			}

			return FALSE;   // Should never get here!
		}

		//============================================================
		// Walks the stack, and writes the results to the report file 
		//============================================================
		void WheatyExceptionReport::WriteStackDetails(
				PCONTEXT pContext,
				bool bWriteVariables )  // true if local/params should be output
		{
			_tprintf( _T("\r\nCall stack:\r\n") );

			_tprintf( _T("Address   Frame     Function            SourceFile\r\n") );

			DWORD dwMachineType = 0;
			// Could use SymSetOptions here to add the SYMOPT_DEFERRED_LOADS flag

			STACKFRAME sf;
			memset( &sf, 0, sizeof(sf) );

			#ifdef _M_IX86
			// Initialize the STACKFRAME structure for the first call.  This is only
			// necessary for Intel CPUs, and isn't mentioned in the documentation.
			sf.AddrPC.Offset       = pContext->Eip;
			sf.AddrPC.Mode         = AddrModeFlat;
			sf.AddrStack.Offset    = pContext->Esp;
			sf.AddrStack.Mode      = AddrModeFlat;
			sf.AddrFrame.Offset    = pContext->Ebp;
			sf.AddrFrame.Mode      = AddrModeFlat;

			dwMachineType = IMAGE_FILE_MACHINE_I386;
			#endif

			while ( 1 )
			{
				// Get the next stack frame
				if ( ! StackWalk(  dwMachineType,
									m_hProcess,
									GetCurrentThread(),
									&sf,
									pContext,
									0,
									SymFunctionTableAccess,
									SymGetModuleBase,
									0 ) )
					break;

				if ( 0 == sf.AddrFrame.Offset ) // Basic sanity check to make sure
					break;                      // the frame is OK.  Bail if not.

				_tprintf( _T("%08X  %08X  "), sf.AddrPC.Offset, sf.AddrFrame.Offset );

				// Get the name of the function for this stack frame entry
				BYTE symbolBuffer[ sizeof(SYMBOL_INFO) + 1024 ];
				PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)symbolBuffer;
				pSymbol->SizeOfStruct = sizeof(symbolBuffer);
				pSymbol->MaxNameLen = 1024;
                        
				DWORD64 symDisplacement = 0;    // Displacement of the input address,
												// relative to the start of the symbol

				if ( SymFromAddr(m_hProcess,sf.AddrPC.Offset,&symDisplacement,pSymbol))
				{
					xgc_lpctstr p = XA2T(pSymbol->Name);
					_tprintf( _T("%hs+%I64X"), p, symDisplacement );
            
				}
				else    // No symbol found.  Print out the logical address instead.
				{
					TCHAR szModule[MAX_PATH] = _T("");
					DWORD section = 0, offset = 0;

					GetLogicalAddress(  (PVOID)(DWORD_PTR)sf.AddrPC.Offset,
										szModule, sizeof(szModule), section, offset );

					_tprintf( _T("%04X:%08X %s"), section, offset, szModule );
				}

				// Get the source line for this stack frame entry
				IMAGEHLP_LINE lineInfo = { sizeof(IMAGEHLP_LINE) };
				DWORD dwLineDisplacement = 0;
				if ( SymGetLineFromAddr( m_hProcess, sf.AddrPC.Offset,
										&dwLineDisplacement, &lineInfo ) )
				{
					xgc_string p = XA2T(lineInfo.FileName);
					_tprintf( _T("  %s line %u"), p.c_str(), lineInfo.LineNumber );
				}

				_tprintf( _T("\r\n") );

				// Write out the variables, if desired
				if ( bWriteVariables )
				{
					// Use SymSetContext to get just the locals/params for this frame
					IMAGEHLP_STACK_FRAME imagehlpStackFrame;
					imagehlpStackFrame.InstructionOffset = sf.AddrPC.Offset;
					SymSetContext( m_hProcess, &imagehlpStackFrame, 0 );

					// Enumerate the locals/parameters
					SymEnumSymbols( m_hProcess, 0, 0, EnumerateSymbolsCallback, &sf );

					//_tprintf( _T("\r\n") );
				}
			}

		}

		//////////////////////////////////////////////////////////////////////////////
		// The function invoked by SymEnumSymbols
		//////////////////////////////////////////////////////////////////////////////

		BOOL CALLBACK
		WheatyExceptionReport::EnumerateSymbolsCallback(
			PSYMBOL_INFO  pSymInfo,
			ULONG         SymbolSize,
			PVOID         UserContext )
		{
			__try
			{
				FormatSymbolValue( pSymInfo, (STACKFRAME*)UserContext );
			}
			__except( 1 )
			{
				_tprintf( _T("punting on symbol %s\r\n"), pSymInfo->Name );
			}

			return TRUE;
		}

		//////////////////////////////////////////////////////////////////////////////
		// Given a SYMBOL_INFO representing a particular variable, displays its
		// contents.  If it's a user defined type, display the members and their
		// values.
		//////////////////////////////////////////////////////////////////////////////
		bool WheatyExceptionReport::FormatSymbolValue(
					PSYMBOL_INFO pSym,
					STACKFRAME * sf )
		{
			// Indicate if the variable is a local or parameter
			if ( pSym->Flags & IMAGEHLP_SYMBOL_INFO_PARAMETER )
				_printf( "Parameter " );
			else if ( pSym->Flags & IMAGEHLP_SYMBOL_INFO_LOCAL )
				_printf( "Local " );

			// If it's a function, don't do anything.
			if ( pSym->Tag == 5 )   // SymTagFunction from CVCONST.H from the DIA SDK
				return false;

			// Emit the variable name
			_printf( "\'%s\'", pSym->Name );

			DWORD_PTR pVariable = 0;    // Will point to the variable's data in memory

			if ( pSym->Flags & IMAGEHLP_SYMBOL_INFO_REGRELATIVE )
			{
				// if ( pSym->Register == 8 )   // EBP is the value 8 (in DBGHELP 5.1)
				{                               //  This may change!!!
					pVariable = sf->AddrFrame.Offset;
					pVariable += (DWORD_PTR)pSym->Address;
				}
				// else
				//  return false;
			}
			else if ( pSym->Flags & IMAGEHLP_SYMBOL_INFO_REGISTER )
			{
				return false;   // Don't try to report register variable
			}
			else
			{
				pVariable = (DWORD_PTR)pSym->Address;   // It must be a global variable
			}

			// Determine if the variable is a user defined type (UDT).  IF so, bHandled
			// will return true.
			bool bHandled;
			DumpTypeIndex( pSym->ModBase, pSym->TypeIndex, 0, pVariable, bHandled );

			if ( !bHandled )
			{
				// The symbol wasn't a UDT, so do basic, stupid formatting of the
				// variable.  Based on the size, we're assuming it's a char, WORD, or
				// DWORD.
				BasicType basicType = GetBasicType( pSym->TypeIndex, pSym->ModBase );
        
				FormatOutputValue( basicType, pSym->Size, (PVOID)pVariable ); 
				_printf( "\r\n" );
			}


			return true;
		}

		//////////////////////////////////////////////////////////////////////////////
		// If it's a user defined type (UDT), recurse through its members until we're
		// at fundamental types.  When he hit fundamental types, return
		// bHandled = false, so that FormatSymbolValue() will format them.
		//////////////////////////////////////////////////////////////////////////////
		void WheatyExceptionReport::DumpTypeIndex(
				DWORD64 modBase,
				DWORD dwTypeIndex,
				unsigned nestingLevel,
				DWORD_PTR offset,
				bool & bHandled )
		{
			if( nestingLevel > 3 ) return;
			bHandled = false;

			// Get the name of the symbol.  This will either be a Type name (if a UDT),
			// or the structure member name.
			WCHAR * pwszTypeName;
			if ( SymGetTypeInfo( m_hProcess, modBase, dwTypeIndex, TI_GET_SYMNAME,
								&pwszTypeName ) )
			{
				if( pwszTypeName[0] == 0 )
				{
					LocalFree( pwszTypeName );
					return ;
				}

				_tprintf( _T(" %ls"), pwszTypeName );
				LocalFree( pwszTypeName );
			}
			else
				return;

			// Determine how many children this type has.
			DWORD dwChildrenCount = 0;
			SymGetTypeInfo( m_hProcess, modBase, dwTypeIndex, TI_GET_CHILDRENCOUNT,
							&dwChildrenCount );

			if ( !dwChildrenCount )     // If no children, we're done
				return;

			// Prepare to get an array of "TypeIds", representing each of the children.
			// SymGetTypeInfo(TI_FINDCHILDREN) expects more memory than just a
			// TI_FINDCHILDREN_PARAMS struct has.  Use derivation to accomplish this.
			struct FINDCHILDREN : TI_FINDCHILDREN_PARAMS
			{
				ULONG   MoreChildIds[1024];
				FINDCHILDREN(){Count = sizeof(MoreChildIds) / sizeof(MoreChildIds[0]);}
			} children;

			children.Count = dwChildrenCount;
			children.Start= 0;

			// Get the array of TypeIds, one for each child type
			if ( !SymGetTypeInfo( m_hProcess, modBase, dwTypeIndex, TI_FINDCHILDREN,
									&children ) )
			{
				return;
			}

			// Append a line feed
			_printf( "\r\n" );

			// Iterate through each of the children
			for ( unsigned i = 0; i < dwChildrenCount; i++ )
			{
				// Add appropriate indentation level (since this routine is recursive)
				for ( unsigned j = 0; j <= nestingLevel+1; j++ )
					_printf( "\t" );

				// Recurse for each of the child types
				bool bHandled2;
				DumpTypeIndex( modBase,
							   children.ChildId[i], nestingLevel+1,
							   offset, bHandled2 );

				// If the child wasn't a UDT, format it appropriately
				if ( !bHandled2 )
				{
					// Get the offset of the child member, relative to its parent
					DWORD dwMemberOffset;
					SymGetTypeInfo( m_hProcess, modBase, children.ChildId[i],
									TI_GET_OFFSET, &dwMemberOffset );

					// Get the real "TypeId" of the child.  We need this for the
					// SymGetTypeInfo( TI_GET_TYPEID ) call below.
					DWORD typeId;
					SymGetTypeInfo( m_hProcess, modBase, children.ChildId[i],
									TI_GET_TYPEID, &typeId );

					// Get the size of the child member
					ULONG64 length;
					SymGetTypeInfo(m_hProcess, modBase, typeId, TI_GET_LENGTH,&length);

					// Calculate the address of the member
					DWORD_PTR dwFinalOffset = offset + dwMemberOffset;

					BasicType basicType = GetBasicType(children.ChildId[i], modBase );

					FormatOutputValue( basicType, length, (PVOID)dwFinalOffset ); 

					_printf( "\r\n" );
				}
			}

			bHandled = true;
		}

		void WheatyExceptionReport::FormatOutputValue(    BasicType basicType,
															DWORD64 length,
															PVOID pAddress )
		{
			// Format appropriately (assuming it's a 1, 2, or 4 bytes (!!!)
			if ( length == 1 )
				_printf( " = %X", *(PBYTE)pAddress );
			else if ( length == 2 )
				_printf( " = %X", *(PWORD)pAddress );
			else if ( length == 4 )
			{
				if ( basicType == btFloat )
				{
					_printf( " = %f", *(PFLOAT)pAddress);
				}
				else if ( basicType == btChar )
				{
					if ( !IsBadStringPtr( *(PTSTR*)pAddress, 32) )
					{
						_printf( " = \"%.31s\"", *(PDWORD)pAddress );
					}
					else
						_printf( " = %X", *(PDWORD)pAddress );
				}
				else
					_printf( " = %X", *(PDWORD)pAddress);
			}
			else if ( length == 8 )
			{
				if ( basicType == btFloat )
				{
					_printf( " = %lf", *(double *)pAddress );
				}
				else
					_printf( " = %I64X", *(DWORD64*)pAddress );
			}
		}

		BasicType
		WheatyExceptionReport::GetBasicType( DWORD typeIndex, DWORD64 modBase )
		{
			BasicType basicType;
			if ( SymGetTypeInfo( m_hProcess, modBase, typeIndex,
								TI_GET_BASETYPE, &basicType ) )
			{
				return basicType;
			}

			// Get the real "TypeId" of the child.  We need this for the
			// SymGetTypeInfo( TI_GET_TYPEID ) call below.
			DWORD typeId;
			if (SymGetTypeInfo(m_hProcess,modBase, typeIndex, TI_GET_TYPEID, &typeId))
			{
				if ( SymGetTypeInfo( m_hProcess, modBase, typeId, TI_GET_BASETYPE,
									&basicType ) )
				{
					return basicType;
				}
			}

			return btNoType;
		}

		//============================================================================
		// Helper function that writes to the report file, and allows the user to use 
		// printf style formating                                                     
		//============================================================================
		int __cdecl WheatyExceptionReport::_tprintf(const TCHAR * format, ...)
		{
			TCHAR szBuff[1024];
			int retValue;
			DWORD cbWritten;
			va_list argptr;
          
			va_start( argptr, format );
			retValue = _vsntprintf( szBuff, _countof(szBuff), format, argptr );
			va_end( argptr );

			//szBuff[retValue] = 0;
			std::string p = XT2A(szBuff);
			WriteFile(m_hReportFile, p.c_str(), p.length(), &cbWritten, 0 );

			return retValue;
		}

		//============================================================================
		// Helper function that writes to the report file, and allows the user to use 
		// printf style formating                                                     
		//============================================================================
		int __cdecl WheatyExceptionReport::_printf(const CHAR * format, ...)
		{
			CHAR szBuff[1024];
			int retValue;
			DWORD cbWritten;
			va_list argptr;
          
			va_start( argptr, format );
			retValue = _vsnprintf( szBuff, sizeof(szBuff), format, argptr );
			va_end( argptr );

			WriteFile(m_hReportFile, szBuff, retValue, &cbWritten, 0 );

			return retValue;
		}
	}
}