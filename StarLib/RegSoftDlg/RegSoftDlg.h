#pragma once
#include <afxwin.h>
#include <StarLib/HyperLink/HyperLink.h>


// CRegSoftDlg �Ի���

class CRegSoftDlg : public CDialog
{
	DECLARE_DYNAMIC(CRegSoftDlg)

public:
	CRegSoftDlg(DWORD dwIDD);   // ��׼���캯��
	virtual ~CRegSoftDlg();

// �Ի�������
	//enum { IDD = IDD_DIALOG_REGSOFT };

private:
	void InitAllControls();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	CString m_strRegUrl;
	CString m_strHardCode;
	CString m_strSerial;
	CHyperLink m_stcGetSerial;
public:
	afx_msg void OnStnClickedStaticGetserial();
	afx_msg void OnBnClickedOk();
};
