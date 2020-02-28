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
#include "indep/Registry.h"
#include "DlgOption.h"
#include "DlgComment.h"
#include "indep/clipboard.h"
#include "indep/globalxp.h"
#include "DlgAbout.h"

#include "CommonDlgWZ.h"
#include "SaveRawDlg.h"

#include "filepath_utils.h"

// ò�� VC6 û�� SHAutoComplete ������������ SHLWAPI.LIB ��ȴ��ʵ��...

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

#define lengthof(arr)					(sizeof(arr)/sizeof(arr[0]))

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
	m_pWindowGluer = NULL;

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
	ON_COMMAND(IDM_RESETFILELISTPOS, OnResetfilelistpos)
	ON_WM_ACTIVATE()
	ON_WM_ACTIVATEAPP()
	ON_BN_CLICKED(IDC_CLOSEFILE, OnClosefile)
	ON_BN_CLICKED(IDC_CUTFILE, OnCutfile)
	ON_BN_CLICKED(IDC_COPYFILE, OnCopyfile)
	ON_COMMAND(IDM_ABOUT, OnAbout)
	ON_WM_MENUSELECT()
	ON_WM_EXITMENULOOP()
	ON_WM_ENTERMENULOOP()
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

	//����RPYAnalyzer�������С
	LoadEditCtrlFont((CEdit*)GetDlgItem(IDC_RPYINFO));

	// ��ʼ����ؼ������ı���
	m_rpyfile = ((CThhylApp*)AfxGetApp())->m_rpyfile;
	m_bOnTop = ((CThhylApp*)AfxGetApp())->m_bOnTop;
	m_bAutocomplete = HasConfigOption(CFG_AUTOCOMP);

	CoInitialize(NULL);

	m_pWindowGluer = new CWindowGluer(this->GetSafeHwnd(), 1);
	m_pWndFileList->Create(IDD_FILELIST, this);
	if (m_rpyfile.IsEmpty()) {
		m_rpyfile.LoadString(IDS_HINTNOFILE);
		m_rpyinfo.LoadString(IDS_HINTSTART);

		UpdateData(FALSE);

		// û���ļ��򿪣����������ڡ�¼���ļ����򣬷��������ļ�·��
		CEdit* const pRpyFileEdit = (CEdit*)GetDlgItem(IDC_RPYFILE);
		pRpyFileEdit->SetFocus();
		pRpyFileEdit->SetSel(0, -1); //ȫѡ
	}
	else {
		UpdateData(FALSE);
		Analyze(); // analyze() ���ý����� RPYAnalyzer ��
	}

	// �ö�
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

	// ���� tooltip 
	m_tooltip.AddTool( GetDlgItem(IDC_RPYFILE), _T("������+���ֿ��Ե��������ڵĲ�͸����") );
	m_tooltip.AddTool( GetDlgItem(IDC_RPYINFO), _T("Ctrl+���ֿ��Ե���¼����Ϣ��������С") );
	m_tooltip.AddTool( GetDlgItem(IDC_BROWSE), _T("��¼���ļ�[CTRL+O]") );
	m_tooltip.AddTool( GetDlgItem(IDC_CLOSEFILE), _T("�ر�¼���ļ�[CTRL+W]") );
	m_tooltip.AddTool( GetDlgItem(IDC_CUTFILE), _T("����[F9]") );
	m_tooltip.AddTool( GetDlgItem(IDC_COPYFILE), _T("����[F10]") );
	m_tooltip.AddTool( GetDlgItem(IDC_CHGFONT), _T("����¼����Ϣ�������") );
	m_tooltip.AddTool( GetDlgItem(IDC_COPY), _T("����¼����Ϣ�������壨����ѡ����ֻ����ѡ�����ݣ�������ȫ����") );
	m_tooltip.AddTool( GetDlgItem(IDC_EDITCOMMENT), _T("�༭¼��ע�ͣ���ħ��������ε�¼��֧�ָù��ܣ�") );
	m_tooltip.AddTool( GetDlgItem(IDC_ONTOP), _T("�ñ�����������ǰ") );
	m_tooltip.AddTool( GetDlgItem(IDC_AUTOCOMP), _T("�ֶ�����·��ʱ�Ƿ���ʾ�Զ���ɵ���ʾ��") );
	// --- �ļ��б�ť�� tooltip
	UINT res_ids[] = {IDC_OPENFILELIST, IDC_FIRSTRPYFILE, IDC_PREVRPYFILE, IDC_NEXTRPYFILE, IDC_LASTRPYFILE, IDC_RELOADFILELIST};
	for (int i = 0; i<lengthof(res_ids); ++i) {
		const UINT id = res_ids[i];
		CString strID;
		strID.LoadString(id);
		m_tooltip.AddTool( GetDlgItem(id), strID );
	}
	m_tooltip.Activate(TRUE);

	// ���ò�͸����
	SetWindowAlpha(this->GetSafeHwnd(), cfg.byteAlpha);

	// ���� DlgBaseWZ �Ի���ѡ��
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

	// ����ʹ�� MB_USERICON ʱ MB_ICON* ��Ч��Ҳ���Ქ�� MB_ICON* ���κ���Ч��
	// �����Ȱ� MB_ICON* �����ó������� MessageBeep() ������Ч
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

