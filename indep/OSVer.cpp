#include <windows.h>
#include <tchar.h>

#ifndef SM_SERVERR2
#define SM_SERVERR2 89
#endif /* SM_SERVERR2 */

static OSVERSIONINFOEX ovi_ex = {sizeof(OSVERSIONINFOEX)};

void OSVerInit()
{
	GetVersionEx((OSVERSIONINFO*)&ovi_ex);
}

BOOL OSVerAboveXP_SP2()
{
	return
		(ovi_ex.dwMajorVersion == 5 && ovi_ex.dwMinorVersion == 1 &&
			ovi_ex.wServicePackMajor >= 2)
		|| (ovi_ex.dwMajorVersion == 5 && ovi_ex.dwMinorVersion > 1 )
		|| ovi_ex.dwMajorVersion > 5;
}

LPCTSTR OSVerGetOSString()
{
	// 在一个GBK编码的文件中，如果多字节字符只有汉字“未知”这两个字，貌似会使很多编辑器会误认为UTF8而导致显示为乱码。
	// 所以本来这文件应该不用写注释的，不得已写了这两行中文注释 一_,一
	LPCTSTR os = _T("(未知)");
	const DWORD minor_ver = ovi_ex.dwMinorVersion;

	switch ( ovi_ex.dwMajorVersion )
	{
		case 5 :
			switch ( minor_ver )
			{
				case 0 :
					return _T("2000");
				case 1 :
					return _T("XP");
				case 2 :
					return GetSystemMetrics( SM_SERVERR2 )
						? _T("Server 2003 R2") : _T("Server 2003");
			}
			break;
		case 6 :
			switch ( minor_ver )
			{
				case 0 :
					return ovi_ex.wProductType == VER_NT_WORKSTATION
						? _T("Vista") : _T("Server 2008");
				case 1 :
					return ovi_ex.wProductType == VER_NT_WORKSTATION
						? _T("7") : _T("Server 2008 R2");
				case 2 :
					return ovi_ex.wProductType == VER_NT_WORKSTATION
						? _T("8") : _T("Server 2012");
				case 3 :
					return ovi_ex.wProductType == VER_NT_WORKSTATION
						? _T("8.1") : _T("Server 2012 R2");
			}
			break;
		case 10 :
			switch ( minor_ver )
			{
				case 0 :
					return ovi_ex.wProductType == VER_NT_WORKSTATION
						? _T("10") : _T("Server 2016");
			}
			break;
	}

	return os;
}

LPCTSTR OSVerGetCSDVersion()
{
	return ovi_ex.szCSDVersion;
}
