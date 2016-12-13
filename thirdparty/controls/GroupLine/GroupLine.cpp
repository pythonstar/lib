//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "GroupLine.h"
#include <Shlwapi.h>  // for IsThemed() function
#include <Uxtheme.h>  // for CloseThemeData (etc)
#include <TmSchema.h> // for BP_GROUPBOX (etc)
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
#pragma comment(lib, "uxtheme.lib")
// *** NOTE ON LINKING ***
//
// If you are going to use this in a program that should run on
// versions of windows before XP (e.g. Windows 2000), you will likely
// want to delay-load the UxTheme.Dll in the project settings.
//
// For Example (in Visual Studio 2003)
// Configuration Properties->Linker->Input->Delay Loaded Dlls->Add-->"UxTheme.DLL" (without the quotes)
// You may also have to add support for delay loading DLLs:
// Configuration Properties->Linker->Input->Additional Depenedencies->Add->"delayimp.lib" (without the quotes)
//
//////////////////////////////////////////////////////////////////////////
// Unfortunately, we cannot use:
// #pragma comment(linker, "/DelayLoad:UxTheme.dll")
// because:
//////////////////////////////////////////////////////////////////////////
// http://dotnet247.com/247reference/msgs/41/208171.aspx
//
// Delay loading is fine. Adding a delayload by using the pragma isn't guaranteed to work.
// The problem is that the linker parses the pragma generated data beyond the
// point where it already made some decisions that it cannot undo that _may_
// conflict with delayloading this dll.
// 
// Ronald Laeremans [MSFT] (VIP)
//////////////////////////////////////////////////////////////////////////


IMPLEMENT_DYNAMIC(CGroupLine, CStatic)
CGroupLine::CGroupLine(eThemeControl useTheme /*= eThemeAutomatic*/, eDisabledThemeStyle disabledThemeStyle /*= eGrayIfDisabled*/)
	: m_automaticUseTheme(eThemeAutomatic)
	, m_useTheme(useTheme)
	, m_disabledThemeStyle(disabledThemeStyle)
{ }

CGroupLine::~CGroupLine()
{ }

void CGroupLine::SetUseTheme(eThemeControl useTheme){
	m_useTheme = useTheme;
}
void CGroupLine::SetDisabledThemeStyle(eDisabledThemeStyle disabledThemeStyle){
	m_disabledThemeStyle = disabledThemeStyle;
}

BEGIN_MESSAGE_MAP(CGroupLine, CStatic)
	ON_WM_PAINT()
END_MESSAGE_MAP()

void CGroupLine::OnPaint()
{
	if(eThemeAutomatic==m_automaticUseTheme){ // uninitialized
		m_automaticUseTheme = (IsThemed()) ? (eThemeOn) : (eThemeOff);
	}

	const bool drawAsEnabled(0!=IsWindowEnabled());

	const bool drawAsThemed( (eThemeOn==m_useTheme) // force themes ON
		|| ((eThemeAutomatic==m_useTheme) && (eThemeOn==m_automaticUseTheme)) // use automatic, and automatic is ON
		);

	CPaintDC dc(this); // device context for painting

	// Get the text
	CString text;
	GetWindowText(text);

	// Get the rectangle
	CRect clientRect;
	GetClientRect(&clientRect);
	CRect textRect(clientRect);
	CRect lineRect(clientRect);

	// Need to select the font (or it will be UGLY)
	CFont* pOldFont(dc.SelectObject(GetFont()));
	const int oldBkMode(dc.SetBkMode(TRANSPARENT)); // needed for non-themed text
	
	// Style is set using the "Align Text" property in the dialog editor, or as a parameter to CreateWindow or CreateWindowEx
	const DWORD windowStyle(GetStyle());
	
	HTHEME theme = (drawAsThemed) ? (::OpenThemeData(this->m_hWnd, L"BUTTON")) : (NULL);

	// Calculate the text size
	CSize textSize(0,0);
	if(NULL!=theme){
		CRect textSizeRect(0,0,0,0);
		const CStringW wideString(text);
		::GetThemeTextExtent(theme, dc.m_hDC, BP_GROUPBOX, GBS_NORMAL, wideString, -1, DT_LEFT | DT_SINGLELINE, NULL, &textSizeRect);
		textSize = textSizeRect.Size();
	}else{
		textSize =  dc.GetTextExtent(text);
	}
	
	
	const int offsetSize(textSize.cy / 2); // offset the start of the line a little bit from the edge of the text
	const DWORD textDrawingFlags(DT_SINGLELINE | DT_NOCLIP | DT_LEFT);

	int lineHeight(0);
	if(SS_CENTERIMAGE & windowStyle){ // text and line are vertically centered
		textRect.top = clientRect.CenterPoint().y - (textSize.cy / 2);
		lineHeight = lineRect.CenterPoint().y;
	}else{ // text and line run along the top edge of the control
		lineHeight = textSize.cy / 2;
	}
	
	CPoint textLocation(0,0); // have to calculate, since DrawState does not have a center flag (is there a DSS_CENTER, and I just missed it ???)

	// Draw The Line(s)
	if(SS_RIGHT & windowStyle){ // right-aligned text
		textRect.left = textRect.right - textSize.cx;
		DrawLine(dc, drawAsThemed, lineHeight,
			lineRect.left,
			lineRect.right - (textSize.cx + offsetSize));

	}else if (SS_CENTER & windowStyle){ // centered text		
		const int eachSideWidth = (clientRect.Width() - (textSize.cx + offsetSize*2)) / 2;
		textRect.left = eachSideWidth + offsetSize;

		DrawLine(dc, drawAsThemed, lineHeight,
			lineRect.left,
			lineRect.left + eachSideWidth);
		
		DrawLine(dc, drawAsThemed, lineHeight,
			lineRect.right - eachSideWidth,
			lineRect.right);

	}else{ // left-aligned text

		DrawLine(dc, drawAsThemed, lineHeight,
			lineRect.left + (textSize.cx + offsetSize),
			lineRect.right);
	}

	// Draw the text
	const bool useThemeDrawing( (drawAsEnabled) ? (drawAsThemed) : (eSameAsEnabled==m_disabledThemeStyle) );
	if(drawAsThemed && useThemeDrawing){
		const int drawThemeFlags(drawAsEnabled ? GBS_NORMAL : GBS_DISABLED); // NOTE: by default GBS_DISABLED looks the same as GBS_NORMAL
		const CStringW wideString(text);
		::DrawThemeText(theme, dc.m_hDC, BP_GROUPBOX, drawThemeFlags, wideString, -1, textDrawingFlags, NULL, &textRect);
	}else{
		const UINT drawStateFlags(drawAsEnabled ? DSS_NORMAL : DSS_DISABLED);
		dc.DrawState(
			CPoint(textRect.left, textRect.top),
			CSize(0,0), // 0,0 says to calculate it (per DrawState SDK docs)
			text, drawStateFlags, TRUE, 0, (HBRUSH)NULL);
	}

	if(NULL!=theme){
		::CloseThemeData(theme);
	}
	
	dc.SetBkMode(oldBkMode);
	if(NULL!=pOldFont){
		dc.SelectObject(pOldFont);
	}
	// Auto-generated note --> Do not call CStatic::OnPaint() for painting messages
}

