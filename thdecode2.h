#ifndef _THDECODE2_H_SHENZHU_V5
#define _THDECODE2_H_SHENZHU_V5

#include <time.h>
#include "thcommon.h"

// Touhou decode routines for
// TH95, TH10, TH11, TH12, TH125, TH128, TH13, TH14, alcostg, TH143
//
// written by wz520
// Last Update: 2015-08-28
//
// ------------ Changelog:
//
// 2015-08-16:
// * 增加 th15 支持。
// * 将 THRPYINFO2::pStageInfo 改成 union 以减少使用时强制转换的需要。
//
// 2014-06-03:
// * added: "nPosX" and "nPosY" data member for TH11_STAGEINFO, TH12_STAGEINFO,
//          TH128_STAGEINFO, TH13_STAGEINFO, TH14_STAGEINFO.
//
// 2013-09-09:
// * added: "dwNon20Bonus" data member for TH14_STAGEINFO.
// * added: THHEADER2 struct and RPYMGC2 enum definitions.
//
// 2013-06-30:
// * fix: renamed THRPYINFO2::dwVersion2 to dwGameVersion.
//
// 2013-05-27:
// * fix: changed type of TH11_STAGEINFO::dwContinuedTimes from char to DWORD.
//        Fortunately, due to the memory alignment, using char type didn't
//        cause errors.
// * add: TH10_STAGEINFO::dwComboGauge.
// * add: THAL_STAGEINFO, TH14_STAGEINFO.
// * add: THRPYINFO2::dwVersion2.
// * fix: In order to simplify the code, the argument pOutInfo to
//        ReplayDecode2() now CANNOT BE NULL.
//
// 2012-02-17:
// * add: some new data members to THRPYINFO2 structure
// * add: add wFlags to THRPYINFO2 structure
// * add: 'RPYFLAG2_*' flags for THRPYINFO2::wFlags
//

// ---------------------- RPYFLAGS2 ------------------------------
// Used by THRPYINFO2::wFlags, see it's description below for details

// "Don't use DirectInput for game pad input" was checked in custom.exe
#define RPYFLAG2_NODINPUT		0x0001

// ShotSlow was checked in custom.exe
#define RPYFLAG2_SHOTSLOW		0x0002

// "Show faith guage..." was checked in custom.exe, th10 only
#define RPYFLAG2_FAITHGUAGE		0x0004

// Indicates that the replay is actually a th14 trial/release version rpy.
// Since it's hard to distinguish between th13 and th14 replays by file
// header, if one of these flag is set, the caller should then treat the data
// as a th14 rpy.
#define RPYFLAG2_TH14TRIAL      0x0008
#define RPYFLAG2_TH14RELEASE    0x0010

// ---------------------- END RPYFLAGS2 --------------------------


// RPY Header struct
typedef struct tagTHHEADER2 {
    RPYMGC magic_number;
    WORD    rpy_version;
    WORD    padding;
    DWORD   unknown1; // usually 0
    DWORD   offsetUSER; // offset of the first USER block
    DWORD   game_version;
    DWORD   unknown2[2]; // usually 0
    DWORD   datasize;
    DWORD   decoded_datasize; // decompressed data size
}THHEADER2;

// Stage Info structs(except TH95, TH125, TH143)
// Currently members named "_u?" are unknown data

// Common header for each TH?_STAGEINFO struct
typedef struct tagTHHEADER_STAGEINFO {
    // start from 1
    //
    // for th128:
    // 1=a1-1,2=a1-2,3=a1-3,4=a2-2,5=a2-3,
    // 6=b1-1,7=b1-2,8=b1-3,9=b2-2...
    // and 16=ex
    WORD  wStageNumber;
    WORD  unused1;
    DWORD dwhdrKeyStateSize;
    DWORD dwhdrDataSize;
}THHDR_STAGEINFO;

typedef struct tagTHAL_STAGEINFO { // alcostg
    THHDR_STAGEINFO hdr;

    DWORD dwScore;
}THAL_STAGEINFO;

