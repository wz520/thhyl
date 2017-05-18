#include "StdAfx.h"
#include "cfgfile.h"
#include "thhyl.h"
#include "RPYAnalyzer.h"
#include "global.h"
#include "cpconv.h"

#include "RPYAnalyzer_common.h"
#include "RPYAnalyzer_formatter.h"

#include "filepath_utils.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define lengthof(arr)				(sizeof(arr)/sizeof(arr[0]))

#define free_then_null(v)			(free(v), (v)=NULL)
#define delete_then_null(v)			((delete (v)), (v)=NULL)

static LPCTSTR const STR_ERRORSTAGEINFO =_T("\r\n獲取各關數據失敗！\r\n");
static LPCTSTR const STR_ERROR =_T("Error!");

static LPCTSTR const common_szPlayTime = _T("     PlayTime: %7u(%d:%02d.%03d)\r\n");
static LPCTSTR const common_szSlowRate = _T("SlowRateByFPS: %f%%\r\n");
static LPCTSTR const common_szTotalPlayTime = _T("\r\nTotal PlayTime: %u(%d:%02d.%03d)");
static LPCTSTR const common_szTotalSlowRate = _T("\r\nTotal SlowRateByFPS: %f%%");


// 坐标转换函数
static inline int transPosX(int x) {return x/128+224;}     // 非 TH10 且 非 黄昏酒厂
static inline int transPosY(int y) {return y/128+16;}      // 非 TH10 且 非 黄昏酒厂
static inline int transTH10PosX(int x) {return x/100+224;} // 黄昏酒场也适用
static inline int transTH10PosY(int y) {return y/100+16;}  // 黄昏酒场也适用

using namespace RPYAnalyzer_formatter;

//--- Implementation of RPYAnalyzer.h ---

CRPYAnalyzer::CRPYAnalyzer(BYTE* data, DWORD size, UINT DefCode)
	:	m_info(), m_UBRpyInfo() 
{
	m_pData     = (BYTE*)malloc(size);
	m_nDataSize = size;
	memcpy(m_pData, data, size);

	m_uDefCode = DefCode;
	m_header   = mgcnone;

	m_pTHRpyInfo  = NULL;
	m_pTHRpyInfo2 = NULL;
	m_pDecompData = NULL;

	m_pUBMgr = new RPYUserBlockMgr(data, size);
	m_pInfoUB = NULL;
}

CRPYAnalyzer::~CRPYAnalyzer()
{
	m_header = mgcnone;

	if ( m_pTHRpyInfo ) {
		ReplayDecode_clean(m_pTHRpyInfo);
		delete_then_null(m_pTHRpyInfo);
	}

	free_then_null(m_pDecompData);
	delete_then_null(m_pTHRpyInfo2);
	free_then_null(m_pData);

	delete_then_null(m_pUBMgr);
	delete_then_null(m_pInfoUB);
}

// 获取 6, 7, 8, 9 各关数据 (6, 7 包括整体信息)
int CRPYAnalyzer::GenInfoWrapper()
{
	m_pTHRpyInfo = new THRPYINFO; // will be deleted in destructor
	int ret = (int)ReplayDecode(m_pData, m_nDataSize, m_pTHRpyInfo);

	if ( ret != RPYINFO_UNKNOWNFORMAT ) { // header is valid
		m_header = m_pTHRpyInfo->header;
		if ( ret == RPYINFO_OK ) { // OK
			switch ( m_header )
			{
			case mgc6: TH6GenInfo(); break;
			case mgc7: TH7GenInfo(); break;
			case mgc8: TH8GenStageInfo(); break;
			case mgc9: TH9GenStageInfo(); break;
			}

			// ReplayDecode_clean(m_pTHRpyInfo); // must call this function to free memory
			// But here this is needless because the destructor will do it
		}
		else { // decode error or malloc error
			if ( m_header == mgc6 || m_header == mgc7 ) {
				m_info.Format( CString((LPCTSTR)IDS_TH67DECERROR),
					m_header == mgc6 ? _T("红魔乡") : _T("妖妖梦"));
			}
		}
	}

	if ( ret!=RPYINFO_OK && (m_header==mgc8 || m_header==mgc9) )
		m_info += STR_ERRORSTAGEINFO;

	return ret;
}

// 获取 10 以后的以及 alcostg 各关数据
int CRPYAnalyzer::GenInfoWrapper2()
{
	RPYMGC const Header = *((RPYMGC*)m_pData);
	m_pTHRpyInfo2 = new THRPYINFO2; // will be deleted in destructor

	// NOTE: 1.70 及以前的版本这里第一个参数本来写成了 m_pTHRpyInfo，是一个小 BUG。
	// 不过貌似没发生过啥问题……
	
	struct STAGEINFOFUNCMAP { //函数表
		RPYMGC header;
		void (CRPYAnalyzer::*pFunc)();  // can be NULL
	}StageInfoFuncMap[]={
		{mgc95,   TH95GenInfo}, // no stageinfo
		{mgc10,   TH10GenStageInfo},
		{mgc11,   TH11GenStageInfo},
		{mgc12,   TH12GenStageInfo},
		{mgc125,  TH125GenInfo}, // no stageinfo
		{mgc128,  TH128GenStageInfo},
		{mgc13,   TH13GenStageInfo}, // include 13,14
		{mgc15,   TH15GenStageInfo},
		{mgc16,   TH16GenStageInfo},
		{mgc143,  TH143GenInfo}, // no stageinfo
		{mgcalco, THALGenStageInfo}
	};
	int ret = RPYINFO_UNKNOWNFORMAT;

	for (int i=0; i<lengthof(StageInfoFuncMap); ++i) {
		if (Header == StageInfoFuncMap[i].header) {
			m_header = Header;
			m_pDecompData = ReplayDecode2(m_pData, m_nDataSize, m_pTHRpyInfo2);
			if ( m_pDecompData ) {

#ifdef _DEBUG
				DumpRPYData(_T("d:\\rpyraw.bin"));
#endif

				if ( m_pTHRpyInfo2->wFlags & (RPYFLAG2_TH14TRIAL | RPYFLAG2_TH14RELEASE) )
					this->TH14GenStageInfo(); // is th14
				else if (StageInfoFuncMap[i].pFunc != NULL) {
					(this->*(StageInfoFuncMap[i].pFunc))();
				}
				// free(m_pDecompData); // must free() it
				// But here this is not needed because the destructor will do it
				ret = RPYINFO_OK;
			}
			else {
				m_info += STR_ERRORSTAGEINFO;
				ret = RPYINFO_DECRYPTERROR;
			}

			break;
		}
	}

	return ret;
}

int CRPYAnalyzer::Analyze()
{
	const RPYMGC Header = *((RPYMGC*)m_pData);

	// Get/Generate Stage Info for 6,7,8,9
	int ret = GenInfoWrapper();
	if ( ret != RPYINFO_OK ) {
		// Get/Generate Stage Info for 10,11,12,128,13,alco,14,15
		if ( ret == RPYINFO_UNKNOWNFORMAT )
			ret = GenInfoWrapper2();

		//if (ret != RPYINFO_OK)
			// return ret;
	}

	// 只有 TH8 的文本格式的录像信息需要自动判断编码
	const BOOL isCNVer = (m_pTHRpyInfo && (m_pTHRpyInfo->wFlags & RPYFLAG_CNVER)) ? true : false;
	m_pInfoUB = new CRpyInfoUserBlock(m_pUBMgr, Header, m_uDefCode, isCNVer);  // 析构函数会 delete
	ret = m_pInfoUB->analyze();
	m_info.Insert(0, m_pInfoUB->getSummary());
	return ret;
}

