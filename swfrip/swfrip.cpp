#include <windows.h>
#include <commctrl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "resource.h"
#include "swf.h"

#define ZeroCounts() ulShape=ulImage=ulMorph=ulFont=ulText=ulSound=ulButton=ulSprite=ulVideo=ulFrame=ulScript=0;

HWND		hMainWnd, hTreeView;
HMENU		hMainMenu;
HINSTANCE	hInst;
HTREEITEM	htiShape, htiImage, htiMorph, htiFont, htiText, htiSound;
HTREEITEM	htiButton, htiSprite, htiVideo, htiFrame, htiScript, htiCurSel;
UI32		ulShape, ulImage, ulMorph, ulFont, ulText;
UI32		ulSound, ulButton, ulSprite, ulVideo, ulFrame, ulScript;

SWFINFO swfiInfo;

char szFileName[4096];
char *szBaseName;
char szTitle[512];
UI16 usCharacter;

CSWF cswfLoaded;

char szOpenFileFilter[]=
	{"Macromedia Flash Files (*.swf)\0" "*.swf;\0"
	 "All Files (*.*)\0" "*.*\0\0"};

char szSaveFileFilter[]=
	{"Macromedia Flash Files (*.swf)\0" "*.swf;\0"
	 /*"ScriptSWF Files (*.sswf)\0" "*.sswf;\0"*/
	 "All Files (*.*)\0" "*.*\0\0"};

int GetOFN(char *szOpenName, const char *szFilter)
{
	char szNameTemp[100]="\0"; 

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize =		sizeof(OPENFILENAME);
	ofn.hwndOwner=			hMainWnd;
	ofn.lpstrInitialDir =	szFileName;
	ofn.lpstrFilter =		szFilter;
	ofn.lpstrFile =			szNameTemp;
	ofn.nMaxFile =			0x7ff;
	ofn.lpstrTitle =		"Open File";
	ofn.Flags =				OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
	if(!GetOpenFileName(&ofn)) return 0; //if error or cancel, bail

	strcpy(szOpenName, szNameTemp);
	return 1;
}

int GetSFN(char *szSaveName, const char *szFilter)
{
	char szNameTemp[128], *szExt;
	strcpy(szNameTemp,szFileName);
	szExt=strrchr(szNameTemp,'.');
	if(szExt) *szExt='\0';
		
	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize =		sizeof(OPENFILENAME);
	ofn.hwndOwner=			hMainWnd;
	ofn.lpstrFilter =		szFilter;
	ofn.lpstrFile =			szNameTemp;
	ofn.nMaxFile =			0x7ff;
	ofn.lpstrTitle =		"Save File";
	ofn.Flags =				OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
	if(!GetSaveFileName(&ofn)) return 0; //if error or cancel, bail
		
	strcpy(szSaveName,szNameTemp);
	return ofn.nFilterIndex;
}
void SetBaseName() 
{
	if(szFileName[0]!='\0') 
		szBaseName=strrchr(szFileName, '\\'); 
	
	szBaseName++;
}
/*
void Register()
{
	HKEY hKey;
	HMODULE hThisApp;
	DWORD usDispos;
	char szExeName[MAX_FILEPATH], szValue[1024];

	hThisApp=GetModuleHandle(NULL);
	GetModuleFileName(hThisApp,szExeName,sizeof(szExeName));

	if(RegCreateKeyEx(HKEY_CLASSES_ROOT,"ShockwaveFlash.ShockwaveFlash\\shell\\SWFRIP\\command",NULL,NULL,REG_OPTION_NON_VOLATILE,
				      KEY_CREATE_SUB_KEY | KEY_ALL_ACCESS,NULL,&hKey,&usDispos))
	{
		MessageBox(hMainWnd,"Failed to register SWFRIP","Error",MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	sprintf(szValue,"\"%s\" ",szExeName);
	strcat(szValue,"%1");
	RegSetValueEx(hKey,NULL,0,REG_SZ,(const BYTE*)(LPCTSTR)szValue,strlen(szValue));
	RegCloseKey(hKey);
}
*/
int IsCategoryNode(HTREEITEM htiNode)
{
	if(htiNode==htiShape) return 1;
	if(htiNode==htiImage) return 1;
	if(htiNode==htiMorph) return 1;
	if(htiNode==htiFont) return 1;
	if(htiNode==htiText) return 1;
	if(htiNode==htiSound) return 1;
	if(htiNode==htiButton) return 1;
	if(htiNode==htiSprite) return 1;
	if(htiNode==htiVideo) return 1;
	if(htiNode==htiFrame) return 1;
	if(htiNode==htiScript) return 1;
	
	return 0;
}

void SetMenu(UI32 ulState)
{
	EnableMenuItem(hMainMenu,IDM_FILE_CLOSE,MF_BYCOMMAND | ulState);
	EnableMenuItem(hMainMenu,IDM_FILE_SAVE,MF_BYCOMMAND | ulState);
	EnableMenuItem(hMainMenu,IDM_FILE_SAVEAS,MF_BYCOMMAND | ulState);
	EnableMenuItem(hMainMenu,IDM_FILE_VIEW,MF_BYCOMMAND | ulState);
	EnableMenuItem(hMainMenu,IDM_RES_SAVE,MF_BYCOMMAND | ulState);
	EnableMenuItem(hMainMenu,IDM_RES_VIEW,MF_BYCOMMAND | ulState);
	EnableMenuItem(hMainMenu,IDM_RES_SAVETEXT,MF_BYCOMMAND | ulState);
	EnableMenuItem(hMainMenu,IDM_RES_SAVEALL,MF_BYCOMMAND | ulState);
	EnableMenuItem(hMainMenu,IDM_VIEW_INFO,MF_BYCOMMAND | ulState);
}

