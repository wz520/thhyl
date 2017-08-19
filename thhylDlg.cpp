// thhylDlg.cpp : implementation file
//

#include "stdafx.h"
#include <commctrl.h>
#include <ObjBase.h>
#include "cfgfile.h"
#include "thhyl.h"
#include "thhylDlg.h"
#include "global.h"
#include "RPYAnalyzer.h"
#include "Registry.h"
#include "DlgOption.h"
#include "DlgComment.h"
#include "clipboard.h"
#include "globalxp.h"
#include "DlgAbout.h"

#include "CommonDlgWZ.h"
#include "SaveRawDlg.h"

#include "filepath_utils.h"

// 貌似 VC6 没有 SHAutoComplete 的声明，但是 SHLWAPI.LIB 里却有实现...

//
// Define API decoration for direct importing of DLL references.
//
#ifndef WINSHLWAPI
#if !defined(_SHLWAPI_)
#define LWSTDAPI          EXTERN_C DECLSPEC_IMPORT HRESULT STDAPICALLTYPE
#define LWSTDAPI_(type)   EXTERN_C DECLSPEC_IMPORT type STDAPICALLTYPE
#define LWSTDAPIV         EXTERN_C DECLSPEC_IMPORT HRESULT STDAPIVCALLTYPE
#define LWSTDAPIV_(type)  EXTERN_C DECLSPEC_IMPORT type STDAPIVCALLTYPE
#else
#define LWSTDAPI          STDAPI
#define LWSTDAPI_(type)   STDAPI_(type)
#define LWSTDAPIV         STDAPIV
#define LWSTDAPIV_(type)  STDAPIV_(type)
#endif
#endif // WINSHLWAPI

LWSTDAPI SHAutoComplete(HWND hwndEdit, DWORD dwFlags);

///////////////// END SHAutoComplete /////////////////////////////////////////


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define lengthof(arr) (sizeof(arr)/sizeof(arr[0]))
#define delete_then_null(v) ((delete (v)), (v)=NULL)
#define delete_array_then_null(v) ((delete[] (v)), (v)=NULL)

/////////////////////////////////////////////////////////////////////////////
// CThhylDlg dialog

CThhylDlg::CThhylDlg(CWnd* pParent /*=NULL*/)
	: CDlgBaseWZ(CThhylDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CThhylDlg)
	m_rpyfile = _T("");
	m_bOnTop = TRUE;
	m_bAutocomplete = TRUE;
	m_rpyinfo = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_filestatus.Clear();

	m_bTHX = FALSE;
	m_pRpyData = NULL;
	m_pRpyAnalyzer = NULL;
	m_dwRpySize = 0;
	m_bNeedReanalyze = TRUE;

	// Load Accelerator
	m_hAccel = ::LoadAccelerators(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_ACCEL)); 
	m_pWndFileList = new CFileListWindow(this);
}