typedef struct tagTH10_STAGEINFO {
    THHDR_STAGEINFO hdr;

    DWORD dwScore;
    DWORD dwPower;          // /20 = integer part, %20 = decimal part
    DWORD dwFaith;          // no trailing zero
    DWORD dwComboGauge;     // 0~130
    DWORD dwPlayer;
    DWORD _u2;
    int   nPosX;            // /100+224, player position X, offset 0x24
    int   nPosY;            // /100+16
    char  _u3[0x1b4-0x2c];  // offset 0x2c
    DWORD dwContinuedTimes; // offset 0x1b4
}TH10_STAGEINFO;

typedef struct tagTH11_STAGEINFO {
    THHDR_STAGEINFO hdr;

    DWORD dwScore;
    DWORD dwPower;          // To show this value as a float value, like in the game:
                            // If the player character is MarisaA:
                            //   integer part = dwPower / 12
                            //   decimal part = dwPower % 12 * 100 / 12
                            // else:
                            //   integer part = dwPower / 20
                            //   decimal part = dwPower % 20 * 5
    DWORD dwConnect;        // bottom-left value of the screen
    short wPlayerInt;       // player integer part
    short wPlayerDec;       // player decimal part
    char  _u1[0x20-0x1c];   // offset 0x1c
    int   nPosX;            // /128+224, player position X, offset 0x20
    int   nPosY;            // /128+16
    DWORD dwContinuedTimes; // offset 0x28
    char  _u2[0x34-0x2c];   // offset 0x2c
    DWORD dwGraze;          // offset 0x34
}TH11_STAGEINFO;

typedef struct tagTH12_STAGEINFO {
    THHDR_STAGEINFO hdr;

    DWORD dwScore;
    DWORD dwPower;          // dwPower / 100 = integer, dwPower % 100 = decimal
    DWORD dwMaxScore;       // dwMaxScore/1000*10
    short wPlayerInt;       // player integer part
    short wPlayerDec;       // player decimal part
    short wBombInt;         // bomb integer
    short wBombDec;         // bomb decimal
    DWORD dwUFOStock[3];    // 0=None, 1=red, 2=blue, 3=green
    char  _u1[0x30-0x2c];   // offset 0x2c
    int   nPosX;            // /128+224, player position X, offset 0x30
    int   nPosY;            // /128+16
    DWORD dwContinuedTimes; // offset 0x38
    char  _u2[0x44-0x3c];   // offset 0x3c
    DWORD dwGraze;          // offset 0x44
}TH12_STAGEINFO;

typedef struct tagTH128_STAGEINFO {
    THHDR_STAGEINFO hdr;

    DWORD dwScore;
    DWORD dwLevel;         // dwLevel + 1
    DWORD dwFreezePower;   // dwFreezePower/1000
    int   nPosX;           // /128+320, player position X, offset 0x18
    int   nPosY;           // /128+16
    char  _u1[0x80-0x20];  // offset 0x20
    DWORD dwYaruki;        // dwYaruki/100, offset 0x80
    DWORD dwPerfectFreeze; // dwPerfectFreeze/100
    float fFrozenArea;     // fFrozenArea*100
}TH128_STAGEINFO;

// This TH13 stage info structure is based on 0.01a trial version.
// To get info from release version (e.g. 1.00a), some offsets must be corrected:
//      if a member offset in this struct is bigger than 0x30, then
//          offset+=4
//      else
//          no correction is needed :)
// For example, in 1.00a version, the offset of dwMaxScore is 0x38(0x34+4)
typedef struct tagTH13_STAGEINFO {
    THHDR_STAGEINFO hdr;

    int   nPosX;            // /128+224, player position X, offset 0x0c
    int   nPosY;            // /128+16
    DWORD _u1[2];           // offset 0x14
    DWORD dwScore;          // offset 0x1c
    DWORD _u2;              // offset 0x20
    DWORD dwContinuedTimes; // offset 0x24
    DWORD _u10;             // offset 0x28
    DWORD dwGraze;          // offset 0x2c
    DWORD _u3;              // offset 0x30
    DWORD dwMaxScore;       // same as th12, offset 0x34
    DWORD _u4[2];           // offset 0x38
    DWORD dwPower;          // same as th12, offset 0x40
    DWORD _u5[2];           // offset 0x44
    DWORD dwPlayer;         // offset 0x4c
    DWORD dwPlayerFragment; // offset 0x50

    // offset 0x54. An index number of an array(extend norms)
    // for 0.01a trial version, the array is [10,15,20,25,30,35,40]
    // for release version, it's [8,10,12,15,18,20,25]
    DWORD dwPlayerFragmentNormIndex; 

    DWORD dwBomb;            // offset 0x58
    DWORD dwBombFragment;    // offset 0x5c
    DWORD dwSpiritWorldGage; // 200, 400, 600
}TH13_STAGEINFO;

