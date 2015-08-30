#include "windows.h"
#include "tchar.h"
#include "Registry.h"

// LOG:
// 2013-07-01: no longer need MFC.

LONG WriteKey( const void *lpInData, HKEY hKey, LPCTSTR lpSubKey, LPCTSTR lpValue, DWORD dwType, DWORD cbData)
{
	HKEY hSubkey;

	LONG lResult = RegCreateKeyEx(hKey, lpSubKey, 0, NULL, REG_OPTION_NON_VOLATILE,
		KEY_WRITE, NULL, &hSubkey, NULL);
	if (lResult != ERROR_SUCCESS) {
		RegCloseKey(hSubkey);
		return lResult;
	}

	switch(dwType)
	{
		case REG_SZ:
		case REG_EXPAND_SZ:
		case REG_MULTI_SZ:
			if (cbData == 0) {
				cbData = _tcslen((LPCTSTR)lpInData)*sizeof(TCHAR);
				cbData += sizeof(TCHAR);
			}
	}

	lResult = RegSetValueEx(hSubkey, lpValue, 0, dwType, (CONST BYTE*)lpInData, cbData);
	RegCloseKey(hSubkey);
	return lResult;
}

LONG ReadKey( HKEY hKey, LPCTSTR lpSubKey, LPCTSTR lpValue, void *lpOutData)
{
	HKEY hSubkey;
	DWORD cbData;

	LONG lResult = RegOpenKeyEx(hKey, lpSubKey, 0, KEY_READ, &hSubkey);
	if (lResult != ERROR_SUCCESS) {
		RegCloseKey(hSubkey);
		return lResult;
	}

	lResult = RegQueryValueEx(hSubkey, lpValue, NULL, NULL, (LPBYTE)lpOutData, &cbData);
	RegCloseKey(hSubkey);
	return lResult;
}

LONG DeleteSubKeyTree(HKEY hKey, LPCTSTR lpSubKey)
{
	HKEY     hSubKey;
	TCHAR    szSubKey[512];
	FILETIME ft;
	DWORD    dwIndex, cbName;

	LONG lResult = RegOpenKeyEx(hKey, lpSubKey, 0, KEY_ALL_ACCESS,&hSubKey);
	if (lResult != ERROR_SUCCESS) {
		RegCloseKey(hSubKey);
		return lResult;
	}
	dwIndex = 0;
	cbName = sizeof(szSubKey)/sizeof(szSubKey[0]);
	while (ERROR_SUCCESS ==
				(lResult =
					RegEnumKeyEx(hSubKey, dwIndex,
					szSubKey, &cbName, NULL, NULL, NULL, &ft)
				)
		  )
	{
		DeleteSubKeyTree(hSubKey, szSubKey);
	}
	RegCloseKey(hSubKey);
	return RegDeleteKey(hKey, lpSubKey);
}

// 若 bForUser 非 0，在 keyname 前面加上 "Software\Classes"
//   并复制到一个 static string 后返回这个 static string
// 否则直接返回 keyname，不作任何修改
static LPCTSTR AdjustKeyName(LPCTSTR keyname, BOOL bForUser)
{
	if (bForUser) {
		static TCHAR newkeyname[384];
		LPCTSTR      user_prefix     = _T("Software\\Classes\\");

		_tcscpy(newkeyname, user_prefix);
		_tcscat(newkeyname, keyname);

		return newkeyname;
	}
	else
		return keyname;
}

static LONG ReadClassesKey(BOOL bForUser, LPCTSTR subkey, void* outdata)
{
	LPCTSTR const realSubkey = AdjustKeyName(subkey, bForUser);
	const HKEY hKey = bForUser ? HKEY_CURRENT_USER : HKEY_CLASSES_ROOT;

	return ReadKey(hKey, realSubkey, NULL, outdata);
}

static LONG WriteClassesKey(const void* indata, BOOL bForUser, LPCTSTR subkey)
{
	LPCTSTR const realSubkey = AdjustKeyName(subkey, bForUser);
	const HKEY    hKey       = bForUser ? HKEY_CURRENT_USER : HKEY_CLASSES_ROOT;

	return WriteKey(indata, hKey, realSubkey, NULL);
}


LONG SetAssociation(
		LPCTSTR FileExt,
		LPCTSTR FileTypeKey,
		LPCTSTR FileTypeName,
		LPCTSTR ExePath,
		LPCTSTR Parameters,
		int nDefaultIcon,
		BOOL bForUser
)
{
	TCHAR   FileType[256]             = {0};
	TCHAR   TestIfFileTypeExists[256] = {0};
	TCHAR	fooStr[256], fooStr2[384];
	LONG    ret;

	ReadClassesKey(bForUser, FileExt, FileType);
	if (FileType[0] == _T('\0')) {
		_tcscpy(FileType, FileTypeKey);
		WriteClassesKey(FileType, bForUser, FileExt);
	}

	//测试FileType所对应的子键是否存在。如果不存在，则使用传进来的参数作为子键。
	ret = ReadClassesKey(bForUser, FileType, TestIfFileTypeExists);
	if (ret != ERROR_SUCCESS)
		_tcscpy(FileType, FileTypeKey);

	//设置类型名
	WriteClassesKey(FileTypeName, bForUser, FileType);

	//设置程序关联
	_stprintf(fooStr, _T("%s\\Shell\\Open\\Command"), FileType);
	_stprintf(fooStr2, _T("\"%s\" \"%%1\" %s"), ExePath, Parameters);
	WriteClassesKey(fooStr2, bForUser, fooStr);

	//设置此文件类型的默认图标。
	_stprintf(fooStr, _T("%s\\DefaultIcon"), FileType);
	_stprintf(fooStr2, _T("\"%s\",%d"), ExePath, nDefaultIcon);
	return WriteClassesKey(fooStr2, bForUser, fooStr);
}

LONG DeleteAssociation(LPCTSTR FileExt, BOOL bForUser)
{
	TCHAR      FileType[256] = {0};
	const HKEY hKey          = bForUser ? HKEY_CURRENT_USER : HKEY_CLASSES_ROOT;

	ReadClassesKey(bForUser, FileExt, FileType);
	if (FileType[0] != 0)
		DeleteSubKeyTree(hKey, AdjustKeyName(FileType, bForUser));

	return DeleteSubKeyTree(hKey, AdjustKeyName(FileExt, bForUser));
}
