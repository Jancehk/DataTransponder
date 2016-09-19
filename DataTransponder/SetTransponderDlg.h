#pragma once


// CSetTransponderDlg dialog

class CSetTransponderDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSetTransponderDlg)

public:
	CSetTransponderDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSetTransponderDlg();

// Dialog Data
	enum { IDD = IDD_SET_TRANS_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
protected:
	int GetDlgItemInteger(int nIDC_NULL);
public:
	afx_msg void OnBnClickedOk();
	CString m_strSrcIP;
	int		m_nSrcPort;
	CString m_strDstIP;
	int		m_nDstPort;
};
