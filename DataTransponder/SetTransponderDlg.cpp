// SetTransponderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DataTransponder.h"
#include "SetTransponderDlg.h"
#include "afxdialogex.h"


// CSetTransponderDlg dialog

IMPLEMENT_DYNAMIC(CSetTransponderDlg, CDialogEx)

CSetTransponderDlg::CSetTransponderDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSetTransponderDlg::IDD, pParent)
{

}

CSetTransponderDlg::~CSetTransponderDlg()
{
}

void CSetTransponderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSetTransponderDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CSetTransponderDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CSetTransponderDlg message handlers

int CSetTransponderDlg::GetDlgItemInteger(int nIDC_NULL)
{
	int nRtnValue = 0;
	CString strTmp;
	GetDlgItemText(nIDC_NULL, strTmp);
	nRtnValue = _wtoi(strTmp.GetBuffer(strTmp.GetLength()));
	strTmp.ReleaseBuffer();
	return nRtnValue;
}

void CSetTransponderDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	GetDlgItemText(IDC_IP_SRC, m_strSrcIP);
	m_nSrcPort = GetDlgItemInteger(IDC_EDIT_SRC);
	GetDlgItemText(IDC_IP_DST, m_strDstIP);
	m_nDstPort = GetDlgItemInteger(IDC_EDIT_DST);
	if (0 >= m_nSrcPort || m_nSrcPort >= 65536)
	{
		MessageBox(_T("Transponder Src Port is incorrect"));
		return;
	}
	if (m_strDstIP.IsEmpty() || 0 == m_strDstIP.Compare(_T("0.0.0.0")))
	{
		MessageBox(_T("Transponder Dst IP is Empty"));
		return;
	}
	if (0 >= m_nDstPort || m_nDstPort >= 65536)
	{
		MessageBox(_T("Transponder Dst Port is incorrect"));
		return;
	}
	CDialogEx::OnOK();
}