typedef struct tagTH14_STAGEINFO {
    THHDR_STAGEINFO hdr;

    int   nPosX;            // /128+224, player position X, offset 0x0c
    int   nPosY;            // /128+16
    DWORD _u1[2];           // offset 0x14
    DWORD dwScore;          // offset 0x1c
    DWORD _u2;              // offset 0x20
    DWORD dwContinuedTimes; // offset 0x24
    DWORD _u10;             // offset 0x28
    DWORD dwGraze;          // offset 0x2c
    DWORD _u3[2];           // offset 0x30
    DWORD dwMaxScore;       // offset 0x38
    DWORD _u4[2];           // offset 0x3c
    DWORD dwPower;          // offset 0x44
    DWORD _u5[2];           // offset 0x48
    DWORD dwPlayer;         // offset 0x50
    DWORD dwPlayerFragment; // offset 0x54
    DWORD dw1upCount;       // offset 0x58

    DWORD dwBomb;           // offset 0x5c
    DWORD dwBombFragment;   // offset 0x60
    char  _u6[0x80-0x64];   // offset 0x64
    DWORD dwNon20Bonus;     // offset 0x80, the counter for non-2.0 item bonus
}TH14_STAGEINFO;

typedef struct tagTH15_STAGEINFO {
    THHDR_STAGEINFO hdr;

    int   nPosX;            // /128+224, player position X, offset 0x0c
    int   nPosY;            // /128+16
    DWORD _u1[7];           // offset 0x14
	DWORD dwScore;			// offset 0x30
    DWORD _u2[3];           // offset 0x34
    DWORD dwGraze;          // offset 0x40
    DWORD _u3[5];           // offset 0x44
    DWORD dwMaxScore;       // offset 0x58
    DWORD _u4[2];           // offset 0x5c
    DWORD dwPower;          // offset 0x64
    DWORD _u5[3];           // offset 0x68
    DWORD dwPlayer;         // offset 0x74
    DWORD dwPlayerFragment; // offset 0x78
    DWORD dw1upCount;       // offset 0x7c

    DWORD dwBomb;           // offset 0x80
    DWORD dwBombFragment;   // offset 0x84
}TH15_STAGEINFO;

// TODO: 用于体验版。不知正式版是否会改变。
// NOTE: 从 dwScore 开始比 TH15 多偏移 4 bytes
typedef struct tagTH16_STAGEINFO {
    THHDR_STAGEINFO hdr;

    int   nPosX;            // /128+224, player position X, offset 0x0c
    int   nPosY;            // /128+16
    DWORD _u1[8];           // offset 0x14
	DWORD dwScore;			// offset 0x34
    DWORD _u2[3];           // offset 0x38
    DWORD dwGraze;          // offset 0x44
    DWORD _u3[5];           // offset 0x48
    DWORD dwMaxScore;       // offset 0x5c
    DWORD _u4[2];           // offset 0x60
    DWORD dwPower;          // offset 0x68
    DWORD _u5[3];           // offset 0x6c
    DWORD dwPlayer;         // offset 0x78
    DWORD _uPlayerFragment; // offset 0x7c  // 因为本结构与上面 TH15_STAGEINFO 相似，如果本作有命碎片的话估计就在这里
    DWORD dw1upCount;       // offset 0x80

    DWORD dwBomb;           // offset 0x84
    DWORD dwBombFragment;   // offset 0x88
    DWORD dwSeasonGauge;    // offset 0x8c  // 本作新要素“季节槽”
}TH16_STAGEINFO;

// for TH95/TH125/TH143
typedef struct tagTHHALF_INFO {
	// Item ID and count, currently available for TH143 only.
	int nDayID;   // zero based, need +1 for showing
	int nSceneID; // zero based, need +1 for showing
	DWORD nMainItemID;
	DWORD nMainItemCount;
	DWORD nSubItemID;
	DWORD nSubItemCount;
	DWORD nMainItemPower; // should /10 to get result.  %10 should == 0
}THHALF_INFO;

