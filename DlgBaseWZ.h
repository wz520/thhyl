#if !defined(AFX_DLGBASEWZ_H__86AEDC29_CC94_4FB9_98EB_5144FCD31136__INCLUDED_)
#define AFX_DLGBASEWZ_H__86AEDC29_CC94_4FB9_98EB_5144FCD31136__INCLUDED_

// 本程序的所有对话框的基类

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgBaseWZ.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgBaseWZ dialog

class CDlgBaseWZ : public CDialog
{
// Construction
public:
	CDlgBaseWZ( UINT nIDTemplate, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgBaseWZ)
	// enum { IDD = _UNKNOWN_RESOURCE_ID_ };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgBaseWZ)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgBaseWZ)
	virtual BOOL OnInitDialog();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CToolTipCtrl	m_tooltip;
	DWORD			m_dlgopt;
};

#define DLGWZ_OPT_ENABLEMAXIMIZE		0x00000001  // 允许 F11 最大化
#define DLGWZ_OPT_ALWAYSANYDRAG			0x00000002  // `位于非标题栏也可拖动` 总是有效，不受选项中的设置的影响

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGBASEWZ_H__86AEDC29_CC94_4FB9_98EB_5144FCD31136__INCLUDED_)
