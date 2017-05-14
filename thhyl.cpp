// thhyl.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "cfgfile.h"
#include "thhyl.h"
#include "thhylDlg.h"
#include "RPYAnalyzer.h"
#include "global.h"
#include "clipboard.h"
#include "OSVer.h"

#include "filepath_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

inline static int MsgBox(LPCTSTR text, UINT options)
{
	return ::MessageBox( NULL, text, g_title, options );
}

/////////////////////////////////////////////////////////////////////////////
// CThhylApp

BEGIN_MESSAGE_MAP(CThhylApp, CWinApp)
	//{{AFX_MSG_MAP(CThhylApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CThhylApp construction

CThhylApp::CThhylApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	m_rpyfile = _T("");
	m_bOnTop = TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CThhylApp object

CThhylApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CThhylApp initialization

//ȫ�ֱ���֮��ʼ��
static TCHAR s_exefullpath[MAX_PATH] = {0};
LPCTSTR g_exefullpath = (LPCTSTR)s_exefullpath;
////////////////

//���������в���
void CThhylApp::ParseArguments()
{
	int argc;
	LPWSTR* argv=CommandLineToArgvW(GetCommandLineW(), &argc);
	ASSERT(argc>0 && argv!=NULL);

	//EXE ����·��
	_tcsncpy(s_exefullpath, argv[0], MAX_PATH);
	s_exefullpath[MAX_PATH-1] = _T('\0');

	//config���ӹ���Ŀ¼ֱ��д��EXE����Ŀ¼�¡�
	CString cfgfilename = argv[0];
	filepath_utils::ChangeFileExtension(cfgfilename, _T("cfg"));
	SetConfigFilename(cfgfilename);
	LoadConfig();

	for (int i=1;i<argc;i++) {
		if (wcscmp(argv[i], _T("-n")) == 0)
			m_bOnTop = FALSE;
		else if (i == 1)
			m_rpyfile = argv[1];
	}

	LocalFree(argv); //�ͷ�argv��ռ�ڴ�
}

BOOL CThhylApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	ParseArguments();
	OSVerInit();

	//����Ƿ���ScrollLock
	if ( IsKeyDown(VK_SCROLL) && !m_rpyfile.IsEmpty()) { //���£�������¼���ļ���
		AnalyzeAndCopy(m_rpyfile); //����¼�񣬲�����Ϣ���Ƶ�������
	}
	else {
		CThhylDlg dlg;
		m_pMainWnd = &dlg;
		dlg.DoModal();
		
		SaveConfig();
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;

}

BYTE* ReadRPYFile(LPCTSTR szFileName, CString& strErrorInfo, HWND hWnd, CFileStatus* pFileStatus, DWORD& dwFileSize)
{
	CFile          cfRpyFile;
	CFileException ex;
	
	if ( !cfRpyFile.Open(szFileName, 
		CFile::modeRead | CFile::shareDenyWrite | CFile::typeBinary, &ex) )
	{
		ex.GetErrorMessage(strErrorInfo.GetBuffer(512), 512);
		strErrorInfo.ReleaseBuffer();
		return NULL;
	}
	else {
		if ( !CheckRPYFileSize((HANDLE)cfRpyFile.m_hFile, hWnd)
			|| (pFileStatus != NULL && !cfRpyFile.GetStatus(*pFileStatus)) )
			return NULL;
	}

	cfRpyFile.SeekToBegin();
	
	dwFileSize = cfRpyFile.GetLength();
	BYTE *pBuf = new BYTE[dwFileSize+MINRPYSIZE];
	
	dwFileSize = cfRpyFile.Read(pBuf, dwFileSize+1);
	pBuf[dwFileSize] = '\0';
	cfRpyFile.Close();

	// ����ǲ��� ����Ļ����������ò���ǹ̶����ļ�ͷ��
	const char mhj_header[] =
		"\xF0\x67\x82\xFF\xCA\xE3\xC8\x5B\xFC\x91\xB5"
		"\x72\xCB\x84\xC2\xEF\xDC\xE1\xC2\xE4\x11\x42"
		"\x66\x2E\x3A\x85\x9C\x1E\xC8\x16\xB5\x6E\x38"
		"\x83\x6F\x7E\xA1\x10\x73\x52\x30\x40\x5A\xC0"
		"\xD0\x4C\x3C\x4D\xCE\xC6\xAA\x7D\x05\xCB\x74"
		"\x97\x5B\x6E\xE2\x47\x3A\xED\x6D\xEB\xDE\x09"
		"\x06\x7F\x0B\xC8\x19\x20\xED\x36\x92\x29\x07"
		"\x0A\x8E\x9E\x6D\x1F\xEF\x03\xF3\xC9\xB7\x96"
		"\x10\xC0\xED\x2C\x83\x9A\x3F\xCC";

	if ( memcmp(mhj_header, pBuf, sizeof(mhj_header) - 1) == 0 ) {
		strErrorInfo =
			_T("��ò���� �|��Ļ�A�� ��¼��Ŷ����\r\n")
			_T("���ǻ�ӳ¼�����������������ߣ�\r\n");
		return delete []pBuf, NULL;
	}

	// ��ʵ����֧�� �����ٻ��磬ֻ�Ǹ㲻������жϣ�����û�й̶��ļ�ͷ����

	return pBuf;
}


void CThhylApp::AnalyzeAndCopy(LPCTSTR szFileName)
{
	CString strInfo;
	DWORD dwRead;

	BYTE* pBuf = ReadRPYFile(szFileName, strInfo, NULL, NULL, dwRead);
	if (pBuf == NULL) { 
		if ( !strInfo.IsEmpty() ) {
			MsgBox(strInfo, MB_ICONSTOP);
		}
		return;
	}
	
	//Start
	CRPYAnalyzer* pRpyInfo = new CRPYAnalyzer(pBuf, dwRead, cfg.InfoCode);
	const int RPYAnalyzer_ret = pRpyInfo->Analyze();

	if ( RPYAnalyzer_ret != RPYINFO_UNKNOWNFORMAT ) {
		strInfo = pRpyInfo->GetFinalInfo();
		if ( pRpyInfo->m_header!=mgc6 && pRpyInfo->m_header!=mgc7 ) {
			if ( RPYAnalyzer_ret == RPYINFO_OK ) {
				CString rpycomment(pRpyInfo->GetComment(cfg.CommentCode));
				if (!rpycomment.IsEmpty()) {
					strInfo += CString(_T("\r\nע�ͣ�\r\n")) + rpycomment;
				}
			}
		}
	}
	
	//��¼����Ϣ���Ƶ�������
	if (RPYAnalyzer_ret==RPYINFO_OK && !strInfo.IsEmpty()) {
		int ret = IDCANCEL;
		CString failedinfo;

		do {
			if ( ::SetClipTextW(strInfo) ) {
				MessageBeep(MB_ICONINFORMATION);
				break;
			}
			else {
				LPCTSTR ErrorMsg = GetErrorMessage( GetLastError() );
				failedinfo.Format(_T("�������ݵ�������ʧ�ܣ�\n\n���ܵ�ԭ��%s"), ErrorMsg);
				ret = MsgBox( failedinfo, MB_ICONSTOP | MB_RETRYCANCEL );
			}
		} while (ret==IDRETRY);
	}
	else {
		CString msg;

		if (strInfo.IsEmpty())
			strInfo = LoadNotRpyString();

		msg.Format(_T("%s\n\n%s"), szFileName, strInfo);
		MsgBox(msg, MB_ICONSTOP);
	}

	delete pRpyInfo;
	delete []pBuf;
}