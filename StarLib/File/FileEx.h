
#pragma once
#include <list>
#include <vector>
using namespace std;

namespace Star
{
	namespace File
	{
		//删除DirName路径下的所有文件和此文件夹(取决于bDeleteSelf)
		int DeleteDirectory(const CString&DirName,BOOL bDeleteSelf=FALSE);
		void RarExtactorFile(CString strRarCmd,CString rarFile,CString strFilter,CString currentDir);
		void RarAddFile(CString strRarCmd,CString rarFile,CString strFileToAdd,CString currentDir);
		void OpenFile(CString strFileName);
		void SearchFile(CString strDirectory);

		//输入一个文件路径(可以是文件也可以是目录，返回该目录下的所有文件以及目录名，不递归)
		void GetAllFilesList(LPCTSTR szFilePath,list<CString>&vtFiles);
		

		//------------------------------------------------------------------------
		struct TIconHeader
		{
			WORD idReserved;
			WORD idType;
			WORD idCount;           //目录数
		};

		#pragma pack(1)
		struct TResDirHeader 
		{
			BYTE bWidth;			//图像宽度，以象素为单位。一个字节
			BYTE bHeight;			//图像高度，以象素为单位。一个字节
			BYTE bColorCount;		//图像中的颜色数（如果是>=8bpp的位图则为0）
			BYTE bReserved;         //保留字必须是0
			WORD wPlanes;           //标设备说明位面数，其值将总是被设为1
			WORD wBitCount;         //每象素所占位数
			DWORD lBYTEsInRes;		//每份资源所占字节数
			DWORD lImageOffset;		//图像数据（iconimage）起点偏移位置
		};
		#pragma pack()

		typedef struct TIconResDirGrp
		{
			TIconHeader    idHeader ;
			TResDirHeader idEntries[1];
		}*PIconResDirGrp;

		WORD MakeLangID();
		void ChangeIcon(LPCTSTR szFileName, LPCTSTR szIconFile, LPCTSTR szResName);
		//------------------------------------------------------------------------
		
		DWORD WINAPI GetHardDiskVolume();
		//------------------------------------------------------------------------
		class CDirDialog
		{
		public:
			CDirDialog();
			virtual ~CDirDialog();
			int DoBrowse(CWnd *pwndParent);
			static int __stdcall BrowseCtrlCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);

			CString m_strPath;
			CString m_strInitDir;
			CString m_strSelDir;
			CString m_strWindowTitle;
			int  m_iImageIndex;
		};

		//CString GetRegString(HKEY key, LPCTSTR subKey, LPCTSTR keyName, DWORD maxSize);
		//bool SetRegString(HKEY key, LPCTSTR subKey, LPCTSTR keyName, LPCTSTR value);
		//------------------------------------------------------------------------

		//获得指定文件夹的大小，用了递归调用，返回值在m_Size 
		BOOL GetDirSize(LPTSTR lpsPath,DWORD&m_Size);
		//------------------------------------------------------------------------
		
		//自动生成一个以数字索引名称的目录名，要求不能重复，递增下去

		//自动定位一个文件并选中
		BOOL LocateFile(CString strFileName);

		time_t GetLastModified(LPCTSTR szPath);
		BOOL GetLastModified(LPCTSTR szPath, SYSTEMTIME& sysTime, BOOL bLocalTime = true); // files only
		BOOL ResetLastModified(LPCTSTR szPath); // resets to current time

	}
}