// FileListWindow.cpp : implementation file
//

#include "stdafx.h"
#include "thhyl.h"
#include "FileListWindow.h"
#include "filepath_utils.h"
#include "cfgfile.h"
#include "indep/globalxp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFileListWindow dialog


static LPCTSTR s_title = _T("�ļ��б�");

CFileListWindow::CFileListWindow(CWnd* pParent /*=NULL*/)
	: CDialog(CFileListWindow::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFileListWindow)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_pWndMain = dynamic_cast<CThhylDlg*>(pParent);
	ASSERT(m_pWndMain != NULL);
	m_hAccel = ::LoadAccelerators(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_ACCEL_FILELIST)); 
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
	ON_WM_TIMER()
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
	if (cfg.byteAlphaForFileList == 0) cfg.byteAlphaForFileList = 255;   // ��һ�������� 1.85 ʱ alpha Ӧ��Ϊ 0����������Ϊ 255
	SetWindowAlpha(this->m_hWnd, cfg.byteAlphaForFileList);

	m_filetree.SetBkColor(0x223344);
	m_filetree.SetTextColor(0x00FFFF);
	this->Clear();

	m_pWndMain->m_pWindowGluer->addFollower(m_hWnd);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFileListWindow::ChangeDir(const CString& strNewDir, BOOL bMustRefresh)
{
	if (bMustRefresh || m_strDir != strNewDir) {
		m_strDir = strNewDir;
		Refresh();
	}
}

CString CFileListWindow::GetFirstFilePath() const
{
	HTREEITEM const hRoot = m_filetree.GetRootItem();
	HTREEITEM const hChild = m_filetree.GetChildItem(hRoot);
	if (hChild != NULL) {
		return m_strDir / m_filetree.GetItemText(hChild);
	}
	else
		return _T("");
}

CString CFileListWindow::GetPreviousFilePath() const
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

CString CFileListWindow::GetNextFilePath() const
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

CString CFileListWindow::GetLastFilePath() const
{
	HTREEITEM const hLastFileItem = this->TV_GetLastFileItem();
	if (hLastFileItem != NULL) {
		return m_strDir / m_filetree.GetItemText(hLastFileItem);
	}
	else
		return _T("");
}

HTREEITEM CFileListWindow::FindFileItemByText(const CString& text) const
{
	HTREEITEM const hRoot = m_filetree.GetRootItem();
	HTREEITEM hWantedItem = m_filetree.GetChildItem(hRoot);
	while (hWantedItem != NULL) {
		if ( m_filetree.GetItemText(hWantedItem) == text ) {
			break;
		}
		hWantedItem = m_filetree.GetNextSiblingItem(hWantedItem);
	}

	return hWantedItem;
}

bool CFileListWindow::SelectFileItemByText(const CString& text)
{
	HTREEITEM const result = FindFileItemByText(text);
	if ( result ) {
		// Select file item if we found
		m_filetree.SelectItem(result);
		return true;
	}
	else
		return false;
}


CString CFileListWindow::GetFileItemText() const
{
	HTREEITEM const hSelected = m_filetree.GetSelectedItem();
	if (hSelected != NULL && hSelected != m_filetree.GetRootItem()) {
		return m_filetree.GetItemText(hSelected);
	}
	else
		return _T("");
}

HTREEITEM CFileListWindow::TV_GetLastFileItem() const
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

