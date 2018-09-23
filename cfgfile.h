#ifndef __CFGFILE_H_51D0CCA3_
#     define __CFGFILE_H_51D0CCA3_

#include <string.h>

// ���� dwOptions
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
#define CFG_NOBALLOONTOOLTIP	0x2000000   // do not use balloon style tool tip

struct CONFIG
{
    UINT            InfoCode;             // ¼����Ϣ����
    UINT            CommentCode;          // ¼��ע�ͱ���
    UINT            CommentCodeForEdit;   // ע�ͱ༭���д�����
    LOGFONT         InfoFont;             // ��Ϣ��ע�ͱ༭�������
    DWORD           dwOptions;            // ѡ��

	// since 1.40
    WINDOWPLACEMENT WinPlace;             // ����λ��

	// since 1.70
	union {
		struct {
			BYTE byteAlpha;
			BYTE byteAlphaForFileList;
		};
		DWORD _dontuse;
	};

	// since 1.85
	WINDOWPLACEMENT WinPlace_FileList;   // �ļ��б���λ��

	CONFIG& operator=(const CONFIG& other);

	CONFIG();

	BOOL loadFont(LOGFONT* pLogFont);
	void saveFont(LOGFONT* pLogFont);

	BOOL has(DWORD dwOption) const {return (this->dwOptions & dwOption) ? TRUE : FALSE;}
	void set(DWORD dwOption, BOOL bState);

private:
	CONFIG(const CONFIG& another);
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
