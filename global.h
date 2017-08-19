#ifndef global_h_thhyl_headerguard_SHENMADE_ZUI_TAOYAN_LE
#define global_h_thhyl_headerguard_SHENMADE_ZUI_TAOYAN_LE

LPCTSTR const g_title = _T("东方回映录");

extern LPCTSTR g_exefullpath;

inline LONG PointToLogical(int lPoint, CDC* dc)
{
	return -MulDiv(lPoint, dc->GetDeviceCaps(LOGPIXELSY), 72);
}
inline LONG PointToLogical(int lPoint, HDC hdc)
{
	return -MulDiv(lPoint, ::GetDeviceCaps(hdc, LOGPIXELSY), 72);
}
inline int LogicalToPoint(LONG lfHeight, HDC hdc)
{
	return MulDiv(-lfHeight, 72, ::GetDeviceCaps(hdc, LOGPIXELSY));
}
inline bool IsKeyDown(int vKey)
{
	return (GetAsyncKeyState(vKey) & 0x8000) ? true : false;
}

void LoadEditCtrlFont(CEdit*);
void SaveEditCtrlFont(CEdit* pEdit, LOGFONT* pLogFont);

//调用完之后要执行UpdateData(FALSE);
void ComboSel2Edit(CComboBox& cb, UINT& uEdit);

void Edit2ComboSel(UINT uEdit, CComboBox& cb);
void ComboAddString(CComboBox& cb, UINT CodePage);

//返回值是指向静态内存的LPCTSTR，不可 free() 或 delete!
LPCTSTR GetErrorMessage(UINT ErrorCode);

// 检查 RPY 文件大小
BOOL CheckRPYFileSize(HANDLE hFile, HWND hwnd);



BOOL DumpBinData(const BYTE* pData, DWORD nSize, LPCTSTR szFilename);

// 读取 UTF-16LE with BOM 编码文件的指定行
BOOL ReadUnicodeFileLine(LPCTSTR filename, int linenum, CString &outStrLine);
LPWSTR ReadLineW(LPWSTR s, int linenum);

CString LoadNotRpyString();

// lEnable = 1:on, 0:off, -1:toggle
// return GetLastError()
LONG EnableMaximize(HWND hwnd, LONG lEnable);


// 用于下面的 SetControlFontSize() 的 dwFlags 参数
#define SCFS_INCREMENTAL			(0x01)  // 将 n 看作一个增量，新字体大小 = 原字体大小 + n
#define SCFS_KEEPOLDHFONT			(0x02)  // 不要 DeleteObject() 原来的 HFONT
// 如果 pOutLogFont 不为 NULL，接收新的 LOGFONT
BOOL SetControlFontSize(HWND hwnd, int n, LOGFONT* pOutLogFont, DWORD dwFlags=0);
BOOL SetControlFontSize(CWnd* pWnd, int n, LOGFONT* pOutLogFont, DWORD dwFlags=0);

DWORD simple_random();

#endif // global_h_thhyl_headerguard_SHENMADE_ZUI_TAOYAN_LE