UI32 MakeCategories()
{
	char szText[512];
	TVINSERTSTRUCT tvisInsert;
	tvisInsert.hParent=TVI_ROOT;
	tvisInsert.hInsertAfter=TVI_ROOT; 
	tvisInsert.itemex.mask=TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvisInsert.itemex.pszText=szText;
    tvisInsert.itemex.iSelectedImage=NULL;
	
	wsprintf(szText,"Shape (%lu)",ulShape);
	tvisInsert.itemex.iImage=tvisInsert.itemex.iSelectedImage=SWF_SHAPE;
	htiShape=TreeView_InsertItem(hTreeView,&tvisInsert);
	
	wsprintf(szText,"Image (%lu)",ulImage);
	tvisInsert.itemex.iImage=tvisInsert.itemex.iSelectedImage=SWF_IMAGE;
	htiImage=TreeView_InsertItem(hTreeView,&tvisInsert);
	
	wsprintf(szText,"Morph (%lu)",ulMorph);
	tvisInsert.itemex.iImage=tvisInsert.itemex.iSelectedImage=SWF_MORPH;
	htiMorph=TreeView_InsertItem(hTreeView,&tvisInsert);

	wsprintf(szText,"Font (%lu)",ulFont);
	tvisInsert.itemex.iImage=tvisInsert.itemex.iSelectedImage=SWF_FONT;
	htiFont=TreeView_InsertItem(hTreeView,&tvisInsert);
	
	wsprintf(szText,"Text (%lu)",ulText);
	tvisInsert.itemex.iImage=tvisInsert.itemex.iSelectedImage=SWF_TEXT;
	htiText=TreeView_InsertItem(hTreeView,&tvisInsert);
	
	wsprintf(szText,"Sound (%lu)",ulSound);
	tvisInsert.itemex.iImage=tvisInsert.itemex.iSelectedImage=SWF_SOUND;
	htiSound=TreeView_InsertItem(hTreeView,&tvisInsert);
	
	wsprintf(szText,"Button (%lu)",ulButton);
	tvisInsert.itemex.iImage=tvisInsert.itemex.iSelectedImage=SWF_BUTTON;
	htiButton=TreeView_InsertItem(hTreeView,&tvisInsert);
	
	wsprintf(szText,"Sprite (%lu)",ulSprite);
	tvisInsert.itemex.iImage=tvisInsert.itemex.iSelectedImage=SWF_SPRITE;
	htiSprite=TreeView_InsertItem(hTreeView,&tvisInsert);

	wsprintf(szText,"Video (%lu)",ulVideo);
	tvisInsert.itemex.iImage=tvisInsert.itemex.iSelectedImage=SWF_VIDEO;
	htiVideo=TreeView_InsertItem(hTreeView,&tvisInsert);
	
	wsprintf(szText,"Frame (%lu)",ulFrame);
	tvisInsert.itemex.iImage=tvisInsert.itemex.iSelectedImage=SWF_FRAME;
	htiFrame=TreeView_InsertItem(hTreeView,&tvisInsert);

	wsprintf(szText,"Script (%lu)",ulScript);
	tvisInsert.itemex.iImage=tvisInsert.itemex.iSelectedImage=SWF_SCRIPT;
	htiScript=TreeView_InsertItem(hTreeView,&tvisInsert);
	
	htiCurSel=NULL;

	return 0;
}

UI32 SetCounts()
{
	TVITEM tviCurItem;
	HTREEITEM htiCategory=NULL;
	char szCategory[128];
		
	/*Get the first type item*/
	memset(&tviCurItem,0,sizeof(TVITEM));
	htiCategory=TreeView_GetChild(hTreeView,TVI_ROOT);
	
	/*Find the correct type item*/
	while(htiCategory)
	{
		/*set insert info*/
		tviCurItem.mask=TVIF_TEXT;
		tviCurItem.hItem=htiCategory;
		tviCurItem.pszText=szCategory;
		tviCurItem.cchTextMax=128;

		if(htiCategory==htiShape)	wsprintf(szCategory,"Shape (%lu)",ulShape);
		if(htiCategory==htiImage)	wsprintf(szCategory,"Image (%lu)",ulImage);
		if(htiCategory==htiMorph)	wsprintf(szCategory,"Morph (%lu)",ulMorph);
		if(htiCategory==htiFont)	wsprintf(szCategory,"Font (%lu)",ulFont);
		if(htiCategory==htiText)	wsprintf(szCategory,"Text (%lu)",ulText);
		if(htiCategory==htiSound)	wsprintf(szCategory,"Sound (%lu)",ulSound);
		if(htiCategory==htiButton)	wsprintf(szCategory,"Button (%lu)",ulButton);
		if(htiCategory==htiSprite)	wsprintf(szCategory,"Sprite (%lu)",ulSprite);
		if(htiCategory==htiVideo)	wsprintf(szCategory,"Video (%lu)",ulVideo);
		if(htiCategory==htiFrame)	wsprintf(szCategory,"Frame (%lu)",ulFrame);
		if(htiCategory==htiScript)	wsprintf(szCategory,"Script (%lu)",ulScript);
		
		/*set category text, and get the next catetory node*/
		TreeView_SetItem(hTreeView,&tviCurItem);
		htiCategory=TreeView_GetNextSibling(hTreeView,htiCategory);
	}

	return 0;
}