void CThhylDlg::DoDataExchange(CDataExchange* pDX)
{
	CDlgBaseWZ::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CThhylDlg)
	DDX_Text(pDX, IDC_RPYFILE, m_rpyfile);
	DDX_Check(pDX, IDC_ONTOP, m_bOnTop);
	DDX_Check(pDX, IDC_AUTOCOMP, m_bAutocomplete);
	DDX_Text(pDX, IDC_RPYINFO, m_rpyinfo);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CThhylDlg, CDlgBaseWZ)
	//{{AFX_MSG_MAP(CThhylDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DROPFILES()
	ON_BN_CLICKED(IDC_ONTOP, OnOntop)
	ON_BN_CLICKED(IDC_AUTOCOMP, OnAutocomplete)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_COPY, OnCopy)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(IDM_OPEN, OnOpen)
	ON_COMMAND(IDM_MENUCOPY, OnMenucopy)
	ON_COMMAND(IDM_EXIT, OnExit)
	ON_COMMAND(IDM_SETASSOC, OnSetassoc)
	ON_COMMAND(IDM_DELASSOC, OnDelassoc)
	ON_COMMAND(IDM_OPENFOLDER, OnOpenfolder)
	ON_COMMAND(IDC_OPEN_SH_ALICE, OnOpenShAlice)
	ON_COMMAND(IDM_FILEMOVETO, OnFilemoveto)
	ON_COMMAND(IDM_FILECOPYTO, OnFilecopyto)
	ON_COMMAND(IDM_FILEDELETE, OnFiledelete)
	ON_WM_MBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_BN_CLICKED(IDC_OPTION, OnOption)
	ON_BN_CLICKED(IDC_CHGFONT, OnChgfont)
	ON_BN_CLICKED(IDC_EDITCOMMENT, OnEditcomment)
	ON_COMMAND(IDM_OPENPROP, OnOpenprop)
	ON_BN_CLICKED(IDC_HELPHTML, OnHelphtml)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_INITMENUPOPUP()
	ON_COMMAND(IDM_MENUHELP, OnMenuhelp)
	ON_COMMAND(IDM_SAVERAW, OnSaveraw)
	ON_COMMAND(IDM_SAVERAWPART, OnSaverawpart)
	ON_WM_ACTIVATE()
	ON_WM_ACTIVATEAPP()
	ON_BN_CLICKED(IDC_CLOSEFILE, OnClosefile)
	ON_BN_CLICKED(IDC_CUTFILE, OnCutfile)
	ON_BN_CLICKED(IDC_COPYFILE, OnCopyfile)
	ON_COMMAND(IDM_ABOUT, OnAbout)
	ON_WM_MENUSELECT()
	ON_BN_CLICKED(IDC_OPENFILELIST, OnOpenfilelist)
	ON_BN_CLICKED(IDC_PREVRPYFILE, OnPrevrpyfile)
	ON_BN_CLICKED(IDC_NEXTRPYFILE, OnNextrpyfile)
	ON_BN_CLICKED(IDC_RELOADFILELIST, OnReloadfilelist)
	ON_BN_CLICKED(IDC_FIRSTRPYFILE, OnFirstrpyfile)
	ON_BN_CLICKED(IDC_LASTRPYFILE, OnLastrpyfile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CThhylDlg message handlers

BOOL CThhylDlg::OnInitDialog()
{
	CDlgBaseWZ::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here

	//设置RPYAnalyzer框字体大小
	LoadEditCtrlFont((CEdit*)GetDlgItem(IDC_RPYINFO));

	// 初始化与控件关联的变量
	m_rpyfile = ((CThhylApp*)AfxGetApp())->m_rpyfile;
	m_bOnTop = ((CThhylApp*)AfxGetApp())->m_bOnTop;
	m_bAutocomplete = HasConfigOption(CFG_AUTOCOMP);

	CoInitialize(NULL);

	m_pWndFileList->Create(IDD_FILELIST, this);
	if (m_rpyfile.IsEmpty()) {
		m_rpyfile.LoadString(IDS_HINTNOFILE);
		m_rpyinfo.LoadString(IDS_HINTSTART);

		UpdateData(FALSE);

		// 没有文件打开，将焦点置于“录像文件”框，方便输入文件路径
		CEdit* const pRpyFileEdit = (CEdit*)GetDlgItem(IDC_RPYFILE);
		pRpyFileEdit->SetFocus();
		pRpyFileEdit->SetSel(0, -1); //全选
	}
	else {
		UpdateData(FALSE);
		Analyze(); // analyze() 将置焦点于 RPYAnalyzer 框
	}

	// 置顶
	if (m_bOnTop) OnOntop();
	if (m_bAutocomplete) OnAutocomplete();

	// Set window pos
	if (cfg.WinPlace.length == sizeof(WINDOWPLACEMENT)) {
		cfg.WinPlace.showCmd = AfxGetApp()->m_nCmdShow;
		SetWindowPlacement(&cfg.WinPlace);
	}
	
	// set button titles to graphical unicode characters
	((CButton*)GetDlgItem(IDC_CUTFILE))->SetWindowText(_T("\x2704"));
	((CButton*)GetDlgItem(IDC_COPYFILE))->SetWindowText(_T("\x2750"));
	((CButton*)GetDlgItem(IDC_RELOADFILELIST))->SetWindowText(_T("\x21bb"));

	// set font size for buttons
	SetControlFontSize( GetDlgItem(IDC_OPENFILELIST), 12, NULL, SCFS_KEEPOLDHFONT );
	SetControlFontSize( GetDlgItem(IDC_RELOADFILELIST), 14, NULL, SCFS_KEEPOLDHFONT );

	// 设置 tooltip 
	m_tooltip.AddTool( GetDlgItem(IDC_RPYFILE), _T("鼠标左键+滚轮可以调整本窗口的不透明度") );
	m_tooltip.AddTool( GetDlgItem(IDC_RPYINFO), _T("Ctrl+滚轮可以调整录像信息框的字体大小") );
	m_tooltip.AddTool( GetDlgItem(IDC_BROWSE), _T("打开录像文件[CTRL+B 或 CTRL+O]") );
	m_tooltip.AddTool( GetDlgItem(IDC_CLOSEFILE), _T("关闭录像文件[CTRL+L 或 CTRL+W]") );
	m_tooltip.AddTool( GetDlgItem(IDC_CUTFILE), _T("剪切[F9]") );
	m_tooltip.AddTool( GetDlgItem(IDC_COPYFILE), _T("复制[F10]") );
	m_tooltip.AddTool( GetDlgItem(IDC_CHGFONT), _T("更改录像信息框的字体") );
	m_tooltip.AddTool( GetDlgItem(IDC_COPY), _T("复制录像信息到剪贴板（若有选区则只复制选区内容，否则复制全部）") );
	m_tooltip.AddTool( GetDlgItem(IDC_EDITCOMMENT), _T("编辑录像注释（红魔乡和妖妖梦的录像不支持该功能）") );
	m_tooltip.AddTool( GetDlgItem(IDC_ONTOP), _T("让本窗口总在最前") );
	m_tooltip.AddTool( GetDlgItem(IDC_AUTOCOMP), _T("手动输入路径时是否显示自动完成的提示框") );
	m_tooltip.AddTool( GetDlgItem(IDC_OPENFILELIST), _T("切换文件列表的显示") );
	m_tooltip.AddTool( GetDlgItem(IDC_FIRSTRPYFILE), _T("打开第一个 *.rpy 文件") );
	m_tooltip.AddTool( GetDlgItem(IDC_PREVRPYFILE), _T("打开上一个 *.rpy 文件") );
	m_tooltip.AddTool( GetDlgItem(IDC_NEXTRPYFILE), _T("打开下一个 *.rpy 文件") );
	m_tooltip.AddTool( GetDlgItem(IDC_LASTRPYFILE), _T("打开最后一个 *.rpy 文件") );
	m_tooltip.AddTool( GetDlgItem(IDC_RELOADFILELIST), _T("刷新文件列表") );
	m_tooltip.AddTool( GetDlgItem(IDC_FIRSTRPYFILE), _T("打开第一个 *.rpy 文件") );
	m_tooltip.Activate(TRUE);

	// 设置不透明度
	SetWindowAlpha(this->GetSafeHwnd(), cfg.byteAlpha);

	// 设置 DlgBaseWZ 对话框选项
	m_dlgopt = DLGWZ_OPT_ENABLEMAXIMIZE;
	
	return FALSE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CThhylDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDlgBaseWZ::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CThhylDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

int CThhylDlg::UserIconedMsgBox(LPCTSTR text, UINT uIconID, UINT uType, LPCTSTR caption)
{
	MSGBOXPARAMS mbp = {0};

	// 由于使用 MB_USERICON 时 MB_ICON* 无效，也不会播放 MB_ICON* 的任何音效，
	// 所以先把 MB_ICON* 单独拿出来，用 MessageBeep() 播放音效
	MessageBeep(uType & MB_ICONMASK);

	if (caption == NULL) caption = g_title;

	mbp.cbSize      = sizeof(mbp);
	mbp.lpszCaption = caption;
	mbp.hwndOwner   = this->GetSafeHwnd();
	mbp.dwStyle     = MB_USERICON | (uType & ~MB_ICONMASK); // remove ICONMASK
	mbp.hInstance   = ::GetModuleHandle(NULL);
	mbp.lpszText    = text;
	mbp.lpszIcon    = uIconID == 1
		? MAKEINTRESOURCE(IDI_REIMU)
		: MAKEINTRESOURCE(IDR_MAINFRAME);
	return ::MessageBoxIndirect(&mbp);
}

void CThhylDlg::Analyze()
{
	DWORD dwRead;
	CString strInfo;
	CloseFile(TRUE);

	m_pRpyData = ReadRPYFile(m_rpyfile, strInfo, this->m_hWnd, &m_filestatus, dwRead);
	if (m_filestatus.IsValid()) {
		m_pWndFileList->ChangeFilePath(m_rpyfile);
	}
	if ( m_pRpyData == NULL ) { 
		if ( !strInfo.IsEmpty() ) {
			m_rpyinfo = strInfo;
			UpdateData(FALSE);
			CloseFile(TRUE);
		}
		else {
			CloseFile();
		}
		return;
	}

	m_dwRpySize = dwRead;

	//Start
	m_pRpyAnalyzer = new CRPYAnalyzer(m_pRpyData, dwRead, cfg.InfoCode);
	const int RPYAnalyzer_ret = m_pRpyAnalyzer->Analyze();

	if ( RPYAnalyzer_ret != RPYINFO_UNKNOWNFORMAT ) {
		m_rpyinfo = m_pRpyAnalyzer->GetFinalInfo();
		m_strCurrComment = m_pRpyAnalyzer->GetComment(cfg.CommentCode);
		if (!m_strCurrComment.IsEmpty()) {
			m_rpyinfo += CString(_T("\r\n注释：\r\n")) + m_strCurrComment;
		}

		if ( m_pRpyAnalyzer->m_header!=mgc6 && m_pRpyAnalyzer->m_header!=mgc7 ) {
			m_bTHX = TRUE;
		}
	}
	else {
		// decoding failed.
		CloseFile(TRUE);
		m_rpyinfo = LoadNotRpyString();
	}

	UpdateTitle();       // 标题栏显示文件名
	UpdateData(FALSE);
	
	// 打开成功，将焦点置于“录像信息”框，方便按 Delete 键删除文件。
	if (m_filestatus.IsValid()) {
		// 记住原先的前台窗口，因为 SetFocus 还会更改前台窗口，这样不方便在文件列表窗口中使用键盘选择文件
		CWnd* const pActiveWindow = CWnd::GetActiveWindow();
		CEdit* const pRpyInfoEdit=(CEdit*)GetDlgItem(IDC_RPYINFO);
		pRpyInfoEdit->SetFocus();
		if (pActiveWindow == m_pWndFileList) { // 只有在原先的前台窗口是文件列表窗口时才让它恢复到前台
			pActiveWindow->SetActiveWindow();
		}
	}
}

void CThhylDlg::CloseFile(BOOL bSilently)
{
	m_filestatus.Clear();
	m_bTHX = FALSE;
	delete_then_null(m_pRpyAnalyzer);
	delete_array_then_null(m_pRpyData);
	if (bSilently)
		return;

	m_rpyfile.LoadString(IDS_HINTNOFILE);
	m_rpyinfo.LoadString(IDS_HINTSTART);
	UpdateTitle();
	UpdateData(FALSE);
}

// 更新标题。若有文件打开，显示打开的文件名
void CThhylDlg::UpdateTitle()
{
	CString newtitle;

	if (m_filestatus.IsValid()) {
		filepath_utils::GetFilename(m_rpyfile, newtitle);
		newtitle += CString(_T(" - ")) + g_title;
	}
	else
		newtitle = g_title;
	SetWindowText(newtitle);
}

void CThhylDlg::OnDropFiles(HDROP hDropInfo) 
{
	// TODO: Add your message handler code here and/or call default
	const UINT nFileCount = ::DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, _MAX_PATH);
	LPTSTR const lpszFileName = new TCHAR[_MAX_PATH];

	for (UINT i=0; i<nFileCount; ++i) {
		::DragQueryFile(hDropInfo, i, lpszFileName,_MAX_PATH);

		if (i==0) { // 第1个文件
			UpdateData(TRUE);
			m_rpyfile = lpszFileName;
			ConvToFullPath();
			UpdateData(FALSE);
		} else //如果拖入了2个或2个以上的文件，则另开一个自身的实例来打开。
			SpawnInstance(lpszFileName);
	}
		
	::DragFinish(hDropInfo);
	delete []lpszFileName;
	
	if (nFileCount > 0) Analyze();
}

void CThhylDlg::OnOntop() 
{
	// TODO: Add your control notification handler code here
	UpdateData();
	
	SetWindowPos( (m_bOnTop) ? &wndTopMost : &wndNoTopMost,
		0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void CThhylDlg::OnAutocomplete() 
{
	// TODO: Add your control notification handler code here
	UpdateData();
	
	SHAutoComplete( ((CEdit*)GetDlgItem(IDC_RPYFILE))->GetSafeHwnd(),
		m_bAutocomplete ? 0x10000043 : 0x20000043 );
}


void CThhylDlg::OnBrowse() 
{
	// TODO: Add your control notification handler code here
	CString filter((LPCTSTR)IDS_DLGFILTER);
	
	CFileDialogWZ dlg(TRUE, NULL, NULL,
		OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT,
		filter, this);
	TCHAR filebuf[32768] = {0};

	dlg.m_ofn.lpstrFile  = filebuf;
	dlg.m_ofn.nMaxFile   = lengthof(filebuf);
	dlg.m_ofn.lpstrTitle = _T("打开录像文件");
	if (dlg.DoModal() == IDOK) {
		BOOL bFirstFile = TRUE;
		CString strFilePath;
		POSITION pos = dlg.GetStartPosition();

		while (pos) {
			strFilePath = dlg.GetNextPathName(pos);

			if (bFirstFile) {
				UpdateData(TRUE);
				m_rpyfile = strFilePath;
				UpdateData(FALSE);
				Analyze();
				bFirstFile = FALSE;
			}
			else {
				SpawnInstance(strFilePath);
			}
		}
	}
}


void CThhylDlg::OnClosefile() 
{
	// TODO: Add your control notification handler code here
	CloseFile();
}

void CThhylDlg::OnCopy() 
{
	// TODO: Add your control notification handler code here
	CWnd  *const  pOldFocus = this->GetFocus();
	CEdit *const  pEdit     = (CEdit*)GetDlgItem(IDC_RPYINFO);
	int startchar, endchar;

	pEdit->SetFocus();
	pEdit->GetSel(startchar, endchar);

	if (startchar == endchar) { //No selection
		pEdit->SetSel(0, -1, TRUE);
		pEdit->Copy();
		pEdit->SetSel(-1, 0, TRUE);
	}
	else {
		pEdit->Copy();
	}

	if (pOldFocus) pOldFocus->SetFocus();

	MessageBeep(MB_ICONINFORMATION);
}

void CThhylDlg::OnOK() 
{
	// TODO: Add extra validation here
	if (GetDlgItem(IDC_RPYFILE) != GetFocus())
		return;

	UpdateData(TRUE);
	ConvToFullPath();
	UpdateData(FALSE);
	Analyze();
}

void CThhylDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// TODO: Add your message handler code here
	const LPARAM lparam = (point.y << 16) | point.x;
	if( SendMessage(WM_NCHITTEST, 0, lparam) != HTCLIENT ) {
		// DefWindowProc(WM_CONTEXTMENU, (WPARAM)pWnd->GetSafeHwnd(), lparam);
		CDlgBaseWZ::OnContextMenu(pWnd, point);
		return;
	}

	ShowPopup(point);
}

void CThhylDlg::OnOpen() 
{
	// TODO: Add your command handler code here
	OnBrowse();	
}

void CThhylDlg::OnMenucopy() 
{
	// TODO: Add your command handler code here
	OnCopy();
}

void CThhylDlg::OnExit() 
{
	// TODO: Add your command handler code here
	CDlgBaseWZ::OnCancel();
}

void CThhylDlg::OnSetassoc() 
{
	// TODO: Add your command handler code here
	const int retmb = UserIconedMsgBox(
		_T("此操作会将本程序与后缀为 .rpy 的文件相关联。\n关联后只要双击 .rpy 文件即可使用本程序打开。\n\n")
		_T("按【是】将只为当前用户设置关联(无需管理员权限)；\n")
		_T("按【否】将为本机的所有用户设置关联(需要管理员权限)；\n")
		_T("按【取消】取消操作。\n\n若您不知道该如何选择，推荐选【是】。"),
		1, MB_ICONQUESTION | MB_YESNOCANCEL);

	if (retmb==IDYES || retmb==IDNO) {
		const CString Parameters = _T("-n");
		const BOOL bForUser = retmb==IDYES ? TRUE : FALSE;
		const LONG result = ::SetAssociation(_T(".rpy"),_T("touhou_rpy"),
			_T("东方STG录像文件"), g_exefullpath, Parameters, 1, bForUser);

		if (result == 0)
			MessageBox(_T("设置关联成功。"), g_title, MB_ICONINFORMATION);
		else {
			CString failedinfo;
			failedinfo.Format(_T("设置关联失败！\n\n可能的原因：%s"), GetErrorMessage(result));
			MessageBox( failedinfo, g_title, MB_ICONSTOP);
		}
	}
}

void CThhylDlg::OnDelassoc() 
{
	// TODO: Add your command handler code here
	const int retmb = UserIconedMsgBox(
		_T("此操作会将后缀为 .rpy 的文件恢复为无关联状态。\n\n")
		_T("按【是】将解除为当前用户设置的关联(无需管理员权限)；\n")
		_T("按【否】将解除为本机所有用户设置的关联(有可能需要管理员权限)；\n")
		_T("按【取消】取消操作。\n\n")
		_T("【提示】若要取消用 1.61 以前的版本设置的关联，请点击【否】。"),
		1, MB_ICONQUESTION | MB_YESNOCANCEL);
	if (retmb==IDYES || retmb==IDNO) {
		const BOOL bForUser = retmb==IDYES ? TRUE : FALSE;
		const LONG result = ::DeleteAssociation(_T(".rpy"), bForUser);
		if (result == 0)
			MessageBox(_T("解除关联成功。"), g_title, MB_ICONINFORMATION);
		else {
			CString failedinfo;
			failedinfo.Format(_T("解除关联失败！\n\n可能的原因：%s"), GetErrorMessage(result));
			MessageBox( failedinfo, g_title, MB_ICONSTOP);
		}
	}
}

void CThhylDlg::OnOpenfolder() 
{
	// TODO: Add your command handler code here
	if (!m_filestatus.IsValid())
		return;
	
	ShellExecute(NULL, _T("open"),  _T("Explorer.exe"),
		CString(_T(" /select,")) + m_rpyfile,
		NULL, SW_SHOWDEFAULT);
}

void CThhylDlg::OnOpenShAlice() 
{
	// TODO: Add your command handler code here
	TCHAR path[MAX_PATH];
	::ExpandEnvironmentStrings( _T("%APPDATA%\\ShanghaiAlice"), path, MAX_PATH);
	ShellExecute(NULL, _T("open"),  path, NULL, NULL, SW_SHOWDEFAULT);
}


void CThhylDlg::OnFilemoveto() 
{
	// TODO: Add your command handler code here
	CopyOrMoveRpy(_T("移动/重命名录像到..."), FALSE);
}

void CThhylDlg::OnFilecopyto() 
{
	// TODO: Add your command handler code here
	CopyOrMoveRpy(_T("复制录像到..."), TRUE);
}

void CThhylDlg::CopyOrMoveRpy(LPCTSTR DialogTitle, BOOL bCopy)
{
	if (!m_filestatus.IsValid())
		return;
	
	CString filter((LPCTSTR)IDS_DLGFILTER);
	
	CFileDialogWZ dlg(FALSE, _T("rpy"), m_rpyfile,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST,
		filter, this);
	dlg.m_ofn.lpstrTitle=DialogTitle;
	if (dlg.DoModal()==IDCANCEL)
		return;
	
	CString newfilename(dlg.GetPathName());
	
	BOOL result = bCopy
		? CopyFile(m_rpyfile, newfilename, FALSE)
		: MoveFileEx(m_rpyfile, newfilename, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);

	if (result) { //复制/移动成功
		//如果是移动，或者在选项中选中了“打开复制后的目标文件”
		if (!bCopy || HasConfigOption(CFG_COPYOPENDEST)) {
			m_rpyfile = newfilename;
			Analyze();
		}
	}
	else {
		LPCTSTR ErrorMsg;
		ErrorMsg = GetErrorMessage(GetLastError());
		MessageBox( ErrorMsg, DialogTitle, MB_ICONSTOP );
	}
}

void CThhylDlg::OnFiledelete() 
{
	// TODO: Add your command handler code here
	if (!m_filestatus.IsValid())
		return;

	int            ret;
	SHFILEOPSTRUCT fos = {0};

	const bool bPressed = IsKeyDown(VK_SHIFT);

	//Perform delete operation via Shell
	{
		const size_t len = m_rpyfile.GetLength()+2;
		LPTSTR rpyfile = new TCHAR[len];

		_tcscpy(rpyfile, m_rpyfile);
		rpyfile[len-1]=_T('\0');

		fos.hwnd   = this->m_hWnd;
		fos.wFunc  = FO_DELETE;
		fos.pFrom  = rpyfile;
		fos.fFlags = FOF_SILENT |
			(bPressed ? 0 : FOF_ALLOWUNDO) |
			(HasConfigOption(CFG_CONFIRM) ? 0 : FOF_NOCONFIRMATION);
		ret = SHFileOperation(&fos);

		delete []rpyfile;
	}

	if (ret==0 && !fos.fAnyOperationsAborted) { //删除成功
		if (HasConfigOption(CFG_AUTOEXIT))
			OnCancel();
		else
			CloseFile();
	}
}

void CThhylDlg::OnMButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	if( HasConfigOption(CFG_MBTN) )
		OnFiledelete();

	//CDlgBaseWZ::OnMButtonUp(nFlags, point);
}

void CThhylDlg::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if( HasConfigOption(CFG_DBLCLK) )
		OnExit();
	else
		EnableMaximize(m_hWnd, -1); // maximize or restore

	//CDlgBaseWZ::OnLButtonDblClk(nFlags, point);
}

