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
	LPCTSTR os = _T("(δ֪)");
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
						? _T("10") : _T("Server Technical Preview");
			}
			break;
	}

	return os;
}

LPCTSTR OSVerGetCSDVersion()
{
	return ovi_ex.szCSDVersion;
}