HTREEITEM AddItem(HTREEITEM htiParent, UI8 ubIcon, char *szTitle, UI32 usID, char *szName=NULL)
{
	char szText[512];
	TVINSERTSTRUCT tvisInsert;

	tvisInsert.hInsertAfter=TVI_LAST;
	tvisInsert.itemex.mask=TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvisInsert.itemex.pszText=szText;
	tvisInsert.hParent=htiParent;
	tvisInsert.itemex.iImage=tvisInsert.itemex.iSelectedImage=ubIcon;
	
	if(htiParent==htiShape) ulShape++;
	if(htiParent==htiImage) ulImage++;
	if(htiParent==htiMorph) ulMorph++;
	if(htiParent==htiFont) ulFont++;
	if(htiParent==htiText) ulText++;
	if(htiParent==htiSound) ulSound++;
	if(htiParent==htiButton) ulButton++;
	if(htiParent==htiSprite) ulSprite++;
	if(htiParent==htiVideo) ulVideo++;
	if(htiParent==htiFrame) ulFrame++;
	if(htiParent==htiScript) ulScript++;


	 sprintf(szText,"%s %05lu",szTitle,usID);

	 if(szName)
	 {
		 strcat(szText,"  ");
		 strcat(szText,szName);
	 }
	
	return TreeView_InsertItem(hTreeView,&tvisInsert);
}

UI32 SetTree()
{
	UI32 ulNumTags, ulNumFrames, iFrame;
	TAGINFO tiInfo;
	HTREEITEM htiParent;
	FRAMELABEL *lpflFrameLabel;

	if(!cswfLoaded.Loaded()) return 0;

	if(cswfLoaded.HasSoundStream(0)) AddItem(htiSound,SWF_SOUND,"Sound",0,"(Movie)");
	
	if(cswfLoaded.HasActions(0)) AddItem(htiScript,SWF_SCRIPT,"Script",0,"(Movie)");

	ulNumTags=cswfLoaded.GetNumTags(0);

	for(UI32 iTag=0; iTag<ulNumTags; iTag++)
	{
		cswfLoaded.GetTagInfo(0,iTag,&tiInfo);

		switch(tiInfo.usTagCode)
		{
			case TAGCODE_DEFINESHAPE:
			case TAGCODE_DEFINESHAPE2:
			case TAGCODE_DEFINESHAPE3:
				AddItem(htiShape,SWF_SHAPE,"Shape",tiInfo.usCharacterID,tiInfo.szName);
				break;

			case TAGCODE_DEFINEBITS:
			case TAGCODE_DEFINEBITSJPEG2:
			case TAGCODE_DEFINEBITSJPEG3:
			case TAGCODE_DEFINEBITSLOSSLESS:
			case TAGCODE_DEFINEBITSLOSSLESS2:
				AddItem(htiImage,SWF_IMAGE,"Image",tiInfo.usCharacterID,tiInfo.szName);
				break;

			case TAGCODE_DEFINEMORPHSHAPE:
				AddItem(htiMorph,SWF_MORPH,"Morph",tiInfo.usCharacterID,tiInfo.szName);
				break;

			case TAGCODE_DEFINEFONT:
			case TAGCODE_DEFINEFONT2:
				AddItem(htiFont,SWF_FONT,"Font",tiInfo.usCharacterID,tiInfo.szName);
				break;

			case TAGCODE_DEFINETEXT:
			case TAGCODE_DEFINETEXT2:
				AddItem(htiText,SWF_TEXT,"Text",tiInfo.usCharacterID,tiInfo.szName);
				break;

			case TAGCODE_DEFINESOUND:
				AddItem(htiSound,SWF_SOUND,"Sound",tiInfo.usCharacterID,tiInfo.szName);
				break;

			case TAGCODE_DEFINEBUTTON:
			case TAGCODE_DEFINEBUTTON2:
				AddItem(htiButton,SWF_BUTTON,"Button",tiInfo.usCharacterID,tiInfo.szName);
				if(cswfLoaded.HasActions(tiInfo.usCharacterID))
					AddItem(htiScript,SWF_SCRIPT,"Script",tiInfo.usCharacterID,"(Button)");
				break;

			case TAGCODE_DEFINESPRITE:
				htiParent=AddItem(htiSprite,SWF_SPRITE,"Sprite",tiInfo.usCharacterID,tiInfo.szName);
				
				if(cswfLoaded.HasSoundStream(tiInfo.usCharacterID))
					AddItem(htiSound,SWF_SOUND,"Sound",tiInfo.usCharacterID,"(Sprite)");				
				
				if(cswfLoaded.HasActions(tiInfo.usCharacterID))
					AddItem(htiScript,SWF_SCRIPT,"Script",tiInfo.usCharacterID,"(Sprite)");


				/*add sprite frames*/
				ulNumFrames=cswfLoaded.GetNumFrames(tiInfo.usCharacterID);

				for(iFrame=1; iFrame<=ulNumFrames; iFrame++) 
				{
					lpflFrameLabel=cswfLoaded.GetFrameLabel(tiInfo.usCharacterID,iFrame);
		
					if(lpflFrameLabel)
					{
						char *szLabel=new char[strlen(lpflFrameLabel->szLabel)+3];
						sprintf(szLabel,"\"%s\"",lpflFrameLabel->szLabel);
						AddItem(htiParent,SWF_FRAME,"Frame",iFrame,szLabel);
						delete (szLabel);
					}
		
					else AddItem(htiParent,SWF_FRAME,"Frame",iFrame);
				}

				break;

			case TAGCODE_DEFINEVIDEOSTREAM:
				AddItem(htiVideo,SWF_VIDEO,"Video",tiInfo.usCharacterID,tiInfo.szName);
				break;
		}
	}

	/*add movie frames*/
	ulNumFrames=cswfLoaded.GetNumFrames(0);

	for(iFrame=1; iFrame<=ulNumFrames; iFrame++) 
	{
		lpflFrameLabel=cswfLoaded.GetFrameLabel(0,iFrame);
		
		if(lpflFrameLabel)
		{
			char *szLabel=new char[strlen(lpflFrameLabel->szLabel)+3];
			sprintf(szLabel,"\"%s\"",lpflFrameLabel->szLabel);
			AddItem(htiFrame,SWF_FRAME,"Frame",iFrame,szLabel);
			delete (szLabel);
		}
		
		else AddItem(htiFrame,SWF_FRAME,"Frame",iFrame);
	}

	return 0;
}

