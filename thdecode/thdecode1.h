#ifndef __THDECODE1_H_
#define __THDECODE1_H_

#include "thcommon.h"
#include "thuserblock.h"

// Touhou decode routines for
// TH6, TH7, TH8, TH9
//
// written by wz520
//
// ------------ Changelog:
// see thdecode1.cpp

// ---------------------- RPYFLAGS ---------------------------------
// Used by THRPYINFO::wFlags, see it's description below for details

// �Ƿ������İ棨�泡�����棩 rpy, TH07,08,09 only
#define RPYFLAG_CNVER			0x0001	

// ShotSlow �Ƿ���, TH07,08 only, TH06,TH09 ��֧��
// ShotSlow ���ǰ�ס��������Ż��Զ����٣���Ϸ������Ӧ��ѡ����Կ�����ر�
#define RPYFLAG_SHOTSLOW		0x0002

// TH09 �� Charge Type: 0=SLOW, 1=CHARGE
// 2 �� player ���Բ�ͬ�������� 2 �� FLAG ��
// ������Ϊ�� SHOTSLOW �����ͻ��TH09 û�� SHOTSLOW����
//   ���� P1CHARGE �� SHOTSLOW ʹ����ͬ�� bit ����ʾ
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
    int   n1upByPointsCount; // ��point�����Ĵ�����
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
	int   n1upByPointsCount;    // ��point�����Ĵ�����
	int   nNextPoints;			// offset 0x10
	int   nNightPoints;			// offset 0x14
	short sYaoLv;               // ���ʡ���ʮ���Ʊ�ʾ�ĵ���λ����Ϸ�е�С�����֣��������������֡�
                                // ����-10000 ����Ϸ�е� -100.00%
	WORD  wRandomSeed;
	BYTE  cPowers;				// offset 0x1c
	BYTE  cLives;
	BYTE  cBombs;
	BYTE  cPlayRank;
	BYTE  cUnknown;				// offset 0x20
	BYTE  cSpellCardBonusCount;
	BYTE  cGameTime;            // ��Ϸ�ڵ�ʱ�̡�0 = PM 11:00��
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

	// ��ս������ ֻ���� Match ģʽ������ģʽ������ 0 ��
	// ����ֻ�� player1 ��ֵ�����壬player2 �Ľṹ�и�ֵͬ������ 0 ��
	//
	// ��ע�⡿�������Ƚ���ֵ + 2��Ȼ����� PlaceList ����������
	// LPCTSTR PlaceList[16] = {
	//  _T("�ɤä�")          , _T("Ҏ���Έ���")       ,
	//  _T("�Ԥ������� REIMU"), _T("�Ԥ������� MARISA"),
	//  _T("�ò�ԭ SAKUYA")   , _T("����¥�A�� YOUMU") ,
	//  _T("���hͤ REISEN")   , _T("�F�κ� CIRNO")     ,
	//  _T("�����Y�� LYRICA") , _T("���֪��� MYSTIA")  ,
	//  _T("�Ԥ������� TEI")  , _T("̫ꖤήx YUKA")    ,
	//  _T("���r�W�γ� AYA")  , _T("�o������ MEDICINE"),
	//  _T("��˼�ε� KOMACHI"), _T("�o�F�V SIKIEIKI")
	// }
	signed char cPlace;  
}TH9_STAGEINFO;

class THRPYINFO : public THRPYINFO_BASE {
public:
	// ���������ݡ�Ӧ���� DecompInfo.pData ָ��ͬһ���ط���
	// ���� ReplayDecode() �ɹ��󣬵�������ҪʱҪ�� ReplayDecode_clean() �ͷš�
	BYTE* pDecodeData;
	
	// ��������ݵĴ�С�����������Ӳ��֣����ı���ʽ��¼����Ϣ��ע�ͣ����� DecompInfo.size Ӧ����ͬ��
	// ���� TH6�������Ӧ�ú� rpy �ļ��Ĵ�Сһ����
	DWORD dwDecodeSize;
	
	// ¼���ļ��Ŀ�ͷ4���ֽڣ�ָʾ¼�������ĸ���Ϸ��
	// ���� ReplayDecode() �Ƿ�ɹ�����ֵ���ᱻ��䡣
	// ���ɹ���Ӧ��Ϊ hdr6, hdr7, hdr8, hdr9 �е�һ����
	RPYMGC header;

