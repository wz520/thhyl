#if !defined(AFX_SAVERAWDLG_H__ADEE1FB7_CA37_467B_8E18_2A7D332A0AC8__INCLUDED_)
#define AFX_SAVERAWDLG_H__ADEE1FB7_CA37_467B_8E18_2A7D332A0AC8__INCLUDED_

#include "thdecode/thcommon.h"	// Added by ClassView
#include "RPYAnalyzer_common.h"
#include "DlgBaseWZ.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SaveRawDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSaveRawDlg dialog

class CSaveRawDlg : public CDlgBaseWZ
{
// Construction
public:
	CString m_strFileName;
	DWORD m_dwRpySize;
	const RPYINFO_POINTERS* m_pRpyInfoPointers;
	const THRPYINFO_BASE* m_pRpyInfoBase;
	CSaveRawDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSaveRawDlg)
	enum { IDD = IDD_SAVERAWPART };
	CListCtrl	m_list;
	BOOL	m_chkExit;
	BOOL	m_chkOverwrite;
	BOOL	m_chkInv;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSaveRawDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSaveRawDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSaverawSelall();
	virtual void OnOK();
	afx_msg void OnSaverawSelhi();
	afx_msg void OnSaverawChkexit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	bool SaveCheckedFiles(LPCTSTR szDir);
	void AutoSetColumnWidth();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SAVERAWDLG_H__ADEE1FB7_CA37_467B_8E18_2A7D332A0AC8__INCLUDED_)
