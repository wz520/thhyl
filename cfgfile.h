#ifndef __CFGFILE_H_51D0CCA3_
#     define __CFGFILE_H_51D0CCA3_

#include <string.h>

// ÓÃÓÚ dwOptions
#define CFG_MBTN				0x1
#define CFG_DBLCLK				0x2
#define CFG_CONFIRM				0x4
#define CFG_AUTOCOMP			0x8
#define CFG_SAVERAW_AUTOEXIT	0x10
#define CFG_SAVERAW_OVERWRITE	0x20
#define CFG_SAVERAW_INV			0x40
#define CFG_SHOWPLAYTIME		0x80000000
#define CFG_SHOWSLOWRATE		0x40000000
#define CFG_9					0x20000000
#define CFG_COPYOPENDEST		0x10000000  // open dest file after copying
#define CFG_ANYDRAG				0x8000000
#define CFG_AUTOEXIT			0x4000000   // auto exit if opened rpy no longer exists

struct CONFIG
{
    UINT            InfoCode;             // Â¼ÏñÐÅÏ¢±àÂë
    UINT            CommentCode;          // Â¼Ïñ×¢ÊÍ±àÂë
    UINT            CommentCodeForEdit;   // ×¢ÊÍ±à¼­¿òµÄÐ´Èë±àÂë
    LOGFONT         InfoFont;             // ÐÅÏ¢¿ò¼°×¢ÊÍ±à¼­¿òµÄ×ÖÌå
    DWORD           dwOptions;            // Ñ¡Ïî

	// since 1.40
    WINDOWPLACEMENT WinPlace;             // ´°¿ÚÎ»ÖÃ

	// since 1.70
	union {
		struct {
			BYTE byteAlpha;
		};
		DWORD _dontuse;
	};

	CONFIG& operator=(const CONFIG& other);

	CONFIG();

	BOOL loadFont(LOGFONT* pLogFont);
	void saveFont(LOGFONT* pLogFont);

	BOOL has(DWORD dwOption) const {return this->dwOptions & dwOption ? TRUE : FALSE;}
	void set(DWORD dwOption, BOOL bState);
};

extern CONFIG cfg;

BOOL LoadConfig();
BOOL SaveConfig();
void SetConfigFilename(LPCTSTR filename);

inline BOOL HasConfigOption(DWORD dwOption)
{
	return cfg.has(dwOption);
}

#endif // __CFGFILE_H_51D0CCA3_
