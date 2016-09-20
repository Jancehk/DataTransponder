
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
DWORD WINAPI ThreadListenCLient(void * lParamInfo);
DWORD WINAPI ThreadDealClient(void * lParamInfo);

// CDataTransponderView

IMPLEMENT_DYNCREATE(CDataTransponderView, CListView)

BEGIN_MESSAGE_MAP(CDataTransponderView, CListView)
	ON_COMMAND(ID_FILE_NEW, &CDataTransponderView::OnFileNew)
	ON_WM_TIMER()
END_MESSAGE_MAP()

// CDataTransponderView construction/destruction

CDataTransponderView::CDataTransponderView()
{
	// TODO: add construction code here
	hThread = 0;
	m_stuSktMgr = NULL;
	m_stuSktMgrTmp = NULL;
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
	LONG lStyle = GetWindowLong(m_pListCtrl->m_hWnd, GWL_STYLE);
	CListView::OnInitialUpdate();
	
	lStyle &= ~LVS_TYPEMASK;
	lStyle |= LVS_REPORT;
	SetWindowLong(m_pListCtrl->m_hWnd, GWL_STYLE, lStyle);

	dwStyle |= LVS_EX_FULLROWSELECT;
	dwStyle |= LVS_EX_GRIDLINES;
	//dwStyle |= LVS_EX_CHECKBOXES;
	m_pListCtrl->SetExtendedStyle(dwStyle); 
	//m_pListCtrl->InsertColumn(m_iNumColumns++, _T("ID"), LVCFMT_CENTER, 40);
	m_pListCtrl->InsertColumn(m_iNumColumns++, _T("Type"), LVCFMT_CENTER, 50);
	m_pListCtrl->InsertColumn(m_iNumColumns++, _T("SRC"), LVCFMT_CENTER, 160);
	m_pListCtrl->InsertColumn(m_iNumColumns++, _T("S_P"), LVCFMT_CENTER, 60);
	m_pListCtrl->InsertColumn(m_iNumColumns++, _T("DST"), LVCFMT_CENTER, 160);
	m_pListCtrl->InsertColumn(m_iNumColumns++, _T("D_P"), LVCFMT_CENTER, 60);
	m_pListCtrl->InsertColumn(m_iNumColumns++, _T("S_Speed"), LVCFMT_CENTER, 120);
	m_pListCtrl->InsertColumn(m_iNumColumns++, _T("D_Speed"), LVCFMT_CENTER, 120);
	m_pListCtrl->InsertColumn(m_iNumColumns++, _T("ThreadID"), LVCFMT_CENTER, 80);
	m_pListCtrl->InsertColumn(m_iNumColumns++, _T("Error Info"), LVCFMT_CENTER, 300);
	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadListenCLient, this, 0, &ThreadID);
	// TODO: You may populate your ListView with items by directly accessing
	//  its list control through a call to GetListCtrl().
	SetTimer(1, 1000, NULL);
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
	if(0 != m_stuSktMgrTmp->CreateSkt(m_setDlg.m_nSrcPort,m_setDlg.m_strSrcIP))
	{
		MessageBox(_T("socket bind faild"));
		delete m_stuSktMgrTmp;
		m_stuSktMgrTmp = NULL;
		return ;
	}
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