CString CRPYAnalyzer::GetComment(UINT codepage)
{
	return m_pInfoUB->getComment(codepage);
}

const BYTE* CRPYAnalyzer::GetDecodedDataPointer(DWORD* pOutDecodeSize) const
{
	switch (m_header)
	{
		case mgc6:
		case mgc7:
		case mgc8:
		case mgc9:
			*pOutDecodeSize = m_pTHRpyInfo->dwDecodeSize;
			return (const BYTE*)(m_pTHRpyInfo->pDecodeData);
		default:
			*pOutDecodeSize = m_pTHRpyInfo2->dwDecodeSize;
			return (const BYTE*)(m_pDecompData);
	}
}

/////////////// protected /////////////////

////////////////////// TH6 /////////////////////


void CRPYAnalyzer::TH6GenInfo()
{
	TH6_RPYINFO*    const pGeneralInfo = &m_pTHRpyInfo->GeneralInfo.th6;
	TH6_STAGEINFO**       ppStageInfo  = m_pTHRpyInfo->pStageInfo.th6;

	CString StrFormat2, StrLives, StrBomb, StrScore, StrStageScores;

	const UINT CharaId[4] = { IDS_RMA, IDS_RMB, IDS_MRSA, IDS_MRSB };
	const UINT RankId[5] = { IDS_EASY, IDS_NORMAL, IDS_HARD, IDS_LUNATIC, IDS_EXTRA };

	CString StrPlayer(pGeneralInfo->szPlayer), StrDate(pGeneralInfo->szDate);
	FormatScore(pGeneralInfo->dwScore, StrScore, FALSE);

	// Chara & Rank
	CString StrChara, StrRank;
	if (pGeneralInfo->cChara < lengthof(CharaId))
		StrChara.LoadString(CharaId[pGeneralInfo->cChara]);
	else
		StrChara = STR_ERROR;
	if (pGeneralInfo->cRank < lengthof(RankId))
		StrRank.LoadString(RankId[pGeneralInfo->cRank]);
	else
		StrRank = STR_ERROR;

	// 整体信息
	m_info.Format(
		_T("遊戲: 東方紅魔郷 %u.%02u\r\n")
		_T("玩家: %s\r\n日期: %s\r\n")
		_T("角色: %s\r\n難度: %s\r\n")
		_T("分數: %s\r\n掉幀: %f%%\r\n"),

		(DWORD)(pGeneralInfo->cGameMajorVersion), (DWORD)(pGeneralInfo->cGameMinorVersion)
		,StrPlayer, StrDate
		,StrChara, StrRank
		,StrScore, pGeneralInfo->fDrop
	);

	// Start to get stage info
	THX_EXTRAINFO extrainfo={0};
	extrainfo.pFPSInfo = NULL;
	for (int i=0; i<m_pTHRpyInfo->nStageCount; ++i) {
		TH6_STAGEINFO* const pCurrStage = ppStageInfo[i];
		// 无效数据则不显示
		if (pCurrStage == NULL)
			continue;

		FormatScore(pCurrStage->dwScores, StrStageScores, FALSE, TRUE);

		Num2Star(pCurrStage->cLives, StrLives, 8);
		Num2Star(pCurrStage->cBombs, StrBomb, 8);

		StrFormat2.Format(
			_T("\r\nStage %s:\r\n")
			_T("        Power:%8d\r\n")
			_T("       Player: %s\r\n")
			_T("         Bomb: %s\r\n")
			_T("     PlayRank:%8d\r\n")
			_T("        Point:%8d\r\n")

			,m_pTHRpyInfo->stagenames[i]
			,(int)pCurrStage->cPowers
			,(LPCTSTR)StrLives
			,(LPCTSTR)StrBomb
			,(int)pCurrStage->cPlayRank
			,(int)pCurrStage->wPoints
		);

		m_info += StrFormat2;
		
		//PlayTime, SlowRate
		extrainfo.nIndex = i;
		extrainfo.dwFrameCounts = m_pTHRpyInfo->dwFrameCounts;
		extrainfo.szPlayTime = common_szPlayTime;
		extrainfo.szSlowRate = NULL;
		THXAddExtraInfo(&extrainfo);
		
		//ClearScore
		StrFormat2.Format(_T("  Clear Score:%s\r\n"), (LPCTSTR)(StrStageScores));

		m_info += StrFormat2;
	}

	extrainfo.nIndex = -1;
	extrainfo.dwFrameCounts = m_pTHRpyInfo->dwFrameCounts;
	extrainfo.szPlayTime = common_szTotalPlayTime;
	extrainfo.szSlowRate = NULL;
	THXAddExtraInfo(&extrainfo);
}

////////////////////// TH7 //////////////////

void CRPYAnalyzer::TH7GenInfo()
{
	TH7_RPYINFO*    const pGeneralInfo = &m_pTHRpyInfo->GeneralInfo.th7;
	TH7_STAGEINFO**       ppStageInfo  = m_pTHRpyInfo->pStageInfo.th7;

	CString StrFormat2, StrLives, StrBomb, StrScore, StrStageScores;

	const UINT CharaId[6] = { IDS_RMA, IDS_RMB, IDS_MRSA, IDS_MRSB, IDS_SKYA, IDS_SKYB };
	const UINT RankId[6] = { IDS_EASY, IDS_NORMAL, IDS_HARD, IDS_LUNATIC, IDS_EXTRA, IDS_PH };
	
	CString StrPlayer(pGeneralInfo->szPlayer), StrDate(pGeneralInfo->szDate), StrVersion(pGeneralInfo->szVersion);
	FormatScore(pGeneralInfo->dwScore, StrScore, TRUE);

	if (m_pTHRpyInfo->wFlags & RPYFLAG_CNVER) // Chinese version
		StrVersion += _T(" 中文版");

	// Chara & Rank
	CString StrChara, StrRank;
	if (pGeneralInfo->cChara < lengthof(CharaId))
		StrChara.LoadString(CharaId[pGeneralInfo->cChara]);
	else
		StrChara = STR_ERROR;
	if (pGeneralInfo->cRank < lengthof(RankId))
		StrRank.LoadString(RankId[pGeneralInfo->cRank]);
	else
		StrRank = STR_ERROR;
	
	// 整体信息
	m_info.Format(
		_T("遊戲: 東方妖々夢 %s\r\n")
		_T("玩家: %s\r\n日期: %s\r\n")
		_T("角色: %s\r\n難度: %s\r\n")
		_T("分數: %s\r\n掉幀: %f%%\r\n"),
			
		StrVersion
		,StrPlayer, StrDate
		,StrChara, StrRank
		,StrScore, pGeneralInfo->fDrop
	);

	AddGameOptionsInfo(m_pTHRpyInfo->wFlags, TRUE);

	// Start to get stage info
	THX_EXTRAINFO extrainfo={0};
	extrainfo.pFPSInfo = &m_pTHRpyInfo->fpsinfo;
	for (int i=0; i<m_pTHRpyInfo->nStageCount; ++i) {
		TH7_STAGEINFO* const pCurrStage = ppStageInfo[i];
		// 无效数据则不显示
		if (pCurrStage == NULL)
			continue;

		FormatScore(pCurrStage->dwScores, StrStageScores, TRUE, TRUE);

		Num2Star(pCurrStage->cLives, StrLives, 8);
		Num2Star(pCurrStage->cBombs, StrBomb, 8);

		StrFormat2.Format(
			_T("\r\nStage %s:\r\n")
			_T("        Power:%8d\r\n")
			_T("       Player: %s\r\n")
			_T("         Bomb: %s\r\n")
			_T("     PlayRank:%8d\r\n")
			_T("        Point:%8d/%d\r\n")
			_T("        Graze:%8d\r\n")
			_T("    SCB Count:%8d\r\n")
			_T("     Point1UP:%8d\r\n")
			_T("          櫻+:%8d\r\n")
			_T("           櫻:%8d/%d\r\n")

			,m_pTHRpyInfo->stagenames[i]
			,(int)pCurrStage->cPowers
			,(LPCTSTR)StrLives
			,(LPCTSTR)StrBomb
			,(int)pCurrStage->cPlayRank
			,pCurrStage->nPoints
			,pCurrStage->nNextPoints
			,pCurrStage->nGrazes
			,pCurrStage->cSpellCardBonusCount
			,pCurrStage->n1upByPointsCount
			,pCurrStage->nCurrCherriesPlus
			,pCurrStage->nCurrCherries
			,pCurrStage->nMaxCherries
		);

		m_info += StrFormat2;
		
		//PlayTime, SlowRate
		extrainfo.nIndex = i;
		extrainfo.dwFrameCounts = m_pTHRpyInfo->dwFrameCounts;
		extrainfo.szPlayTime = common_szPlayTime;
		extrainfo.szSlowRate = common_szSlowRate;
		THXAddExtraInfo(&extrainfo);
		
		//ClearScore
		StrFormat2.Format(_T("  Clear Score:%s\r\n"), (LPCTSTR)(StrStageScores));
		m_info += StrFormat2;
	}

	extrainfo.nIndex = -1;
	extrainfo.dwFrameCounts = m_pTHRpyInfo->dwFrameCounts;
	extrainfo.szPlayTime = common_szTotalPlayTime;
	extrainfo.szSlowRate = common_szTotalSlowRate;
	THXAddExtraInfo(&extrainfo);
}