void SetDlgInfo()
{
	char szText[1024];

	sprintf(szText,"%i",swfiInfo.bVersion);
	SetDlgItemText(hMainWnd,IDC_VERSION,szText);

	if(swfiInfo.bCompressed) sprintf(szText,"ZLIB Compressed");
	else sprintf(szText,"Uncompressed");
	SetDlgItemText(hMainWnd,IDC_COMPRESSED,szText);

	sprintf(szText,"%lu",swfiInfo.ulFileLength);
	SetDlgItemText(hMainWnd,IDC_FILELENGTH,szText);

	sprintf(szText,"%i",swfiInfo.usFrameCount);
	SetDlgItemText(hMainWnd,IDC_FRAMECOUNT,szText);

	sprintf(szText,"%0.2f",swfiInfo.spfFrameRate);
	SetDlgItemText(hMainWnd,IDC_FRAMERATE,szText);

	sprintf(szText,"%i",swfiInfo.srFrameRect.slXMin);
	SetDlgItemText(hMainWnd,IDC_XMIN,szText);

	sprintf(szText,"%i",swfiInfo.srFrameRect.slXMax);
	SetDlgItemText(hMainWnd,IDC_XMAX,szText);

	sprintf(szText,"%i",swfiInfo.srFrameRect.slYMin);
	SetDlgItemText(hMainWnd,IDC_YMIN,szText);

	sprintf(szText,"%i",swfiInfo.srFrameRect.slYMax);
	SetDlgItemText(hMainWnd,IDC_YMAX,szText);

	sprintf(szText,"#%02X%02X%02X",
					swfiInfo.rgbaBackground.ubRed,
					swfiInfo.rgbaBackground.ubGreen,
					swfiInfo.rgbaBackground.ubBlue);
	SetDlgItemText(hMainWnd,IDC_BACKGROUND,szText);
}

void GetDlgInfo()
{
	char szText[1024];

	GetDlgItemText(hMainWnd,IDC_XMIN,szText,sizeof(szText));
	swfiInfo.srFrameRect.slXMin=atol(szText);

	GetDlgItemText(hMainWnd,IDC_XMAX,szText,sizeof(szText));
	swfiInfo.srFrameRect.slXMax=atol(szText);

	GetDlgItemText(hMainWnd,IDC_YMIN,szText,sizeof(szText));
	swfiInfo.srFrameRect.slYMin=atol(szText);

	GetDlgItemText(hMainWnd,IDC_YMAX,szText,sizeof(szText));
	swfiInfo.srFrameRect.slYMax=atol(szText);
	
	GetDlgItemText(hMainWnd,IDC_BACKGROUND,szText,sizeof(szText));
	sscanf(szText,"#%02X%02X%02X",
					&swfiInfo.rgbaBackground.ubRed,
					&swfiInfo.rgbaBackground.ubGreen,
					&swfiInfo.rgbaBackground.ubBlue);
	
}

void ClearDlgInfo()
{
	SetDlgItemText(hMainWnd,IDC_VERSION,"");
	SetDlgItemText(hMainWnd,IDC_COMPRESSED,"");
	SetDlgItemText(hMainWnd,IDC_FILELENGTH,"");
	SetDlgItemText(hMainWnd,IDC_FRAMECOUNT,"");
	SetDlgItemText(hMainWnd,IDC_FRAMERATE,"");
	SetDlgItemText(hMainWnd,IDC_XMIN,"");
	SetDlgItemText(hMainWnd,IDC_XMAX,"");
	SetDlgItemText(hMainWnd,IDC_YMIN,"");
	SetDlgItemText(hMainWnd,IDC_YMAX,"");
	SetDlgItemText(hMainWnd,IDC_BACKGROUND,"");
}

DWORD WINAPI LoadFile(PVOID pvVoid)
{
	UI32 ulTagsRead;
	char szMessage[1024];

	SetBaseName();

	ZeroCounts();
	TreeView_DeleteAllItems(hTreeView);
	MakeCategories();
	ClearDlgInfo();
	
	sprintf(szTitle,"SWFRIP - (Loading)"); 
	SetWindowText(hMainWnd,szTitle);
	SetMenu(MF_GRAYED);

	ulTagsRead=cswfLoaded.Load((const char*)szFileName); 
	
	if(!ulTagsRead) 
	{
		sprintf(szMessage,"%s is not a valid Macromedia Flash file.",(const char*)szBaseName);
		MessageBox(hMainWnd,szMessage,"Invalid File",MB_OK | MB_ICONERROR);
		sprintf(szTitle,"SWFRIP"); 
	}
	
	else 
	{
		cswfLoaded.GetInfo(&swfiInfo);
		SetDlgInfo();
		
		sprintf(szTitle,"SWFRIP - (Adding Items)");
		SetWindowText(hMainWnd,szTitle);
		SetTree();
		SetCounts();

		sprintf(szTitle, "SWFRIP - %s (%lu tags)",szBaseName,ulTagsRead);
		SetMenu(MF_ENABLED);
	}
	
	SetWindowText(hMainWnd,szTitle);

	return ulTagsRead;
}

