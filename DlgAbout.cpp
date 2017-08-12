// DlgAbout.cpp : implementation file
//

#include "stdafx.h"
#include <mmsystem.h>
#include "thhyl.h"
#include "DlgAbout.h"
#include "global.h"
#include "OSVer.h"

#pragma comment (lib, "winmm.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static BOOL yinxiao = FALSE;
static int yinxiao_count = 0;
static LPCTSTR myemail = _T("wingzero1040@gmail.com");
static LPCTSTR updateurl = _T("https://www.zybuluo.com/wz520/note/15842");

/////////////////////////////////////////////////////////////////////////////
// CDlgAbout dialog


CDlgAbout::CDlgAbout(CWnd* pParent /*=NULL*/)
	: CDlgBaseWZ(CDlgAbout::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgAbout)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgAbout::DoDataExchange(CDataExchange* pDX)
{
	CDlgBaseWZ::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgAbout)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgAbout, CDlgBaseWZ)
	//{{AFX_MSG_MAP(CDlgAbout)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_MYEMAIL, OnMyemail)
	ON_BN_CLICKED(IDC_UPDATEURL, OnUpdateurl)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_ABOUTPIC, OnAboutpic)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgAbout message handlers

void CDlgAbout::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CDlgBaseWZ::OnLButtonDown(nFlags, point);
	PostMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));
}

static BOOL IsWow64()
{
    BOOL bIsWow64 = FALSE;

    //IsWow64Process is not available on all supported versions of Windows.
    //Use GetModuleHandle to get a handle to the DLL that contains the function
    //and GetProcAddress to get a pointer to the function if available.
	
	typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
    LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(
        GetModuleHandle(TEXT("kernel32")),"IsWow64Process");

    if (NULL != fnIsWow64Process) {
        fnIsWow64Process(GetCurrentProcess(),&bIsWow64);
    }
    return bIsWow64;
}

// calc count of bit 1, using Hamming Weight
static DWORD calcbit1count(DWORD n)
{
	n = (n&0x55555555) + ((n>>1)&0x55555555);
	n = (n&0x33333333) + ((n>>2)&0x33333333);
	n = (n&0x0f0f0f0f) + ((n>>4)&0x0f0f0f0f);
	n = (n&0x00ff00ff) + ((n>>8)&0x00ff00ff);
	n = (n&0x0000ffff) + ((n>>16)&0x0000ffff);
	return n;
}

static DWORD GetCPUCount()
{
	DWORD_PTR procmask=0, sysmask=0;
	GetProcessAffinityMask(GetCurrentProcess(), &procmask, &sysmask);
	return calcbit1count(sysmask);
}

