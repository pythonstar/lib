// Link.cpp: implementation of the CShellLink class.
// By Bjarke Viksøe
// Description:
//   Create desktop groups and links
// Ver:
//   1.0   11 May.1998 Started
//
// Requires:
//   class CDir (DIR.CPP/H)
//   class CReg (REG.CPP/H)
//   GLOBAL.H
//   SHELL32.LIB
//   SHLOBJ.H
//   SHELLAPI.H

//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ShellLink.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifdef _UNICODE
#error This code is <NOT> UNICODE compatible
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CShellLink::CShellLink()
{

}

CShellLink::~CShellLink()
{

}

BOOL CShellLink::IsSupported() const
{
   DWORD dwVersion;
   dwVersion = ::GetVersion();
   switch(HIWORD(dwVersion)&0xC000) {
     case 0x0000: if (LOBYTE(LOWORD(dwVersion))<=3)
               return FALSE;  // NT 3.xx and below does NOT support IShell...
               return TRUE;   // NT 4.0 and above supports IShell...
     case 0x8000:
            return FALSE;  // Win32s, does NOT support IShell...
     case 0xC000:
            return TRUE;   // Win95, supports IShell...
   }
   return TRUE; // None of the above, attempt to use IShell...
};

RETCODE CShellLink::Init( int DesktopType )
{
   if( !IsSupported() ) return RET_UNSUPPORTED;
   switch( DesktopType ) {
   case CSIDL_COMMON_DESKTOPDIRECTORY:
   case CSIDL_COMMON_PROGRAMS:
   case CSIDL_DESKTOP:
   case CSIDL_PROGRAMS:
   case CSIDL_DESKTOPDIRECTORY:
   case CSIDL_STARTUP:
      break;
   default:
      return RET_INVALIDARGS;
   };
   m_DesktopType = DesktopType;
   return RET_OK;
};

CString CShellLink::STRRETToCString(LPCITEMIDLIST pidl, LPSTRRET lpStr)
{
   ASSERT( lpStr );
   switch (lpStr->uType) { 
   case STRRET_WSTR: 
      return CString(lpStr->pOleStr);
   case STRRET_OFFSET: 
      return CString(((char *) pidl) + lpStr->uOffset); 
   case STRRET_CSTR: 
      return CString(lpStr->cStr); 
   default:
      ASSERT(FALSE);
      return CString("");
   } 
}

RETCODE CShellLink::GetShellPath( CString &Path )
{
   HRESULT ret;
   Path.Empty();
   LPITEMIDLIST pidl;
   LPSHELLFOLDER pFolder; 
   LPMALLOC gpMalloc;
   if (!SUCCEEDED(::SHGetMalloc(&gpMalloc))) return RET_OUTOFMEMORY;
   if (!SUCCEEDED(::SHGetDesktopFolder(&pFolder))) return RET_ERROR;
   ret = ::SHGetSpecialFolderLocation( NULL, m_DesktopType, &pidl );
   if( ret!=NOERROR ) {
      gpMalloc->Release();
      return RET_ERROR;
   };
   STRRET sName;
   if (SUCCEEDED(pFolder->GetDisplayNameOf( pidl, SHGDN_INFOLDER, &sName ))) {
      Path = STRRETToCString(pidl, &sName); 
   }
   gpMalloc->Release();
   ADDBACKSLASH( Path );
   return RET_OK;
};

RETCODE CShellLink::CreateFolder( LPCTSTR Title )
{
   CDir dir;
   CString strPath;
   RETCODE ret;
   if( (ret = GetShellPath( strPath )) != RET_OK ) return ret;
   strPath += Title;
   return dir.Create( strPath );
};

RETCODE CShellLink::DeleteFolder( LPCTSTR Title )
{
   CDir dir;
   CString strPath;
   RETCODE ret;
   if( (ret = GetShellPath( strPath )) != RET_OK ) return ret;
   strPath += Title;
   return dir.Delete( strPath );
};

RETCODE CShellLink::FocusFolder( LPCTSTR Title )
{
   return CreateFolder( Title );
};

