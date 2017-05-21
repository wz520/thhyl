#define _WIN32_IE 0x0501
#define WINVER 0x0501
#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <tchar.h>
#include <commctrl.h>
#include "globalxp.h"
#include "OSVer.h"

BOOL ShowBalloonMsg(HWND hEdit, LPCWSTR pszText, LPCWSTR pszTitle,
					INT ttiIcon, BOOL bShowMsgBox/*=TRUE*/,
					HICON userIcon/*=NULL*/)
{
	EDITBALLOONTIP balloonTip;
		
	balloonTip.cbStruct = sizeof(EDITBALLOONTIP);
	balloonTip.pszText  = pszText;
	balloonTip.pszTitle = pszTitle;
	balloonTip.ttiIcon  = userIcon && OSVerAboveXP_SP2() ? (INT)userIcon : ttiIcon;

	if (SendMessage(hEdit, EM_SHOWBALLOONTIP, 0, (LPARAM)&balloonTip))
		return TRUE;
	else if (!bShowMsgBox)
		return FALSE;
	else {
		UINT uType = 0;
		switch (ttiIcon)
		{
		case TTI_INFO:    uType = MB_ICONINFORMATION; break;
		case TTI_WARNING: uType = MB_ICONWARNING; break;
		case TTI_ERROR:   uType = MB_ICONERROR; break;
		}

		if (uType != 0)
			::MessageBoxW(::GetParent(hEdit), pszText, pszTitle, uType);
		else
			MessageBeep(0);

		return FALSE;
	}
}

void SetWindowAlpha(HWND hWnd, BYTE alpha)
{
	if (alpha == (BYTE)255) {
		SetLayeredWindowAttributes(hWnd, 0, alpha, LWA_ALPHA);
		SetWindowLong(hWnd, GWL_EXSTYLE,
				GetWindowLong(hWnd, GWL_EXSTYLE) & ~WS_EX_LAYERED);
	}
	else {
		SetWindowLong(hWnd, GWL_EXSTYLE,
				GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
		SetLayeredWindowAttributes(hWnd, 0, alpha, LWA_ALPHA);
	}
}

BYTE GetWindowAlpha(HWND hWnd)
{
	BYTE alpha = 255;

	if ( GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_LAYERED ) {
		DWORD dwFlags = 0;
		if ( !GetLayeredWindowAttributes(hWnd, NULL, &alpha, &dwFlags) || !(dwFlags & LWA_ALPHA) )
			alpha = 255;		
	}

	return alpha;
}

void IncreaseWindowAlpha(HWND hWnd, int inc)
{
	int nAlpha = (int)GetWindowAlpha(hWnd) + inc;
	if ( nAlpha > 255 )
		nAlpha = 255;
	else if ( nAlpha < 5 )
		nAlpha = 5;

	SetWindowAlpha(hWnd, (BYTE)nAlpha);
}
