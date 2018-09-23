#include "StdAfx.h"
#include "cfgfile.h"
#include "thhyl.h"
#include "global.h"

#define lengthof(arr) (sizeof(arr)/sizeof(arr[0]))

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static void SetEditCtrlFont(CEdit* pEdit, LOGFONT* pLogFont)
{
	CFont *pFont = new CFont;
	pFont->CreateFontIndirect(pLogFont);
	pEdit->SetFont(pFont, TRUE);
	pFont->Detach();
	delete pFont;
}

void LoadEditCtrlFont(CEdit* pEdit)
{
	LOGFONT lf;
	
	if ( !cfg.loadFont(&lf) ) {
		// �����ϴ��˳�ǰ����������ʧ�ܣ�ʹ��ϵͳĬ�����塣
		CFont *pFont = pEdit->GetFont();
		pFont->GetLogFont(&lf);
		CDC *cdc = pEdit->GetDC();
		ASSERT(cdc!=0);
		lf.lfHeight = PointToLogical(12, cdc);
		pEdit->ReleaseDC(cdc);
	}
	SetEditCtrlFont(pEdit, &lf);
}

void SaveEditCtrlFont(CEdit* pEdit, LOGFONT* pLogFont)
{
	SetEditCtrlFont(pEdit, pLogFont);
	cfg.saveFont(pLogFont);
}

void ComboAddString(CComboBox& cb, UINT CodePage)
{
	cb.AddString(_T("����ҳ��"));
	cb.AddString(_T("����Shift-JIS"));
	cb.AddString(_T("��������GBK"));
	cb.AddString(_T("��������Big5"));
	cb.AddString(_T("UTF8"));
	cb.AddString(_T("ϵͳĬ��"));
	Edit2ComboSel(CodePage, cb);
}

void ComboSel2Edit(CComboBox& cb, UINT& uEdit)
{
	int index = cb.GetCurSel();
	
	switch (index)
	{
	case 1: uEdit=932; break;
	case 2: uEdit=936; break;
	case 3: uEdit=950; break;
	case 4: uEdit=CP_UTF8; break;
	case 5: uEdit=0; break;
	}
}

void Edit2ComboSel(UINT uEdit, CComboBox& cb)
{
	int SelIndex = 0;

	switch (uEdit)
	{
	case 932: SelIndex=1; break;
	case 936: SelIndex=2; break;
	case 950: SelIndex=3; break;
	case CP_UTF8: SelIndex=4; break;
	case 0: SelIndex=5; break;
	}

	cb.SetCurSel(SelIndex);
}

//����ֵ��ָ��̬�ڴ��LPCTSTR������ free() �� delete!
LPCTSTR GetErrorMessage(UINT ErrorCode)
{
	static TCHAR ErrorMsg[256];

	return FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, ErrorCode, 0, ErrorMsg, 255, NULL)
		? const_cast<LPCTSTR>(ErrorMsg) : NULL;
}

BOOL CheckRPYFileSize(HANDLE hFile, HWND hwnd)
{
	LARGE_INTEGER lint;

	if (!GetFileSizeEx(hFile, &lint))
		return FALSE;

	if (lint.QuadPart > 32*1024*1024) {
		if (lint.QuadPart > 100*1024*1024) {
			int mbret = ::MessageBox(hwnd, _T("���ļ����� 100MB ������Ļ��Ƕ���STG¼���ļ���"),
				g_title, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2);
			if (mbret==IDYES)
				::MessageBox(hwnd, _T("���أ��򲻿� �����ߣ���"), g_title, MB_ICONSTOP);
			return FALSE;
		}
		else {
			int mbret = ::MessageBox(hwnd, _T("���ļ����� 32M ��ȷ��Ҫ����"),
				g_title, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2);
			return (mbret==IDYES) ? TRUE : FALSE;
		}
	}
	else
		return TRUE;
}



BOOL DumpBinData(const BYTE* pData, DWORD nSize, LPCTSTR szFilename)
{
	CFile dfile;
	if (!dfile.Open(szFilename, CFile::shareDenyRead | CFile::modeCreate | CFile::modeWrite | CFile::typeBinary)) {
		AfxMessageBox(_T("Open dump file error!"), MB_ICONSTOP);
		return FALSE;
	}
	else {
		dfile.Write(pData, nSize);
		dfile.Close();
		return TRUE;
	}
}

