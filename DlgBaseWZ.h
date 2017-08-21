#if !defined(AFX_DLGBASEWZ_H__86AEDC29_CC94_4FB9_98EB_5144FCD31136__INCLUDED_)
#define AFX_DLGBASEWZ_H__86AEDC29_CC94_4FB9_98EB_5144FCD31136__INCLUDED_

// 本程序的所有对话框的基类

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgBaseWZ.h : header file
//

#ifndef TTS_USEVISUALSTYLE
# define TTS_USEVISUALSTYLE (0x100)
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgBaseWZ dialog

class CDlgBaseWZ : public CDialog
{
// Construction
public:
	CDlgBaseWZ( UINT nIDTemplate, CWnd* pParent = NULL);   // standard constructor

	// s_dialogcount相关的方法，目前不使用
	//static int GetDialogCount() { return s_dialogcount; }
	//static void AddRefCount() { ++s_dialogcount; }
	//static void DecRefCount() { --s_dialogcount; }

// Dialog Data
	//{{AFX_DATA(CDlgBaseWZ)
	// enum { IDD = _UNKNOWN_RESOURCE_ID_ };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgBaseWZ)
	public:
	virtual BOOL DestroyWindow();
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
private:
	// 【目前不使用，因为目前根据主窗口是否Enabled来判断是否可以更改打开的文件。以后可能会有改变】
	// 该值表示一个引用计数。
	// 如果 m_dlgopt 没有设置 DLGWZ_OPT_NOCHANGECOUNT，那么表示该对话框会用到录像文件的数据。在 InitDialog() 和 DestroyWindow() 中会分别增减 s_dialogcount 的值。
	// 目前用于控制文件列表(CFileListWindow)的动作：当该计数大于1时，不允许从文件列表中更改打开的文件，以防止产生野指针。
	// 对于某些不会用到录像文件数据的对话框，可以在构造函数中写上：m_dlgopt |= DLGWZ_OPT_NOCHANGECOUNT 。
	//static int s_dialogcount;  
};

#define DLGWZ_OPT_ENABLEMAXIMIZE		0x00000001  // 允许 F11 最大化
#define DLGWZ_OPT_ALWAYSANYDRAG			0x00000002  // `位于非标题栏也可拖动` 总是有效，不受选项中的设置的影响
#define DLGWZ_OPT_NOCHANGECOUNT			0x00000004  // 不要更改 s_dialogcount（目前不使用）

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGBASEWZ_H__86AEDC29_CC94_4FB9_98EB_5144FCD31136__INCLUDED_)