int CDataTransponderView::SetSpeedInItem(int nItem, int nSubItem, int nSpeed)
{
	CString m_strTmp;
	CListCtrl * m_pListCtrl = &GetListCtrl();
	int nPrevSpeed = 0;
	/*if (nSpeed<=100)
	{
		nPrevSpeed = GetSpeedInItem(nItem, nSubItem);
		if (nSpeed < nPrevSpeed)
		{
			nSpeed = nPrevSpeed - nPrevSpeed / 7;
		}
	}*/
	if (nSpeed>1024 * 1024 * 1024)
	{
		m_strTmp.Format(_T("%.2fGB"), (float)nSpeed / (1024 * 1024 * 1024));
	}
	else if (nSpeed>1024 * 1024)
	{
		m_strTmp.Format(_T("%.2fMB"), (float)nSpeed / (1024 * 1024));
	}
	else if (nSpeed>1024)
	{
		m_strTmp.Format(_T("%.2fKB"), (float)nSpeed / 1024);
	}
	else
	{
		m_strTmp.Format(_T("%dByte"), nSpeed);
	}
	m_pListCtrl->SetItemText(nItem, nSubItem, m_strTmp);
	return 0;
}
int CDataTransponderView::GetSpeedInItem(int nItem, int nSubItem)
{
	CString m_strTmp;
	CListCtrl * m_pListCtrl = &GetListCtrl();
	m_strTmp = m_pListCtrl->GetItemText(nItem, nSubItem);
	if (m_strTmp.IsEmpty())
	{
		return 0;
	}
	if (m_strTmp.Find(_T("GB")) > 0)
	{
		return int(_wtof(m_strTmp.GetBuffer()) * 1024 * 1024*1024);
	}
	else if (m_strTmp.Find(_T("MB")) > 0)
	{
		return int(_wtof(m_strTmp.GetBuffer()) * 1024 * 1024) ;
	}
	else if (m_strTmp.Find(_T("KB")) > 0)
	{
		return int(_wtof(m_strTmp.GetBuffer()) * 1024 );
	}
	else if (m_strTmp.Find(_T("Byte")) > 0)
	{
		return _wtoi(m_strTmp.GetBuffer());
	}
	return 0;
}

void CDataTransponderView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	SocketMgr * m_stuSktMgrTmp = m_stuSktMgr;
	int nRowNum = 0,nLine = 0;
	CString m_DstIP;
	int nDstPort=0,nSpeed = 0;
	if(1== nIDEvent)
	{
		while(NULL != m_stuSktMgrTmp)
		{
			if (SKT_SVR_FLG == m_stuSktMgrTmp->nSvrFlg)
			{
				m_stuSktMgrTmp = m_stuSktMgrTmp->GetNextSkt();
				continue;
			}
			if (SKT_SRC_FLG == m_stuSktMgrTmp->nSvrFlg)
			{
				nLine = 5;
				m_DstIP = GetSvrIPFrome(m_stuSktMgrTmp, _T("C"), nDstPort, nRowNum);
			}
			if (SKT_DST_FLG == m_stuSktMgrTmp->nSvrFlg)
			{
				nLine = 6;
				m_DstIP = GetSvrIPFrome(m_stuSktMgrTmp->pCDstSocketMgr, _T("C"), nDstPort, nRowNum);
			}
			if (m_DstIP.IsEmpty())
			{
				m_stuSktMgrTmp = m_stuSktMgrTmp->GetNextSkt();
				continue;
			}
			nSpeed = m_stuSktMgrTmp->GetSpeed();
			if(nSpeed<0)
			{
				m_stuSktMgrTmp = m_stuSktMgrTmp->GetNextSkt();
				continue;
			}
			SetSpeedInItem(nRowNum-1, nLine, nSpeed);

			m_stuSktMgrTmp = m_stuSktMgrTmp->GetNextSkt();
		}
		return ;
	}
	CListView::OnTimer(nIDEvent);
}