////////////////////// TH8 //////////////////

void CRPYAnalyzer::TH8GenStageInfo()
{
	TH8_STAGEINFO** ppStageInfo = m_pTHRpyInfo->pStageInfo.th8;

	CString StrFormat2, StrStageScores, StrLives, StrBomb, strGameTime;

	THX_EXTRAINFO extrainfo = {0};
	extrainfo.pFPSInfo = &m_pTHRpyInfo->fpsinfo;

	AddGameOptionsInfo(m_pTHRpyInfo->wFlags, TRUE);

	for (int i=0; i<m_pTHRpyInfo->nStageCount; ++i) {
		TH8_STAGEINFO* const pCurrStage = ppStageInfo[i];
		// 无效数据则不显示
		if (pCurrStage == NULL)
			continue;

		FormatScore(pCurrStage->dwScores, StrStageScores, TRUE, TRUE);

		Num2Star(pCurrStage->cLives, StrLives, 8);
		Num2Star(pCurrStage->cBombs, StrBomb, 8);

		StrFormat2.Format(
			_T("\r\nStage %s:\r\n")
			_T("        Power:%8d\r\n")
			_T("       Player: %s\r\n")
			_T("         Bomb: %s\r\n")
			_T("     PlayRank:%8d\r\n")
			_T("        Point:%8d/%d\r\n")
			_T("        Graze:%8d\r\n")
			_T("    SCB Count:%8d\r\n")
			_T("     Point1UP:%8d\r\n")
			_T("         妖率:%5d.%02d%%\r\n")
			_T("         夜點:%8d\r\n")
			_T("         時刻:%8d 刻 (%s)\r\n")

			,m_pTHRpyInfo->stagenames[i]
			,(int)pCurrStage->cPowers
			,(LPCTSTR)StrLives
			,(LPCTSTR)StrBomb
			,(int)pCurrStage->cPlayRank
			,pCurrStage->nPoints
			,pCurrStage->nNextPoints
			,pCurrStage->nGrazes
			,pCurrStage->cSpellCardBonusCount
			,pCurrStage->n1upByPointsCount
			,pCurrStage->sYaoLv/100, abs(pCurrStage->sYaoLv%100) //妖率
			,pCurrStage->nNightPoints
			,pCurrStage->cGameTime, (LPCTSTR)TH8FormatGameTime(pCurrStage->cGameTime, strGameTime) // 时刻
		);
		
		m_info += StrFormat2;
		
		//PlayTime, SlowRate
		extrainfo.nIndex = i;
		extrainfo.dwFrameCounts = m_pTHRpyInfo->dwFrameCounts;
		extrainfo.szPlayTime = common_szPlayTime;
		extrainfo.szSlowRate = common_szSlowRate;
		THXAddExtraInfo(&extrainfo);

		//ClearScore
		StrFormat2.Format(_T("  Clear Score:%s\r\n"), (LPCTSTR)(StrStageScores));
		m_info += StrFormat2;
	}

	extrainfo.nIndex = -1;
	extrainfo.dwFrameCounts = m_pTHRpyInfo->dwFrameCounts;
	extrainfo.szPlayTime = common_szTotalPlayTime;
	extrainfo.szSlowRate = common_szTotalSlowRate;
	THXAddExtraInfo(&extrainfo);
}