void View(char *szName)
{
	SHELLEXECUTEINFO seiShellExec;

	memset(&seiShellExec,0,sizeof(SHELLEXECUTEINFO));
	seiShellExec.cbSize=sizeof(SHELLEXECUTEINFO);
	seiShellExec.lpFile=szName;
	seiShellExec.nShow=SW_SHOWNORMAL;
	ShellExecuteEx(&seiShellExec);
}

UI16 GetItemID(HTREEITEM htiItem)
{
	TVITEM tviItem;
	char szText[128], *szID;

	if(!htiItem) return 0;

	tviItem.mask=TVIF_TEXT;
	tviItem.hItem=htiItem;
	tviItem.pszText=szText;
	tviItem.cchTextMax=sizeof(szText);
	TreeView_GetItem(hTreeView,&tviItem);

	szID=strchr(szText,' ')+1;
	return atoi(szID);
}

int SaveItem()
{
	HTREEITEM htiParent;
	UI16 usID, usParentID;

	if(!cswfLoaded.Loaded() || !htiCurSel) return 0;

	usID=GetItemID(htiCurSel);
	htiParent=TreeView_GetParent(hTreeView,htiCurSel);
	usParentID=GetItemID(htiParent);

	if(!htiParent) /*save all of this type*/
	{
		if(htiCurSel==htiShape)		cswfLoaded.SaveAll(SWF_SHAPE);
		if(htiCurSel==htiImage)		cswfLoaded.SaveAll(SWF_IMAGE);
		if(htiCurSel==htiMorph)		cswfLoaded.SaveAll(SWF_MORPH);
		if(htiCurSel==htiFont)		cswfLoaded.SaveAll(SWF_FONT);
		if(htiCurSel==htiText)		cswfLoaded.SaveAll(SWF_TEXT);
		if(htiCurSel==htiSound)		cswfLoaded.SaveAll(SWF_SOUND);
		if(htiCurSel==htiButton)	cswfLoaded.SaveAll(SWF_BUTTON);
		if(htiCurSel==htiSprite)	cswfLoaded.SaveAll(SWF_SPRITE);
		if(htiCurSel==htiVideo)		cswfLoaded.SaveAll(SWF_VIDEO);
		//if(htiCurSel==htiFrame)	cswfLoaded.SaveAll(SWF_FRAME);
		if(htiCurSel==htiScript)	cswfLoaded.SaveAll(SWF_SCRIPT);

		return 0;
	}

	else /*save one resource and shell execute it*/
	{
		/*sprite frame*/
		if(!IsCategoryNode(htiParent))	
			cswfLoaded.SaveFrame(usParentID,usID);

		else if(htiParent==htiSound)	cswfLoaded.SaveSound(usID);
		else if(htiParent==htiFrame)	cswfLoaded.SaveFrame(0,usID);
		else if(htiParent==htiScript)	cswfLoaded.SaveScript(usID);
		else cswfLoaded.SaveCharacter(usID);

		return 1;
	}

	return 0;
}

UI32 SetFrameTags()
{
	UI32 ulStart;
	UI16 usClipID, usID;
	UI8 fIsFrame;
	HTREEITEM htiParent;
	TAGINFO tiInfo;

	while(SendDlgItemMessage(hMainWnd,IDC_FRAMETAGS,LB_DELETESTRING,0,0)!=LB_ERR) true;
	
	SetDlgItemText(hMainWnd,IDC_TAGINFO,"");

	htiParent=TreeView_GetParent(hTreeView,htiCurSel);

	if(!htiParent) return 0;

	if(htiParent==htiFrame) 
	{
		usClipID=0;
		fIsFrame=true;
	}
	
	else if(!IsCategoryNode(htiParent)) 
	{
		usClipID=GetItemID(htiParent);
		fIsFrame=true;
	}

	else fIsFrame=false;
	
	usID=GetItemID(htiCurSel);

	if(fIsFrame)
	{
		ulStart=cswfLoaded.FindFrameStart(usClipID,usID);
		memset(&tiInfo,0,sizeof(TAGINFO));
	
		for(UI32 iTag=ulStart; tiInfo.usTagCode!=TAGCODE_SHOWFRAME; iTag++)
		{
			cswfLoaded.GetTagInfo(usClipID,iTag,&tiInfo);
			SendDlgItemMessage(hMainWnd,IDC_FRAMETAGS,LB_ADDSTRING,0,(LPARAM)tiInfo.szTagName);
		}
	}

	else /*not a frame*/
	{
		cswfLoaded.GetTagInfo(-1,usID,&tiInfo);
		SetDlgItemText(hMainWnd,IDC_TAGINFO,tiInfo.szInfo);
	}

	return 0;
}

