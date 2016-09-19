
// DataTransponderView.cpp : implementation of the CDataTransponderView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "DataTransponder.h"
#endif

#include "DataTransponderDoc.h"
#include "DataTransponderView.h"
#include "SetTransponderDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
DWORD WINAPI ThreadRefushEditInfo(void * lParamInfo);

// CDataTransponderView

IMPLEMENT_DYNCREATE(CDataTransponderView, CListView)

BEGIN_MESSAGE_MAP(CDataTransponderView, CListView)
	ON_COMMAND(ID_FILE_NEW, &CDataTransponderView::OnFileNew)
END_MESSAGE_MAP()

// CDataTransponderView construction/destruction

CDataTransponderView::CDataTransponderView()
{
	// TODO: add construction code here
	hThread = 0;
	m_stuSktMgr = NULL;
}

CDataTransponderView::~CDataTransponderView()
{
	if (0 != hThread)
	{
		TerminateThread(hThread,-1);
		CloseHandle(hThread);
	}
}

BOOL CDataTransponderView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CListView::PreCreateWindow(cs);
}

void CDataTransponderView::OnInitialUpdate()
{
	DWORD ThreadID = 0;
	CListCtrl * m_pListCtrl = &GetListCtrl();
	int m_iNumColumns = 0;
	DWORD dwStyle = m_pListCtrl->GetExtendedStyle();
	LONG lStyle = GetWindowLong(m_pListCtrl->m_hWnd, GWL_STYLE);//获取当前窗口style
	CListView::OnInitialUpdate();
	
	lStyle &= ~LVS_TYPEMASK; //清除显示方式位
	lStyle |= LVS_REPORT; //设置style
	SetWindowLong(m_pListCtrl->m_hWnd, GWL_STYLE, lStyle);//设置style

	dwStyle |= LVS_EX_FULLROWSELECT;//选中某行使整行高亮（只适用与report风格的listctrl）
	dwStyle |= LVS_EX_GRIDLINES;//网格线（只适用与report风格的listctrl）
	//dwStyle |= LVS_EX_CHECKBOXES;//item前生成checkbox控件
	m_pListCtrl->SetExtendedStyle(dwStyle); 
	//m_pListCtrl->InsertColumn(m_iNumColumns++, _T("ID"), LVCFMT_CENTER, 40);
	m_pListCtrl->InsertColumn(m_iNumColumns++, _T("Type"), LVCFMT_CENTER, 60);
	m_pListCtrl->InsertColumn(m_iNumColumns++, _T("SRC"), LVCFMT_CENTER, 300);
	m_pListCtrl->InsertColumn(m_iNumColumns++, _T("S_P"), LVCFMT_CENTER, 60);
	m_pListCtrl->InsertColumn(m_iNumColumns++, _T("DST"), LVCFMT_CENTER, 300);
	m_pListCtrl->InsertColumn(m_iNumColumns++, _T("D_P"), LVCFMT_CENTER, 60);
	m_pListCtrl->InsertColumn(m_iNumColumns++, _T("S_Speed"), LVCFMT_CENTER, 120);
	m_pListCtrl->InsertColumn(m_iNumColumns++, _T("D_Speed"), LVCFMT_CENTER, 120);
	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadRefushEditInfo, this, 0, &ThreadID);
	// TODO: You may populate your ListView with items by directly accessing
	//  its list control through a call to GetListCtrl().
}


// CDataTransponderView diagnostics

#ifdef _DEBUG
void CDataTransponderView::AssertValid() const
{
	CListView::AssertValid();
}

void CDataTransponderView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

CDataTransponderDoc* CDataTransponderView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDataTransponderDoc)));
	return (CDataTransponderDoc*)m_pDocument;
}
#endif //_DEBUG


// CDataTransponderView message handlers


