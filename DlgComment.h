#if !defined(AFX_DLGCOMMENT_H__E61C3AC8_557B_4C4A_AD67_87B3818AEA2B__INCLUDED_)
#define AFX_DLGCOMMENT_H__E61C3AC8_557B_4C4A_AD67_87B3818AEA2B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgComment.h : header file
//

#include "DlgBaseWZ.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgComment dialog

class CDlgComment : public CDlgBaseWZ
{
// Construction
public:
	CString* m_pCommentData;
	CString  m_filename;
	const BYTE* m_pRpyData;
	DWORD m_dwRpySize;
	CDlgComment(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgComment)
	enum { IDD = IDD_COMMENT };
	CComboBox	m_CommentCode;
	CEdit	m_CommentEdit;
	UINT	m_CommentCodeEdit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgComment)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgComment)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelchangeCommentcode();
	virtual void OnOK();
	afx_msg void OnSaveas();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	BOOL WriteComment(LPCTSTR infile, LPCTSTR outfile);
	BOOL UpdateCommentToFile(LPCSTR pComment, size_t dwCommentSize, LPCTSTR newfilename);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCOMMENT_H__E61C3AC8_557B_4C4A_AD67_87B3818AEA2B__INCLUDED_)