UI32 SetInfo()
{
	HTREEITEM htiParent;
	UI16 usClipID, usID;
	UI32 ulTag;
	TAGINFO tiInfo;

	htiParent=TreeView_GetParent(hTreeView,htiCurSel);

	if(!htiParent) return 0;

	if(htiParent==htiFrame) 
		usClipID=0;

	else if(!IsCategoryNode(htiParent)) 
		usClipID=GetItemID(htiParent);

	else return 0;

	usID=GetItemID(htiCurSel);

	ulTag=cswfLoaded.FindFrameStart(usClipID,usID);
	ulTag+=SendDlgItemMessage(hMainWnd,IDC_FRAMETAGS,LB_GETCURSEL,0,0);
	memset(&tiInfo,0,sizeof(TAGINFO));
	
	usCharacter=cswfLoaded.GetTagInfo(usClipID,ulTag,&tiInfo);
	SetDlgItemText(hMainWnd,IDC_TAGINFO,tiInfo.szInfo);
	return 0;
}

LRESULT CALLBACK OptionsProc(HWND hWnd, UINT iMsg, 
						     WPARAM wParam, LPARAM lParam)
{
	UI8 ubFormat;

	switch(iMsg)
	{
		case WM_INITDIALOG:
			SendDlgItemMessage(hWnd,IDC_SHAPEFORMAT,CB_ADDSTRING,0,(LPARAM)"*.swf");
			SendDlgItemMessage(hWnd,IDC_SHAPEFORMAT,CB_ADDSTRING,0,(LPARAM)"*.svg");

			SendDlgItemMessage(hWnd,IDC_IMAGEFORMAT,CB_ADDSTRING,0,(LPARAM)"*.bmp");
			SendDlgItemMessage(hWnd,IDC_IMAGEFORMAT,CB_ADDSTRING,0,(LPARAM)"*.pcx");
			SendDlgItemMessage(hWnd,IDC_IMAGEFORMAT,CB_ADDSTRING,0,(LPARAM)"*.png");

			SendDlgItemMessage(hWnd,IDC_TEXTFORMAT,CB_ADDSTRING,0,(LPARAM)"*.swf");
			SendDlgItemMessage(hWnd,IDC_TEXTFORMAT,CB_ADDSTRING,0,(LPARAM)"*.txt");

			SendDlgItemMessage(hWnd,IDC_FRAMEFORMAT,CB_ADDSTRING,0,(LPARAM)"*.swf");
			SendDlgItemMessage(hWnd,IDC_FRAMEFORMAT,CB_ADDSTRING,0,(LPARAM)"*.svg");

			ubFormat=cswfLoaded.GetExportFormat(SWF_SHAPE);
			SendDlgItemMessage(hWnd,IDC_SHAPEFORMAT,CB_SETCURSEL,ubFormat,0);
			
			ubFormat=cswfLoaded.GetExportFormat(SWF_IMAGE);
			SendDlgItemMessage(hWnd,IDC_IMAGEFORMAT,CB_SETCURSEL,ubFormat,0);
			
			ubFormat=cswfLoaded.GetExportFormat(SWF_TEXT);
			SendDlgItemMessage(hWnd,IDC_TEXTFORMAT,CB_SETCURSEL,ubFormat,0);

			ubFormat=cswfLoaded.GetExportFormat(SWF_FRAME);
			SendDlgItemMessage(hWnd,IDC_FRAMEFORMAT,CB_SETCURSEL,ubFormat,0);
			return 0;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
					ubFormat=SendDlgItemMessage(hWnd,IDC_SHAPEFORMAT,CB_GETCURSEL,0,0);
					cswfLoaded.SetExportFormat(SWF_SHAPE,ubFormat);
					ubFormat=SendDlgItemMessage(hWnd,IDC_IMAGEFORMAT,CB_GETCURSEL,0,0);
					cswfLoaded.SetExportFormat(SWF_IMAGE,ubFormat);
					ubFormat=SendDlgItemMessage(hWnd,IDC_TEXTFORMAT,CB_GETCURSEL,0,0);
					cswfLoaded.SetExportFormat(SWF_TEXT,ubFormat);
					ubFormat=SendDlgItemMessage(hWnd,IDC_FRAMEFORMAT,CB_GETCURSEL,0,0);
					cswfLoaded.SetExportFormat(SWF_FRAME,ubFormat);
	
				case IDCANCEL:
					EndDialog(hWnd,NULL);
					return 0;
			}
	}

	return 0;
}