void CThhylDlg::ConvToFullPath()
{
	TCHAR FullPath[MAX_PATH] = {0};
	GetFullPathName(m_rpyfile, MAX_PATH, FullPath, NULL);
	m_rpyfile = FullPath;
}

void CThhylDlg::OnOption() 
{
	// TODO: Add your control notification handler code here
	CDlgOption dlg;
	if (IDOK == dlg.DoModal() && m_filestatus.IsValid())
		Analyze();
}

void CThhylDlg::OnChgfont() 
{
	// TODO: Add your control notification handler code here
	LOGFONT  lf;
	CEdit   *pRpyInfoEdit = (CEdit *)GetDlgItem(IDC_RPYINFO);
	CFont   *pFont        = pRpyInfoEdit->GetFont();
	pFont->GetLogFont(&lf);
	
	CFontDialogWZ fontdlg(&lf);
	if (IDOK == fontdlg.DoModal()) {
		pFont->DeleteObject();
		SaveEditCtrlFont(pRpyInfoEdit, &lf);
	}
}

void CThhylDlg::OnEditcomment() 
{
	// TODO: Add your control notification handler code here
	if (!(m_bTHX && m_filestatus.IsValid())) {
		const HWND hRpyFile = ((CButton*)GetDlgItem(IDC_RPYFILE))->GetSafeHwnd();
		LPCTSTR const msgtitle = _T("无法编辑注释。可能的原因：");
		LPCTSTR const msg = _T("1. 红魔乡和妖妖梦的录像文件无法添加注释；\n")
							_T("2. 尚未打开任何文件或文件格式不支持。");
		const HICON icon = AfxGetApp()->LoadIcon(MAKEINTRESOURCE(IDR_MAINFRAME));

		if (!ShowBalloonMsg( hRpyFile, msg, msgtitle, TTI_WARNING, FALSE, icon)) {
			// 如果 balloon 不支持，则显示 msgbox
			UserIconedMsgBox(msg, 0, MB_ICONEXCLAMATION, msgtitle);
		}
	}
	else {
		CDlgComment dlg;
		dlg.m_pCommentData = &m_strCurrComment;
		dlg.m_filename = m_rpyfile;
		dlg.m_pRpyData = m_pRpyData;
		dlg.m_dwRpySize = m_dwRpySize;
		if (dlg.DoModal()==IDOK)
			Analyze();
		
		cfg.CommentCodeForEdit = dlg.m_CommentCodeEdit;
	}
}