RETCODE CShellLink::CreateItem( LPCTSTR Title, 
                        LPCTSTR Filename,
                        LPCTSTR Args /*=NULL*/,
                         LPCTSTR Folder, /*=NULL*/
                        LPCTSTR WorkPath /*=NULL*/,
                        LPCTSTR IconPath /*=NULL*/,
                        long nCmdShow /*=3*/ )
{
   ASSERT( AfxIsValidString(Title) );
   ASSERT( AfxIsValidString(Filename) );

   IShellLink *psl;
   CString     strDest;
   CFile       f;
   WCHAR       wszLocation[MAX_PATH];   // buffer for Unicode string
   RETCODE     ret = RET_ERROR;
   HRESULT     hres;

   // We use this to extract the filepath and filetitle
   f.SetFilePath( Filename );

   // Go ahead and create Shell interface
   hres = ::CoCreateInstance( CLSID_ShellLink, 
                              NULL, 
                              CLSCTX_INPROC_SERVER, 
                              IID_IShellLink, 
                              (void **)&psl );
   if( !SUCCEEDED(hres) ) return RET_ERROR;

   // Query IShellLink for the IPersistFile interface for
   // saving the shortcut in persistent storage.
   IPersistFile *ppf;
   hres = psl->QueryInterface (IID_IPersistFile, (void **)&ppf);
   if( !SUCCEEDED(hres) ) {
      psl->Release();
      return RET_ERROR;
   };

   // Set new default error code   
   ret = RET_ERROR;

   // Set filename
   hres = psl->SetPath(Filename);
   if( !SUCCEEDED(hres) ) goto DoneItem;

   // Set the Arguments of the shortcut.
   hres = psl->SetArguments(Args);
   if( !SUCCEEDED(hres) ) goto DoneItem;

   // Set the description of the shortcut.
   hres = psl->SetDescription(Title);
   if (!SUCCEEDED(hres)) goto DoneItem;
   // Set the Working Directory of the shortcut.
   if( WorkPath!=NULL ) {
      hres = psl->SetWorkingDirectory(WorkPath);
   }
   else {
      hres = psl->SetWorkingDirectory(BfxGetFilePath(Filename));
   };
   if( !SUCCEEDED(hres) ) goto DoneItem;

   // Set the ShowWindow of the shortcut.
   hres = psl->SetShowCmd(nCmdShow);
   if( !SUCCEEDED(hres) ) goto DoneItem;

   // Set icon location
   if( IconPath!=NULL ) {
      hres = psl->SetIconLocation(IconPath,0);
      if( !SUCCEEDED(hres) ) goto DoneItem;
   };

   // Create the filename of the new link
   if( GetShellPath( strDest )!=RET_OK ) goto DoneItem;
   if( Folder!=NULL ) {
      strDest += Folder;
      ADDBACKSLASH( strDest );
   };
   strDest += f.GetFileTitle();
   strDest += _T(".Lnk");
   TOUNICODE( strDest, wszLocation );
   hres = ppf->Save( (LPWSTR)wszLocation, TRUE );
   ret = (SUCCEEDED(hres) ? RET_OK : RET_ERROR);

DoneItem:
   // Release the pointer to Shell interface.
   ppf->Release();
   // Release the pointer to IShellLink.
   psl->Release();
   // Done
   return ret;
};

RETCODE CShellLink::DeleteItem( LPCTSTR Title, LPCTSTR Folder )
{
   CString strFilename;
   CString strGroup( Folder );
   RETCODE ret;
   if( (ret = GetShellPath( strFilename )) != RET_OK ) return ret;
   if( !strGroup.IsEmpty() ) strFilename += strGroup;
   ADDBACKSLASH( strFilename );
   strFilename += Title;
   CFile f;
   TRY {
      f.Remove( strFilename );
      return RET_OK;
   }
   CATCH_ALL(e) {
      return RET_FILEERROR;
   }
   END_CATCH_ALL;
};

RETCODE CShellLink::Done(void)
{
   return RET_OK;
};

