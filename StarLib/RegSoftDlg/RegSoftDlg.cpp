// RegSoftDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "RegSoftDlg.h"
#include <StarLib/SerialManager/SerialManager.h>
#include "StarLib/Common/common.h"
#include "StarLib/VMProtect/VMProtectSDK.h"

// CRegSoftDlg �Ի���

IMPLEMENT_DYNAMIC(CRegSoftDlg, CDialog)

CRegSoftDlg::CRegSoftDlg(DWORD dwIDD) : CDialog(dwIDD)
{
	m_strHardCode=_T("");
	m_strSerial=_T("");
}

CRegSoftDlg::~CRegSoftDlg()
{
}

void CRegSoftDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CRegSoftDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CRegSoftDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CRegSoftDlg ��Ϣ�������

BOOL CRegSoftDlg::OnInitDialog()
{
	__super::OnInitDialog();

	InitAllControls();

	VMPBEGIN;
	if ( IsRregistered(HARDCODEPREFIX)==FALSE ){
		CSerialManager sm(HARDCODEPREFIX);
		m_strHardCode=sm.m_strMachineCode;
	}else{
		//
		AfxMessageBox("�Ѿ���ע��汾��");
		OnOK();
	}
	VMPEND;

	UpdateData(FALSE);

	return TRUE;
}

void CRegSoftDlg::InitAllControls()
{
	m_stcGetSerial.SetUnderline(CHyperLink::ulAlways);
	m_stcGetSerial.SetColours(RGB(0,0,204),RGB( 85, 26,139));
	m_stcGetSerial.SetURL(m_strRegUrl);
}

void CRegSoftDlg::OnStnClickedStaticGetserial()
{
	Star::Common::OpenUrl(m_strRegUrl);
}

void CRegSoftDlg::OnBnClickedOk()
{
	VMPBEGIN;
	UpdateData(TRUE);
	m_strSerial.Trim();
	if ( m_strSerial.IsEmpty()==TRUE ){
		return;
	}
	CSerialManager sm(HARDCODEPREFIX);
	if ( sm.ValidateSerial(m_strSerial)==TRUE ){
		//�ɹ�
		CString strRegFile;
		strRegFile.Format( "%s%s.usr", Star::Common::GetStartPath(), APPNAME );
		CFile file(strRegFile,CFile::modeReadWrite|CFile::modeCreate);
		CArchive ar(&file,CArchive::store);
		sm.Serialize(ar);
		ar.Close();
		file.Close();
		//AfxMessageBox("ע��ɹ���");

	}else{
		//ʧ��
		//AfxMessageBox("ע��ʧ�ܣ�");
	}

	OnOK();
	VMPEND;
}