void CThhylDlg::OnOpenprop() 
{
	// TODO: Add your command handler code here
	if (!m_filestatus.IsValid())
		return;

	SHELLEXECUTEINFO se;

	ZeroMemory(&se, sizeof(se));
	se.lpVerb = _T("properties");
	se.lpFile = m_rpyfile;
	se.fMask  = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_INVOKEIDLIST;
	se.cbSize = sizeof(se);
	::ShellExecuteEx(&se);
}

void CThhylDlg::OnMenuhelp() 
{
	// TODO: Add your command handler code here
	CString exedir, exefilename;
	filepath_utils::GetDir(g_exefullpath, exedir);
	exedir += _T("docs\\");
	filepath_utils::GetFilename(g_exefullpath, exefilename);
	filepath_utils::ChangeFileExtension(exefilename, _T("html"));
	CString exefullpath = exedir + exefilename;
	
	if ( (int)::ShellExecute(NULL, NULL, exefullpath, NULL, NULL, SW_SHOWDEFAULT) <= 32 )
		MessageBox(CString(_T("无法打开帮助文件:\n\n")) + exefullpath, g_title, 16);	
}

void CThhylDlg::OnHelphtml() 
{
	// TODO: Add your control notification handler code here
	
	// “菜单”按钮
	CWnd *pBtnHelp = GetDlgItem(IDC_HELPHTML);
	RECT rect;
	pBtnHelp->GetWindowRect(&rect);

	POINT point;
	point.x = rect.left;
	point.y = rect.bottom;

	ShowPopup(point);
}

void CThhylDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDlgBaseWZ::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	
	if ( IsWindow(this->GetSafeHwnd()) ) {
		struct tagresizeinfo_t {
			int ctrlID;
			int x;
			int y;
			int w;
			int h;
		} resize_info[] = {
			{IDC_STATIC_RPYFILE, 10, 10, cx-20, 50},
			{IDC_RPYFILE, 20, 30, cx-40, 20},
			{IDC_BROWSE, cx-150, 5, 30, 25},
			{IDC_CLOSEFILE, cx-120, 5, 30, 25},
			{IDC_CUTFILE, cx-80, 5, 30, 25},
			{IDC_COPYFILE, cx-50, 5, 30, 25},
			
			{IDC_STATIC_RPYINFO, 10, 70, cx-20, cy-110},
			{IDC_RPYINFO, 20, 90, cx-40, cy-140},
			{IDC_CHGFONT, cx-300, 60, 85, 25},
			{IDC_COPY, cx-210, 60, 80, 25},
			{IDC_EDITCOMMENT, cx-125, 60, 105, 25},
			
			{IDC_ONTOP, 10, cy-35, 80, 25},
			{IDC_OPTION, cx-285, cy-35, 85, 25},
			{IDC_HELPHTML, cx-190, cy-35, 85, 25},
			{IDCANCEL, cx-95, cy-35, 85, 25},
		};

		// 比起 MoveWindow，使用 DeferWindowPos 系列函数可以防止控件重绘后文字可能无法正常显示的问题。
		const HDWP hdwp = BeginDeferWindowPos(lengthof(resize_info));
		for (int i=0; i < lengthof(resize_info); ++i) {
			CWnd *pWnd = GetDlgItem(resize_info[i].ctrlID);
			DeferWindowPos(hdwp, pWnd->GetSafeHwnd(), this->GetSafeHwnd(),
				resize_info[i].x, resize_info[i].y, resize_info[i].w, resize_info[i].h, SWP_NOZORDER);
		}
		EndDeferWindowPos(hdwp);
	}
}

void CThhylDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
	// TODO: Add your message handler code here and/or call default
	
	lpMMI->ptMinTrackSize.x = 540;
	lpMMI->ptMinTrackSize.y = 240;
}

void CThhylDlg::SpawnInstance(LPCTSTR lpszFileName)
{
	CString szParam;

	szParam.Format(_T("\x22%s\x22 -n"), lpszFileName);
	ShellExecute(NULL, NULL, g_exefullpath, szParam, NULL, SW_SHOWDEFAULT);
}

void CThhylDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu) 
{
	CDlgBaseWZ::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
	
	// TODO: Add your message handler code here
	if (!bSysMenu && nIndex==0) {
		const UINT uEnable = m_filestatus.IsValid() ? MF_ENABLED : MF_GRAYED;
		pPopupMenu->EnableMenuItem( IDM_SAVERAW, uEnable);
		pPopupMenu->EnableMenuItem( IDM_SAVERAWPART, uEnable);
		pPopupMenu->EnableMenuItem( IDM_OPENFOLDER, uEnable);
		pPopupMenu->EnableMenuItem( IDM_FILECOPYTO, uEnable);
		pPopupMenu->EnableMenuItem( IDM_FILEMOVETO, uEnable);
		pPopupMenu->EnableMenuItem( IDM_FILEDELETE, uEnable);
		pPopupMenu->EnableMenuItem( IDM_OPENPROP, uEnable);

		// encoding
#define CHECKMENUITEM_BY_CP(idx,cp) (pPopupMenu->CheckMenuItem( IDM_CHANGEENCODESTART+idx, \
				cfg.CommentCode == cp && cfg.InfoCode == cp ? MF_CHECKED : MF_UNCHECKED))

		CHECKMENUITEM_BY_CP(0, 932);
		CHECKMENUITEM_BY_CP(1, 936);
		CHECKMENUITEM_BY_CP(2, 950);
		CHECKMENUITEM_BY_CP(3, 0);
#undef CHECKMENUITEM_BY_CP
	}
}


void CThhylDlg::ShowPopup(POINT point)
{
	CMenu menu;
	menu.LoadMenu(IDR_POPMENU);
	CMenu* pPopup = menu.GetSubMenu(0);
	
	pPopup->TrackPopupMenu( TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
}

BOOL CThhylDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	const WORD notifycode = HIWORD(wParam);
	const WORD id         = LOWORD(wParam);

	if (id == IDC_RPYFILE) { // “录像文件”编辑框
		if (notifycode == EN_KILLFOCUS) {
			// 焦点离开编辑框时不接受用户在编辑框中输入的路径，除非用户按了回车。
			UpdateData(FALSE);
			return TRUE;
		}
	}

	// menu commands and others
	switch(id)
	{
	case IDM_CHANGEENCODESTART+0: cfg.InfoCode = 932; break;
	case IDM_CHANGEENCODESTART+1: cfg.InfoCode = 936; break;
	case IDM_CHANGEENCODESTART+2: cfg.InfoCode = 950; break;
	case IDM_CHANGEENCODESTART+3: cfg.InfoCode = 0; break;
	default: return CDlgBaseWZ::OnCommand(wParam, lParam);
	}
	
	cfg.CommentCode = cfg.InfoCode;
	if(m_filestatus.IsValid())
		Analyze();
	return TRUE;
}

BOOL CThhylDlg::DestroyWindow() 
{
	// TODO: Add your specialized code here and/or call the base class

	// Save normal window place
	ZeroMemory(&cfg.WinPlace, sizeof(WINDOWPLACEMENT));
	cfg.WinPlace.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(&cfg.WinPlace);
	cfg.byteAlpha = GetWindowAlpha(this->GetSafeHwnd());
	cfg.set(CFG_AUTOCOMP, m_bAutocomplete);

	m_pWndFileList->DestroyWindow();
	delete_then_null(m_pWndFileList);

	CoUninitialize();
	
	return CDlgBaseWZ::DestroyWindow();
}

BOOL CThhylDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class

	if (m_hAccel) {
		if (::TranslateAccelerator(m_hWnd, m_hAccel, pMsg))
			return TRUE;

		// other keys
		switch (pMsg->message)
		{
		case WM_MOUSEWHEEL: {
				const WORD vKey = LOWORD(pMsg->wParam);

				if ( (vKey & MK_LBUTTON) && GetDlgItem(IDC_RPYINFO) != GetFocus() ) {
					// 鼠标左键+鼠标滚轮，且焦点不在“录像信息”框内: 设置不透明度
					const HWND hWnd = this->GetSafeHwnd();
					const HWND hEdit = ((CEdit*)GetDlgItem(IDC_RPYFILE))->GetSafeHwnd();
					const SHORT threshold = HIWORD(pMsg->wParam);

					IncreaseWindowAlpha(hWnd, threshold/WHEEL_DELTA*5);
					CString strMsg;
					const int alpha = (int)GetWindowAlpha(hWnd);
					strMsg.Format(_T("当前的不透明度 = %d/255(%.2f%%)"), alpha, (double)(alpha)*100/255 );
					ShowBalloonMsg(hEdit, strMsg, L"不透明度已更改", TTI_INFO, FALSE);
					return TRUE;
				}
				else if ( vKey & MK_CONTROL ) {
					// Ctrl+鼠标滚轮: 改变字体大小
					const HWND hEdit = ((CEdit*)GetDlgItem(IDC_RPYINFO))->GetSafeHwnd();
					const SHORT threshold = HIWORD(pMsg->wParam);

					LOGFONT newlf;
					SetControlFontSize(hEdit, threshold/WHEEL_DELTA, &newlf, TRUE);
					cfg.saveFont(&newlf);
					return TRUE;
				}
			}
			break;
		case WM_KEYDOWN:
			switch (pMsg->wParam)
			{
			case VK_DELETE:
				if (GetDlgItem(IDC_RPYFILE) != GetFocus()) { // 只在焦点不在“录像文件”编辑框时有效
					this->OnFiledelete();
					return TRUE;
				}
				break;
			case VK_F8: // F8 无上面 Delete 键的限制
				this->OnFiledelete();
				return TRUE;
			}
			break;

		}
	}

	return CDlgBaseWZ::PreTranslateMessage(pMsg);
}

void CThhylDlg::WinHelp(DWORD dwData, UINT nCmd) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	// 屏蔽MFC自带的F1响应
	// CDlgBaseWZ::WinHelp(dwData, nCmd);
}


void CThhylDlg::OnSaveraw()
{
	if (!m_filestatus.IsValid())
		return;

	CString filter((LPCTSTR)IDS_DLGFILTERALL);
	CString rawfile(m_rpyfile);
	filepath_utils::ChangeFileExtension(rawfile, _T("bin"));
	
	CFileDialogWZ dlg(FALSE, NULL, rawfile,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST,
		filter, this);
	
	DWORD rawsize = 0;
	m_pRpyAnalyzer->GetDecodedDataPointer(&rawsize);
	CString title;
	title.Format(_T("保存原始数据(原始数据大小: %d, rpy 文件大小: %d)"), (int)rawsize, (int)m_dwRpySize);
	dlg.m_ofn.lpstrTitle = title;
	if (dlg.DoModal() == IDOK) {
		// dump
		if (!m_pRpyAnalyzer->DumpRPYData( dlg.GetPathName() ))
			// dumping is not supported?
			// for current version, this should not happen.
			MessageBox(_T("ERR01"), g_title, MB_ICONEXCLAMATION);
	}
}