DWORD WINAPI ThreadListenCLient(void * lParamInfo)
{
	CDataTransponderView *  pView = (CDataTransponderView*)lParamInfo;
	CString m_DstIP, m_strTmp;
	DWORD	ThreadID = 0;
	int		nDstPort;
	int		nRowNum = 0, nRow = 0;
	SocketMgr * pstuSktCli;
	SocketMgr * pstuSktNewCli;
	SocketMgr * pstuSktNewDst;
	int nSelectTimeOut = 10;
	CListCtrl * m_pListCtrl = NULL;
	if (NULL == pView )
	{
		return -1;
	}
	while (NULL == pView->m_stuSktMgr)
	{
		Sleep(1000);
	}
	m_pListCtrl = &pView->GetListCtrl();
	if (NULL == m_pListCtrl)
	{
		return -2;
	}
	while (1)
	{
		pstuSktCli = pView->m_stuSktMgr->SelectSkt(nSelectTimeOut);
		if (NULL == pstuSktCli)
		{
			continue;
		}
		if (SKT_SVR_FLG != pstuSktCli->nSvrFlg)
		{
			m_strTmp.Format(_T("Skt%p[%s:%d] Flg is incorrect[%d]"), pstuSktCli, pstuSktCli->m_strIP, pstuSktCli->n_port, pstuSktCli->nSvrFlg);
			m_pListCtrl->SetItemText(0, 8, m_strTmp);
			continue;
		}

		pstuSktNewCli = pstuSktCli->AcceptSkt();
		if (NULL == pstuSktNewCli)
		{
			m_strTmp.Format(_T("Skt%p[%s:%d] accept new client faild"), pstuSktCli, pstuSktCli->m_strIP, pstuSktCli->n_port);
			m_pListCtrl->SetItemText(0, 8, m_strTmp);
			continue;
		}
		while (NULL != pView->m_stuSktMgrTmp)
		{
			Sleep(1000);
		}
		pstuSktNewDst = pstuSktNewCli->GetNewDst();
		if (NULL == pstuSktNewDst)
		{
			m_strTmp.Format(_T("Skt%p[%s:%d]->%p get new dst client faild"), pstuSktCli, pstuSktCli->m_strIP, pstuSktCli->n_port, pstuSktNewCli);
			m_pListCtrl->SetItemText(0, 8, m_strTmp);
			continue;
		}
		m_DstIP = pView->GetSvrIPFrome(pstuSktCli, _T("S"), nDstPort, nRowNum);
		if (m_DstIP.IsEmpty() || 0 == nDstPort || 0 == nRowNum)
		{
			m_strTmp.Format(_T("Skt%p[%s:%d] not found in list"), pstuSktCli, pstuSktCli->m_strIP, pstuSktCli->n_port);
			m_pListCtrl->SetItemText(0, 8, m_strTmp);
			delete pstuSktNewCli;
			pstuSktNewCli = NULL;
			continue;
		}
		if (0 != pstuSktNewDst->CreateSkt(m_DstIP, nDstPort))
		{
			m_strTmp.Format(_T("connect to [%s:%d] faild"), m_DstIP, nDstPort);
			m_pListCtrl->SetItemText(nRowNum-1, 8, m_strTmp);
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
		m_pListCtrl->SetItemText(nRow, 6, _T("0"));
		pView->m_stuSktMgrTmp = pstuSktNewCli;
		pstuSktNewCli->hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadDealClient, lParamInfo, 0, &ThreadID);

		m_strTmp.Format(_T("%d"), ThreadID);
		m_pListCtrl->SetItemText(nRow, 7, m_strTmp);
	}
	return 0;
}
DWORD WINAPI ThreadDealClient(void * lParamInfo)
{
	CDataTransponderView *  pView = (CDataTransponderView*)lParamInfo;
	CString m_DstIP, m_strTmp;
	int		nDstPort;
	int		nRowNum = 0, nRow = 0;
	SocketMgr * pstuSktCli;
	SocketMgr * pstuSktNewCli;
	int nSelectTimeOut = 10;
	CListCtrl * m_pListCtrl = NULL;
	if (NULL == pView ||
		NULL == pView->m_stuSktMgrTmp)
	{
		return -1;
	}
	m_pListCtrl = &pView->GetListCtrl();
	if (NULL == m_pListCtrl)
	{
		return -2;
	}
	pstuSktNewCli = pView->m_stuSktMgrTmp;
	pView->m_stuSktMgrTmp = NULL;
	while (1)
	{
		pstuSktCli = pstuSktNewCli->SelectSkt(nSelectTimeOut,1);
		if (NULL == pstuSktCli)
		{
			continue;
		}
		if (SKT_SVR_FLG == pstuSktCli->nSvrFlg)
		{
			continue;
		}
		if (0 == pstuSktCli->TransData())
		{
			continue;
		}
		m_DstIP = pView->GetSvrIPFrome(pstuSktCli, _T("C"), nDstPort, nRowNum);
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
		break;
	}
	return 0;
}