////////////////////// TH9 //////////////////
void CRPYAnalyzer::TH9GenStageInfo()
{
	typedef TH9_STAGEINFO* (*PPTH9_STAGEINFO)[10];

	// ppStageInfo[0][0]=player1第一关指针, ppStageInfo[1][0]=player2 第一关指针...
	// 也可以这样定义:
	// TH9_STAGEINFO** ppStageInfo = (TH9_STAGEINFO)m_pTHRpyInfo->pStageInfo.th9;
	// 不过这样的话 player2 的各关指针就要用 ppStageInfo[10+关卡索引] 来取
	PPTH9_STAGEINFO ppStageInfo = (PPTH9_STAGEINFO)m_pTHRpyInfo->pStageInfo.th9; 

	// 对战场所数组（日文版）  【注意】：索引 = m_pTHRpyInfo->cPlace + 2
	LPCTSTR PlaceListJP[] = { // start from -2
		_T("どっか")          , _T("規定の場所")       , // -2, -1
		_T("迷いの竹林 REIMU"), _T("迷いの竹林 MARISA"), // 0, 1
		_T("幻草原 SAKUYA")   , _T("白玉楼階段 YOUMU") , // ...
		_T("永遠亭 REISEN")   , _T("霧の湖 CIRNO")     ,
		_T("幽明結界 LYRICA") , _T("妖怪獣道 MYSTIA")  ,
		_T("迷いの竹林 TEI")  , _T("太陽の畑 YUKA")    ,
		_T("大蝦蟇の池 AYA")  , _T("無名の丘 MEDICINE"),
		_T("再思の道 KOMACHI"), _T("無縁塚 SIKIEIKI")
	};
	// 对战场所数组（中文版）  【注意】：索引 = m_pTHRpyInfo->cPlace + 2
	LPCTSTR PlaceListCN[] = { // start from -2
		_T("隨機選擇")        , _T("預定的場所")       , // -2 , -1
		_T("迷之竹林 REIMU")  , _T("迷之竹林 MARISA")  , // 0  , 1
		_T("幻草原 SAKUYA")   , _T("白玉樓臺階 YOUMU") , // ...
		_T("永遠亭 REISEN")   , _T("霧之湖 CIRNO")     ,
		_T("幽冥結界 LYRICA") , _T("妖怪獸道 MYSTIA")  ,
		_T("迷之竹林 TEI")    , _T("太陽花佃 YUKA")    ,
		_T("大蛤蟆之池 AYA")  , _T("無名之丘 MEDICINE"),
		_T("再思之道 KOMACHI"), _T("無縁冢 SIKIEIKI")
	};


	//Chara List
	LPCTSTR szChara[16] = {
		_T("Reimu")  , _T("Marisa")  , _T("Sakuya"), _T("Youmu")   ,
		_T("Reisen") , _T("Cirno")   , _T("Lyrica"), _T("Mystia")  ,
		_T("Tewi")   , _T("Yuka")    , _T("Aya")   , _T("Medicine"),
		_T("Komachi"), _T("Sikieiki"), _T("Merlin"), _T("Lunasa")
	};
	BYTE cChara[2]={0};

	ASSERT(lengthof(PlaceListJP) == lengthof(PlaceListCN) &&
			lengthof(PlaceListJP) == lengthof(szChara));

	CString StrFormat2, StrStageScores[2]; // StrStageScores[2] stands for Player1 & Player2

	// Cirno is ⑨ ?
	if (HasConfigOption(CFG_9))	{
		szChara[5] = _T("⑨");
		PlaceListJP[7] = _T("霧の湖 ⑨");
		PlaceListCN[7] = _T("霧之湖 ⑨");
	}

	//Charge Type
	{
		LPCTSTR const szChargeType[2] = { _T("SLOW"), _T("CHARGE") };
		StrFormat2.Format(
				_T("Player1 Charge Type: %s\r\n")
				_T("Player2 Charge Type: %s\r\n")
				, szChargeType[m_pTHRpyInfo->wFlags&RPYFLAG_P1CHARGE ? 1 : 0]
				, szChargeType[m_pTHRpyInfo->wFlags&RPYFLAG_P2CHARGE ? 1 : 0]
		);
		m_info += StrFormat2;
	}

	THX_EXTRAINFO extrainfo={0};
	extrainfo.pFPSInfo = &m_pTHRpyInfo->fpsinfo;
	bool bStrFormatModified = false;
	CString StrFormat(
				_T("\r\nStage %s:\r\n")
				_T("               %17s vs %s\r\n") //Player1 vs Player2
				_T("       Player: %17d    %d\r\n")
				_T("        Score: %14s    %s\r\n"));
	for (int i=0; i<m_pTHRpyInfo->nStageCount; i++) {
		//无效数据则不显示
		if(!ppStageInfo[0][i])
			continue;

		FormatScore(ppStageInfo[0][i]->dwScores, StrStageScores[0], TRUE, FALSE);
		FormatScore(ppStageInfo[1][i]->dwScores, StrStageScores[1], TRUE, FALSE);
		cChara[0] = ppStageInfo[0][i]->cChara;
		cChara[1] = ppStageInfo[1][i]->cChara;
		
		// 如果 player1 是 ⑨，并且开启了“⑨”显示，显示会稍稍错位（向右偏一个字符），
		// 把 StrFormat 中的 "%17s" 改成 "%16s" 即可修正
		// 由于正常情况下 player1 角色不会在中途更改，所以只需更改 StrFormat 一次
		if ( cChara[0] == 5 && HasConfigOption(CFG_9) && !bStrFormatModified) {
			StrFormat.Replace(_T("%17s"), _T("%16s"));
			bStrFormatModified = true;
		}

		StrFormat2.Format(StrFormat,
			m_pTHRpyInfo->stagenames[i],
			(cChara[0]>=16?STR_ERROR:szChara[cChara[0]]), (cChara[1]>=16?STR_ERROR:szChara[cChara[1]]),
			ppStageInfo[0][i]->cLives, ppStageInfo[1][i]->cLives,
			(LPCTSTR)(StrStageScores[0]), (LPCTSTR)(StrStageScores[1])
		);
		
		m_info += StrFormat2;

		// place, for match mode only
		if (i==9) { // match mode
			signed char cPlace = ppStageInfo[0][i]->cPlace + 2;
			LPCTSTR szPlace = NULL;
			LPCTSTR* const PlaceList = (m_pTHRpyInfo->wFlags & RPYFLAG_CNVER)
				? PlaceListCN
				: PlaceListJP;

			if (cPlace>=0 && cPlace<lengthof(PlaceListJP))
				szPlace = PlaceList[cPlace];
			else // out of range
				szPlace = _T("Error");

			StrFormat2.Format(_T("        Place: %s\r\n"), szPlace);
			m_info += StrFormat2;
		}

		//PlayTime, SlowRate
		extrainfo.nIndex = i;
		extrainfo.dwFrameCounts = m_pTHRpyInfo->dwFrameCounts;
		extrainfo.szPlayTime = _T("     PlayTime: %u(%d:%02d.%03d)\r\n");
		extrainfo.szSlowRate = common_szSlowRate;
		THXAddExtraInfo(&extrainfo);
	}

	extrainfo.nIndex = -1;
	extrainfo.dwFrameCounts = m_pTHRpyInfo->dwFrameCounts;
	extrainfo.szPlayTime = common_szTotalPlayTime;
	extrainfo.szSlowRate = common_szTotalSlowRate;
	THXAddExtraInfo(&extrainfo);
}

void CRPYAnalyzer::THXAddExtraInfo(const THX_EXTRAINFO* pExtraInfo)
{
	DWORD dwTotalPlayTime;
	double slowrate;
	
	if (pExtraInfo->nIndex < 0) { // get total
		// 计算所有关卡的合计总帧数
		// pExtraInfo->dwFrameCounts 应该有 20 个数据，但这里只累加前 10 个。
		// 因为只有 TH9 的关卡数据会超过 10 个，
		// 而 TH9 从第 11 个数据开始就是 P2 的第 1 关数据，计算帧数自然用不到。
		dwTotalPlayTime = 0;
		for (int i=0; i<10; ++i)
			dwTotalPlayTime += pExtraInfo->dwFrameCounts[i];

		if (pExtraInfo->pFPSInfo != NULL)
			slowrate = pExtraInfo->pFPSInfo->total_slowrate;
	}
	else { // get stage
		dwTotalPlayTime = pExtraInfo->dwFrameCounts[pExtraInfo->nIndex];
		if (pExtraInfo->pFPSInfo != NULL)
			slowrate = pExtraInfo->pFPSInfo->slowrates[pExtraInfo->nIndex];
	}

	//Total PlayTime
	if( HasConfigOption(CFG_SHOWPLAYTIME) ) {
		const DWORD m = dwTotalPlayTime/3600;
		const DWORD s = dwTotalPlayTime/60%60;
		const DWORD ms = dwTotalPlayTime%60*1000/60;
		CString StrFormat;
		StrFormat.Format(pExtraInfo->szPlayTime, dwTotalPlayTime, m, s, ms);
		m_info += StrFormat;
	}
	
	//Total SlowRate
	if( HasConfigOption(CFG_SHOWSLOWRATE) && pExtraInfo->szSlowRate != NULL ) {
		CString StrFormat;
		StrFormat.Format(pExtraInfo->szSlowRate, slowrate);
		m_info += StrFormat;
	}

	if (pExtraInfo->nIndex < 0)
		m_info += _T("\r\n");
}