void CFileListWindow::ChangeFilePath(const CString& strFilePath, BOOL bMustRefresh)
{
	CString dir_part;
	filepath_utils::GetDir(strFilePath, dir_part, false);
	this->ChangeDir(dir_part, bMustRefresh);

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


CString CFileListWindow::GetFilePath() const
{
	CString filename = this->GetFileItemText();
	if (filename.IsEmpty()) {
		return filename;
	}
	else {
		return m_strDir / filename;
	}
}

CString CFileListWindow::PrepareForDeleteFile()
{
	// �Ȼ�ȡ�����
	CString newfilepath = GetNextFilePath();
	CString newitemtext;
	if ( newfilepath.IsEmpty() ) {
		newfilepath = GetPreviousFilePath(); // �������û�У���ȡ�����
	}
	if ( !newfilepath.IsEmpty() ) {
		filepath_utils::GetFilename(newfilepath, newitemtext);
	}
	Refresh(); // ˢ��һ��
	// ���ˢ�º�ǰһ�����һ���ļ�Ҳû���ˣ����ܴ��ⲿɾ���ˣ�
	// ����ˢ��ǰɾ���ľ������һ���ļ�
	// ��ô���ص�һ���ļ�
	if ( newitemtext.IsEmpty() || FindFileItemByText(newitemtext) == NULL )
		return GetFirstFilePath();
	else
		return newfilepath;
}

void CFileListWindow::Clear()
{
	m_filetree.DeleteAllItems();
	m_filetree.InsertItem(_T("��֪������ʾɶ���ȴ򿪸��ļ��ɡ� >_<"));
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
		case TVN_SELCHANGING:
			// if (m_pWndMain->GetDialogCount() > 1) {
			if (!m_pWndMain->IsWindowEnabled()) {
				CString const strMessage = _T("���ڲ����Դ������ļ�����Ϊ�������Ի�������ʹ���Ѵ򿪵��ļ���\r\n\r\n��ر���Щ�Ի�������ԡ�");
				MessageBox(strMessage, s_title, MB_ICONEXCLAMATION);
				/* // TODO: ����ڵ�ǰ�����ö�ʱ�����Կ����´���
				LPNMTREEVIEW const pnmtv = (LPNMTREEVIEW)lParam;
				CString strFilename = m_strDir / m_filetree.GetItemText(pnmtv->itemNew.hItem);
				CString strMessage = _T("���ɸ��Ĵ򿪵��ļ�����Ϊ�������Ի�������ʹ���Ѵ򿪵��ļ���\r\n\r\n�Ƿ�Ҫ���´����д򿪸ո�ѡ�������ļ���\r\n\r\n");
				strMessage += strFilename;
				const int mbret = MessageBox(strMessage, s_title, MB_ICONEXCLAMATION | MB_YESNO);
				if (mbret == IDYES) {
					if (!m_strDir.IsEmpty())
						m_pWndMain->SpawnInstance( strFilename );
					else
						MessageBox(_T("ERR100"), s_title, MB_ICONSTOP);  // this should not happen
				}
				*/

				*pResult = static_cast<LRESULT>(TRUE);
				return TRUE;
			}
			break;
		case NM_DBLCLK:
			OnOK();
			return TRUE;
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
	cfg.byteAlphaForFileList = GetWindowAlpha(this->m_hWnd);

	return CDialog::DestroyWindow();
}


#define IDTIMER_RESETTITLE				(42)

BOOL CFileListWindow::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (m_hAccel) {
		if (::TranslateAccelerator(m_hWnd, m_hAccel, pMsg))
			return TRUE;

		if (m_pWndMain->m_pWindowGluer->handleSwitchMessage(this->GetSafeHwnd(), pMsg->message, pMsg->wParam, pMsg->lParam)) return TRUE;

		// other keys
		switch (pMsg->message)
		{
		case WM_MOUSEWHEEL:
			{
				const WORD vKey = LOWORD(pMsg->wParam);
				
				if ( (vKey & MK_LBUTTON) ) {
					// ������+������: ���ò�͸����
					const HWND hWnd = this->GetSafeHwnd();
					const SHORT threshold = HIWORD(pMsg->wParam);
					
					IncreaseWindowAlpha(hWnd, threshold/WHEEL_DELTA*5);
					CString strMsg;
					const int alpha = (int)GetWindowAlpha(hWnd);
					strMsg.Format(_T("%s - ��ǰ�Ĳ�͸���� = %d/255(%.2f%%)"), s_title, alpha, (double)(alpha)*100/255 );
					SetWindowText(strMsg);
					SetTimer(IDTIMER_RESETTITLE, 1000, NULL);
					return TRUE;
				}
			}
		}
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}

void CFileListWindow::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default

	switch(nIDEvent)
	{
	case IDTIMER_RESETTITLE:
		SetWindowText(s_title);
		KillTimer(IDTIMER_RESETTITLE);
		break;
	}
	
	CDialog::OnTimer(nIDEvent);
}

LRESULT CFileListWindow::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (m_pWndMain->m_pWindowGluer->handleFollowerMessage(this->GetSafeHwnd(), message, wParam, lParam)) return 0;
	
	return CDialog::WindowProc(message, wParam, lParam);
}