BOOL CDlgAbout::OnInitDialog() 
{
	CDlgBaseWZ::OnInitDialog();

	// TODO: Add extra initialization here
	m_dlgopt = DLGWZ_OPT_ALWAYSANYDRAG;

	m_tooltip.AddTool( GetDlgItem(IDC_UPDATEURL), updateurl );
	m_tooltip.AddTool( GetDlgItem(IDC_MYEMAIL), myemail );

	CEdit* const pEdit = (CEdit*)GetDlgItem(IDC_EDITABOUT);
	CString abt;
	const DWORD pid = GetCurrentProcessId();
	const DWORD tid = GetCurrentThreadId();
	const DWORD tickcount = GetTickCount();
	abt.Format(
		_T("%s %d.%d alpha\r\n\r\n")
		_T("Written by wz520\r\n")
		_T("贴吧ID: 天使的枷锁\r\n")
		_T("邮箱: %s\r\n\r\n")
		_T("更新地址：\r\n%s\r\nhttp://pan.baidu.com/s/1ntt4845\r\n\r\n")
		_T("-----------------------------------------------\r\n")
		_T("操作系统: Windows %s %s %s\r\n")
		_T("处理器数量: %u\r\n")
		_T("当前进程 PID: %u(0x%x)\r\n")
		_T("当前线程 TID: %u(0x%x)\r\n")
		_T("GetTickCount(): %u(0x%x)\r\n")
		, g_title, 1, 82
		, myemail
		, updateurl

		, OSVerGetOSString(), (IsWow64() ? _T("64位") : _T("32位")), OSVerGetCSDVersion()
		, GetCPUCount()
		, pid, pid
		, tid, tid
		, tickcount, tickcount
	);
	pEdit->SetWindowText(abt);
	
	// yinxiao
	static bool firstcall = true;
	yinxiao_count = 0;
	if ( !firstcall ) {
		CButton* const pButton = (CButton*)GetDlgItem(IDOK);
		VERIFY(pButton->ModifyStyle( 0, BS_ICON, 0 ));
		pButton->SetIcon( ::LoadIcon( ::GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_YINSMILE) ) );
		VERIFY(pButton->ModifyStyle( BS_DEFPUSHBUTTON | WS_TABSTOP, 0, 0 ));

		// this->GetNextDlgTabItem(pButton)->SetFocus();
		(CButton*)GetDlgItem(IDC_MYEMAIL)->SetFocus();

		yinxiao = TRUE;
	}
	else {
		firstcall = false;
		yinxiao = FALSE;
	}

	this->CenterWindow();

	return !yinxiao;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgAbout::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	if (yinxiao) {
		CButton* const pButton = (CButton*)GetDlgItem(IDOK);
		RECT winrect, rect;
		this->GetClientRect(&winrect);
		pButton->GetWindowRect(&rect);
		this->ScreenToClient(&rect);
		TRACE(_T("winrect=%d,%d,%d,%d; rect=%d,%d,%d,%d; mouse=%d,%d"),
			winrect.left, winrect.top, winrect.right, winrect.bottom,
			rect.left, rect.top, rect.right, rect.bottom,
			point.x, point.y
		);

		if (rect.bottom < 200 && point.y < rect.bottom + 50) {
			pButton->MoveWindow(rect.left, rect.top + 180, rect.right-rect.left, rect.bottom - rect.top);
			++yinxiao_count;
		}
		else if (rect.bottom > 200 && point.y > rect.top - 50) {
			pButton->MoveWindow(rect.left, rect.top - 180, rect.right-rect.left, rect.bottom - rect.top);
			++yinxiao_count;
		}

		if (yinxiao_count == 20) {
			CEdit* const pEdit = (CEdit*)GetDlgItem(IDC_EDITABOUT);
			pEdit->SetWindowText(_T("鼠标娘要被你磨坏了。还是按 ESC 键吧 =v="));
		}
	}
	
	CDlgBaseWZ::OnMouseMove(nFlags, point);
}

void CDlgAbout::OnMyemail() 
{
	// TODO: Add your control notification handler code here
	ShellExecute(NULL, _T("open"), CString("mailto:") + myemail, NULL, NULL, SW_SHOWDEFAULT);
}

void CDlgAbout::OnUpdateurl() 
{
	// TODO: Add your control notification handler code here
	ShellExecute(NULL, _T("open"), updateurl, NULL, NULL, SW_SHOWDEFAULT);
}

void CDlgAbout::OnOK() 
{
	// TODO: Add extra validation here
	
	if (yinxiao) {
		CButton* const pButton = (CButton*)GetDlgItem(IDOK);
		CPoint pt;
		CRect rect;

		GetCursorPos(&pt);
		pButton->GetWindowRect(&rect);

		if (rect.PtInRect(pt)) {
			PlaySound(MAKEINTRESOURCE(IDR_PLDEAD00), ::GetModuleHandle(NULL), SND_ASYNC | SND_RESOURCE);
		}
	}

	OnCancel();
}

void CDlgAbout::OnCancel()
{
	DestroyWindow();
}

HBRUSH CDlgAbout::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDlgBaseWZ::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	switch (nCtlColor)
	{
	case CTLCOLOR_DLG:
		{
			static HBRUSH bkhbr = NULL;
			if (bkhbr != NULL)
				DeleteObject(bkhbr); // delete old brush
			
			COLORREF bkcolor = (simple_random() & 0xffffff) | 0x808080;
			bkhbr = CreateSolidBrush(bkcolor);
			hbr = bkhbr;
		}
		break;
	case CTLCOLOR_STATIC:
		{
			static HBRUSH bkhbr = NULL;
			if (bkhbr != NULL)
				DeleteObject(bkhbr); // delete old brush

			COLORREF bkcolor = (simple_random() & 0xffffff) | 0x808080;
			pDC->SetBkColor(bkcolor);
			pDC->SetTextColor(0x000000); // force black
			bkhbr = CreateSolidBrush(bkcolor);
			hbr = bkhbr;
		}
		break;
	}
	
	// TODO: Return a different brush if the default is not desired
	return hbr;
}

void CDlgAbout::OnAboutpic() 
{
	// TODO: Add your control notification handler code here
	CStatic* const pPic = (CStatic*)GetDlgItem(IDC_ABOUTPIC);
	HICON const hMarisa = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	bool const isMarisa = pPic->GetIcon() == hMarisa;

	if (isMarisa)
		pPic->SetIcon( AfxGetApp()->LoadIcon(IDI_REIMU) );
	else
		pPic->SetIcon( hMarisa );
}
