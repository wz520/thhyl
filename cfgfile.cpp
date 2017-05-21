#include "stdafx.h"
#include "cfgfile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define CFGHEADER1			(0x464E4F43) //'CONF'
#define CFGHEADER2_170		(0xCCA2AA03) //Ver Info, before 1.71
#define CFGHEADER2			(0xCCD2A5BC) //Ver Info
// #define CFGOLDLENGTH		116          //.cfg file size before v1.40, ABANDONED
#define CFGOLDLENGTH		160          //.cfg file size before v1.70

CONFIG cfg;

CONFIG::CONFIG() {
	InfoCode               = 932;
	CommentCode            = 932;
	CommentCodeForEdit     = 0;
	InfoFont.lfFaceName[0] = 0;  // 无字体
	dwOptions              = CFG_CONFIRM | CFG_ANYDRAG | CFG_SHOWPLAYTIME | CFG_SHOWSLOWRATE;
	WinPlace.length        = 0;  // 无效 windowplacement
	_dontuse               = 0;
	byteAlpha              = (BYTE)255;
}

BOOL CONFIG::loadFont(LOGFONT* pLogFont)
{
	if (this->InfoFont.lfFaceName[0] != 0) {
		memcpy(pLogFont, &this->InfoFont, sizeof(LOGFONT));
		return TRUE;
	}
	return FALSE;
}

void CONFIG::saveFont(LOGFONT* pLogFont)
{
	memcpy(&this->InfoFont, pLogFont, sizeof(LOGFONT));
}

void CONFIG::set(DWORD dwOption, BOOL bState)
{
	if (bState) {
		this->dwOptions |= dwOption;
	}
	else {
		this->dwOptions &= ~dwOption;
	}
}


CONFIG& CONFIG::operator=(const CONFIG& other)
{
    if (this != &other) { // self-assignment check expected
		memmove(this, &other, sizeof(*this));
    }
    return *this;
}



/////////////////// END CONFIG implementation /////////////////////


static TCHAR cfgfilename[MAX_PATH] = {0};

struct CFGCONTENTS
{
	DWORD h1;
	DWORD h2;
	CONFIG cfg;
};

void SetConfigFilename(LPCTSTR filename)
{
	_tcsncpy(cfgfilename, filename, MAX_PATH);
	cfgfilename[MAX_PATH-1] = _T('\0');
}

BOOL LoadConfig()
{
	CFile cfile;

	if (!cfile.Open(cfgfilename, CFile::modeRead | CFile::shareDenyWrite | CFile::typeBinary))
		return FALSE;
	
	CFGCONTENTS cfgcontents;
	const UINT uBytesRead = cfile.Read(&cfgcontents, sizeof(cfgcontents));
	cfile.Close();


	//Check counts of bytes read and header
	if ( (uBytesRead == sizeof(cfgcontents) || uBytesRead == CFGOLDLENGTH)
			&& cfgcontents.h1==CFGHEADER1 && (cfgcontents.h2==CFGHEADER2 || cfgcontents.h2==CFGHEADER2_170) )
	{
		// 如果是 1.70 版的 CFG，则强制勾选 PlayTime 和 SlowRateByFPS
		if (cfgcontents.h2 == CFGHEADER2_170) {
			cfgcontents.cfg.dwOptions |= CFG_SHOWPLAYTIME | CFG_SHOWSLOWRATE;
		}

		// Use the configuration read from .cfg file
		cfg = cfgcontents.cfg;
		return TRUE;
	}
	else
		return FALSE;
}

BOOL SaveConfig()
{
	CFile cfile;
	
	if (!cfile.Open(cfgfilename, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyRead | CFile::typeBinary))
		return FALSE;

	CFGCONTENTS cfgcontents;
	cfgcontents.h1  = CFGHEADER1;
	cfgcontents.h2  = CFGHEADER2;
	cfgcontents.cfg = cfg;

	cfile.Write(&cfgcontents, sizeof(cfgcontents));
	cfile.Close();

	return TRUE;
}
