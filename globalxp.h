#ifndef __GLOBALXP_H_5227E4D4_
#     define __GLOBALXP_H_5227E4D4_

// show messagebox if balloon not supported and bShowMsgBox is not FALSE.
//   But if ttiIcon is neither TTI_INFO, nor TTI_WARNING, nor TTI_ERROR,
//   play a messsagebeep instead of showing a messagebox
//
// if userIcon is not NULL, and the running Windows version >= XP SP2,
// then use userIcon as the title icon. ttiIcon will be ignored.
//
// return FALSE if balloon not supported, TRUE otherwise.
BOOL ShowBalloonMsg(HWND hEdit, LPCWSTR pszText, LPCWSTR pszTitle,
					INT ttiIcon, BOOL bShowMsgBox=TRUE,
					HICON userIcon=NULL);

void SetWindowAlpha(HWND hWnd, BYTE alpha);
BYTE GetWindowAlpha(HWND hWnd);
void IncreaseWindowAlpha(HWND hWnd, int inc);

#endif /* __GLOBALXP_H_5227E4D4_ */