LRESULT CALLBACK AboutProc(HWND hWnd, UINT iMsg, 
						   WPARAM wParam, LPARAM lParam)
{
	char szAbout[1024];

	switch(iMsg)
	{
		case WM_INITDIALOG:
			strcpy(szAbout,"SWFRIP v.2\r\n\r\n");
			strcat(szAbout,"Wesley Hopper (hopperw2000@yahoo.com)\r\n\r\n");
			strcat(szAbout,"SWF and Flash are Trademarks of Macromedia, Inc.");
			SetDlgItemText(hWnd,IDC_ABOUT,szAbout);
			return 0;

		case WM_LBUTTONDOWN:
			EndDialog(hWnd,NULL);
			return 0;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDCANCEL:
					EndDialog(hWnd,NULL);
					return 0;
			}
	}

	return 0;
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT iMsg, 
							 WPARAM wParam, LPARAM lParam)
{
	HANDLE hScanThread=NULL;
	DWORD dwScanThreadID;
	HMENU hTreeMenu;
	POINT pntClickPoint;
	TVHITTESTINFO tvhtiHitTestInfo;
	char szText[1024], *szBase;
	UI8 ubType;	long *x;
	
	switch(iMsg)
	{
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDM_FILE_OPEN:
					if(!GetOFN(szFileName,szOpenFileFilter)) return 0;
					CreateThread(NULL,4096,LoadFile,NULL,NULL,NULL);
					return 0;

				case IDM_FILE_CLOSE:
					if(!cswfLoaded.Loaded()) return 0;
					ZeroCounts();
					TreeView_DeleteAllItems(hTreeView);
					MakeCategories();
					ClearDlgInfo();
					cswfLoaded.Clear();
					SetWindowText(hMainWnd,"SWFRIP");
					SetMenu(MF_GRAYED);
					return 0;

				case IDM_FILE_SAVE:
					if(!cswfLoaded.Loaded()) return 0;
					GetDlgInfo();
					cswfLoaded.SetInfo(&swfiInfo);
					cswfLoaded.Save(szFileName);
					return 0;

				case IDM_FILE_SAVEAS:
					if(!cswfLoaded.Loaded()) return 0;
					ubType=GetSFN(szText,szSaveFileFilter);
					GetDlgInfo();
					cswfLoaded.SetInfo(&swfiInfo);
					szBase=strrchr(szText,'\\');

					switch(ubType)
					{
						case 0: break;
						
						case 1:
							if(!strrchr(szBase,'.'))
								strcat(szText,".swf"); 
							cswfLoaded.Save(szText);
							break;
						
						case 2:
							if(!strrchr(szBase,'.'))
								strcat(szText,".sswf");
							cswfLoaded.SaveSSWF(szText);
							break;
					}					
					return 0;
				
				case IDM_FILE_VIEW:
					View(szFileName);
					return 0;
				
				case IDM_RES_SAVE:
					if(SaveItem())
					{
						cswfLoaded.GetLastRes(szText);
						View(szText);
					}
					return 0;

				case IDM_RES_SAVEAS:
					if(SaveItem())
					{
						cswfLoaded.GetLastRes(szText);
						View(szText);
					}
					return 0;

				case IDM_RES_SAVEALL:
					if(!cswfLoaded.Loaded()) return 0;
					cswfLoaded.SaveAll(SWF_ALL);
					return 0;

				case IDM_RES_SAVETEXT:
					if(!cswfLoaded.Loaded()) return 0;
					cswfLoaded.SaveText();
					cswfLoaded.GetLastRes(szText);
					View(szText);
					return 0;

				case IDM_VIEW_OPTIONS:
					DialogBox(hInst,MAKEINTRESOURCE(IDD_OPTIONS),hMainWnd,(DLGPROC)OptionsProc);
					return 0;

				case IDM_HELP_CONTENTS:
					GetModuleFileName(GetModuleHandle(0),szText,sizeof(szText));
					strcpy(strrchr(szText,'.')+1,"txt");
					View(szText);
					return 0;

				case IDM_HELP_ABOUT:
					DialogBox(hInst,MAKEINTRESOURCE(IDD_ABOUT),hMainWnd,(DLGPROC)AboutProc);
					return 0;

				case IDCANCEL:
					//DestroyWindow(hMainWnd);
					EndDialog(hMainWnd,NULL);
					PostQuitMessage(NULL);
					return 0;

				case IDC_FRAMETAGS:
					switch(HIWORD(wParam))
					{
						case LBN_SELCHANGE:
							SetInfo();
							break;

						case LBN_DBLCLK:
							if(!usCharacter) return 0;
							cswfLoaded.SaveCharacter(usCharacter);
							cswfLoaded.GetLastRes(szText);
							View(szText);
							break;
					}
					return 0;

				default: return 0;
			}
		
		case WM_NOTIFY:
			if(LPNMHDR(lParam)->hwndFrom==hTreeView) /*Control ID*/
				switch(LPNMHDR(lParam)->code) /*Notify Code*/
				{
					case TVN_SELCHANGING:
						htiCurSel=((LPNMTREEVIEW)lParam)->itemNew.hItem;
						TreeView_Select(hTreeView,htiCurSel,0);
						SetFrameTags();
						return 0;

					case NM_DBLCLK:
						if(IsCategoryNode(htiCurSel)) return 0;
						SendMessage(hMainWnd,WM_COMMAND,IDM_RES_SAVE,NULL);
						return 0;

					case NM_RCLICK:
						tvhtiHitTestInfo.pt.x=*(&((NMTREEVIEW*)lParam)->ptDrag.x-2);
						tvhtiHitTestInfo.pt.y=*(&((NMTREEVIEW*)lParam)->ptDrag.y-2);
						tvhtiHitTestInfo.flags=TVHT_ONITEM;
						
						htiCurSel=TreeView_HitTest(hTreeView,&tvhtiHitTestInfo);
						htiCurSel=tvhtiHitTestInfo.hItem;
						
						TreeView_Select(hTreeView,htiCurSel,TVGN_CARET);
						
						GetCursorPos(&pntClickPoint);
						
						hTreeMenu=CreatePopupMenu();
									
						if(IsCategoryNode(htiCurSel)) 
							AppendMenu(hTreeMenu,0,IDM_RES_SAVEALL,"&Save All");

						else 
						{
							AppendMenu(hTreeMenu,0,IDM_RES_SAVE,"&Save");
							//AppendMenu(hTreeMenu,0,IDM_RES_SAVEAS,"&Save As...");
						}
								
						TrackPopupMenu(hTreeMenu,TPM_LEFTALIGN | TPM_RIGHTBUTTON,
									   pntClickPoint.x,pntClickPoint.y,0,hMainWnd,0);

						DestroyMenu(hTreeMenu);
						break;

					case NM_RETURN:
						if(IsCategoryNode(htiCurSel)) return 0;
						SendMessage(hMainWnd,WM_COMMAND,IDM_RES_SAVE,NULL);
						return 0;

					default: return 0;
				}

			return 0;

		case WM_DROPFILES:
			DragQueryFile((HDROP)wParam,0,szFileName,sizeof(szFileName));
			CreateThread(NULL,4096,LoadFile,NULL,NULL,NULL);
			DragFinish((HDROP)wParam);
			return 0;
			
		case WM_CLOSE:
			PostMessage(hMainWnd,WM_DESTROY,NULL,NULL);
			return 0;

		case WM_DESTROY: 
			PostQuitMessage(0); 
			return 0;
	}

	//return DefWindowProc(hWnd,iMsg,wParam,lParam);
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
				   LPSTR lpszCmdLine, int nCmdShow)
{
	hInst=hInstance;
	MSG Msg;

	/*Initialize common controls*/
    INITCOMMONCONTROLSEX icInitCC;
    icInitCC.dwSize=sizeof(INITCOMMONCONTROLSEX);
    icInitCC.dwICC=ICC_TREEVIEW_CLASSES;
    InitCommonControlsEx(&icInitCC);

	/*Register main window class & Create main window*/
	/*WNDCLASSEX wcMainClass;
	memset(&wcMainClass,0,sizeof(WNDCLASSEX));
	wcMainClass.lpszClassName = "FileScanClass";
	wcMainClass.style = CS_HREDRAW | CS_VREDRAW;
	wcMainClass.cbSize = sizeof(WNDCLASSEX);
	wcMainClass.lpfnWndProc = MainWndProc;
	wcMainClass.hInstance = hInst;
	wcMainClass.hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_APPLICATION));
	wcMainClass.hCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
	wcMainClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcMainClass.lpszMenuName = MAKEINTRESOURCE(IDR_MAINMENU);
	wcMainClass.hIconSm = LoadIcon(NULL, MAKEINTRESOURCE(IDI_APPLICATION));
	RegisterClassEx(&wcMainClass);
	
	hMainWnd=CreateWindowEx(NULL,"FileScanClass","Flash Decompiler",
							WS_OVERLAPPEDWINDOW | WS_SYSMENU | WS_MINIMIZEBOX,
							CW_USEDEFAULT,CW_USEDEFAULT,400,300,
							NULL,NULL,hInst,NULL);
	if(!hMainWnd) PostQuitMessage(0);*/

	hMainWnd=CreateDialog(hInst,MAKEINTRESOURCE(IDD_MAINDLG),NULL,(DLGPROC)MainWndProc);
	SendMessage(hMainWnd,WM_SETICON,ICON_BIG,(WPARAM)LoadIcon(hInst,MAKEINTRESOURCE(IDI_FLASH)));
	SetWindowText(hMainWnd,"SWFRIP");
	ShowWindow(hMainWnd,SW_SHOWNORMAL);

	//DWORD dwStyle=WS_VISIBLE | WS_TABSTOP | WS_CHILD | TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT;
	//hTreeView=CreateWindowEx(WS_EX_STATICEDGE,WC_TREEVIEW,"Files",dwStyle,25,33,250,225,hMainWnd,NULL,hInst,NULL);
	//ShowWindow(hTreeView,SW_SHOWNORMAL);

	hTreeView=GetDlgItem(hMainWnd,IDC_RESTREE);

	hMainMenu=GetMenu(hMainWnd);
	SetMenu(MF_GRAYED);

	HIMAGELIST hilList=ImageList_Create(16,16,ILC_COLOR8 | ILC_MASK,11,15);
	ImageList_AddIcon(hilList,LoadIcon(hInst,MAKEINTRESOURCE(IDI_SHAPE)));
	ImageList_AddIcon(hilList,LoadIcon(hInst,MAKEINTRESOURCE(IDI_IMAGE)));
	ImageList_AddIcon(hilList,LoadIcon(hInst,MAKEINTRESOURCE(IDI_MORPH)));
	ImageList_AddIcon(hilList,LoadIcon(hInst,MAKEINTRESOURCE(IDI_FONT)));
	ImageList_AddIcon(hilList,LoadIcon(hInst,MAKEINTRESOURCE(IDI_TEXT)));
	ImageList_AddIcon(hilList,LoadIcon(hInst,MAKEINTRESOURCE(IDI_SOUND)));
	ImageList_AddIcon(hilList,LoadIcon(hInst,MAKEINTRESOURCE(IDI_BUTTON)));
	ImageList_AddIcon(hilList,LoadIcon(hInst,MAKEINTRESOURCE(IDI_SPRITE)));
	ImageList_AddIcon(hilList,LoadIcon(hInst,MAKEINTRESOURCE(IDI_VIDEO)));
	ImageList_AddIcon(hilList,LoadIcon(hInst,MAKEINTRESOURCE(IDI_FRAME)));
	ImageList_AddIcon(hilList,LoadIcon(hInst,MAKEINTRESOURCE(IDI_SCRIPT)));
	TreeView_SetImageList(hTreeView,hilList,TVSIL_NORMAL);

	ZeroCounts();
	MakeCategories();
	htiCurSel=NULL;

	//Register();
	cswfLoaded.SetDeleteTags(true);

	/*Open From Command Line*/
	if(lpszCmdLine[0]!='\0') 
	{
		strcpy(szFileName,lpszCmdLine); 
		CreateThread(NULL,4096,LoadFile,NULL,NULL,NULL);
	}

	while(GetMessage(&Msg,NULL,0,0))
		if(!IsDialogMessage(hMainWnd,&Msg))
		{	
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}

	return(Msg.wParam);
}