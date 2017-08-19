// FileListWindow.cpp : implementation file
//

#include "stdafx.h"
#include "thhyl.h"
#include "FileListWindow.h"
#include "filepath_utils.h"
#include "cfgfile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFileListWindow dialog


CFileListWindow::CFileListWindow(CWnd* pParent /*=NULL*/)
	: CDialog(CFileListWindow::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFileListWindow)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_pWndMain = dynamic_cast<CThhylDlg*>(pParent);
	ASSERT(m_pWndMain != NULL);
}


void CFileListWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFileListWindow)
	DDX_Control(pDX, IDC_FILELISTTREE, m_filetree);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFileListWindow, CDialog)
	//{{AFX_MSG_MAP(CFileListWindow)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileListWindow message handlers

void CFileListWindow::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	if (m_filetree.m_hWnd != NULL) {
		m_filetree.SetWindowPos(NULL, 0, 0, cx, cy, 0);
	}
}

BOOL CFileListWindow::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	if (cfg.WinPlace_FileList.length == sizeof(WINDOWPLACEMENT)) {
		SetWindowPlacement(&cfg.WinPlace_FileList);
	}

	m_filetree.SetBkColor(0x000000);
	m_filetree.SetTextColor(0x00FFFF);
	this->Clear();

// 	if (cfg.dwStatus & CFGSTATUS_FILELISTOPENING) {
// 		this->ShowWindow(SW_SHOW);
// 	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFileListWindow::ChangeDir(const CString& strNewDir)
{
	if (m_strDir != strNewDir) {
		m_strDir = strNewDir;
		Refresh();
	}
}

CString CFileListWindow::GetFirstFilePath()
{
	HTREEITEM const hRoot = m_filetree.GetRootItem();
	HTREEITEM const hChild = m_filetree.GetChildItem(hRoot);
	if (hChild != NULL) {
		return m_strDir / m_filetree.GetItemText(hChild);
	}
	else
		return _T("");
}

CString CFileListWindow::GetPreviousFilePath()
{
	HTREEITEM const hSelected = m_filetree.GetSelectedItem();
	if (hSelected != NULL && hSelected != m_filetree.GetRootItem()) {
		HTREEITEM const hPrevious = m_filetree.GetPrevSiblingItem(hSelected);
		if (hPrevious != NULL) {
			return m_strDir / m_filetree.GetItemText(hPrevious);
		}
	}
	return _T("");
}

CString CFileListWindow::GetNextFilePath()
{
	HTREEITEM const hSelected = m_filetree.GetSelectedItem();
	if (hSelected != NULL && hSelected != m_filetree.GetRootItem()) {
		HTREEITEM const hNext = m_filetree.GetNextSiblingItem(hSelected);
		if (hNext != NULL) {
			return m_strDir / m_filetree.GetItemText(hNext);
		}
	}
	return _T("");
}

CString CFileListWindow::GetLastFilePath()
{
	HTREEITEM const hLastFileItem = this->TV_GetLastFileItem();
	if (hLastFileItem != NULL) {
		return m_strDir / m_filetree.GetItemText(hLastFileItem);
	}
	else
		return _T("");
}

bool CFileListWindow::SelectFileItemByText(const CString& text)
{
	HTREEITEM const hRoot = m_filetree.GetRootItem();
	HTREEITEM hWantedItem = m_filetree.GetChildItem(hRoot);
	while (hWantedItem != NULL) {
		if ( m_filetree.GetItemText(hWantedItem) == text ) {
			break;
		}
		hWantedItem = m_filetree.GetNextSiblingItem(hWantedItem);
	}
	
	if (hWantedItem != NULL) {
		// Select file item if we found
		m_filetree.SelectItem(hWantedItem);
		return true;
	}
	else
		return false;
}


CString CFileListWindow::GetFileItemText()
{
	HTREEITEM const hSelected = m_filetree.GetSelectedItem();
	if (hSelected != NULL && hSelected != m_filetree.GetRootItem()) {
		return m_filetree.GetItemText(hSelected);
	}
	else
		return _T("");
}

