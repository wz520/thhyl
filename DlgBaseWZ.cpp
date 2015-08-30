// DlgBaseWZ.cpp : implementation file
//

#include "stdafx.h"
#include "thhyl.h"
#include "DlgBaseWZ.h"
#include "global.h"
#include "cfgfile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgBaseWZ dialog


CDlgBaseWZ::CDlgBaseWZ( UINT nIDTemplate, CWnd* pParent /*=NULL*/)
	: CDialog(nIDTemplate, pParent)
{
	//{{AFX_DATA_INIT(CDlgBaseWZ)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_dlgopt = 0;
}


void CDlgBaseWZ::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgBaseWZ)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgBaseWZ, CDialog)
	//{{AFX_MSG_MAP(CDlgBaseWZ)
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgBaseWZ message handlers

BOOL CDlgBaseWZ::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class

	// other keys
	switch (pMsg->message)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MOUSEMOVE:
		m_tooltip.RelayEvent(pMsg);
		break;
	case WM_KEYDOWN:
		switch (pMsg->wParam)
		{
		case VK_F11: // ÇÐ»»×î´ó»¯
			if (m_dlgopt & DLGWZ_OPT_ENABLEMAXIMIZE) {
				EnableMaximize(m_hWnd, -1);
				return TRUE;
			}
		break;
		}

	}
	
	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CDlgBaseWZ::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_tooltip.Create(this);
	m_tooltip.SetDelayTime(10000, 200);
	m_tooltip.SetTipTextColor( 0x0000ff );
	m_tooltip.SetTipBkColor( 0x00ffff );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgBaseWZ::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnLButtonDown(nFlags, point);

	if (HasConfigOption(CFG_ANYDRAG) || (m_dlgopt & DLGWZ_OPT_ALWAYSANYDRAG) )
		PostMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));
}