struct THRPYINFO2 : public THRPYINFO_BASE {
public:
    // General Info
    DWORD dwClearScore;
    WORD wVersion;       // replay file version number, differs from game version.
    DWORD dwGameVersion; // game version, useful for distinguishing th13/th14.
	DWORD dwDecodeSize;  // 文件解码后的大小

	// Player name: char[8] + '\0'
	// However, sometimes some garbage data would be written into this
	//   field and 'Name' in text format rpyinfo, which usually are 'Ai'.
	// Keep 12 bytes here.
	char szPlayerName[12];

	// Clear Time.
	// NOTE:
	//   For portability, using time_t.
	//   In most environment, this is a 32bit value.
	//   However using 64bit value would be better.
	time_t tClearTime;
	
	float fSlowRate;

	// Spell Card Number.  Available for TH95/TH125/TH13/TH14/TH143.
    // For other replay files, this value is always 0.
	// TH95/TH125:
	//   (Level-1) * (Scene-1) + 1
    int nSpellPracticeNumber;

	// receive additional replay info.
	// NOTE: not available for TH125
	// it consists of zero or more values defined as RPYFLAG2_*
    // use bitwise-and(&) to get
    WORD wFlags;

	// info for TH95/TH125/TH143
	THHALF_INFO halfinfo;

    // Currently, the following members are meaningless(always 0) for TH95 and TH125 replays
    // and, for alco replays, only nStageCount and dwLastStage are meaningful.
    int nStageCount;    // total count of the stage data
    DWORD dwID;         // player character id, zero-based. (0=reimu, 1=marisa...)
                        //   NOTE: for TH128, this is route id(0~6):
                        //   (0=A1, 1=A2, 2=B1, 3=B2, 4=C1, 5=C2, 6=EX)
    DWORD dwEquipID;    // character equipment id, zero-based. (0=1st equip...)
                        //   for TH13 and TH128, this is always 0
    DWORD dwDifficulty; // difficulty(0=easy, 1=normal, 2=hard,
                        //   3=lunatic, 4=extra, 5=overdrive(TH13 only))
    DWORD dwLastStage;  // the last stage(GAMEOVER stage) in the replay.
                        // For 'all clear' replay, use special 'all clear' value.
                        //   for alco replays, should be 1~4:
                        //     1~3=stage 1~3, 4=all clear.
                        //   for TH128 replays, should be 1~23:
                        //     1~16: see description for TH128_STAGEINFO::wStageNumber above
                        //     17=A1 all clear, 18=A2 all clear, 19=B1 all clear...
                        //       and 23=extra stage clear
                        //   for other replays, should be 1~8:
                        //     1~6=stage 1~6, 7=extra, 8=all clear.


	// 原来使用这里的 pStageInfo 是 void* 类型的，使用时大多需要强制转换
	// 现在使用 union 来尽量避免这个问题。
	// 使用时请选择一个正确的类型。
	// v 是 void* 类型。一般不推荐使用
    //
    // Do not free() these pointers,
    // just free() the pointer returned by ReplayDecode2()
	union {
		void* v[6]; 
		THAL_STAGEINFO* alco[6];
		TH10_STAGEINFO* th10[6];
		TH11_STAGEINFO* th11[6];
		TH12_STAGEINFO* th12[6];
		TH128_STAGEINFO* th128[3];
		TH13_STAGEINFO* th13[6];
		TH14_STAGEINFO* th14[6];
		TH15_STAGEINFO* th15[6];
		TH16_STAGEINFO* th16[6];
		THHDR_STAGEINFO* thhdr[6];
	 } pStageInfo;

	THRPYINFO2();

	inline bool isTrialVersion() {
		return dwGameVersion < 0x100;
	}
};

// ReplayDecode2()
// returns a pointer to the decoded data.
// The decoded data is allocated by malloc(), Remember to free() it if no longer needed.
// The decoded data should start with the player name of the .rpy file.
// If the replay data cannot be determined, it returns a NULL pointer.
BYTE* ReplayDecode2(
        BYTE* pRpyData,        // .rpy file data. Decrypted data will also be here
        DWORD  rpysize,        // .rpy file size
        THRPYINFO2* pInfo      // receive rpyinfo, **CANNOT BE NULL**
);

#endif /* _THDECODE2_H_SHENZHU_V5 */
