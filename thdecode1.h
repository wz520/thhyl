#ifndef __THDECODE1_H_
#define __THDECODE1_H_

#include "thcommon.h"
#include "thuserblock.h"
#include "RPYAnalyzer_userblock.h"

// Touhou decode routines for
// TH6, TH7, TH8, TH9
//
// written by wz520
// Last Update: 2015-08-28
//
// ------------ Changelog:
// see thdecode1.cpp

// ---------------------- RPYFLAGS ---------------------------------
// Used by THRPYINFO::wFlags, see it's description below for details

// 是否是中文版（渔场汉化版） rpy, TH07,08,09 only
#define RPYFLAG_CNVER			0x0001	

// ShotSlow 是否开启, TH07,08 only, TH06,TH09 不支持
// ShotSlow 就是按住射击键不放会自动低速，游戏中有相应的选项可以开启或关闭
#define RPYFLAG_SHOTSLOW		0x0002

// TH09 的 Charge Type: 0=SLOW, 1=CHARGE
// 2 个 player 可以不同，所以有 2 个 FLAG 。
// 而且因为和 SHOTSLOW 不会冲突（TH09 没有 SHOTSLOW），
//   所以 P1CHARGE 和 SHOTSLOW 使用相同的 bit 来表示
#define RPYFLAG_P1CHARGE		0x0002
#define RPYFLAG_P2CHARGE		0x0004

// ---------------------- END RPYFLAGS -----------------------------

// ReplayDecode() Return values
typedef enum tagRPYINFOERR {
	RPYINFO_OK = 0,
    RPYINFO_UNKNOWNFORMAT, // header is invalid(not TH6~TH9).
    RPYINFO_DECRYPTERROR,  // The file may be modified. Or unsupported version
    RPYINFO_MALLOCERROR    // malloc() returned NULL.
}RPYINFOERR;

// used by TH7-TH9
typedef struct tagTH_DECOMPINFO{
    DWORD  offset;   // [in] Offset of the decompressed data size in the decrypted data
    int    ibase;    // [in] Start offset
    BYTE  *pData;    // [out] Pointer to the decompressed data(allocated by malloc())
    DWORD  size;     // [out] Size of the decompressed data
}TH_DECOMPINFO;


// -------- StageInfo --------------
//TH6
typedef struct tagTH6_STAGEINFO{
	DWORD dwScores;
	WORD  wRandomSeed;
	WORD  wPoints;      // 'Point' count from game start
						// Seems to be not actually used by the game system, just a counter
	BYTE  cPowers;
	BYTE  cLives;
	BYTE  cBombs;
	BYTE  cPlayRank;
}TH6_STAGEINFO;
typedef struct tagTH6_RPYINFO{
	char  szDate[9];         // date
	char  szPlayer[9];       // player name
	BYTE  cGameMinorVersion; // for ver 1.02, this should be 2
	BYTE  cGameMajorVersion; // for ver 1.02, this should be 1
	BYTE  cChara;            // chara(0:ReimuA 1:ReimuB 2:MarisaA 3:MarisaB)
	BYTE  cRank;             // rank(0:Easy 1:Normal 2:Hard 3:Lunatic 4:Extra)
	DWORD dwScore;           // score
	float fDrop;             // drop
}TH6_RPYINFO;

//TH7
typedef struct tagTH7_STAGEINFO{
	DWORD dwScores;
	int   nPoints;
	int   nCurrCherries;
	int   nMaxCherries;
	int   nCurrCherriesPlus;
	int   nGrazes;
    int   n1upByPointsCount; // 由point奖命的次数。
    int   nNextPoints;
    WORD  wRandomSeed;       // offset 0x20
    BYTE  cPowers;
    BYTE  cLives;
    BYTE  cBombs;            // offset 0x24
	BYTE  cPlayRank;
	BYTE  cUnknown;
	BYTE  cSpellCardBonusCount;
}TH7_STAGEINFO;
typedef struct tagTH7_RPYINFO{
	char  szDate[6];	// date
	char  szPlayer[9];	// player name
	BYTE  cChara;		// chara(0:ReimuA 1:ReimuB 2:MarisaA 3:MarisaB 4:SakuyaA 5:SakuyaB)
	BYTE  cRank;		// rank(0:Easy 1:Normal 2:Hard 3:Lunatic 4:Extra 5:Phantasm)
	DWORD dwScore;		// score
	char  szVersion[7];	// version
	float fDrop;		// drop
}TH7_RPYINFO;