void CThhylDlg::OnSaverawpart() 
{
	// TODO: Add your command handler code here
	if (!m_filestatus.IsValid())
		return;
	
	CSaveRawDlg dlg;
	RPYINFO_POINTERS pointers;
	m_pRpyAnalyzer->GetResultPointers(&pointers);
	if (pointers.pRPYInfo2){
		dlg.m_pRpyInfoBase = pointers.pRPYInfo2;
	}
	else
		dlg.m_pRpyInfoBase = pointers.pRPYInfo1;
	dlg.m_pRpyInfoPointers = &pointers;
	dlg.m_dwRpySize = m_dwRpySize;
	dlg.m_strFileName = m_rpyfile;
	
	dlg.DoModal();
	
}

void CThhylDlg::OnActivateApp(BOOL bActive, HTASK hTask) 
{
	CDlgBaseWZ::OnActivateApp(bActive, hTask);
	
	// TODO: Add your message handler code here
	// TRACE1("thhylDlg OnActivateApp %d", bActive);
	if (bActive) m_bNeedReanalyze = TRUE;
}

void CThhylDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CDlgBaseWZ::OnActivate(nState, pWndOther, bMinimized);
	
	// TODO: Add your message handler code here
	// TRACE1("thhylDlg OnActivate %d", nState == WA_ACTIVE || nState == WA_CLICKACTIVE);
	if (nState == WA_ACTIVE || nState == WA_CLICKACTIVE) {
		if (m_bNeedReanalyze && m_filestatus.IsValid()) {
			CFileStatusWZ newfilestatus;

			m_bNeedReanalyze = FALSE;
			if (CFile::GetStatus(m_filestatus.m_szFullName, newfilestatus)) {
				// file still exists, Re-analyze if the file has been modified
				if ( !newfilestatus.IsSameDateAndSize(m_filestatus) ) {
					Analyze();
				}
				m_filestatus.Update(newfilestatus);
			}
			else {
				// file does not exist or cannot be opened, close it.
				if (HasConfigOption(CFG_AUTOEXIT))
					OnCancel();
				else
					CloseFile();
			}
		}
	}
}




void CThhylDlg::CopyOrCutFile(BOOL bCopy)
{
	LPWSTR const msg      = bCopy ? L"文件已复制到剪贴板" : L"文件已剪切到剪贴板";
	LPWSTR const msgerr   = bCopy ? L"无法复制文件到剪贴板" : L"无法剪切文件到剪贴板";
	LPWSTR const msgtitle = bCopy ? L"无法复制" : L"无法剪切";
	const HWND hRpyFile = ((CEdit*)GetDlgItem(IDC_RPYFILE))->GetSafeHwnd();

	if (!m_filestatus.IsValid()) {
		ShowBalloonMsg( hRpyFile, L"尚未打开录像文件或文件格式不支持", msgtitle, TTI_WARNING);
	}
	else {
		if (CopyFileToClipboardW(m_rpyfile, bCopy))
			ShowBalloonMsg( hRpyFile, msg, L"", TTI_NONE);
		else
			ShowBalloonMsg( hRpyFile, msgerr, L"错误", TTI_ERROR);
	}
}

void CThhylDlg::OnCutfile() 
{
	// TODO: Add your control notification handler code here

	CopyOrCutFile(FALSE);
}

void CThhylDlg::OnCopyfile() 
{
	// TODO: Add your control notification handler code here
	CopyOrCutFile(TRUE);
}

void CThhylDlg::OnAbout() 
{
	// TODO: Add your command handler code here
	static CDlgAbout* pDlg = NULL;

	if ( !pDlg->GetSafeHwnd() ) {
		if (pDlg == NULL)
			pDlg = new CDlgAbout(this);
		pDlg->Create(IDD_ABOUT, this);
	}
	pDlg->ShowWindow(SW_SHOW);
	pDlg->SetActiveWindow();
}




// 保存，载入 EDIT 控件的选区和垂直滚动条位置
static class CEditControlInfo {
public:
	int sel_start;
	int sel_end;
	int scrollpos;
	explicit CEditControlInfo(HWND hEdit) : sel_start(0), sel_end(0), scrollpos(0) {
		m_hEdit = hEdit;
	}
	void save() {
		SendMessage(m_hEdit, EM_GETSEL, (WPARAM)&sel_start, (LPARAM)&sel_end);
		scrollpos = GetScrollPos(m_hEdit, SB_VERT);
	}
	void load() {
		SendMessage(m_hEdit, WM_VSCROLL, (scrollpos << 16) | SB_THUMBPOSITION, 0);
		SendMessage(m_hEdit, EM_SETSEL, sel_start, sel_end);	
	}
private:
	HWND m_hEdit;
}*s_pEditCtrlInfo = NULL;

