#pragma once
//////////////////////////////////////////////////////////////////////////
// #include "GroupLine.h"
//////////////////////////////////////////////////////////////////////////


// *** NOTE *** read the NOTE ON LINKING (in the .cpp file)
class CGroupLine : public CStatic 
{
	DECLARE_DYNAMIC(CGroupLine)

public:
	// controls use of themes (aka visual styles)
	enum eThemeControl {
		eThemeAutomatic, // checks if themes are used for the app
		eThemeOff,       // no use of themes
		eThemeOn,        // use themes
	};
	
	// controls how the disabled state is drawn
	enum eDisabledThemeStyle {
		eGrayIfDisabled, // draws disabled state with typical disabled gray text
		eSameAsEnabled,  // draws disabled state the same as the enabled state (default for XP Themed GroupBoxes)
	};

	CGroupLine(eThemeControl useTheme = eThemeAutomatic, eDisabledThemeStyle disabledThemeStyle = eGrayIfDisabled);
	virtual ~CGroupLine();

	void SetUseTheme(eThemeControl useTheme);
	void SetDisabledThemeStyle(eDisabledThemeStyle disabledThemeStyle);

protected:
	eThemeControl m_useTheme;          // to force the theme drawing on or off
	eThemeControl m_automaticUseTheme; // is rest of the program using themes
	eDisabledThemeStyle m_disabledThemeStyle;
	
	static void DrawLine(CDC& dc, bool drawAsThemed, int height, int left, int right);
	static BOOL IsThemed();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
};


