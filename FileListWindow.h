#if !defined(AFX_FILELISTWINDOW_H__7D5483BC_99EA_447E_A51F_22D4B7A818B2__INCLUDED_)
#define AFX_FILELISTWINDOW_H__7D5483BC_99EA_447E_A51F_22D4B7A818B2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FileListWindow.h : header file
//

// forward declarations are needed to deal with errors because the headers files are being included each other
class CFileListWindow;  
class CThhylDlg;

#include "thhylDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CFileListWindow dialog

class CFileListWindow : public CDialog
{
// Construction
public:
	void ChangeFilePath(const CString& strFilePath);
	void Refresh();
	void ChangeDir(const CString& szNewDir);
	CString GetFirstFilePath() const;
	CString GetPreviousFilePath() const;
	CString GetNextFilePath() const;
	CString GetLastFilePath() const;
	CString GetFilePath() const; // get filepath from the selected item text, returns empty string if nothing is selected
	// return a filepath that is suggested to be selected after deleting the current file
	CString PrepareForDeleteFile();
	void Clear();
	CFileListWindow(CWnd* pParent = NULL);   // standard constructor


// Dialog Data
	//{{AFX_DATA(CFileListWindow)
	enum { IDD = IDD_FILELIST };
	CTreeCtrl	m_filetree;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFileListWindow)
	public:
	virtual BOOL DestroyWindow();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFileListWindow)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CString m_strDir;

	bool SelectFileItemByText(const CString& text);  // return true if the item is successfully selected, false otherwise
	HTREEITEM FindFileItemByText(const CString& text) const;
	CString GetFileItemText() const; // return emtpy string if nothing or the root node is selected.
	HTREEITEM TV_GetLastFileItem() const;
	CThhylDlg* m_pWndMain;
	HACCEL m_hAccel;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILELISTWINDOW_H__7D5483BC_99EA_447E_A51F_22D4B7A818B2__INCLUDED_)
