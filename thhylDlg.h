// thhylDlg.h : header file
//

#if !defined(AFX_THHYLDLG_H__92D9CA16_D836_44C9_BAB8_C5C64D90CCC6__INCLUDED_)
#define AFX_THHYLDLG_H__92D9CA16_D836_44C9_BAB8_C5C64D90CCC6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DlgBaseWZ.h"
#include "FileStatusWZ.h"
#include "RPYAnalyzer.h"

/////////////////////////////////////////////////////////////////////////////
// CThhylDlg dialog

class CThhylDlg : public CDlgBaseWZ
{
// Construction
public:
	CString m_strCurrComment;
	CRPYAnalyzer* m_pRpyAnalyzer;
	CThhylDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CThhylDlg)
	enum { IDD = IDD_THHYL_DIALOG };
	CString	m_rpyfile;
	BOOL	m_bOnTop;
	CString	m_rpyinfo;
	BOOL	m_bAutocomp;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CThhylDlg)
	public:
	virtual BOOL DestroyWindow();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CThhylDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnOntop();
	afx_msg void OnAutocomp();
	afx_msg void OnBrowse();
	afx_msg void OnCopy();
	virtual void OnOK();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnOpen();
	afx_msg void OnMenucopy();
	afx_msg void OnExit();
	afx_msg void OnSetassoc();
	afx_msg void OnDelassoc();
	afx_msg void OnOpenfolder();
	afx_msg void OnOpenShAlice();
	afx_msg void OnFilemoveto();
	afx_msg void OnFilecopyto();
	afx_msg void OnFiledelete();
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnOption();
	afx_msg void OnChgfont();
	afx_msg void OnEditcomment();
	afx_msg void OnOpenprop();
	afx_msg void OnHelphtml();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnMenuhelp();
	afx_msg void OnSaveraw();
	afx_msg void OnSaverawpart();
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
	afx_msg void OnClosefile();
	afx_msg void OnCutfile();
	afx_msg void OnCopyfile();
	afx_msg void OnAbout();
	afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
	afx_msg void OnExitMenuLoop(BOOL);
	afx_msg void OnEnterMenuLoop(BOOL);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	HACCEL m_hAccel;
	CFileStatusWZ m_filestatus;
	BOOL m_bNeedReanalyze; // Need re-analyze when main window has been activated
	BOOL m_bTHX;        // TRUE if the rpy is TH8 or later

	BYTE* m_pRpyData;  // m_pRpyAnalyzer 不保存未解码的 RPY 文件数据，使用这个保存
	DWORD m_dwRpySize; // RPY 文件大小

	void ShowPopup(POINT);
	void SpawnInstance(LPCTSTR lpszFileName);
	void ConvToFullPath();
	void CopyOrMoveRpy(LPCTSTR DialogTitle, BOOL bCopy=TRUE);
	void CopyOrCutFile(BOOL bCopy);
	void Analyze();
	void CloseFile(BOOL bSilently=FALSE);
	void UpdateTitle();
	int UserIconedMsgBox(
			LPCTSTR text,
			UINT uIconID, // 0=MAINFRAME(MARISA), 1=REIMU, others=MARISA
			UINT uType=0,
			LPCTSTR title=NULL);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_THHYLDLG_H__92D9CA16_D836_44C9_BAB8_C5C64D90CCC6__INCLUDED_)
