#include <windows.h>
#include <tchar.h>
#include <shlobj.h>
#include "clipboard.h"

BOOL SetClipTextW(LPCWSTR unitext)
{
	size_t len = wcslen(unitext);
	BOOL   ret = FALSE;

	if (OpenClipboard(NULL)) {
		HGLOBAL hMem;

		EmptyClipboard();

		hMem = GlobalAlloc(GMEM_MOVEABLE, (len + 1) * 2 );
		if (hMem) {
			LPWSTR pBuf = (LPWSTR)GlobalLock(hMem);
			if (pBuf) {
				wcscpy(pBuf, unitext);
				GlobalUnlock(hMem);
				SetClipboardData(CF_UNICODETEXT, hMem);
				ret = TRUE;
			}
		}

		CloseClipboard();
	}

	return ret;
}

BOOL CopyFileToClipboardW(LPCWSTR szFileName, BOOL bCopy)
{
    DROPFILES stDrop;

    HGLOBAL   hGblFiles     = NULL;
    LPWSTR    lpwData       = NULL;
    UINT      uDropEffect   = RegisterClipboardFormat(_T("Preferred DropEffect"));
    HGLOBAL   hGblEffect    = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_DDESHARE, sizeof(DWORD));
    LPDWORD   lpdDropEffect = (LPDWORD)GlobalLock(hGblEffect);
	if (lpdDropEffect) {
		*lpdDropEffect = bCopy ? DROPEFFECT_COPY : DROPEFFECT_MOVE;
		GlobalUnlock(hGblEffect);
	}

    stDrop.pFiles = sizeof(DROPFILES);
    stDrop.pt.x   = 0;
    stDrop.pt.y   = 0;
    stDrop.fNC    = FALSE;
    stDrop.fWide  = TRUE;

    hGblFiles = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_DDESHARE,
        sizeof(DROPFILES) + (wcslen(szFileName)+2) * sizeof(WCHAR));
    lpwData = (LPWSTR)GlobalLock(hGblFiles);
	if (lpwData) {
		memcpy(lpwData, &stDrop, sizeof(DROPFILES));
		wcscpy( (LPWSTR)((LPSTR)lpwData + sizeof(DROPFILES)), szFileName);
		GlobalUnlock(hGblFiles);
	}

    if (lpdDropEffect && lpwData && OpenClipboard(NULL)) {
		EmptyClipboard();
		SetClipboardData(CF_HDROP, hGblFiles);
		SetClipboardData(uDropEffect, hGblEffect);
		CloseClipboard();
		return TRUE;
	}
	else {
		if (hGblFiles) GlobalFree(hGblFiles);
		if (hGblEffect) GlobalFree(hGblEffect);
		return FALSE;
	}
}
