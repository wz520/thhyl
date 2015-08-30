// DlgComment.cpp : implementation file
//

#include "stdafx.h"
#include "cfgfile.h"
#include "thhyl.h"
#include "global.h"
#include "DlgComment.h"
#include "cpconv.h"

#include "thuserblock.h"
#include "CommonDlgWZ.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgComment dialog


CDlgComment::CDlgComment(CWnd* pParent /*=NULL*/)
	: CDlgBaseWZ(CDlgComment::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgComment)
	m_CommentCodeEdit = cfg.CommentCodeForEdit;
	//}}AFX_DATA_INIT

	m_pCommentData=NULL;
	m_pRpyData = NULL;
	m_dwRpySize = 0;
}


void CDlgComment::DoDataExchange(CDataExchange* pDX)
{
	CDlgBaseWZ::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgComment)
	DDX_Control(pDX, IDC_COMMENTCODE, m_CommentCode);
	DDX_Control(pDX, IDC_COMMENTEDIT, m_CommentEdit);
	DDX_Text(pDX, IDC_COMMENTCODEEDIT, m_CommentCodeEdit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgComment, CDlgBaseWZ)
	//{{AFX_MSG_MAP(CDlgComment)
	ON_WM_SIZE()
	ON_CBN_SELCHANGE(IDC_COMMENTCODE, OnSelchangeCommentcode)
	ON_BN_CLICKED(IDC_SAVEAS, OnSaveas)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgComment message handlers

BOOL CDlgComment::OnInitDialog() 
{
	CDlgBaseWZ::OnInitDialog();
	
	// TODO: Add extra initialization here

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon( AfxGetApp()->LoadIcon(IDR_MAINFRAME), TRUE);		// Set big icon
	SetIcon( AfxGetApp()->LoadIcon(IDR_MAINFRAME), FALSE);		// Set small icon

	m_tooltip.AddTool( GetDlgItem(IDOK), _T("将新注释写回到原录像文件中，并更新主窗口中显示的录像信息") );
	m_tooltip.AddTool( GetDlgItem(IDCANCEL), _T("放弃更改，关闭本窗口") );
	m_tooltip.AddTool( GetDlgItem(IDC_SAVEAS), _T("将更新注释后的录像另存为一个新文件") );

	ComboAddString(m_CommentCode, m_CommentCodeEdit);

	LoadEditCtrlFont(&m_CommentEdit);
	m_CommentEdit.SetWindowText((LPCTSTR)(*m_pCommentData));

	m_CommentEdit.SetSel(0, -1, TRUE);
	m_CommentEdit.SetFocus();

	m_dlgopt = DLGWZ_OPT_ENABLEMAXIMIZE;
	
	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgComment::OnSize(UINT nType, int cx, int cy) 
{
	CDlgBaseWZ::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here

	if(IsWindow(m_CommentEdit.GetSafeHwnd())) {
		m_CommentEdit.MoveWindow(10, 100, cx-20, cy-110);
	}
}

void CDlgComment::OnSelchangeCommentcode() 
{
	// TODO: Add your control notification handler code here
	ComboSel2Edit(m_CommentCode, m_CommentCodeEdit);
	UpdateData(FALSE);
}

BOOL CDlgComment::UpdateCommentToFile(
		LPCSTR pComment,       // 指向注释文本的指针(ANSI)
		size_t dwCommentSize,  // 注释文本大小
		LPCTSTR newfilename    // 要更新注释的 RPY 文件名
)
{
	// 为新注释准备 user block 对象
	RPYUserBlock ubComment((const BYTE*)pComment, dwCommentSize, ubid_comment);
	
	// 将 user block 对象保存到 user block manager 中
	RPYUserBlockMgr ubm(m_pRpyData, m_dwRpySize);
	const int nCommentID = ubm.getCommentIndex();
	if (nCommentID >= 0) {
		ubm.remove(nCommentID); // remove original comment
	}
	if (!ubm.append(&ubComment)) {
		MessageBox(
			_T("无法创建新注释。\r\n\r\n这种事不应该发生，也许是个 BUG……\r\n如果方便的话，请将该 rpy 文件提交给作者。m(_ _)m"),
			g_title, MB_ICONSTOP);
		return FALSE;
	}
	
	// 保存到新的 buffer 中
	size_t newrpysize = 0;
	BYTE* pNewRpyData = ubm.saveToRPY(m_pRpyData, m_dwRpySize, &newrpysize);
	BOOL ret = FALSE;

	// Write to new file
	CFile cfFile;
	if (!cfFile.Open(newfilename, CFile::modeCreate | CFile::modeWrite |
		CFile::shareExclusive | CFile::typeBinary))
	{
		MessageBox(CString(_T("打开文件失败：\n\n")) + newfilename, g_title, MB_ICONSTOP);		
		// 1.70 版这里直接 return 了，造成了内存泄露……
	}
	else {
		cfFile.Write(pNewRpyData, newrpysize);
		cfFile.Close();
		ret = TRUE;
	}

	free(pNewRpyData);
	return ret;
}

BOOL CDlgComment::WriteComment(LPCTSTR infile, LPCTSTR outfile)
{
	BOOL result = FALSE;
	ASSERT(m_pRpyData != NULL && m_dwRpySize != 0);

	// 获取文本框内的注释内容
	CString strComment;
	m_CommentEdit.GetWindowText(strComment);

	// 将注释文本转成 ANSI
	BOOL bUsedDefaultChar;
	size_t lenCommentA;
	LPSTR lpszCommentA = Unicode2Ansi(strComment, m_CommentCodeEdit, &lenCommentA, &bUsedDefaultChar);
	if (lpszCommentA != NULL) { // 转换成ANSI成功
		int retmb = IDYES;
		if (bUsedDefaultChar) { // 如果有字符无法转换
			retmb = MessageBox(_T("部分字符无法转换。\r\n这可能是编码设置不当引起的。\r\n\r\n是否仍要继续？（注意：无法转换的字符将丢失！）"), g_title, MB_YESNO | MB_ICONEXCLAMATION);
		}
		if (retmb == IDYES) {
			// 一切妥当，写入到新文件中
			if ( UpdateCommentToFile(lpszCommentA, lenCommentA, outfile) )
				result = TRUE;
		}

		delete []lpszCommentA;
	}
	else { // 转换成ANSI失败
		LPCTSTR errmsg = ::GetErrorMessage(GetLastError());
		CString strerr((LPCTSTR)IDS_CONVTOANSIERROR);
		strerr += errmsg;
		
		MessageBox(strerr, g_title, MB_ICONSTOP);
	}
	
	return result;
}


void CDlgComment::OnOK() 
{
	// TODO: Add extra validation here
	
	if(!UpdateData(TRUE))
		return;

	if(!WriteComment(m_filename, m_filename))
		return;

	CDlgBaseWZ::OnOK();
}

void CDlgComment::OnSaveas()
{
	// TODO: Add your control notification handler code here
	if (!UpdateData(TRUE))
		return;

	CString filter((LPCTSTR)IDS_DLGFILTER), newfilename;

	CFileDialogWZ dlg(FALSE, _T("rpy"), m_filename, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter, this);
	if (dlg.DoModal() == IDOK) {
		newfilename = dlg.GetPathName();
		WriteComment(m_filename, newfilename);
	}
}