//TH8
typedef struct tagTH8_STAGEINFO {
	DWORD dwScores;
	int   nPoints;
	int   nGrazes;
	int   n1upByPointsCount;    // 由point奖命的次数。
	int   nNextPoints;			// offset 0x10
	int   nNightPoints;			// offset 0x14
	short sYaoLv;               // 妖率。其十进制表示的低两位是游戏中的小数部分，其他是整数部分。
                                // 例：-10000 即游戏中的 -100.00%
	WORD  wRandomSeed;
	BYTE  cPowers;				// offset 0x1c
	BYTE  cLives;
	BYTE  cBombs;
	BYTE  cPlayRank;
	BYTE  cUnknown;				// offset 0x20
	BYTE  cSpellCardBonusCount;
	BYTE  cGameTime;            // 游戏内的时刻。0 = PM 11:00。
}TH8_STAGEINFO;

//TH9
typedef struct tagTH9_STAGEINFO {
	DWORD dwScores; // stage start score, not clear score
	WORD  wRandomSeed;

	// const char* CharaList[16]={
	//	"Reimu"  , "Marisa"  , "Sakuya", "Youmu"   ,
	//	"Reisen" , "Cirno"   , "Lyrica", "Mystia"  ,
	//	"Tewi"   , "Yuka"    , "Aya"   , "Medicine",
	//	"Komachi", "Sikieiki", "Merlin", "Lunasa"
	// };
    BYTE cChara;         // zero-based index of CharaList array

    BYTE cCPUPlayer;     // Is CPU Player? 1=Yes, 0=No
	BYTE cLives;         // offset 0x08

	// 对战场所。 只用于 Match 模式，其他模式中总是 0 。
	// 并且只有 player1 的值有意义，player2 的结构中该值同样总是 0 。
	//
	// 【注意】：必须先将该值 + 2，然后才是 PlaceList 数组索引。
	// LPCTSTR PlaceList[16] = {
	//  _T("どっか")          , _T("規定の場所")       ,
	//  _T("迷いの竹林 REIMU"), _T("迷いの竹林 MARISA"),
	//  _T("幻草原 SAKUYA")   , _T("白玉楼階段 YOUMU") ,
	//  _T("永遠亭 REISEN")   , _T("霧の湖 CIRNO")     ,
	//  _T("幽明結界 LYRICA") , _T("妖怪獣道 MYSTIA")  ,
	//  _T("迷いの竹林 TEI")  , _T("太陽の畑 YUKA")    ,
	//  _T("大蝦蟇の池 AYA")  , _T("無名の丘 MEDICINE"),
	//  _T("再思の道 KOMACHI"), _T("無縁塚 SIKIEIKI")
	// }
	signed char cPlace;  
}TH9_STAGEINFO;

class THRPYINFO : public THRPYINFO_BASE {
public:
	// 解码后的数据。应该与 DecompInfo.pData 指向同一个地方。
	// 调用 ReplayDecode() 成功后，当不再需要时要用 ReplayDecode_clean() 释放。
	BYTE* pDecodeData;
	
	// 解码后数据的大小，不包括附加部分（即文本格式的录像信息和注释）。与 DecompInfo.size 应该相同。
	// 对于 TH6，这个数应该和 rpy 文件的大小一样。
	DWORD dwDecodeSize;
	
	// 录像文件的开头4个字节，指示录像属于哪个游戏。
	// 无论 ReplayDecode() 是否成功，该值都会被填充。
	// 若成功，应该为 hdr6, hdr7, hdr8, hdr9 中的一个。
	RPYMGC header;

	// 录像版本号，录像文件偏移 0x04 处的一个 WORD。
	// 目前只知道以下录像版本号所对应的游戏版本:
	// TH6 1.02h: 0x0102
	// TH7 1.00b: 0x1100
	// TH8 1.00d: 0x0006
	// TH9 1.50a: 0x0002
	// 不过，别的游戏版本生成的录像也可能是上述录像版本号，具体不明... >_<
	WORD  wVersion;

	// RPYFLAG_ 开头的一些 FLAG。
	// 包含有关 RPY 的其他信息，例如是否是中文版录像
	// 每个 FLAG 占 1 个 bit，使用 位与(&) 来获取。
	WORD  wFlags;

	// rpy 文件的摘要信息。
	// 只有 TH6, TH7 才可用。
	// 这是 union, 请根据需要使用其中一个
	// 注意：该成员已经不再是指针，名字开头的 p 也去掉了
	union {
		TH6_RPYINFO th6;
		TH7_RPYINFO th7;
	}GeneralInfo;