/////////////
///////////// THRPYINFO2 //////////////////////
/////////////
void CRPYAnalyzer::THALGenStageInfo()
{
	THAL_STAGEINFO** ppStageInfo = m_pTHRpyInfo2->pStageInfo.alco;
	CString StrFormat2, StrStageScore;

	AddGameOptionsInfo(m_pTHRpyInfo2->wFlags);

	for (int i=0; i<m_pTHRpyInfo2->nStageCount; i++) {
		THAL_STAGEINFO* const pCurrStage = ppStageInfo[i];

		FormatScore(
			(i == m_pTHRpyInfo2->nStageCount-1) //最后一关取 dwClearScore
				? m_pTHRpyInfo2->dwClearScore
				: ppStageInfo[i+1]->dwScore,
			StrStageScore, FALSE, TRUE, 0);

		StrFormat2.Format(
			_T("\r\nStage %s:\r\n")
			, m_pTHRpyInfo2->stagenames[pCurrStage->hdr.wStageNumber-1]
		);

		m_info += StrFormat2;

		THXAddExtraInfo2(i);
		
		//ClearScore
		StrFormat2.Format(_T("  Clear Score:%s\r\n"), (LPCTSTR)(StrStageScore));
		m_info += StrFormat2;
	}
	THXAddExtraInfo2(-1);
}

void CRPYAnalyzer::TH10GenStageInfo()
{
	TH10_STAGEINFO** ppStageInfo = m_pTHRpyInfo2->pStageInfo.th10;
	CString StrFormat2, StrStageScore, strPlayer;
	
	AddGameOptionsInfo(m_pTHRpyInfo2->wFlags);

	for (int i=0; i<m_pTHRpyInfo2->nStageCount; i++) {
		TH10_STAGEINFO* const pCurrStage = ppStageInfo[i];

		FormatScore(
			(i == m_pTHRpyInfo2->nStageCount-1) //最后一关取 dwClearScore
				? m_pTHRpyInfo2->dwClearScore
				: ppStageInfo[i+1]->dwScore,
			StrStageScore, TRUE, TRUE, ppStageInfo[i]->dwContinuedTimes);

		Num2Star(pCurrStage->dwPlayer, strPlayer, 9);

		StrFormat2.Format(
			_T("\r\nStage %s:\r\n")
			_T("       Player: %s\r\n")
			_T("        Power:%5d.%02d(%u)\r\n")
			_T("         信仰:%8d\r\n")
			_T("       連擊槽:%8d\r\n")
			_T("         座標:%8d/%d(%d/%d)\r\n")
			
			, m_pTHRpyInfo2->stagenames[pCurrStage->hdr.wStageNumber-1]
			, strPlayer
			, pCurrStage->dwPower/20, pCurrStage->dwPower%20*5, pCurrStage->dwPower
			, pCurrStage->dwFaith*10
			, pCurrStage->dwComboGauge
			, pCurrStage->nPosX, pCurrStage->nPosY, transTH10PosX(pCurrStage->nPosX), transTH10PosY(pCurrStage->nPosY)
		);

		m_info += StrFormat2;

		THXAddExtraInfo2(i);

		//ClearScore
		StrFormat2.Format(_T("  Clear Score:%s\r\n"), (LPCTSTR)(StrStageScore));
		m_info += StrFormat2;
	}
	THXAddExtraInfo2(-1);
}

void CRPYAnalyzer::TH11GenStageInfo()
{
	TH11_STAGEINFO** ppStageInfo = m_pTHRpyInfo2->pStageInfo.th11;
	CString StrFormat2, StrStageScore, strPlayer;

	AddGameOptionsInfo(m_pTHRpyInfo2->wFlags);

	// 判断是否是魔理沙A（魔爱组）。因为魔理沙A 的 power 显示值算法与众不同
	const bool isMarisaA = (m_pTHRpyInfo2->dwID == 1 && m_pTHRpyInfo2->dwEquipID == 0);
	DWORD dwPowerInt, dwPowerDec; // 用于 power 显示
	
	for (int i=0; i<m_pTHRpyInfo2->nStageCount; i++) {
		TH11_STAGEINFO* const pCurrStage = ppStageInfo[i];
		
		FormatScore(
			(i == m_pTHRpyInfo2->nStageCount-1) //最后一关取 dwClearScore
				? m_pTHRpyInfo2->dwClearScore
				: ppStageInfo[i+1]->dwScore,
			StrStageScore, TRUE, TRUE, ppStageInfo[i]->dwContinuedTimes);
		
		Num2Star(pCurrStage->wPlayerInt, strPlayer, 9, pCurrStage->wPlayerDec);
		
		// 计算 power 显示值
		if (isMarisaA) {
            dwPowerInt = pCurrStage->dwPower/12;        // 整数部分
            dwPowerDec = pCurrStage->dwPower%12*100/12; // 小数部分
		}
		else {
            dwPowerInt = pCurrStage->dwPower/20;   // 整数部分
            dwPowerDec = pCurrStage->dwPower%20*5; // 小数部分
		}

		// 计算最大得点
		const DWORD base_pt = pCurrStage->dwConnect - pCurrStage->dwConnect % 10; // 确保个位为 0
		const DWORD rate = min(pCurrStage->dwGraze/100, 899) + 100;
		const DWORD max_pt = rate * base_pt / 100;

		StrFormat2.Format(
			_T("\r\nStage %s:\r\n")
			_T("       Player: %s\r\n")
			_T("        Power:%5u.%02u(%u)\r\n")
			_T("        Graze:%8u\r\n")
			_T("     最大得點:%8u(%u×%u.%02u)\r\n")
			_T("         座標:%8d/%d(%d/%d)\r\n")

			, m_pTHRpyInfo2->stagenames[pCurrStage->hdr.wStageNumber-1]
			, strPlayer
			, dwPowerInt, dwPowerDec, pCurrStage->dwPower
			, pCurrStage->dwGraze
			, max_pt, pCurrStage->dwConnect, rate/100, rate%100  // 为了显示确切的内部值，这里使用未作修改的 dwConnect 而不是 base_pt。正常情况下 dwConnect 应该等于 base_pt
			, pCurrStage->nPosX, pCurrStage->nPosY, transPosX(pCurrStage->nPosX), transPosY(pCurrStage->nPosY)
		);
		
		m_info += StrFormat2;

		THXAddExtraInfo2(i);

		//ClearScore
		StrFormat2.Format(_T("  Clear Score:%s\r\n"), (LPCTSTR)(StrStageScore));
		m_info += StrFormat2;
	}
	THXAddExtraInfo2(-1);
}