void CThhylDlg::OnEnterMenuLoop(BOOL bIsTrackPopupMenu)
{
	if (bIsTrackPopupMenu) {
#define LISTITEMSIGN	_T("\x2713 ")
		static LPCTSTR shenmegui[] = {
			LISTITEMSIGN _T("东方LL化不可避，junkojunkojun~ \x270c"),
			LISTITEMSIGN _T("车万车万车～ \x270c\x2620\x270c"),
			LISTITEMSIGN _T("东舰共荣不可避，就缺岛风当自机"),
			LISTITEMSIGN _T("百糟必有一发 poi"),
			LISTITEMSIGN _T("POI...POI? POI! POIPOIPOI~"),
			LISTITEMSIGN _T("【系统提示】：灵梦的节操严重不足。为保证您能继续使用灵梦，请您尽快通过官方游戏、官方漫画、及(哔~)等方式充值。"),
			LISTITEMSIGN _T("嘘~你们看，我发现了什么？这里有一只落单的赤蛮奇，我们可以尝试捕捉她。一只赤蛮奇可以为我们提供好几天的能量，她们富含大量的发气，不过赤蛮奇可不好对付。我们从后面慢慢接近她，小心别发出任何声音……嘿，我抓到了，她挣扎得很厉害！我们把她的头拧下来，其余的部位可以生吃，她的发气含量是触手的6倍，当然，如果时间不紧迫，我们可以先prpr，那样会更美味……嗯，她们的口感嘎嘣脆，味道就像小碎骨一样。"),
			LISTITEMSIGN _T("GZZLLNMNBNS？是在下输了"),
			LISTITEMSIGN _T("CAUTION: SJF is watching you!"),
			LISTITEMSIGN _T("看录像(DELETE)，多练习(THE)，相信自己(GAME)"),
			LISTITEMSIGN _T("偷猴普若捷克特 普瑞曾特得 掰 贼有恩"),
			LISTITEMSIGN _T("亲民船……真亲民？"),
			LISTITEMSIGN _T("曾经有一只萌萌的西瓜摆在我的面前，我没有吃掉她……"),
			LISTITEMSIGN _T("其实第一次听说要干猪的时候我是拒绝的……\r\n而现在，我专注干猪30年 \x270c"),
			LISTITEMSIGN _T("诸君，我喜欢干猪！"),
			LISTITEMSIGN _T("其实世上本没有幻想乡，入得事物多了，也便成了幻想乡"),
			LISTITEMSIGN _T("你是我的小呀小猪猡，怎么干你都不嫌多\x266a"),
			LISTITEMSIGN _T("宁做炸B，也不抱B"),
			LISTITEMSIGN _T("不在沉默中放B，就在沉默中MISS"),
			LISTITEMSIGN _T("燃～燃～入～"),
			LISTITEMSIGN _T("B还是要放的，万一撞了呢？"),
			LISTITEMSIGN _T("听说在贵圈17岁的叫BBA，500岁的叫萝莉？我读书少你们不要骗我啊！"),
			LISTITEMSIGN _T("BBA我喜欢你啊！"),
			LISTITEMSIGN _T("大丈夫萌大奶，早苗本买买买！"),
			LISTITEMSIGN _T("俺の郷長がこんなに可愛いわけがない！"),
			LISTITEMSIGN _T("zun://www.幻想郷.com/魔法の森/森近霖之助×雲山.avi"),
			LISTITEMSIGN _T("红魔乡可以玩，妖妖梦开着（姐姐）玩，永夜抄随便玩，风神录撞着玩，地灵殿擦着玩，星莲船不能玩，神灵庙Ｃ着玩，辉针城炸着玩，绀珠传逗你玩"),
			LISTITEMSIGN _T("\x0425\x043e\x0440\x043e\x0448\x043e"), // Xopowo
			LISTITEMSIGN _T("2un你头伸过来，我给你加个buff"),
			LISTITEMSIGN _T("从沙包到触手需要几步？\r\n\r\n答：第一步，糟；第二步，发；第三步，成为触手"),
			LISTITEMSIGN _T("米国妖精prpr"),
			LISTITEMSIGN _T("这……这个菜单才不是为了你而打开的呢，不要搞错了啊///")
		};
#undef LISTITEMSIGN

		if ( s_pEditCtrlInfo == NULL ) {
			s_pEditCtrlInfo = new CEditControlInfo(((CEdit*)(GetDlgItem(IDC_RPYINFO)))->GetSafeHwnd());
		}
		s_pEditCtrlInfo->save();
		const DWORD i = simple_random() % lengthof(shenmegui);
		((CEdit*)(GetDlgItem(IDC_RPYINFO)))->SetWindowText(shenmegui[i]);
	}
}

void CThhylDlg::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu) 
{
	CDlgBaseWZ::OnMenuSelect(nItemID, nFlags, hSysMenu);
	
	// TODO: Add your message handler code here

	TRACE3("load: nItemID=%u, nFlags=%X, hSYsMenu=%X", nItemID, nFlags, hSysMenu);
	if (!(nFlags & (MF_SYSMENU | MF_POPUP))) {
		CString menuhelp;
		
		if (nItemID == 0) {
			menuhelp = _T("o( =·ω·= )m 这是什么都没有的分割线喵～");
		}
		else if (nFlags & MF_GRAYED) {
			menuhelp = _T("o( =·ω·= )m 你需要先打开一个录像文件才可以点喵~");
		}
		else if (!menuhelp.LoadString(nItemID)) {
			menuhelp = _T("o( =·ω·= )m 喵～");
		}
		((CEdit*)(GetDlgItem(IDC_RPYINFO)))->SetWindowText(menuhelp);
	}
}


void CThhylDlg::OnExitMenuLoop(BOOL bIsTrackPopupMenu )
{
	if (bIsTrackPopupMenu) {
		UpdateData(FALSE);
		s_pEditCtrlInfo->load();
		delete_then_null(s_pEditCtrlInfo);
	}
}

void CThhylDlg::OnOpenfilelist() 
{
	// TODO: Add your control notification handler code here
	// toggle file list
	if (m_pWndFileList->IsWindowVisible()) {
		m_pWndFileList->ShowWindow(SW_HIDE);
	}
	else {
		m_pWndFileList->ShowWindow(SW_SHOW);
		m_pWndFileList->SetActiveWindow();
	}
}

void CThhylDlg::OnPrevrpyfile() 
{
	// TODO: Add your control notification handler code here
	CString filepath = m_pWndFileList->GetPreviousFilePath();
	if (!filepath.IsEmpty()) {
		m_rpyfile = filepath;
		Analyze();
	}
	else {
		ShowBalloonMsg( ((CButton*)GetDlgItem(IDC_RPYFILE))->GetSafeHwnd(), L"O_O", L"前面没有录像了", TTI_WARNING, FALSE);
	}
}

void CThhylDlg::OnNextrpyfile() 
{
	// TODO: Add your control notification handler code here
	CString filepath = m_pWndFileList->GetNextFilePath();
	if (!filepath.IsEmpty()) {
		m_rpyfile = filepath;
		Analyze();
	}
	else {
		ShowBalloonMsg( ((CButton*)GetDlgItem(IDC_RPYFILE))->GetSafeHwnd(), L"O_O", L"后面没有录像了", TTI_WARNING, FALSE);
	}
}

void CThhylDlg::OnReloadfilelist() 
{
	// TODO: Add your control notification handler code here
	m_pWndFileList->Refresh();
}

void CThhylDlg::OnFirstrpyfile() 
{
	// TODO: Add your control notification handler code here
	CString filepath = m_pWndFileList->GetFirstFilePath();
	if (!filepath.IsEmpty()) {
		m_rpyfile = filepath;
		Analyze();
	}
	else {
		ShowBalloonMsg( ((CButton*)GetDlgItem(IDC_RPYFILE))->GetSafeHwnd(), L">_<b", L"再怎么找也没有啦", TTI_WARNING, FALSE);
	}	
}

void CThhylDlg::OnLastrpyfile() 
{
	// TODO: Add your control notification handler code here
	CString filepath = m_pWndFileList->GetLastFilePath();
	if (!filepath.IsEmpty()) {
		m_rpyfile = filepath;
		Analyze();
	}
	else {
		ShowBalloonMsg( ((CButton*)GetDlgItem(IDC_RPYFILE))->GetSafeHwnd(), L">_<b", L"再怎么找也没有啦", TTI_WARNING, FALSE);
	}		
}