	// 关卡数
	int nStageCount;
	
	// pStageInfo: 指向各关数据的指针数组
	// 原本使用时需要强制转换，现在只需根据实际情况选择其一使用。

	// 用于通过 [玩家索引(0或1)][关卡索引] 来访问 th9 的每一关数据
	typedef TH9_STAGEINFO* (*TH9_STAGEINFO_BY_PLAYER)[10];
    union pStageInfo {
		void* v[20];
		TH6_STAGEINFO* th6[7];  // 1,2,3,4,5,6,EX
		TH7_STAGEINFO* th7[7];  // 1,2,3,4,5,6,EX/PH
		TH8_STAGEINFO* th8[9];  // 1,2,3,4A,4B,5,6A,6B,EX
		TH9_STAGEINFO* th9[20]; // Player1 的 1,2,3,4,5,6,7,8,9,Match，Player2 的 1,2,3,4,5,6,7,8,9,Match, 共 20

		// 建议使用该成员获取TH9的数据。举个栗子：th9_by_player[1][5]=玩家2的第四关数据。
		// 如果使用上面的 th9，则要写成 th9[15]。
		TH9_STAGEINFO* th9_by_player[2][10];
	} pStageInfo;

	// 各关的数据长度，以 byte 为单位。同样有 20 个。
	// 如果 pStageInfo[n] 为 NULL，dwStageSizes[n] 为 0 。
	DWORD dwStageSizes[20];

	// 各关的总帧数，20 个
	DWORD dwFrameCounts[20];

    TH_DECOMPINFO  DecompInfo; // 解压相关的结构


	THRPYINFO();
};

// ReplayDecode()
//
// Returns RPYINFOERR enum that indicates an error.
// If no error occurred, it returns RPYINFO_OK.
// See the definition of enum RPYINFOERR(beginning of this file) for details.
//
// Upon success(RPYINFO_OK):
//	pRpyData will point to the decrypted(not decompressed) data.
//		The original data will be overwritten!
//	when pInfo is no longer needed,
//		you must call ReplayDecode_clean(pInfo) to free the memory
//		that's allocated for the pointers inside *pInfo by ReplayDecode().
// Upon failure:
//  If the return value is RPYINFO_UNKNOWNFORMAT, the data in *pRpyData will 
// not be modified, and all members in *pInfo will be zero-filled.
//  If it returns other values, the data in *pData is unpredictable.
//	No need to call ReplayDecode_clean().
// Whatever the result, the extra sections(text format rpyinfo,
//		comment) will not be affected.
//
// 若无错误，返回 RPYINFO_OK 。
// 否则返回 RPYINFOERR enum 里的其他值。
//
// 若成功(RPYINFO_OK)：
//	pRpyData 将指向解密（非解压）后的数据。原有数据被覆盖！
//		所以如果你还需要原 rpy 文件的数据，需要事先备份！
//	当使用完 pInfo 后，必须调用下面的 ReplayDecode_clean(pInfo)
//		以释放 ReplayDecode() 为 *pInfo 结构体中的各种指针所分配的内存
//		（并且会将它们置为 NULL。当然 *pInfo 本身占据的内存是不会释放的，
//		因为这不是 ReplayDecode() 分配的）。
// 若失败：
//  若返回 RPYINFO_UNKNOWNFORMAT，*pRpyData 的数据不会作任何更改。
// 并且 *pInfo 里的所有成员都被清零。
//  若是其他返回值，则 *pRpyData 里的数据不可预知（可能被修改也可能没被修改）。
//	无需调用 ReplayDecode_clean() ，不过 *pInfo 里的任何东西都不应该被使用。
// 对于 TH8 和 TH9，无论结果如何，都不会影响 .rpy 文件的附加部分（文件末尾的
// 文本格式的录像信息和注释）
RPYINFOERR ReplayDecode(
        BYTE* pRpyData,  // .rpy file data. Decrypted data will also be here.
        DWORD  rpysize,  // .rpy file size
        THRPYINFO* pInfo // receive rpyinfo, must not be NULL.
);

// ReplayDecode_clean(THRPYINFO*)
// Free the memory that's allocated for the pointers inside THRPYINFO structure by ReplayDecode(),
// and set them to NULL.
void ReplayDecode_clean(THRPYINFO* pInfo);

#endif //__THDECODE1_H_
