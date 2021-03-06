
// DataTransponderView.h : interface of the CDataTransponderView class
//

#pragma once
#include "SocketMgr.h"


class CDataTransponderView : public CListView
{
protected: // create from serialization only
	CDataTransponderView();
	DECLARE_DYNCREATE(CDataTransponderView)

// Attributes
public:
	CDataTransponderDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate(); // called first time after construct

// Implementation
public:
	virtual ~CDataTransponderView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnFileNew();
private:
	HANDLE hThread;
public:
	SocketMgr * m_stuSktMgr;
	SocketMgr * m_stuSktMgrTmp;
	CString GetSvrIPFrome(SocketMgr * pstuSkt, CString m_strKey, int &nPort, int& nRow);
	int SetSpeedInItem(int nItem, int nSubItem, int nSpeed);
	int GetSpeedInItem(int nItem, int nSubItem);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	int DelSvrInfo(void);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnViewHidden();
};

#ifndef _DEBUG  // debug version in DataTransponderView.cpp
inline CDataTransponderDoc* CDataTransponderView::GetDocument() const
   { return reinterpret_cast<CDataTransponderDoc*>(m_pDocument); }
#endif