	// ¼��汾�ţ�¼���ļ�ƫ�� 0x04 ����һ�� WORD��
	// Ŀǰֻ֪������¼��汾������Ӧ����Ϸ�汾:
	// TH6 1.02h: 0x0102
	// TH7 1.00b: 0x1100
	// TH8 1.00d: 0x0006
	// TH9 1.50a: 0x0002
	// �����������Ϸ�汾���ɵ�¼��Ҳ����������¼��汾�ţ����岻��... >_<
	WORD  wVersion;

	// RPYFLAG_ ��ͷ��һЩ FLAG��
	// �����й� RPY ��������Ϣ�������Ƿ������İ�¼��
	// ÿ�� FLAG ռ 1 �� bit��ʹ�� λ��(&) ����ȡ��
	WORD  wFlags;

	// rpy �ļ���ժҪ��Ϣ��
	// ֻ�� TH6, TH7 �ſ��á�
	// ���� union, �������Ҫʹ������һ��
	// ע�⣺�ó�Ա�Ѿ�������ָ�룬���ֿ�ͷ�� p Ҳȥ����
	union {
		TH6_RPYINFO th6;
		TH7_RPYINFO th7;
	}GeneralInfo;

	// �ؿ���
	int nStageCount;
	
	// pStageInfo: ָ��������ݵ�ָ������
	// ԭ��ʹ��ʱ��Ҫǿ��ת��������ֻ�����ʵ�����ѡ����һʹ�á�

	// ����ͨ�� [�������(0��1)][�ؿ�����] ������ th9 ��ÿһ������
	typedef TH9_STAGEINFO* (*TH9_STAGEINFO_BY_PLAYER)[10];
    union pStageInfo {
		void* v[20];
		TH6_STAGEINFO* th6[7];  // 1,2,3,4,5,6,EX
		TH7_STAGEINFO* th7[7];  // 1,2,3,4,5,6,EX/PH
		TH8_STAGEINFO* th8[9];  // 1,2,3,4A,4B,5,6A,6B,EX
		TH9_STAGEINFO* th9[20]; // Player1 �� 1,2,3,4,5,6,7,8,9,Match��Player2 �� 1,2,3,4,5,6,7,8,9,Match, �� 20

		// ����ʹ�øó�Ա��ȡTH9�����ݡ��ٸ����ӣ�th9_by_player[1][5]=���2�ĵ��Ĺ����ݡ�
		// ���ʹ������� th9����Ҫд�� th9[15]��
		TH9_STAGEINFO* th9_by_player[2][10];
	} pStageInfo;

	// ���ص����ݳ��ȣ��� byte Ϊ��λ��ͬ���� 20 ����
	// ��� pStageInfo[n] Ϊ NULL��dwStageSizes[n] Ϊ 0 ��
	DWORD dwStageSizes[20];

	// ���ص���֡����20 ��
	DWORD dwFrameCounts[20];

    TH_DECOMPINFO  DecompInfo; // ��ѹ��صĽṹ


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
// ���޴��󣬷��� RPYINFO_OK ��
// ���򷵻� RPYINFOERR enum �������ֵ��
//
// ���ɹ�(RPYINFO_OK)��
//	pRpyData ��ָ����ܣ��ǽ�ѹ��������ݡ�ԭ�����ݱ����ǣ�
//		��������㻹��Ҫԭ rpy �ļ������ݣ���Ҫ���ȱ��ݣ�
//	��ʹ���� pInfo �󣬱����������� ReplayDecode_clean(pInfo)
//		���ͷ� ReplayDecode() Ϊ *pInfo �ṹ���еĸ���ָ����������ڴ�
//		�����һὫ������Ϊ NULL����Ȼ *pInfo ����ռ�ݵ��ڴ��ǲ����ͷŵģ�
//		��Ϊ�ⲻ�� ReplayDecode() ����ģ���
// ��ʧ�ܣ�
//  ������ RPYINFO_UNKNOWNFORMAT��*pRpyData �����ݲ������κθ��ġ�
// ���� *pInfo ������г�Ա�������㡣
//  ������������ֵ���� *pRpyData ������ݲ���Ԥ֪�����ܱ��޸�Ҳ����û���޸ģ���
//	������� ReplayDecode_clean() ������ *pInfo ����κζ�������Ӧ�ñ�ʹ�á�
// ���� TH8 �� TH9�����۽����Σ�������Ӱ�� .rpy �ļ��ĸ��Ӳ��֣��ļ�ĩβ��
// �ı���ʽ��¼����Ϣ��ע�ͣ�
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
