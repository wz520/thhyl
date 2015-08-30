#ifndef __CPCONV_H__5387D040_
#     define __CPCONV_H__5387D040_

// Win32 Codepage conversion functions

#include <windows.h>

/*
 * outSizeW: 获取转换后的长度，留空或为NULL则不获取。长度包含\0
 * cbMultiByte: 要转换的字符串长度。
 *		-1 表示 \0 终止，否则只转换指定长度的字节。
 *		无论如何，转换后的 Unicode 字符串总是空终止的。
 * HINT: 返回值是用new分配的LPWSTR，用完了别忘了delete。
 */
LPWSTR Ansi2Unicode(LPCSTR lpszSrc, UINT CodePage, int* outSizeW, int cbMultiByte);

LPSTR Unicode2Ansi(LPCWSTR lpszSrcW, UINT CodePage, size_t *outSize, BOOL* bUsedDefaultChar);

#endif /* __CPCONV_H__5387D040_ */
