
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
	CString GetSvrIPFrome(SocketMgr * pstuSkt, CString m_strKey, int &nPort, int& nRow);
};

#ifndef _DEBUG  // debug version in DataTransponderView.cpp
inline CDataTransponderDoc* CDataTransponderView::GetDocument() const
   { return reinterpret_cast<CDataTransponderDoc*>(m_pDocument); }
#endif