void CRPYAnalyzer::TH12GenStageInfo()
{
	TH12_STAGEINFO** ppStageInfo = m_pTHRpyInfo2->pStageInfo.th12;
	CString StrFormat2, StrStageScore, strPlayer, strBomb, strUFOStock;

	AddGameOptionsInfo(m_pTHRpyInfo2->wFlags);
	
	for (int i=0; i<m_pTHRpyInfo2->nStageCount; i++)
	{
		TH12_STAGEINFO* const pCurrStage = ppStageInfo[i];
		
		FormatScore(
			(i == m_pTHRpyInfo2->nStageCount-1) //最后一关取 dwClearScore
				? m_pTHRpyInfo2->dwClearScore
				: ppStageInfo[i+1]->dwScore,
			StrStageScore, TRUE, TRUE, ppStageInfo[i]->dwContinuedTimes);
		
		Num2Star(pCurrStage->wPlayerInt, strPlayer, 9, pCurrStage->wPlayerDec);
		Num2Star(pCurrStage->wBombInt, strBomb, 9, pCurrStage->wBombDec);
		TH12FormatUFOStock(pCurrStage->dwUFOStock, strUFOStock);
		
		StrFormat2.Format(
			_T("\r\nStage %s:\r\n")
			_T("       Player: %s\r\n")
			_T("         Bomb: %s\r\n")
			_T("        Power:%9d.%02d\r\n")
			_T("        Graze:%12d\r\n")
			_T("     最大得點:%9d.%02d\r\n")
			_T("          UFO:      %s\r\n")
			_T("         座標:%8d/%d(%d/%d)\r\n")

			, m_pTHRpyInfo2->stagenames[pCurrStage->hdr.wStageNumber-1]
			, strPlayer
			, strBomb
			, pCurrStage->dwPower/100, pCurrStage->dwPower%100
			, pCurrStage->dwGraze
			, pCurrStage->dwMaxScore/100, pCurrStage->dwMaxScore%100
			, strUFOStock
			, pCurrStage->nPosX, pCurrStage->nPosY, transPosX(pCurrStage->nPosX), transPosY(pCurrStage->nPosY)
		);
		
		m_info += StrFormat2;

		THXAddExtraInfo2(i);

		//ClearScore
		StrFormat2.Format(_T("  Clear Score:%s\r\n"), (LPCTSTR)(StrStageScore));
		m_info += StrFormat2;
	}
	THXAddExtraInfo2(-1);
}

void CRPYAnalyzer::TH128GenStageInfo()
{
	TH128_STAGEINFO **ppStageInfo = m_pTHRpyInfo2->pStageInfo.th128;
	CString StrFormat2, StrStageScore;

	AddGameOptionsInfo(m_pTHRpyInfo2->wFlags);
	
	for (int i=0; i<m_pTHRpyInfo2->nStageCount; i++) {
		TH128_STAGEINFO* const pCurrStage = ppStageInfo[i];
		
		FormatScore(
			(i == m_pTHRpyInfo2->nStageCount-1) //最后一关取 dwClearScore
				? m_pTHRpyInfo2->dwClearScore
				: ppStageInfo[i+1]->dwScore,
			StrStageScore, TRUE, TRUE);
		
		StrFormat2.Format(
			_T("\r\nStage %s:\r\n")
			_T("         幹勁:%11d.%02d%%\r\n")
			_T("       凍結力:%10d.%03d%%\r\n")
			_T("     完美凍結:%11d.%02d%%\r\n")
			_T("     凍結面積:%14f%%\r\n")
			_T("        LEVEL:%15d\r\n")
			_T("         座標:%15d/%d(%d/%d)\r\n")

			, m_pTHRpyInfo2->stagenames[pCurrStage->hdr.wStageNumber-1]
			, pCurrStage->dwYaruki/100, pCurrStage->dwYaruki%100
			, pCurrStage->dwFreezePower/1000, pCurrStage->dwFreezePower%1000
			, pCurrStage->dwPerfectFreeze/100, pCurrStage->dwPerfectFreeze%100
			, pCurrStage->fFrozenArea
			, pCurrStage->dwLevel + 1
			, pCurrStage->nPosX, pCurrStage->nPosY, transPosX(pCurrStage->nPosX), transPosY(pCurrStage->nPosY)
		);
		
		m_info += StrFormat2;

		THXAddExtraInfo2(i);

		//ClearScore
		StrFormat2.Format(_T("  Clear Score:%s\r\n"), (LPCTSTR)(StrStageScore));
		m_info += StrFormat2;
	}
	THXAddExtraInfo2(-1);
}

void CRPYAnalyzer::TH13GenStageInfo()
{
	TH13_STAGEINFO** ppStageInfo = m_pTHRpyInfo2->pStageInfo.th13;
	DWORD dwPlayerFragmentNorm = (DWORD)-1;
	DWORD const PlayerFragmentNormList[] = {8, 10, 12, 15, 18, 20, 25}; // 奖命碎片定额列表，只用于正式版。
	DWORD const dwBombFragmentNorm = (m_pTHRpyInfo2->wVersion > 1) // 加Bomb碎片的定额根据游戏版本而定
		? 8 // 正式版
		: 10; // 体验版 0.01a
	AddSpellPracticeInfo(m_pTHRpyInfo2->nSpellPracticeNumber, _T("th13sc.lst"));
	AddGameOptionsInfo(m_pTHRpyInfo2->wFlags);
	
	CString StrFormat2, StrStageScore, strPlayer, strBomb;
	
	for (int i=0; i<m_pTHRpyInfo2->nStageCount; i++) {
		TH13_STAGEINFO* const pCurrStage = ppStageInfo[i];
		TH13_STAGEINFO* const pCurrStageRelease = (m_pTHRpyInfo2->wVersion > 1) // 对于正式版部分偏移需要修正
			? (TH13_STAGEINFO*)(((BYTE*)pCurrStage)+4) // 正式版的部分数据偏移需要+4
			: pCurrStage; // 体验版 0.01a
		
		FormatScore(
			(i == m_pTHRpyInfo2->nStageCount-1) //最后一关取 dwClearScore
				? m_pTHRpyInfo2->dwClearScore
				: ppStageInfo[i+1]->dwScore,
			StrStageScore, TRUE, TRUE);

		if ( pCurrStageRelease->dwPlayerFragmentNormIndex >= lengthof(PlayerFragmentNormList) )
			dwPlayerFragmentNorm = (DWORD)-1; // -1 means invalid norm
		else {
			// 正式版 和 体验版 奖命碎片的定额不同（正式版少了）。
			// 体验版的定额列表为 [10,15,20,25,30,35,40]
			// 本程序对于体验版不使用列表。因为直接这么算更简单： 10+i*5 。
			if (m_pTHRpyInfo2->wVersion > 1) // 正式版
				dwPlayerFragmentNorm = PlayerFragmentNormList[pCurrStageRelease->dwPlayerFragmentNormIndex];
			else
				dwPlayerFragmentNorm = 10 + pCurrStageRelease->dwPlayerFragmentNormIndex*5;
		}

		Num2Star(pCurrStageRelease->dwPlayer, strPlayer, 9);
		Num2Star(pCurrStageRelease->dwBomb, strBomb, 9);
		
		StrFormat2.Format(
			_T("\r\nStage %s:\r\n")
			_T("       Player: %s\r\n")
			_T("  Player 碎片:%12d/%d\r\n")
			_T("         Bomb: %s\r\n")
			_T("    Bomb 碎片:%12d/%d\r\n")
			_T("        Power:%9d.%02d\r\n")
			_T("        Graze:%12d\r\n")
			_T("     最大得點:%9d.%02d\r\n")
			_T("       靈界槽:%12d\r\n")
			_T("         座標:%12d/%d(%d/%d)\r\n")
			
			, m_pTHRpyInfo2->stagenames[pCurrStage->hdr.wStageNumber-1]
			, strPlayer
			, pCurrStageRelease->dwPlayerFragment, dwPlayerFragmentNorm
			, strBomb
			, pCurrStageRelease->dwBombFragment, dwBombFragmentNorm
			, pCurrStageRelease->dwPower/100, pCurrStageRelease->dwPower%100
			, pCurrStage->dwGraze
			, pCurrStageRelease->dwMaxScore/100, pCurrStageRelease->dwMaxScore%100
			, pCurrStageRelease->dwSpiritWorldGage
			, pCurrStage->nPosX, pCurrStage->nPosY, transPosX(pCurrStage->nPosX), transPosY(pCurrStage->nPosY)
			);
		
		m_info += StrFormat2;

		THXAddExtraInfo2(i);

		//ClearScore
		StrFormat2.Format(_T("  Clear Score:%s\r\n"), (LPCTSTR)(StrStageScore));
		m_info += StrFormat2;
	}
	THXAddExtraInfo2(-1);
}

