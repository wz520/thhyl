#ifndef __COMMONDLGWZ_H__55CFEA95_
#     define __COMMONDLGWZ_H__55CFEA95_

// written by wz520. need MFC
//
// 派生自一些 MFC 公共对话框相关类。用于解决 MFC 公共对话框在 WIN7 下无法显示成 WIN7 STYLE 的问题

class CFileDialogWZ : public CFileDialog
{
public:
	CFileDialogWZ(BOOL bOpenFileDialog,
		LPCTSTR lpszDefExt = NULL, LPCTSTR lpszFileName = NULL, DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL, CWnd* pParentWnd = NULL );

	int DoModal();
};

class CFontDialogWZ : public CFontDialog
{
public:
	CFontDialogWZ(LPLOGFONT lplfInitial = NULL, DWORD dwFlags = CF_EFFECTS | CF_SCREENFONTS, CDC* pdcPrinter = NULL, CWnd* pParentWnd = NULL );
	
	int DoModal();
};


#endif /* __COMMONDLGWZ_H__55CFEA95_ */
