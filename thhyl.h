// thhyl.h : main header file for the THHYL application
//

#if !defined(AFX_THHYL_H__88A11EBE_6591_4DF3_B9B8_C5F0AED75194__INCLUDED_)
#define AFX_THHYL_H__88A11EBE_6591_4DF3_B9B8_C5F0AED75194__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CThhylApp:
// See thhyl.cpp for the implementation of this class
//

class CThhylApp : public CWinApp
{
public:
	CThhylApp();

	BOOL m_bOnTop;
	CString m_rpyfile;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CThhylApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CThhylApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void AnalyzeAndCopy(LPCTSTR szFileName);
	void ParseArguments();
};

BYTE* ReadRPYFile(LPCTSTR szFileName, CString& strErrorInfo, HWND hWnd, CFileStatus* pFileStatus, DWORD& dwFileSize);

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_THHYL_H__88A11EBE_6591_4DF3_B9B8_C5F0AED75194__INCLUDED_)