void CDataTransponderView::OnFileNew()
{
	// TODO: Add your command handler code here
	CSetTransponderDlg m_setDlg;
	SocketMgr * m_stuSktMgrTmp;
	CListCtrl * m_pListCtrl = &GetListCtrl();
	CString m_strTmp;
	int nIndexCount = 0;
	int nRow = 0;
	if (IDOK != m_setDlg.DoModal())
	{
		return;
	}
	if (NULL == m_stuSktMgr)
	{
		m_stuSktMgrTmp = new SocketMgr();
		m_stuSktMgr = m_stuSktMgrTmp;
	}
	else
	{
		m_stuSktMgrTmp = m_stuSktMgr->GetNewSkt();
	}
	if (NULL == m_stuSktMgrTmp)
	{
		MessageBox(_T("Get Skt is faild"));
		return ;
	}
	m_stuSktMgrTmp->CreateSkt(m_setDlg.m_nSrcPort,m_setDlg.m_strSrcIP);
	nIndexCount = m_pListCtrl->GetItemCount();
	nRow = m_pListCtrl->InsertItem(nIndexCount,_T("S"));
	m_pListCtrl->SetItemText(nRow, 1, m_setDlg.m_strSrcIP);
	m_strTmp.Format(_T("%d"), m_setDlg.m_nSrcPort);
	m_pListCtrl->SetItemText(nRow, 2, m_strTmp);
	m_pListCtrl->SetItemText(nRow, 3, m_setDlg.m_strDstIP);
	m_strTmp.Format(_T("%d"), m_setDlg.m_nDstPort);
	m_pListCtrl->SetItemText(nRow, 4, m_strTmp);
	m_pListCtrl->SetItemText(nRow, 5, _T("0"));
	m_pListCtrl->SetItemText(nRow, 6, _T("0"));
}
CString CDataTransponderView::GetSvrIPFrome(SocketMgr * pstuSkt, CString m_strKey,int &nPort, int& nRow)
{
	CListCtrl * m_pListCtrl = &GetListCtrl();
	int nIndexCount = m_pListCtrl->GetItemCount();
	CString m_strTmp;
	if (NULL == pstuSkt)
	{
		return NULL;
	}
	for (nRow = 0; nRow <nIndexCount; nRow++)
	{
		m_strTmp = m_pListCtrl->GetItemText(nRow, 0);
		if (m_strTmp != m_strKey)
		{
			continue;
		}
		m_strTmp = m_pListCtrl->GetItemText(nRow, 1);
		if (m_strTmp != pstuSkt->m_strIP)
		{
			continue;
		}
		m_strTmp = m_pListCtrl->GetItemText(nRow, 2);

		if (_wtoi(m_strTmp.GetBuffer()) != pstuSkt->n_port)
		{
			continue;
		}
		break;
	}
	nPort = 0;
	if (nRow >= nIndexCount)
	{
		nRow = 0;
		return _T("");
	}
	nPort = _wtoi(m_pListCtrl->GetItemText(nRow, 4).GetBuffer());
	return m_pListCtrl->GetItemText(nRow++, 3);
}

DWORD WINAPI ThreadRefushEditInfo(void * lPalramInfo)
{
	CDataTransponderView *  pView = (CDataTransponderView*)lPalramInfo;
	CString m_DstIP, m_strTmp;
	int		nDstPort;
	int		nRowNum = 0, nRow = 0;
	SocketMgr * pstuSktCli;
	SocketMgr * pstuSktNewCli;
	SocketMgr * pstuSktNewDst;
	int nSelectTimeOut = 500;
	CListCtrl * m_pListCtrl = NULL;
	while (1)
	{
		if (NULL == pView ||
			NULL == pView->m_stuSktMgr)
		{
			Sleep(500);
			continue;
		}
		if (NULL == m_pListCtrl)
		{
			m_pListCtrl = &pView->GetListCtrl();
		}
		pstuSktCli = pView->m_stuSktMgr->SelectSkt(nSelectTimeOut);
		if (NULL == pstuSktCli)
		{
			continue;
		}
		if (1 != pstuSktCli->nSvrFlg)
		{
			if (0 != pstuSktCli->TransData())
			{
				m_DstIP = pView->GetSvrIPFrome(pstuSktCli,_T("C"), nDstPort, nRowNum);
				if (m_DstIP.IsEmpty())
				{
					m_DstIP = pView->GetSvrIPFrome(pstuSktCli->pCDstSocketMgr, _T("C"), nDstPort, nRowNum);
				}
				if (0 != nRowNum)
				{
					m_pListCtrl->DeleteItem(--nRowNum);
				}
				delete pstuSktCli;
				pstuSktCli = NULL;
				continue;
			}
		}

		pstuSktNewCli = pstuSktCli->AcceptSkt();
		if (NULL == pstuSktNewCli)
		{
			continue;
		}
		pstuSktNewDst = pstuSktNewCli->GetNewDst();
		if (NULL == pstuSktNewDst)
		{
			continue;
		}
		m_DstIP = pView->GetSvrIPFrome(pstuSktCli, _T("S"), nDstPort, nRowNum);
		if (m_DstIP.IsEmpty() || 0 == nDstPort || 0 == nRowNum)
		{
			delete pstuSktNewCli;
			pstuSktNewCli = NULL;
			continue;
		}
		if (0 != pstuSktNewDst->CreateSkt(m_DstIP, nDstPort))
		{
			delete pstuSktNewCli;
			pstuSktNewCli = NULL;
			continue;
		}
		nRow = m_pListCtrl->InsertItem(nRowNum, _T("C"));
		m_pListCtrl->SetItemText(nRow, 1, pstuSktNewCli->m_strIP);
		m_strTmp.Format(_T("%d"), pstuSktNewCli->n_port);
		m_pListCtrl->SetItemText(nRow, 2, m_strTmp);

		m_pListCtrl->SetItemText(nRow, 3, m_DstIP);
		m_strTmp.Format(_T("%d"), nDstPort);
		m_pListCtrl->SetItemText(nRow, 4, m_strTmp);

		m_pListCtrl->SetItemText(nRow, 5, _T("0"));
	}
	return 0;
}