void CGroupLine::DrawLine(CDC& dc, bool drawAsThemed, int height, int left, int right)
{
	if(drawAsThemed){
		// NOTE: color is hard-coded here. It should probably use something like 
		// DrawThemeEdge / DrawThemeBackground / GetThemeColor - but I could not find
		// the correct set of parameters (there is  a DrawThemeEdge call that gives a similar line, but slightly darker)
		CPen pen(PS_SOLID, 1, RGB(208,208,191));
		CPen* pOldPen = dc.SelectObject(&pen);
		
		dc.MoveTo(left , height);
		dc.LineTo(right, height);
		
		if(NULL!=pOldPen){
			dc.SelectObject(pOldPen);
		}
	}else{		
		CPen penTop(PS_SOLID, 1, ::GetSysColor(COLOR_3DHIGHLIGHT));
		CPen* pOldPen = dc.SelectObject(&penTop);
		dc.MoveTo(left , height);
		dc.LineTo(right, height);
		
		CPen penBottom(PS_SOLID, 1, ::GetSysColor(COLOR_3DSHADOW));
		dc.SelectObject(&penBottom);
		dc.MoveTo(left , height - 1);
		dc.LineTo(right, height - 1);

		if(NULL!=pOldPen){
			dc.SelectObject(pOldPen);
		}
	}
}


BOOL CGroupLine::IsThemed()
{
	//////////////////////////////////////////////////////////////////////////
	// 99% of this function is taken from the following article:
	//
	// How to accurately detect if an application is theme-enabled?
	// By Nishant Sivakumar
	// http://www.codeproject.com/tips/DetectTheme.asp
	//
	// Except: 
	//  1) changed the version check to include Windows versions beyond XP
	//  2) _T() macro removed from GetProcAddress functions
	//
	//////////////////////////////////////////////////////////////////////////

	BOOL ret = FALSE;
	OSVERSIONINFO ovi = {0};
	ovi.dwOSVersionInfoSize = sizeof ovi;
	GetVersionEx(&ovi);	
	if(   (ovi.dwMajorVersion  > 5) ||
	    ( (ovi.dwMajorVersion == 5) && (ovi.dwMinorVersion >= 1) ) )
	{
		//Windows >= XP detected
		typedef BOOL WINAPI ISAPPTHEMED();
		typedef BOOL WINAPI ISTHEMEACTIVE();
		ISAPPTHEMED* pISAPPTHEMED = NULL;
		ISTHEMEACTIVE* pISTHEMEACTIVE = NULL;
		HMODULE hMod = LoadLibrary(_T("uxtheme.dll"));
		if(hMod)
		{
			pISAPPTHEMED = reinterpret_cast<ISAPPTHEMED*>(
				GetProcAddress(hMod, "IsAppThemed"));
			pISTHEMEACTIVE = reinterpret_cast<ISTHEMEACTIVE*>(
				GetProcAddress(hMod, "IsThemeActive"));
			if(pISAPPTHEMED && pISTHEMEACTIVE)
			{
				if(pISAPPTHEMED() && pISTHEMEACTIVE())                
				{                
					typedef HRESULT CALLBACK DLLGETVERSION(DLLVERSIONINFO*);
					DLLGETVERSION* pDLLGETVERSION = NULL;

					HMODULE hModComCtl = LoadLibrary(_T("comctl32.dll"));
					if(hModComCtl)
					{
						pDLLGETVERSION = reinterpret_cast<DLLGETVERSION*>(
							GetProcAddress(hModComCtl, "DllGetVersion"));
						if(pDLLGETVERSION)
						{
							DLLVERSIONINFO dvi = {0};
							dvi.cbSize = sizeof dvi;
							if(pDLLGETVERSION(&dvi) == NOERROR )
							{
								ret = dvi.dwMajorVersion >= 6;
							}
						}
						FreeLibrary(hModComCtl);                    
					}
				}
			}
			FreeLibrary(hMod);
		}
	}    
	return ret;
}
