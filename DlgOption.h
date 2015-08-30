#if !defined(AFX_DLGOPTION_H__689C42C1_C013_4844_8AA1_D2E0280CDA7F__INCLUDED_)
#define AFX_DLGOPTION_H__689C42C1_C013_4844_8AA1_D2E0280CDA7F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgOption.h : header file
//

#include "DlgBaseWZ.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgOption dialog

class CDlgOption : public CDlgBaseWZ
{
// Construction
public:
	CDlgOption(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgOption)
	enum { IDD = IDD_OPTION };
	CComboBox	m_InfoCode;
	CComboBox	m_CommentCode;
	UINT	m_CommentCodeEdit;
	UINT	m_InfoCodeEdit;
	BOOL	m_chkDblClk;
	BOOL	m_chkMBtn;
	BOOL	m_chkConfirm;
	BOOL    m_chkPlayTime;
	BOOL    m_chkSlowRate;
	BOOL	m_chk9;
	BOOL	m_chkCopyOpenDest;
	BOOL	m_chkAnyDrag;
	BOOL	m_chkAutoExit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgOption)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgOption)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeCommentcode();
	virtual void OnOK();
	afx_msg void OnSelchangeInfocode();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGOPTION_H__689C42C1_C013_4844_8AA1_D2E0280CDA7F__INCLUDED_)
