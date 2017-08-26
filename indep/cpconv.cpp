#include "cpconv.h"

LPWSTR Ansi2Unicode(LPCSTR lpszSrc, UINT CodePage, int* outSizeW, int cbMultiByte)
{
	const int nSize = MultiByteToWideChar(CodePage, 0, lpszSrc, cbMultiByte, NULL, 0);
	if (nSize == 0)
		return NULL;

	LPWSTR lpszSrcw = new WCHAR[nSize+1];
	
	MultiByteToWideChar(CodePage, 0, lpszSrc, cbMultiByte, lpszSrcw, nSize*2);

	if (outSizeW != NULL)
		*outSizeW = nSize;

	if (cbMultiByte != -1) // Not null terminated
		lpszSrcw[nSize] = 0; // add NULL

	return lpszSrcw;
}

LPSTR Unicode2Ansi(LPCWSTR lpszSrcW, UINT CodePage, size_t *outSize, BOOL* bUsedDefaultChar)
{
	const int nSize = WideCharToMultiByte(CodePage, 0, lpszSrcW, -1, NULL, 0, NULL, NULL);
	if (nSize == 0)
		return NULL;

	LPSTR lpszSrc = new CHAR[nSize + 1];
	WideCharToMultiByte(CodePage, 0, lpszSrcW, -1, lpszSrc, nSize, NULL,
		(CodePage==CP_UTF8 || CodePage==CP_UTF7)
			? NULL
			: bUsedDefaultChar
	);
	
	if (outSize != NULL)
		*outSize = nSize;
	
	return lpszSrc;
}
