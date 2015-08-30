#ifndef __CLIPBOARD_H_5227B41E_
#     define __CLIPBOARD_H_5227B41E_

BOOL SetClipTextW(LPCWSTR unitext);

// BCopy==TRUE: Copy
// BCopy==FALSE: Cut
BOOL CopyFileToClipboardW(LPCWSTR szFileName, BOOL bCopy);

#endif /* __CLIPBOARD_H_5227B41E_ */