// Used by ReadUnicodeFileLine()
// Return value is a pointer to the specified line.(NULL for not found)
// linenum is 1 based.
// The data in s will be modified by one or more null terminator(s).
// No additional memory will be allocated.
LPWSTR ReadLineW(LPWSTR s, int linenum)
{
	LPWSTR pszCurrLine   = s;
	LPWSTR pszWantedLine = NULL;

	for (int i=1; 1; ++i) {
		if ( i==linenum )
			pszWantedLine = pszCurrLine; // line i is wanted, get it

		if ( (pszCurrLine=wcschr(pszCurrLine, 0x0A)) != NULL ) {
			if (pszWantedLine) { // found wanted line, so pszWantedLine is pointing to the end of line.
				*pszCurrLine = 0; // null-terminate
				if (*(--pszCurrLine) == 0x0D) // 0x0D before 0x0A?
					*pszCurrLine = 0;
				break;
			}
			++pszCurrLine;
		} else
			break;
	}

	return pszWantedLine;
}

// The encoding of the file content must be UTF-16LE with BOM
BOOL ReadUnicodeFileLine(LPCTSTR filename, int linenum, CString &outStrLine)
{
	long  filesize;
	BOOL  ret       = FALSE;
	FILE * const fp = _tfopen(filename, _T("rb"));
	if (fp == NULL)
		return FALSE;

	fseek(fp, 0, SEEK_END);
	filesize = ftell(fp);
	rewind(fp);

	// Starting to read file
	do { // used for breaking
		if (filesize > 2) { // must bigger than 2 bytes
			// The file is bigger than 1MB, here it is not allowed.
			if ((unsigned long)filesize > 1024*1024UL)
				break;
			else {
				// read file
				BYTE* const buf = (BYTE *)malloc(filesize+2); // extra 2 bytes is used for unicode null terminator
				if (buf) {
					const size_t elements_read = fread(buf, 1, filesize, fp); // read
					if (elements_read != 0) {
						const WORD bom = 0xfeff;
						if (*(WORD *)buf == bom) { // has BOM, read
							LPWSTR pszText = (LPWSTR)(buf+2);
							*(LPWSTR)(buf+filesize) = L'\0'; // append \0

							// find wanted line
							pszText = ReadLineW(pszText, linenum);
							if (pszText) { // found specified line
								outStrLine = pszText; // copy to CString
								ret = TRUE;
							}
						} // no BOM, not allowed
					}
					free(buf);
				}
			}
		}
	}
	while (0);

	fclose(fp);
	return ret;
}



// �����ϰ :)
// ����ʹ�� RtlGenRandom ��ȡ����������û�иú��������� GetTickCount
// ��˵ GetTickCount ����������ȡ������ģ��ͻ�ӳ¼������������
// srand �� rand ʲô�Ĳ����棨
DWORD simple_random()
{
	DWORD          i;
	static FARPROC funcaddr = NULL;
	LPCTSTR const  dllname  = _T("advapi32.dll");
	LPCSTR const   funcname = "SystemFunction036"; // RtlGenRandom

	__asm {
		// �Ƿ��Ѽ��ع� RtlGenRandom��
		mov eax, funcaddr;
		test eax, eax;
		jnz luse036;

		// funcaddr Ϊ NULL�����Լ���
		push dllname;
		call dword ptr[LoadLibrary];
		push funcname;
		push eax; // hmodule
		call dword ptr[GetProcAddress];

		test eax, eax;
		mov funcaddr, eax;
		jz luse_gtc; // ������ʧ�ܣ����� GetTickCount()
luse036:
		// ���سɹ������� eax = RtlGenRandom()
		push 4;  // sizeof DWORD
		lea edx, i;
		push edx;
		call eax;
		test eax, eax;
		jz luse_gtc; // ����ʧ��(FALSE)������ GetTickCount()
		mov eax, i;
		jmp lexit;
luse_gtc:
 		call dword ptr[GetTickCount];
lexit:  // return ����ʡ�ˡ�����ʲô�����Ӱ�
	}
}