void CRPYAnalyzer::TH14GenStageInfo()
{
	TH14_STAGEINFO** ppStageInfo = m_pTHRpyInfo2->pStageInfo.th14;
	CString StrFormat2, StrStageScore, strPlayer, strBomb;

	AddSpellPracticeInfo(m_pTHRpyInfo2->nSpellPracticeNumber, _T("th14sc.lst"));
	AddGameOptionsInfo(m_pTHRpyInfo2->wFlags);
	
	for (int i=0; i<m_pTHRpyInfo2->nStageCount; i++) {
		TH14_STAGEINFO* const pCurrStage = ppStageInfo[i];
		
		FormatScore(
			(i == m_pTHRpyInfo2->nStageCount-1) //最后一关取 dwClearScore
				? m_pTHRpyInfo2->dwClearScore
				: ppStageInfo[i+1]->dwScore,
			StrStageScore, TRUE, TRUE, 0);
		
		Num2Star(pCurrStage->dwPlayer, strPlayer, 9, pCurrStage->dwPlayerFragment);
		Num2Star(pCurrStage->dwBomb, strBomb, 9, pCurrStage->dwBombFragment);
		
		StrFormat2.Format(
			_T("\r\nStage %s:\r\n")
			_T("       Player: %s\r\n")
			_T("         Bomb: %s\r\n")
			_T("     1UPCount:%12d\r\n")
			_T("  Non2.0Bonus:%12d\r\n")
			_T("        Power:%9d.%02d\r\n")
			_T("        Graze:%12d\r\n")
			_T("     最大得點:%9d.%02d\r\n")
			_T("         座標:%12d/%d(%d/%d)\r\n")
			
			, m_pTHRpyInfo2->stagenames[pCurrStage->hdr.wStageNumber-1]
			, strPlayer
			, strBomb
			, pCurrStage->dw1upCount
			, pCurrStage->dwNon20Bonus
			, pCurrStage->dwPower/100, pCurrStage->dwPower%100
			, pCurrStage->dwGraze
			, pCurrStage->dwMaxScore/100, pCurrStage->dwMaxScore%100
			, pCurrStage->nPosX, pCurrStage->nPosY, transPosX(pCurrStage->nPosX), transPosY(pCurrStage->nPosY)
		);
		
		m_info += StrFormat2;

		THXAddExtraInfo2(i);

		//ClearScore
		StrFormat2.Format(_T("  Clear Score:%s\r\n"), (LPCTSTR)(StrStageScore));
		m_info += StrFormat2;
	}
	THXAddExtraInfo2(-1);
}

void CRPYAnalyzer::TH15GenStageInfo()
{
	TH15_STAGEINFO** ppStageInfo = m_pTHRpyInfo2->pStageInfo.th15;
	CString StrFormat2, StrStageScore, strPlayer, strBomb;

	AddGameOptionsInfo(m_pTHRpyInfo2->wFlags);
	
	for (int i=0; i<m_pTHRpyInfo2->nStageCount; i++) {
		TH15_STAGEINFO* const pCurrStage = ppStageInfo[i];
		
		FormatScore(
			(i == m_pTHRpyInfo2->nStageCount-1) //最后一关取 dwClearScore
				? m_pTHRpyInfo2->dwClearScore
				: ppStageInfo[i+1]->dwScore,
			StrStageScore, TRUE, TRUE, 0);
		
		Num2Star(pCurrStage->dwPlayer, strPlayer, 9, pCurrStage->dwPlayerFragment);
		Num2Star(pCurrStage->dwBomb, strBomb, 9, pCurrStage->dwBombFragment);
		
		StrFormat2.Format(
			_T("\r\nStage %s:\r\n")
			_T("       Player: %s\r\n")
			_T("         Bomb: %s\r\n")
			_T("     1UPCount:%12d\r\n")
			_T("        Power:%9d.%02d\r\n")
			_T("        Graze:%12d\r\n")
			_T("     最大得點:%9d.%02d\r\n")
			_T("         座標:%12d/%d(%d/%d)\r\n")
			
			, m_pTHRpyInfo2->stagenames[pCurrStage->hdr.wStageNumber-1]
			, strPlayer
			, strBomb
			, pCurrStage->dw1upCount
			, pCurrStage->dwPower/100, pCurrStage->dwPower%100
			, pCurrStage->dwGraze
			, pCurrStage->dwMaxScore/100, pCurrStage->dwMaxScore%100
			, pCurrStage->nPosX, pCurrStage->nPosY, transPosX(pCurrStage->nPosX), transPosY(pCurrStage->nPosY)
		);
		
		m_info += StrFormat2;

		THXAddExtraInfo2(i);

		//ClearScore
		StrFormat2.Format(_T("  Clear Score:%s\r\n"), (LPCTSTR)(StrStageScore));
		m_info += StrFormat2;
	}
	THXAddExtraInfo2(-1);
}

void CRPYAnalyzer::TH16GenStageInfo()
{
	TH16_STAGEINFO**       ppStageInfo    = m_pTHRpyInfo2->pStageInfo.th16;
	
	CString StrFormat2, StrStageScore, strPlayer, strBomb;

	// 自带录像信息中缺少子机季节
	{
		CString strSubWeapon;
		switch(m_pTHRpyInfo2->dwEquipID) {
		case 0: strSubWeapon = _T("Spring"); break;
		case 1: strSubWeapon = _T("Summer"); break;
		case 2: strSubWeapon = _T("Autumn"); break;
		case 3: strSubWeapon = _T("Winter"); break;
		default: strSubWeapon = _T("ERROR!"); break;
		}
		StrFormat2.Format(_T("Season %s\r\n"), strSubWeapon);
		m_info += StrFormat2;
	}

	AddGameOptionsInfo(m_pTHRpyInfo2->wFlags);
	
	for (int i=0; i<m_pTHRpyInfo2->nStageCount; i++) {
		TH16_STAGEINFO* const pCurrStage = ppStageInfo[i];
		
		FormatScore(
			(i == m_pTHRpyInfo2->nStageCount-1) //最后一关取 dwClearScore
				? m_pTHRpyInfo2->dwClearScore
				: ppStageInfo[i+1]->dwScore,
			StrStageScore, TRUE, TRUE, 0);
		
		Num2Star(pCurrStage->dwPlayer, strPlayer, 9);
		Num2Star(pCurrStage->dwBomb, strBomb, 9, pCurrStage->dwBombFragment);

		// 转换季节槽为子机
		DWORD dwSeasonNorm = 0;
		double dblSubWeapon = TH16FormatSeasonGauge(pCurrStage->dwSeasonGauge, dwSeasonNorm); 
		
		StrFormat2.Format(
			_T("\r\nStage %s:\r\n")
			_T("       Player: %s\r\n")
			_T("         Bomb: %s\r\n")
			_T("     1UPCount:%12d\r\n")
			_T("        Power:%9d.%02d\r\n")
			_T("        Graze:%12d\r\n")
			_T("       季節槽:%12.3f(%d/%d)\r\n")
			_T("     最大得點:%9d.%02d\r\n")
			_T("         座標:%12d/%d(%d/%d)\r\n")
			
			, m_pTHRpyInfo2->stagenames[pCurrStage->hdr.wStageNumber-1]
			, strPlayer
			, strBomb
			, pCurrStage->dw1upCount
			, pCurrStage->dwPower/100, pCurrStage->dwPower%100
			, pCurrStage->dwGraze
			, dblSubWeapon, pCurrStage->dwSeasonGauge, dwSeasonNorm
			, pCurrStage->dwMaxScore/100, pCurrStage->dwMaxScore%100
			, pCurrStage->nPosX, pCurrStage->nPosY, transPosX(pCurrStage->nPosX), transPosY(pCurrStage->nPosY)
		);
		
		m_info += StrFormat2;

		THXAddExtraInfo2(i);

		//ClearScore
		StrFormat2.Format(_T("  Clear Score:%s\r\n"), (LPCTSTR)(StrStageScore));
		m_info += StrFormat2;
	}
	THXAddExtraInfo2(-1);
}

