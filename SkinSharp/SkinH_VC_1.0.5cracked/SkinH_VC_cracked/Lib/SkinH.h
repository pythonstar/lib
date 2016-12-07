#ifndef SKIN_H
#define SKIN_H

	extern "C"
	{
	__declspec(dllimport) int  __stdcall SkinH_Attach();
	__declspec(dllimport) int  __stdcall SkinH_Attach_Ex(LPCTSTR strSkinFile,LPCTSTR strPassWd);
	__declspec(dllimport) int  __stdcall SkinH_Detach();
	__declspec(dllimport) int  __stdcall SkinH_Detach_Ex(HWND hWnd);
	__declspec(dllimport) int  __stdcall SkinH_Attach_Ext(LPCTSTR strSkinFile,CHAR strPassWd[16],int nHue,int nSat,int nBri);
	__declspec(dllimport) int  __stdcall SkinH_Attach_Res(LPBYTE pShe,DWORD dwSize,CHAR strPassWd[16],int nHue,int nSat,int nBri);
	__declspec(dllimport) int  __stdcall SkinH_Alpha(HWND hWnd,int nAlpha);
	__declspec(dllimport) int  __stdcall SkinH_Adjust_HSV(int nHue,int nSat,int nBri);
	__declspec(dllimport) int  __stdcall SkinH_GetColor(HWND hWnd,int nX,int nY);
	__declspec(dllimport) int  __stdcall SkinH_Map(HWND hWnd,int nType);
	__declspec(dllimport) int  __stdcall SkinH_Aero(int bAero);
	__declspec(dllimport) int  __stdcall SkinH_Aero_Adjust(int nAlpha, int nShwDark, int nShwSharp,int nShwSize,int nX,int nY,int nRed,int nGreen,int nBlue);
	__declspec(dllimport) int  __stdcall SkinH_DisMove(HWND hWnd,BOOL bMove);
	__declspec(dllimport) int  __stdcall SkinH_BGColor(HWND hWnd,int nRed, int nGreen, int nBlue);
	__declspec(dllimport) int  __stdcall SkinH_TextColor(HWND hWnd,int nRed, int nGreen, int nBlue);
	__declspec(dllimport) int  __stdcall SkinH_LockUpdate(HWND hWnd,int bUpdate);
	__declspec(dllimport) int  __stdcall SkinH_NineBlt(HDC hDtDC,int left,int top,int right, int bottom, int nMRect);
	__declspec(dllimport) int  __stdcall SkinH_Alpha_Menu(int nAlpha);
	}
#endif