// Load 'is not rpy' message:
CString LoadNotRpyString()
{
	CString strNotRpy;
	strNotRpy.LoadString(IDS_ISNOTRPY);

	static LPCTSTR varStrings[] = {
		_T("����ȱǮ�ˡ����������ʮ��Ԫ�������粩������"),
		_T("���ļ���ħ��ɳ������Ѿ��õ���"),
		_T("��С�����ϲ��㡣ɾ�����ļ����Ը���С���������ϡ�������������R��������"),
		_T("���ļ�����С���滵��"),
		_T("�Dҹ��ʾ����Ǹ���ֻ�����Զ̽��ġ����ְ�����ʲô���Ѿ������ˣ���������ļ��ǲ���Ҳ���Բ弸�������أ�"),
		_T("�����ʾ�����������㣬�����̴򿪡���������¼���ռ�����"),
		_T("�����ӱ�ʾ���ļ��������Ʈ������¥"),
		_T("���ϱ�ʾ���ļ�ûζ����һ����ˮ�ˡ�������û�м������谷���������ǲ�֪����"),
		_T("����������¶ŵ�����"),
		_T("���ʾʲô����֪��"),
		_T("��������ҹ��ʾ���ļ������� 1 ���ֽ��⵽�ƻ�������MD5ֵ�кܴ�ı仯�����޷���ȷ����"),
		_T("�����ӳ���Ҫ�������εİ��飬�Ƿ���԰ѱ��ļ�����ɰ���������׸��������أ�ɾ���ļ���ʾͬ��"),
		_T("��ݱ�ʾ�� 0 �� 1 ��ɵ��ִ�ħ�����Ѽ�Ԧ"),
		_T("����˿����Ҫ�Ķ�����͵����"),
		_T("�������ʾ���쳯����Լ����ֵ��˺ܶ࣬��д�����ֵ���ȴ��ë���"),
		_T("���Ĵ�������֧�� Zuntel ϵ��ָ�������ļ���Ҫ����ָ�MOVZUNX, ADDBEER, MULBEER, JMUS, JNMUS")
	};

	DWORD i = simple_random() % lengthof(varStrings);
	strNotRpy.Replace(_T("#@"), varStrings[i]);
	return strNotRpy;
}

// lEnable = 1:on, 0:off, -1:toggle
// return GetLastError()
LONG EnableMaximize(HWND hwnd, LONG lEnable)
{
	if (lEnable < 0) // toggle
		lEnable = (LONG)(!IsZoomed(hwnd));
	
	PostMessage(hwnd, WM_SYSCOMMAND,
		lEnable ? SC_MAXIMIZE : SC_RESTORE, 0);
	
	return GetLastError();
}


BOOL SetControlFontSize(HWND hwnd, int n, LOGFONT* pOutLogFont, DWORD dwFlags)
{
	HFONT const hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
	
	if (hFont) {
		const int pointmin = 9;
		const int pointmax = 108;
		LOGFONT   lf       = {0};

		GetObject((HGDIOBJ)hFont, sizeof(lf), &lf);

		HDC hdc = GetDC(hwnd);
		int point = n;
		if (dwFlags & SCFS_INCREMENTAL) {
			point = LogicalToPoint(lf.lfHeight, hdc) + n;
			
			if ( point > pointmax )
				point = pointmax;
			else if ( point < pointmin )
				point = pointmin;
		}

		// Set new font to the window
		{
			lf.lfHeight = PointToLogical(point, hdc);

			HFONT hNewFont = CreateFontIndirect(&lf);
			SendMessage(hwnd, WM_SETFONT, (WPARAM)hNewFont, TRUE);
			if (!(dwFlags & SCFS_KEEPOLDHFONT)) {
				DeleteObject(hFont); // delete old font
			}
		}

		ReleaseDC(hwnd, hdc);
		if (pOutLogFont != NULL)
			memcpy(pOutLogFont, &lf, sizeof(lf));
		return TRUE;
	}
	else
		return FALSE;
}

BOOL SetControlFontSize(CWnd* pWnd, int n, LOGFONT* pOutLogFont, DWORD dwFlags)
{
	HWND const hWnd = pWnd->GetSafeHwnd();
	if (hWnd != NULL) {
		return SetControlFontSize(hWnd, n, pOutLogFont, dwFlags);
	}
	else
		return FALSE;
}