// Show info about SpellCard or Scene
// for TH95/TH125/TH143, isScene should be TRUE
void CRPYAnalyzer::AddSpellPracticeInfo(int SCNum, LPCTSTR filename, BOOL isScene/*=FALSE*/)
{
	if (SCNum != 0) {
		CString strfmt;
		CString strSCName;
		CString strSCFileName(g_exefullpath);

		if (isScene)
			strfmt.Format(_T("Scene No. %d"), SCNum);
		else
			strfmt.Format(_T("SpellCard No. %d"), SCNum);
		m_info += strfmt;

		filepath_utils::ChangeFilename(strSCFileName, filename);
		if ( ::ReadUnicodeFileLine(strSCFileName, SCNum, strSCName) ) {
			strfmt.Format(_T(" %s"), strSCName);
			m_info += strfmt;
		}

		m_info += _T("\r\n");
	}
}

void CRPYAnalyzer::TH95GenInfo()
{
	AddSpellPracticeInfo(m_pTHRpyInfo2->nSpellPracticeNumber, _T("th95sc.lst"), TRUE);
	AddGameOptionsInfo(m_pTHRpyInfo2->wFlags);

	THXAddExtraInfo2(0);
}

void CRPYAnalyzer::TH125GenInfo()
{
	AddSpellPracticeInfo(m_pTHRpyInfo2->nSpellPracticeNumber, _T("th125sc.lst"), TRUE);
	THXAddExtraInfo2(0);
}

void CRPYAnalyzer::TH143GenInfo()
{
	AddSpellPracticeInfo(m_pTHRpyInfo2->nSpellPracticeNumber, _T("th143sc.lst"), TRUE);
	AddGameOptionsInfo(m_pTHRpyInfo2->wFlags);

	LPCTSTR const item_names[] = {
		_T("ひらり布"), _T("天狗のトイカメラ"), _T("隙間の折りたたみ傘"),
		_T("亡霊の送り提灯"), _T("血に飢えた陰陽玉"), _T("四尺マジックボム"),
		_T("身代わり地蔵"), _T("呪いのデコイ人形"), _T("打ち出の小槌（レプリカ）")
	};

	const DWORD   mainID    = m_pTHRpyInfo2->halfinfo.nMainItemID;
	const DWORD   subID     = m_pTHRpyInfo2->halfinfo.nSubItemID;
	LPCTSTR const mainName  = mainID < lengthof(item_names) ? item_names[mainID] : STR_ERROR;
	LPCTSTR const subName   = subID < lengthof(item_names) ? item_names[subID] : STR_ERROR;
	const DWORD   mainPower = m_pTHRpyInfo2->halfinfo.nMainItemPower;

	// 解释 power
	CString strMainPower;
	switch (mainID)
	{
	case 0: case 2: case 6: case 7: // time
		strMainPower.Format(_T("(%.2fs)"), (double)mainPower / 60.0); break;
	case 1: // photography range
		strMainPower.Format(_T("(%u%%)"), mainPower); break;
	case 5: // range
		strMainPower.Format(_T("(%.0f%%)"), (double)mainPower / 1.28); break;
	}

	CString strfmt;
	strfmt.Format(
		_T("主道具: %s(%u)\r\n")
		_T("能力值: %u%s\r\n")
		_T("副道具: %s(%u)\r\n\r\n")
	
		, mainName, m_pTHRpyInfo2->halfinfo.nMainItemCount
		, mainPower, (LPCTSTR)strMainPower
		, subName, m_pTHRpyInfo2->halfinfo.nSubItemCount
	);
	m_info += strfmt;

	THXAddExtraInfo2(0);
}

void CRPYAnalyzer::THXAddExtraInfo2(int nIndex)
{
	THX_EXTRAINFO extrainfo = {0};
	DWORD dwFrameCounts[10] = {0};
	const int nStageCount = m_pTHRpyInfo2->nStageCount;
	const int denominator = this->m_header == mgc95 ? 6 : 1; // th95 需要将 keystatesize / 6 才是正确的帧数

	if (nIndex < 0) { // get total
		for (int i=0; i<nStageCount; ++i) {
			dwFrameCounts[i] = m_pTHRpyInfo2->pStageInfo.thhdr[i]->dwhdrKeyStateSize / denominator;
		}
		extrainfo.szPlayTime = common_szTotalPlayTime;
		extrainfo.szSlowRate = common_szTotalSlowRate;
	}
	else {
		dwFrameCounts[nIndex] = m_pTHRpyInfo2->pStageInfo.thhdr[nIndex]->dwhdrKeyStateSize / denominator;
		extrainfo.szPlayTime  = common_szPlayTime;
		extrainfo.szSlowRate  = common_szSlowRate;
	}

	extrainfo.pFPSInfo = (TH_FPSINFO*)&m_pTHRpyInfo2->fpsinfo;
	extrainfo.nIndex = nIndex;
	extrainfo.dwFrameCounts = dwFrameCounts;
	THXAddExtraInfo(&extrainfo);
}

void CRPYAnalyzer::AddGameOptionsInfo(WORD wFlags, BOOL b6to9)
{
	CString strOptions;

	if (b6to9) {
		if (wFlags & RPYFLAG_SHOTSLOW)
			strOptions += _T("ShotSlow, ");
	}
	else {
		if (wFlags & RPYFLAG2_NODINPUT)
			strOptions += _T("NoDirectInput, ");
		if (wFlags & RPYFLAG2_FAITHGUAGE)
			strOptions += _T("FaithGuage, ");
		if (wFlags & RPYFLAG2_SHOTSLOW)
			strOptions += _T("ShotSlow, ");
	}

	if (!strOptions.IsEmpty()) {
		strOptions.Insert(0, _T("※ "));
		strOptions.Delete(strOptions.GetLength()-2, 2); // delete the last ', '
		m_info += strOptions + _T("\r\n");
	}
}

///////////////////// 其他 ///////////////////////

BOOL CRPYAnalyzer::DumpRPYData(LPCTSTR filename) const
{
	DWORD size;
	const BYTE* const pData = GetDecodedDataPointer(&size);
	if (pData)
		return ::DumpBinData(pData, size, filename);
	else
		return FALSE;
}

void CRPYAnalyzer::GetResultPointers(RPYINFO_POINTERS* pointers) const
{
	pointers->pRPYInfo1 = m_pTHRpyInfo;
	pointers->pRPYInfo2 = m_pTHRpyInfo2;

	DWORD size;
	pointers->pDecompData = GetDecodedDataPointer(&size);
	pointers->dwDecompDataSize = size;
}