HTREEITEM CFileListWindow::TV_GetLastFileItem()
{
	HTREEITEM const hRoot = m_filetree.GetRootItem();
	HTREEITEM hChild = m_filetree.GetChildItem(hRoot);
	HTREEITEM hMaybeLast = hChild;
	while (hChild != NULL) {
		hMaybeLast = hChild;
		hChild = m_filetree.GetNextSiblingItem(hChild);
	}
	return hMaybeLast;
}

void CFileListWindow::ChangeFilePath(const CString& strFilePath)
{
	CString dir_part;
	filepath_utils::GetDir(strFilePath, dir_part, false);
	this->ChangeDir(dir_part);

	CString filename_part;
	filepath_utils::GetFilename(strFilePath, filename_part);
	this->SelectFileItemByText(filename_part);
}

void CFileListWindow::Refresh()
{
	CString filename = GetFileItemText();
	this->Clear();
	
	if (m_strDir.IsEmpty()) {
		return;
	}
	
	// m_filetree.InsertItem(m_strDir);
	m_filetree.SetItemText(m_filetree.GetRootItem(), m_strDir);
	
	// Search
	HTREEITEM hRoot = m_filetree.GetRootItem();
	CFileFind ff;
	for (BOOL bWorking=ff.FindFile( m_strDir + _T("\\*.rpy") ); bWorking;  ) {
		bWorking = ff.FindNextFile();
		if (!ff.IsDirectory()) {
			m_filetree.InsertItem(ff.GetFileName(), hRoot);
		}
	}
	
	m_filetree.Expand(hRoot, TVE_EXPAND);
	if (!filename.IsEmpty()) {
		SelectFileItemByText(filename);
	}
}


CString CFileListWindow::GetFilePath()
{
	CString filename = this->GetFileItemText();
	if (filename.IsEmpty()) {
		return filename;
	}
	else {
		return m_strDir / filename;
	}
}


void CFileListWindow::Clear()
{
	m_filetree.DeleteAllItems();
	m_filetree.InsertItem(_T("不知道该显示啥…先打开个文件吧… >_<"));
}

BOOL CFileListWindow::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	// TODO: Add your specialized code here and/or call the base class
	LPNMHDR const pnmh = (LPNMHDR)lParam;
	if (pnmh->hwndFrom == m_filetree.m_hWnd) {
		switch (pnmh->code) {
		case TVN_SELCHANGED:
			{
				LPNMTREEVIEW const pnmtv = (LPNMTREEVIEW)lParam;
				switch(pnmtv->action) {
				case TVC_BYKEYBOARD:
				case TVC_BYMOUSE:
					{
						CString selected_file = this->GetFilePath();
						if (!selected_file.IsEmpty()) {
							m_pWndMain->m_rpyfile = selected_file;
							m_pWndMain->Analyze();
						}
					}
					return TRUE;
				}
				
				break;
			}
		case NM_DBLCLK:
			{
				OnOK();
			}
			return TRUE;
		case TVN_KEYDOWN:
			{
				// MessageBox(_T("aaaa"));
			}
			break;
		}
	}

	return CDialog::OnNotify(wParam, lParam, pResult);
}

void CFileListWindow::OnOK()
{
	CString selected_file = this->GetFilePath();
	if (!selected_file.IsEmpty()) {
		m_pWndMain->SpawnInstance(selected_file);
	}	
}

BOOL CFileListWindow::DestroyWindow() 
{
	// TODO: Add your specialized code here and/or call the base class
	ZeroMemory(&cfg.WinPlace_FileList, sizeof(WINDOWPLACEMENT));
	cfg.WinPlace_FileList.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(&cfg.WinPlace_FileList);
	cfg.WinPlace_FileList.showCmd = IsWindowVisible() ? SW_SHOW : SW_HIDE;
	
	// cfg.setStatus(CFGSTATUS_FILELISTOPENING, this->IsWindowVisible());

	return CDialog::DestroyWindow();
}