void CThhylDlg::Analyze(BOOL bMustRefreshFileList)
{
	DWORD dwRead;
	CString strInfo;
	CloseFile(TRUE);

	m_pRpyData = ReadRPYFile(m_rpyfile, strInfo, this->m_hWnd, &m_filestatus, dwRead);
	if (m_filestatus.IsValid()) {
		m_pWndFileList->ChangeFilePath(m_rpyfile, bMustRefreshFileList);
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
			m_rpyinfo += CString(_T("\r\nע�ͣ�\r\n")) + m_strCurrComment;
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

	UpdateTitle();       // ��������ʾ�ļ���
	UpdateData(FALSE);
	
	// �򿪳ɹ������������ڡ�¼����Ϣ���򣬷��㰴 Delete ��ɾ���ļ���
	if (m_filestatus.IsValid()) {
		// ��סԭ�ȵ�ǰ̨���ڣ���Ϊ SetFocus �������ǰ̨���ڣ��������������ļ��б�����ʹ�ü���ѡ���ļ�
		CWnd* const pActiveWindow = CWnd::GetActiveWindow();
		CEdit* const pRpyInfoEdit=(CEdit*)GetDlgItem(IDC_RPYINFO);
		pRpyInfoEdit->SetFocus();
		if (pActiveWindow == m_pWndFileList) { // ֻ����ԭ�ȵ�ǰ̨�������ļ��б���ʱ�������ָ���ǰ̨
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

// ���±��⡣�����ļ��򿪣���ʾ�򿪵��ļ���
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

		if (i==0) { // ��1���ļ�
			UpdateData(TRUE);
			m_rpyfile = lpszFileName;
			ConvToFullPath();
			UpdateData(FALSE);
		} else //���������2����2�����ϵ��ļ�������һ�������ʵ�����򿪡�
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
	dlg.m_ofn.lpstrTitle = _T("��¼���ļ�");
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
		_T("�˲����Ὣ���������׺Ϊ .rpy ���ļ��������\n������ֻҪ˫�� .rpy �ļ�����ʹ�ñ�����򿪡�\n\n")
		_T("�����ǡ���ֻΪ��ǰ�û����ù���(�������ԱȨ��)��\n")
		_T("�����񡿽�Ϊ�����������û����ù���(��Ҫ����ԱȨ��)��\n")
		_T("����ȡ����ȡ��������\n\n������֪�������ѡ���Ƽ�ѡ���ǡ���"),
		1, MB_ICONQUESTION | MB_YESNOCANCEL);

	if (retmb==IDYES || retmb==IDNO) {
		const CString Parameters = _T("-n");
		const BOOL bForUser = retmb==IDYES ? TRUE : FALSE;
		const LONG result = ::SetAssociation(_T(".rpy"),_T("touhou_rpy"),
			_T("����STG¼���ļ�"), g_exefullpath, Parameters, 1, bForUser);

		if (result == 0)
			MessageBox(_T("���ù����ɹ���"), g_title, MB_ICONINFORMATION);
		else {
			CString failedinfo;
			failedinfo.Format(_T("���ù���ʧ�ܣ�\n\n���ܵ�ԭ��%s"), GetErrorMessage(result));
			MessageBox( failedinfo, g_title, MB_ICONSTOP);
		}
	}
}

void CThhylDlg::OnDelassoc() 
{
	// TODO: Add your command handler code here
	const int retmb = UserIconedMsgBox(
		_T("�˲����Ὣ��׺Ϊ .rpy ���ļ��ָ�Ϊ�޹���״̬��\n\n")
		_T("�����ǡ������Ϊ��ǰ�û����õĹ���(�������ԱȨ��)��\n")
		_T("�����񡿽����Ϊ���������û����õĹ���(�п�����Ҫ����ԱȨ��)��\n")
		_T("����ȡ����ȡ��������\n\n")
		_T("����ʾ����Ҫȡ���� 1.61 ��ǰ�İ汾���õĹ������������񡿡�"),
		1, MB_ICONQUESTION | MB_YESNOCANCEL);
	if (retmb==IDYES || retmb==IDNO) {
		const BOOL bForUser = retmb==IDYES ? TRUE : FALSE;
		const LONG result = ::DeleteAssociation(_T(".rpy"), bForUser);
		if (result == 0)
			MessageBox(_T("��������ɹ���"), g_title, MB_ICONINFORMATION);
		else {
			CString failedinfo;
			failedinfo.Format(_T("�������ʧ�ܣ�\n\n���ܵ�ԭ��%s"), GetErrorMessage(result));
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
	CopyOrMoveRpy(_T("�ƶ�/������¼��..."), FALSE);
}

void CThhylDlg::OnFilecopyto() 
{
	// TODO: Add your command handler code here
	CopyOrMoveRpy(_T("����¼��..."), TRUE);
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

	if (result) { //����/�ƶ��ɹ�
		//������ƶ���������ѡ����ѡ���ˡ��򿪸��ƺ��Ŀ���ļ���
		if (!bCopy || HasConfigOption(CFG_COPYOPENDEST)) {
			m_rpyfile = newfilename;
			Analyze(TRUE);
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
	const CString newfilepath = m_pWndFileList->PrepareForDeleteFile();

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

	if (ret==0 && !fos.fAnyOperationsAborted) { //ɾ���ɹ�
		if (HasConfigOption(CFG_AUTOEXIT))
			OnCancel();
		else {
			m_pWndFileList->Refresh();
			if (newfilepath == m_rpyfile) {
				CloseFile();
			}
			else {
				m_rpyfile = newfilepath;
				Analyze();
			}
		}
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
		LPCTSTR const msgtitle = _T("�޷��༭ע�͡����ܵ�ԭ��");
		LPCTSTR const msg = _T("1. ��ħ��������ε�¼���ļ��޷����ע�ͣ�\n")
							_T("2. ��δ���κ��ļ����ļ���ʽ��֧�֡�");
		const HICON icon = AfxGetApp()->LoadIcon(MAKEINTRESOURCE(IDR_MAINFRAME));

		if (!ShowBalloonMsg( hRpyFile, msg, msgtitle, TTI_WARNING, FALSE, icon)) {
			// ��� balloon ��֧�֣�����ʾ msgbox
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
		MessageBox(CString(_T("�޷��򿪰����ļ�:\n\n")) + exefullpath, g_title, 16);	
}

void CThhylDlg::OnHelphtml() 
{
	// TODO: Add your control notification handler code here
	
	// ���˵�����ť
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

		// ���� MoveWindow��ʹ�� DeferWindowPos ϵ�к������Է�ֹ�ؼ��ػ�����ֿ����޷�������ʾ�����⡣
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

	if (id == IDC_RPYFILE) { // ��¼���ļ����༭��
		if (notifycode == EN_KILLFOCUS) {
			// �����뿪�༭��ʱ�������û��ڱ༭���������·���������û����˻س���
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
	delete_then_null(m_pWindowGluer);

	CoUninitialize();
	
	return CDlgBaseWZ::DestroyWindow();
}

BOOL CThhylDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class

	if (m_hAccel) {
		if (::TranslateAccelerator(m_hWnd, m_hAccel, pMsg))
			return TRUE;

		if (m_pWindowGluer->handleSwitchMessage(this->GetSafeHwnd(), pMsg->message, pMsg->wParam, pMsg->lParam)) return TRUE;

		// other keys
		switch (pMsg->message)
		{
		case WM_MOUSEWHEEL: {
				const WORD vKey = LOWORD(pMsg->wParam);

				if ( (vKey & MK_LBUTTON) && GetDlgItem(IDC_RPYINFO) != GetFocus() ) {
					// ������+�����֣��ҽ��㲻�ڡ�¼����Ϣ������: ���ò�͸����
					const HWND hWnd = this->GetSafeHwnd();
					const HWND hEdit = ((CEdit*)GetDlgItem(IDC_RPYFILE))->GetSafeHwnd();
					const SHORT threshold = HIWORD(pMsg->wParam);

					IncreaseWindowAlpha(hWnd, threshold/WHEEL_DELTA*5);
					CString strMsg;
					const int alpha = (int)GetWindowAlpha(hWnd);
					strMsg.Format(_T("��ǰ�Ĳ�͸���� = %d/255(%.2f%%)"), alpha, (double)(alpha)*100/255 );
					ShowBalloonMsg(hEdit, strMsg, L"��͸�����Ѹ���", TTI_INFO, FALSE);
					return TRUE;
				}
				else if ( vKey & MK_CONTROL ) {
					// Ctrl+������: �ı������С
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
				if (GetDlgItem(IDC_RPYFILE) != GetFocus()) { // ֻ�ڽ��㲻�ڡ�¼���ļ����༭��ʱ��Ч
					this->OnFiledelete();
					return TRUE;
				}
				break;
			case VK_F8: // F8 ������ Delete ��������
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
	
	// ����MFC�Դ���F1��Ӧ
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
	title.Format(_T("����ԭʼ����(ԭʼ���ݴ�С: %d, rpy �ļ���С: %d)"), (int)rawsize, (int)m_dwRpySize);
	dlg.m_ofn.lpstrTitle = title;
	if (dlg.DoModal() == IDOK) {
		// dump
		if (!m_pRpyAnalyzer->DumpRPYData( dlg.GetPathName() ))
			// dumping is not supported?
			// for current version, this may happen for th15 trial rpy.
			MessageBox(_T("�Բ��𣬵�ǰ�汾��֧�ֱ����¼���ļ���ԭʼ���ݡ�"), g_title, MB_ICONSTOP);
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
	LPWSTR const msg      = bCopy ? L"�ļ��Ѹ��Ƶ�������" : L"�ļ��Ѽ��е�������";
	LPWSTR const msgerr   = bCopy ? L"�޷������ļ���������" : L"�޷������ļ���������";
	LPWSTR const msgtitle = bCopy ? L"�޷�����" : L"�޷�����";
	const HWND hRpyFile = ((CEdit*)GetDlgItem(IDC_RPYFILE))->GetSafeHwnd();

	if (!m_filestatus.IsValid()) {
		ShowBalloonMsg( hRpyFile, L"��δ��¼���ļ����ļ���ʽ��֧��", msgtitle, TTI_WARNING);
	}
	else {
		if (CopyFileToClipboardW(m_rpyfile, bCopy))
			ShowBalloonMsg( hRpyFile, msg, L"", TTI_NONE);
		else
			ShowBalloonMsg( hRpyFile, msgerr, L"����", TTI_ERROR);
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




// ���棬���� EDIT �ؼ���ѡ���ʹ�ֱ������λ��
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
			LISTITEMSIGN _T("����LL�����ɱܣ�junkojunkojun~ \x270c"),
			LISTITEMSIGN _T("�����򳵡� \x270c\x2620\x270c"),
			LISTITEMSIGN _T("�������ٲ��ɱܣ���ȱ���統�Ի�"),
			LISTITEMSIGN _T("�������һ�� poi"),
			LISTITEMSIGN _T("POI...POI? POI! POIPOIPOI~"),
			LISTITEMSIGN _T("��ϵͳ��ʾ�������εĽڲ����ز��㡣Ϊ��֤���ܼ���ʹ�����Σ���������ͨ���ٷ���Ϸ���ٷ���������(��~)�ȷ�ʽ��ֵ��"),
			LISTITEMSIGN _T("��~���ǿ����ҷ�����ʲô��������һֻ�䵥�ĳ����棬���ǿ��Գ��Բ�׽����һֻ���������Ϊ�����ṩ�ü�������������Ǹ��������ķ���������������ɲ��öԸ������ǴӺ��������ӽ�����С�ı𷢳��κ����������٣���ץ���ˣ��������ú����������ǰ�����ͷš����������Ĳ�λ�������ԣ����ķ��������Ǵ��ֵ�6������Ȼ�����ʱ�䲻���ȣ����ǿ�����prpr�����������ζ�����ţ����ǵĿڸи��Դ࣬ζ������С���һ����"),
			LISTITEMSIGN _T("GZZLLNMNBNS������������"),
			LISTITEMSIGN _T("CAUTION: SJF is watching you!"),
			LISTITEMSIGN _T("��¼��(DELETE)������ϰ(THE)�������Լ�(GAME)"),
			LISTITEMSIGN _T("͵�������ݿ��� �������ص� �� ���ж�"),
			LISTITEMSIGN _T("���񴬡���������"),
			LISTITEMSIGN _T("������һֻ���ȵ����ϰ����ҵ���ǰ����û�гԵ�������"),
			LISTITEMSIGN _T("��ʵ��һ����˵Ҫ�����ʱ�����Ǿܾ��ġ���\r\n�����ڣ���רע����30�� \x270c"),
			LISTITEMSIGN _T("�������ϲ������"),
			LISTITEMSIGN _T("��ʵ���ϱ�û�л����磬���������ˣ�Ҳ����˻�����"),
			LISTITEMSIGN _T("�����ҵ�СѽС��⤣���ô���㶼���Ӷ�\x266a"),
			LISTITEMSIGN _T("����ըB��Ҳ����B"),
			LISTITEMSIGN _T("���ڳ�Ĭ�з�B�����ڳ�Ĭ��MISS"),
			LISTITEMSIGN _T("ȼ��ȼ���롫"),
			LISTITEMSIGN _T("B����Ҫ�ŵģ���һײ���أ�"),
			LISTITEMSIGN _T("��˵�ڹ�Ȧ17��Ľ�BBA��500��Ľ������Ҷ��������ǲ�Ҫƭ�Ұ���"),
			LISTITEMSIGN _T("BBA��ϲ���㰡��"),
			LISTITEMSIGN _T("���ɷ��ȴ��̣����籾������"),
			LISTITEMSIGN _T("�����_�L������ʤ˿ɐۤ��櫓���ʤ���"),
			LISTITEMSIGN _T("zun://www.�����_.com/ħ����ɭ/ɭ����֮�����ɽ.avi"),
			LISTITEMSIGN _T("��ħ������棬�����ο��ţ���㣩�棬��ҹ������棬����¼ײ���棬���������棬�����������棬����������棬�����ը���棬��鴫������"),
			LISTITEMSIGN _T("\x0425\x043e\x0440\x043e\x0448\x043e"), // Xopowo
			LISTITEMSIGN _T("2un��ͷ��������Ҹ���Ӹ�buff"),
			LISTITEMSIGN _T("��ɳ����������Ҫ������\r\n\r\n�𣺵�һ�����㣻�ڶ�������������������Ϊ����"),
			LISTITEMSIGN _T("�׹�����prpr"),
			LISTITEMSIGN _T("�⡭������˵��Ų���Ϊ������򿪵��أ���Ҫ����˰�///")
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
			menuhelp = _T("o( =���ء�= )m ����ʲô��û�еķָ�������");
		}
		else if (nFlags & MF_GRAYED) {
			menuhelp = _T("o( =���ء�= )m ����Ҫ�ȴ�һ��¼���ļ��ſ��Ե���~");
		}
		else if (!menuhelp.LoadString(nItemID)) {
			menuhelp = _T("o( =���ء�= )m ����");
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
		ShowBalloonMsg( ((CButton*)GetDlgItem(IDC_RPYFILE))->GetSafeHwnd(), L"O_O", L"ǰ��û��¼����", TTI_WARNING, FALSE);
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
		ShowBalloonMsg( ((CButton*)GetDlgItem(IDC_RPYFILE))->GetSafeHwnd(), L"O_O", L"����û��¼����", TTI_WARNING, FALSE);
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
		ShowBalloonMsg( ((CButton*)GetDlgItem(IDC_RPYFILE))->GetSafeHwnd(), L">_<b", L"����ô��Ҳû����", TTI_WARNING, FALSE);
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
		ShowBalloonMsg( ((CButton*)GetDlgItem(IDC_RPYFILE))->GetSafeHwnd(), L">_<b", L"����ô��Ҳû����", TTI_WARNING, FALSE);
	}		
}

void CThhylDlg::OnResetfilelistpos()
{
	m_pWndFileList->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	m_pWndFileList->ShowWindow(SW_SHOW);
	m_pWndFileList->SetActiveWindow();
}

LRESULT CThhylDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (m_pWindowGluer->handleMessage(message, wParam, lParam)) return 0;
	
	return CDlgBaseWZ::WindowProc(message, wParam, lParam);
}
