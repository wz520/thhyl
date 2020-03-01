#include "StdAfx.h"
#include "cfgfile.h"
#include "thhyl.h"
#include "RPYAnalyzer.h"
#include "global.h"
#include "indep/cpconv.h"

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

static LPCTSTR const STR_ERRORSTAGEINFO =_T("\r\n�@ȡ���P����ʧ����\r\n");
static LPCTSTR const STR_ERROR =_T("Error!");

static LPCTSTR const common_szPlayTime = _T("     PlayTime: %7u(%d:%02d.%03d)\r\n");
static LPCTSTR const common_szSlowRate = _T("SlowRateByFPS: %f%%\r\n");
static LPCTSTR const common_szTotalPlayTime = _T("\r\nTotal PlayTime: %u(%d:%02d.%03d)");
static LPCTSTR const common_szTotalSlowRate = _T("\r\nTotal SlowRateByFPS: %f%%");


// ����ת������
static inline int transPosX(int x) {return x/128+224;}     // �� TH10 �� �� �ƻ�Ƴ�
static inline int transPosY(int y) {return y/128+16;}      // �� TH10 �� �� �ƻ�Ƴ�
static inline int transTH10PosX(int x) {return x/100+224;} // �ƻ�Ƴ�Ҳ����
static inline int transTH10PosY(int y) {return y/100+16;}  // �ƻ�Ƴ�Ҳ����

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

	m_pUBMgr  = new RPYUserBlockMgr(data, size);
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

// ��ȡ 6, 7, 8, 9 �������� (6, 7 ����������Ϣ)
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
					m_header == mgc6 ? _T("��ħ��") : _T("������"));
			}
		}
	}

	if ( ret!=RPYINFO_OK && (m_header==mgc8 || m_header==mgc9) )
		m_info += STR_ERRORSTAGEINFO;

	return ret;
}

// ��ȡ 10 �Ժ���Լ� alcostg ��������
int CRPYAnalyzer::GenInfoWrapper2()
{
	RPYMGC const Header = *((RPYMGC*)m_pData);
	m_pTHRpyInfo2 = new THRPYINFO2; // will be deleted in destructor

	// NOTE: 1.70 ����ǰ�İ汾�����һ����������д���� m_pTHRpyInfo����һ��С BUG��
	// ����ò��û������ɶ���⡭��
	
	struct STAGEINFOFUNCMAP { //������
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
		{mgc17,   TH17GenStageInfo},
		{mgc17tr, TH17GenStageInfo},
		{mgc143,  TH143GenInfo}, // no stageinfo
		{mgc165,  TH165GenInfo}, // no stageinfo
		{mgcalco, THALGenStageInfo}
	};
	int ret = RPYINFO_UNKNOWNFORMAT;

	for (int i=0; i<lengthof(StageInfoFuncMap); ++i) {
		if (Header == StageInfoFuncMap[i].header) {
			m_header = Header;
			m_pDecompData = ReplayDecode2(m_pData, m_nDataSize, m_pTHRpyInfo2);
			if ( m_pDecompData ) {

#if 0
# ifdef _DEBUG
				DumpRPYData(_T("c:\\rpyraw.bin"));
# endif
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

	// ֻ�� TH8 ���ı���ʽ��¼����Ϣ��Ҫ�Զ��жϱ���
	const BOOL isCNVer = (m_pTHRpyInfo && (m_pTHRpyInfo->wFlags & RPYFLAG_CNVER)) ? true : false;
	m_pInfoUB = new CRpyInfoUserBlock(m_pUBMgr, Header, m_uDefCode, isCNVer);  // ���������� delete
	ret = m_pInfoUB->analyze();
	m_info.Insert(0, m_pInfoUB->getSummary());
	return ret;
}

CString CRPYAnalyzer::GetComment(UINT codepage)
{
	return m_pInfoUB->getComment(codepage); // �˴����ؼ�� m_pInfoUB �ĺϷ���
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

	CString StrFormat2, StrBomb, StrScore, StrStageScores;

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

	// ������Ϣ
	m_info.Format(
		_T("�[��: �|���tħ�_ %lu.%02lu\r\n")
		_T("���: %s\r\n����: %s\r\n")
		_T("��ɫ: %s\r\n�y��: %s\r\n")
		_T("�֔�: %s\r\n��: %f%%\r\n"),

		(DWORD)(pGeneralInfo->cGameMajorVersion), (DWORD)(pGeneralInfo->cGameMinorVersion)
		,(LPCTSTR)StrPlayer, (LPCTSTR)StrDate
		,(LPCTSTR)StrChara, (LPCTSTR)StrRank
		,(LPCTSTR)StrScore, pGeneralInfo->fDrop
	);

	// Start to get stage info
	THX_EXTRAINFO extrainfo={0};
	extrainfo.pFPSInfo = NULL;
	for (int i=0; i<m_pTHRpyInfo->nStageCount; ++i) {
		TH6_STAGEINFO* const pCurrStage = ppStageInfo[i];
		// ��Ч��������ʾ
		if (pCurrStage == NULL)
			continue;

		FormatScore(pCurrStage->dwScores, StrStageScores, FALSE, TRUE);

		Num2Star(pCurrStage->cLives, StrPlayer, 8);
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
			,(LPCTSTR)StrPlayer
			,(LPCTSTR)StrBomb
			,(int)pCurrStage->cPlayRank
			,(int)pCurrStage->wPoints
		);

		m_info += StrFormat2;
		
		//PlayTime, SlowRate
		extrainfo.nIndex        = i;
		extrainfo.dwFrameCounts = m_pTHRpyInfo->dwFrameCounts;
		extrainfo.szPlayTime    = common_szPlayTime;
		extrainfo.szSlowRate    = NULL;
		THXAddExtraInfo(&extrainfo);
		
		//ClearScore
		StrFormat2.Format(_T("  Clear Score:%s\r\n"), (LPCTSTR)(StrStageScores));

		m_info += StrFormat2;
	}

	extrainfo.nIndex        = -1;
	extrainfo.dwFrameCounts = m_pTHRpyInfo->dwFrameCounts;
	extrainfo.szPlayTime    = common_szTotalPlayTime;
	extrainfo.szSlowRate    = NULL;
	THXAddExtraInfo(&extrainfo);
}

////////////////////// TH7 //////////////////

void CRPYAnalyzer::TH7GenInfo()
{
	TH7_RPYINFO*    const pGeneralInfo = &m_pTHRpyInfo->GeneralInfo.th7;
	TH7_STAGEINFO**       ppStageInfo  = m_pTHRpyInfo->pStageInfo.th7;

	CString StrFormat2, StrBomb, StrScore, StrStageScores;

	const UINT CharaId[6] = { IDS_RMA, IDS_RMB, IDS_MRSA, IDS_MRSB, IDS_SKYA, IDS_SKYB };
	const UINT RankId[6] = { IDS_EASY, IDS_NORMAL, IDS_HARD, IDS_LUNATIC, IDS_EXTRA, IDS_PH };
	
	CString StrPlayer(pGeneralInfo->szPlayer), StrDate(pGeneralInfo->szDate), StrVersion(pGeneralInfo->szVersion);
	FormatScore(pGeneralInfo->dwScore, StrScore, TRUE);

	if (m_pTHRpyInfo->wFlags & RPYFLAG_CNVER) // Chinese version
		StrVersion += _T(" ���İ�");

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
	
	// ������Ϣ
	m_info.Format(
		_T("�[��: �|�������� %s\r\n")
		_T("���: %s\r\n����: %s\r\n")
		_T("��ɫ: %s\r\n�y��: %s\r\n")
		_T("�֔�: %s\r\n��: %f%%\r\n"),
			
		(LPCTSTR)StrVersion
		,(LPCTSTR)StrPlayer, (LPCTSTR)StrDate
		,(LPCTSTR)StrChara, (LPCTSTR)StrRank
		,(LPCTSTR)StrScore, pGeneralInfo->fDrop
	);

	AddGameOptionsInfo(m_pTHRpyInfo->wFlags, TRUE);

	// Start to get stage info
	THX_EXTRAINFO extrainfo={0};
	extrainfo.pFPSInfo = &m_pTHRpyInfo->fpsinfo;
	for (int i=0; i<m_pTHRpyInfo->nStageCount; ++i) {
		TH7_STAGEINFO* const pCurrStage = ppStageInfo[i];
		// ��Ч��������ʾ
		if (pCurrStage == NULL)
			continue;

		FormatScore(pCurrStage->dwScores, StrStageScores, TRUE, TRUE);

		Num2Star(pCurrStage->cLives, StrPlayer, 8);
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
			_T("          ��+:%8d\r\n")
			_T("           ��:%8d/%d\r\n")

			,m_pTHRpyInfo->stagenames[i]
			,(int)pCurrStage->cPowers
			,(LPCTSTR)StrPlayer
			,(LPCTSTR)StrBomb
			,(int)pCurrStage->cPlayRank
			,pCurrStage->nPoints, pCurrStage->nNextPoints
			,pCurrStage->nGrazes
			,pCurrStage->cSpellCardBonusCount
			,pCurrStage->n1upByPointsCount
			,pCurrStage->nCurrCherriesPlus
			,pCurrStage->nCurrCherries, pCurrStage->nMaxCherries
		);

		m_info += StrFormat2;
		
		//PlayTime, SlowRate
		extrainfo.nIndex        = i;
		extrainfo.dwFrameCounts = m_pTHRpyInfo->dwFrameCounts;
		extrainfo.szPlayTime    = common_szPlayTime;
		extrainfo.szSlowRate    = common_szSlowRate;
		THXAddExtraInfo(&extrainfo);
		
		//ClearScore
		StrFormat2.Format(_T("  Clear Score:%s\r\n"), (LPCTSTR)(StrStageScores));
		m_info += StrFormat2;
	}

	extrainfo.nIndex        = -1;
	extrainfo.dwFrameCounts = m_pTHRpyInfo->dwFrameCounts;
	extrainfo.szPlayTime    = common_szTotalPlayTime;
	extrainfo.szSlowRate    = common_szTotalSlowRate;
	THXAddExtraInfo(&extrainfo);
}

////////////////////// TH8 //////////////////

void CRPYAnalyzer::TH8GenStageInfo()
{
	TH8_STAGEINFO** ppStageInfo = m_pTHRpyInfo->pStageInfo.th8;

	CString StrFormat2, StrStageScores, StrPlayer, StrBomb, strGameTime;

	THX_EXTRAINFO extrainfo = {0};
	extrainfo.pFPSInfo = &m_pTHRpyInfo->fpsinfo;

	AddGameOptionsInfo(m_pTHRpyInfo->wFlags, TRUE);

	for (int i=0; i<m_pTHRpyInfo->nStageCount; ++i) {
		TH8_STAGEINFO* const pCurrStage = ppStageInfo[i];
		// ��Ч��������ʾ
		if (pCurrStage == NULL)
			continue;

		FormatScore(pCurrStage->dwScores, StrStageScores, TRUE, TRUE);

		Num2Star(pCurrStage->cLives, StrPlayer, 8);
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
			_T("         ����:%5d.%02d%%\r\n")
			_T("         ҹ�c:%8d\r\n")
			_T("         �r��:%8d �� (%s)\r\n")

			,m_pTHRpyInfo->stagenames[i]
			,(int)pCurrStage->cPowers
			,(LPCTSTR)StrPlayer
			,(LPCTSTR)StrBomb
			,(int)pCurrStage->cPlayRank
			,pCurrStage->nPoints, pCurrStage->nNextPoints
			,pCurrStage->nGrazes
			,pCurrStage->cSpellCardBonusCount
			,pCurrStage->n1upByPointsCount
			,pCurrStage->sYaoLv/100, abs(pCurrStage->sYaoLv%100) //����
			,pCurrStage->nNightPoints
			,pCurrStage->cGameTime, (LPCTSTR)TH8FormatGameTime(pCurrStage->cGameTime, strGameTime) // ʱ��
		);
		
		m_info += StrFormat2;
		
		//PlayTime, SlowRate
		extrainfo.nIndex        = i;
		extrainfo.dwFrameCounts = m_pTHRpyInfo->dwFrameCounts;
		extrainfo.szPlayTime    = common_szPlayTime;
		extrainfo.szSlowRate    = common_szSlowRate;
		THXAddExtraInfo(&extrainfo);

		//ClearScore
		StrFormat2.Format(_T("  Clear Score:%s\r\n"), (LPCTSTR)(StrStageScores));
		m_info += StrFormat2;
	}

	extrainfo.nIndex        = -1;
	extrainfo.dwFrameCounts = m_pTHRpyInfo->dwFrameCounts;
	extrainfo.szPlayTime    = common_szTotalPlayTime;
	extrainfo.szSlowRate    = common_szTotalSlowRate;
	THXAddExtraInfo(&extrainfo);
}

////////////////////// TH9 //////////////////
void CRPYAnalyzer::TH9GenStageInfo()
{
	THRPYINFO::TH9_STAGEINFO_BY_PLAYER ppStageInfo = m_pTHRpyInfo->pStageInfo.th9_by_player;

	// ��ս�������飨���İ棩  ��ע�⡿������ = m_pTHRpyInfo->cPlace + 2
	LPCTSTR PlaceListJP[] = { // start from -2
		_T("�ɤä�")          , _T("Ҏ���Έ���")       , // -2, -1
		_T("�Ԥ������� REIMU"), _T("�Ԥ������� MARISA"), // 0, 1
		_T("�ò�ԭ SAKUYA")   , _T("����¥�A�� YOUMU") , // ...
		_T("���hͤ REISEN")   , _T("�F�κ� CIRNO")     ,
		_T("�����Y�� LYRICA") , _T("���֪��� MYSTIA")  ,
		_T("�Ԥ������� TEI")  , _T("̫ꖤήx YUKA")    ,
		_T("���r�W�γ� AYA")  , _T("�o������ MEDICINE"),
		_T("��˼�ε� KOMACHI"), _T("�o�F�V SIKIEIKI")
	};
	// ��ս�������飨���İ棩  ��ע�⡿������ = m_pTHRpyInfo->cPlace + 2
	LPCTSTR PlaceListCN[] = { // start from -2
		_T("�S�C�x��")        , _T("�A���Ĉ���")       , // -2 , -1
		_T("��֮���� REIMU")  , _T("��֮���� MARISA")  , // 0  , 1
		_T("�ò�ԭ SAKUYA")   , _T("������_�A YOUMU") , // ...
		_T("���hͤ REISEN")   , _T("�F֮�� CIRNO")     ,
		_T("��ڤ�Y�� LYRICA") , _T("���֫F�� MYSTIA")  ,
		_T("��֮���� TEI")    , _T("̫ꖻ��� YUKA")    ,
		_T("����֮�� AYA")  , _T("�o��֮�� MEDICINE"),
		_T("��˼֮�� KOMACHI"), _T("�o�Fڣ SIKIEIKI")
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

	// Cirno is �� ?
	if (HasConfigOption(CFG_9))	{
		szChara[5] = _T("��");
		PlaceListJP[7] = _T("�F�κ� ��");
		PlaceListCN[7] = _T("�F֮�� ��");
	}

	//Charge Type
	{
		LPCTSTR const szChargeType[2] = { _T("SLOW"), _T("CHARGE") };
		StrFormat2.Format(
				_T("Player1 Charge Type: %s\r\n")
				_T("Player2 Charge Type: %s\r\n")
				, szChargeType[(m_pTHRpyInfo->wFlags & RPYFLAG_P1CHARGE) ? 1 : 0]
				, szChargeType[(m_pTHRpyInfo->wFlags & RPYFLAG_P2CHARGE) ? 1 : 0]
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
		//��Ч��������ʾ
		if (!ppStageInfo[0][i])
			continue;

		LPCTSTR szCurrChara[2] = {STR_ERROR, STR_ERROR};
		for ( int p = 0; p < 2; ++p ) {  // 2 players
			FormatScore(ppStageInfo[p][i]->dwScores, StrStageScores[p], TRUE, FALSE);
			cChara[p] = ppStageInfo[p][i]->cChara;
			if (cChara[p] < 16)
				szCurrChara[p] = szChara[cChara[p]];
		}
		
		// ��� player1 �� �ᣬ���ҿ����ˡ��ᡱ��ʾ����ʾ�����Դ�λ������ƫһ���ַ�����
		// �� StrFormat �е� "%17s" �ĳ� "%16s" ��������
		// ������������� player1 ��ɫ��������;���ģ�����ֻ����� StrFormat һ��
		if ( cChara[0] == 5 && HasConfigOption(CFG_9) && !bStrFormatModified) {
			StrFormat.Replace(_T("%17s"), _T("%16s"));
			bStrFormatModified = true;
		}

		StrFormat2.Format(StrFormat,
			m_pTHRpyInfo->stagenames[i],
			szCurrChara[0], szCurrChara[1],
			ppStageInfo[0][i]->cLives, ppStageInfo[1][i]->cLives,
			(LPCTSTR)(StrStageScores[0]), (LPCTSTR)(StrStageScores[1])
		);
		
		m_info += StrFormat2;

		// place, for match mode only
		if (i==9) { // match mode
			signed char cPlace = ppStageInfo[0][i]->cPlace + 2;
			LPCTSTR* const PlaceList = (m_pTHRpyInfo->wFlags & RPYFLAG_CNVER)
				? PlaceListCN
				: PlaceListJP;

			LPCTSTR const szPlace = cPlace>=0 && cPlace<lengthof(PlaceListJP)
				? PlaceList[cPlace]
				: _T("Error");  // out of range

			StrFormat2.Format(_T("        Place: %s\r\n"), szPlace);
			m_info += StrFormat2;
		}

		//PlayTime, SlowRate
		extrainfo.nIndex        = i;
		extrainfo.dwFrameCounts = m_pTHRpyInfo->dwFrameCounts;
		extrainfo.szPlayTime    = _T("     PlayTime: %u(%d:%02d.%03d)\r\n");
		extrainfo.szSlowRate    = common_szSlowRate;
		THXAddExtraInfo(&extrainfo);
	}

	extrainfo.nIndex        = -1;
	extrainfo.dwFrameCounts = m_pTHRpyInfo->dwFrameCounts;
	extrainfo.szPlayTime    = common_szTotalPlayTime;
	extrainfo.szSlowRate    = common_szTotalSlowRate;
	THXAddExtraInfo(&extrainfo);
}

void CRPYAnalyzer::THXAddExtraInfo(const THX_EXTRAINFO* pExtraInfo)
{
	DWORD dwTotalPlayTime;
	double slowrate;
	
	if (pExtraInfo->nIndex < 0) { // get total
		// �������йؿ��ĺϼ���֡��
		// pExtraInfo->dwFrameCounts Ӧ���� 20 �����ݣ�������ֻ�ۼ�ǰ 10 ����
		// ��Ϊֻ�� TH9 �Ĺؿ����ݻᳬ�� 10 ����
		// �� TH9 �ӵ� 11 �����ݿ�ʼ���� P2 �ĵ� 1 �����ݣ�����֡����Ȼ�ò�����
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
		const DWORD m  = dwTotalPlayTime/3600;
		const DWORD s  = dwTotalPlayTime/60%60;
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
			(i == m_pTHRpyInfo2->nStageCount-1) //���һ��ȡ dwClearScore
				? m_pTHRpyInfo2->dwClearScore : ppStageInfo[i+1]->dwScore,
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
			(i == m_pTHRpyInfo2->nStageCount-1) //���һ��ȡ dwClearScore
				? m_pTHRpyInfo2->dwClearScore : ppStageInfo[i+1]->dwScore,
			StrStageScore, TRUE, TRUE, ppStageInfo[i]->dwContinuedTimes);

		Num2Star(pCurrStage->dwPlayer, strPlayer, 9);

		StrFormat2.Format(
			_T("\r\nStage %s:\r\n")
			_T("       Player: %s\r\n")
			_T("        Power:%5lu.%02lu(%lu)\r\n")
			_T("         ����:%8lu\r\n")
			_T("       �B����:%8lu\r\n")
			_T("         ����:%8d/%d(%d/%d)\r\n")
			
			, m_pTHRpyInfo2->stagenames[pCurrStage->hdr.wStageNumber-1]
			, (LPCTSTR)strPlayer
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

	// �ж��Ƿ���ħ��ɳA��ħ���飩����Ϊħ��ɳA �� power ��ʾֵ�㷨���ڲ�ͬ
	const bool isMarisaA = (m_pTHRpyInfo2->dwID == 1 && m_pTHRpyInfo2->dwEquipID == 0);
	DWORD dwPowerInt, dwPowerDec; // ���� power ��ʾ
	
	for (int i=0; i<m_pTHRpyInfo2->nStageCount; i++) {
		TH11_STAGEINFO* const pCurrStage = ppStageInfo[i];
		
		FormatScore(
			(i == m_pTHRpyInfo2->nStageCount-1) //���һ��ȡ dwClearScore
				? m_pTHRpyInfo2->dwClearScore : ppStageInfo[i+1]->dwScore,
			StrStageScore, TRUE, TRUE, ppStageInfo[i]->dwContinuedTimes);
		
		Num2Star(pCurrStage->wPlayerInt, strPlayer, 9, pCurrStage->wPlayerDec);
		
		// ���� power ��ʾֵ
		if (isMarisaA) {
            dwPowerInt = pCurrStage->dwPower/12;        // ��������
            dwPowerDec = pCurrStage->dwPower%12*100/12; // С������
		}
		else {
            dwPowerInt = pCurrStage->dwPower/20;   // ��������
            dwPowerDec = pCurrStage->dwPower%20*5; // С������
		}

		// ������С�õ�����õ�
		const DWORD base_pt = pCurrStage->dwConnect - pCurrStage->dwConnect % 10; // ȷ����λΪ 0
		const DWORD min_rate = min(pCurrStage->dwGraze/100, 899);
		const DWORD max_rate = min_rate + 100;
		const DWORD min_pt = min_rate * base_pt / 100;
		const DWORD max_pt = max_rate * base_pt / 100;

		StrFormat2.Format(
			_T("\r\nStage %s:\r\n")
			_T("       Player: %s\r\n")
			_T("        Power:%5lu.%02lu(%lu)\r\n")
			_T("        Graze:%8lu\r\n")
			_T("     ��С���c:%8lu(%lu��%lu.%02lu)\r\n")
			_T("     �����c:%8lu(%lu��%lu.%02lu)\r\n")
			_T("         ����:%8d/%d(%d/%d)\r\n")

			, m_pTHRpyInfo2->stagenames[pCurrStage->hdr.wStageNumber-1]
			, (LPCTSTR)strPlayer
			, dwPowerInt, dwPowerDec, pCurrStage->dwPower
			, pCurrStage->dwGraze
			, min_pt, pCurrStage->dwConnect, min_rate/100, min_rate%100
			, max_pt, pCurrStage->dwConnect, max_rate/100, max_rate%100
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
			(i == m_pTHRpyInfo2->nStageCount-1) //���һ��ȡ dwClearScore
				? m_pTHRpyInfo2->dwClearScore : ppStageInfo[i+1]->dwScore,
			StrStageScore, TRUE, TRUE, ppStageInfo[i]->dwContinuedTimes);
		
		Num2Star(pCurrStage->wPlayerInt, strPlayer, 9, pCurrStage->wPlayerDec);
		Num2Star(pCurrStage->wBombInt, strBomb, 9, pCurrStage->wBombDec);
		TH12FormatUFOStock(pCurrStage->dwUFOStock, strUFOStock);
		
		StrFormat2.Format(
			_T("\r\nStage %s:\r\n")
			_T("       Player: %s\r\n")
			_T("         Bomb: %s\r\n")
			_T("        Power:%9lu.%02lu\r\n")
			_T("        Graze:%12lu\r\n")
			_T("     �����c:%9lu.%02lu\r\n")
			_T("          UFO:      %s\r\n")
			_T("         ����:%8d/%d(%d/%d)\r\n")

			, m_pTHRpyInfo2->stagenames[pCurrStage->hdr.wStageNumber-1]
			, (LPCTSTR)strPlayer
			, (LPCTSTR)strBomb
			, pCurrStage->dwPower/100, pCurrStage->dwPower%100
			, pCurrStage->dwGraze
			, pCurrStage->dwMaxScore/100, pCurrStage->dwMaxScore%100
			, (LPCTSTR)strUFOStock
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
			(i == m_pTHRpyInfo2->nStageCount-1) //���һ��ȡ dwClearScore
				? m_pTHRpyInfo2->dwClearScore : ppStageInfo[i+1]->dwScore,
			StrStageScore, TRUE, TRUE);
		
		StrFormat2.Format(
			_T("\r\nStage %s:\r\n")
			_T("         �ք�:%11lu.%02lu%%\r\n")
			_T("       ���Y��:%10lu.%03lu%%\r\n")
			_T("     �������Y:%11lu.%02lu%%\r\n")
			_T("     ���Y��e:%14f%%\r\n")
			_T("        LEVEL:%15lu\r\n")
			_T("         ����:%15d/%d(%d/%d)\r\n")

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
	DWORD const PlayerFragmentNormList[] = {8, 10, 12, 15, 18, 20, 25}; // ������Ƭ�����б�ֻ������ʽ�档
	DWORD const dwBombFragmentNorm = (m_pTHRpyInfo2->wVersion > 1) // ��Bomb��Ƭ�Ķ��������Ϸ�汾����
		? 8 // ��ʽ��
		: 10; // ����� 0.01a
	AddSpellPracticeInfo(m_pTHRpyInfo2->nSpellPracticeNumber, _T("th13.sclist"));
	AddGameOptionsInfo(m_pTHRpyInfo2->wFlags);
	
	CString StrFormat2, StrStageScore, strPlayer, strBomb;
	
	for (int i=0; i<m_pTHRpyInfo2->nStageCount; i++) {
		TH13_STAGEINFO* const pCurrStage = ppStageInfo[i];
		TH13_STAGEINFO* const pCurrStageRelease = (m_pTHRpyInfo2->wVersion > 1) // ������ʽ�沿��ƫ����Ҫ����
			? (TH13_STAGEINFO*)(((BYTE*)pCurrStage)+4) // ��ʽ��Ĳ�������ƫ����Ҫ+4
			: pCurrStage; // ����� 0.01a
		
		FormatScore(
			(i == m_pTHRpyInfo2->nStageCount-1) //���һ��ȡ dwClearScore
				? m_pTHRpyInfo2->dwClearScore : ppStageInfo[i+1]->dwScore,
			StrStageScore, TRUE, TRUE);

		if ( pCurrStageRelease->dwPlayerFragmentNormIndex >= lengthof(PlayerFragmentNormList) )
			dwPlayerFragmentNorm = (DWORD)-1; // -1 means invalid norm
		else {
			// ��ʽ�� �� ����� ������Ƭ�Ķ��ͬ����ʽ�����ˣ���
			// �����Ķ����б�Ϊ [10,15,20,25,30,35,40]
			// �������������治ʹ���б���Ϊֱ����ô����򵥣� 10+i*5 ��
			if (m_pTHRpyInfo2->wVersion > 1) // ��ʽ��
				dwPlayerFragmentNorm = PlayerFragmentNormList[pCurrStageRelease->dwPlayerFragmentNormIndex];
			else
				dwPlayerFragmentNorm = 10 + pCurrStageRelease->dwPlayerFragmentNormIndex*5;
		}

		Num2Star(pCurrStageRelease->dwPlayer, strPlayer, 9);
		Num2Star(pCurrStageRelease->dwBomb, strBomb, 9);
		
		StrFormat2.Format(
			_T("\r\nStage %s:\r\n")
			_T("       Player: %s\r\n")
			_T("  Player ��Ƭ:%12lu/%lu\r\n")
			_T("         Bomb: %s\r\n")
			_T("    Bomb ��Ƭ:%12lu/%lu\r\n")
			_T("        Power:%9lu.%02lu\r\n")
			_T("        Graze:%12lu\r\n")
			_T("     �����c:%9lu.%02lu\r\n")
			_T("       �`���:%12lu\r\n")
			_T("         ����:%12d/%d(%d/%d)\r\n")
			
			, m_pTHRpyInfo2->stagenames[pCurrStage->hdr.wStageNumber-1]
			, (LPCTSTR)strPlayer
			, pCurrStageRelease->dwPlayerFragment, dwPlayerFragmentNorm
			, (LPCTSTR)strBomb
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

	AddSpellPracticeInfo(m_pTHRpyInfo2->nSpellPracticeNumber, _T("th14.sclist"));
	AddGameOptionsInfo(m_pTHRpyInfo2->wFlags);
	
	for (int i=0; i<m_pTHRpyInfo2->nStageCount; i++) {
		TH14_STAGEINFO* const pCurrStage = ppStageInfo[i];
		
		FormatScore(
			(i == m_pTHRpyInfo2->nStageCount-1) //���һ��ȡ dwClearScore
				? m_pTHRpyInfo2->dwClearScore : ppStageInfo[i+1]->dwScore,
			StrStageScore, TRUE, TRUE, 0);
		
		Num2Star(pCurrStage->dwPlayer, strPlayer, 9, pCurrStage->dwPlayerFragment);
		Num2Star(pCurrStage->dwBomb, strBomb, 9, pCurrStage->dwBombFragment);
		
		StrFormat2.Format(
			_T("\r\nStage %s:\r\n")
			_T("       Player: %s\r\n")
			_T("         Bomb: %s\r\n")
			_T("     1UPCount:%12lu\r\n")
			_T("  Non2.0Bonus:%12lu\r\n")
			_T("        Power:%9lu.%02lu\r\n")
			_T("        Graze:%12lu\r\n")
			_T("     �����c:%9lu.%02lu\r\n")
			_T("         ����:%12d/%d(%d/%d)\r\n")
			
			, m_pTHRpyInfo2->stagenames[pCurrStage->hdr.wStageNumber-1]
			, (LPCTSTR)strPlayer
			, (LPCTSTR)strBomb
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
			(i == m_pTHRpyInfo2->nStageCount-1) //���һ��ȡ dwClearScore
				? m_pTHRpyInfo2->dwClearScore : ppStageInfo[i+1]->dwScore,
			StrStageScore, TRUE, TRUE, 0);
		
		Num2Star(pCurrStage->dwPlayer, strPlayer, 9, pCurrStage->dwPlayerFragment);
		Num2Star(pCurrStage->dwBomb, strBomb, 9, pCurrStage->dwBombFragment);
		
		StrFormat2.Format(
			_T("\r\nStage %s:\r\n")
			_T("       Player: %s\r\n")
			_T("         Bomb: %s\r\n")
			_T("     1UPCount:%12lu\r\n")
			_T("        Power:%9lu.%02lu\r\n")
			_T("        Graze:%12lu\r\n")
			_T("     �����c:%9lu.%02lu\r\n")
			_T("         ����:%12d/%d(%d/%d)\r\n")
			
			, m_pTHRpyInfo2->stagenames[pCurrStage->hdr.wStageNumber-1]
			, (LPCTSTR)strPlayer
			, (LPCTSTR)strBomb
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

	// �Դ�¼����Ϣ��ȱ���ӻ�����
	{
		LPCTSTR szSubWeapon;
		switch (m_pTHRpyInfo2->dwEquipID) {
		case 0: szSubWeapon = _T("Spring"); break;
		case 1: szSubWeapon = _T("Summer"); break;
		case 2: szSubWeapon = _T("Autumn"); break;
		case 3: szSubWeapon = _T("Winter"); break;
		case 4: szSubWeapon = _T("Full"); break;
		default: szSubWeapon = _T("ERROR!"); break;
		}
		StrFormat2.Format(_T("Season %s\r\n"), szSubWeapon);
		m_info += StrFormat2;
	}

	AddSpellPracticeInfo(m_pTHRpyInfo2->nSpellPracticeNumber, _T("th16.sclist"));
	AddGameOptionsInfo(m_pTHRpyInfo2->wFlags);
	
	for (int i=0; i<m_pTHRpyInfo2->nStageCount; i++) {
		TH16_STAGEINFO* const pCurrStage = ppStageInfo[i];
		
		FormatScore(
			(i == m_pTHRpyInfo2->nStageCount-1) //���һ��ȡ dwClearScore
				? m_pTHRpyInfo2->dwClearScore : ppStageInfo[i+1]->dwScore,
			StrStageScore, TRUE, TRUE, 0);
		
		Num2Star(pCurrStage->dwPlayer, strPlayer, 9);
		Num2Star(pCurrStage->dwBomb, strBomb, 9, pCurrStage->dwBombFragment);

		// ת�����ڲ�Ϊ�ӻ�
		DWORD dwSeasonNorm = 0;
		double dblSubWeapon = TH16FormatSeasonGauge(pCurrStage->dwSeasonGauge, dwSeasonNorm); 
		
		StrFormat2.Format(
			_T("\r\nStage %s:\r\n")
			_T("       Player: %s\r\n")
			_T("         Bomb: %s\r\n")
			_T("     1UPCount:%12lu\r\n")
			_T("        Power:%9lu.%02lu\r\n")
			_T("        Graze:%12lu\r\n")
			_T("       ������:%12.3f(%lu/%lu)\r\n")
			_T("     �����c:%9lu.%02lu\r\n")
			_T("         ����:%12d/%d(%d/%d)\r\n")
			
			, m_pTHRpyInfo2->stagenames[pCurrStage->hdr.wStageNumber-1]
			, (LPCTSTR)strPlayer
			, (LPCTSTR)strBomb
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

void CRPYAnalyzer::TH17GenStageInfo()
{
	TH17_STAGEINFO**       ppStageInfo    = m_pTHRpyInfo2->pStageInfo.th17;
	
	CString StrFormat2, StrStageScore, strPlayer, strBomb, strSpiritStock;

	AddSpellPracticeInfo(m_pTHRpyInfo2->nSpellPracticeNumber, _T("th17.sclist"));
	AddGameOptionsInfo(m_pTHRpyInfo2->wFlags);
	
	for (int i=0; i<m_pTHRpyInfo2->nStageCount; i++) {
		TH17_STAGEINFO* const pCurrStage = ppStageInfo[i];
		
		FormatScore(
			(i == m_pTHRpyInfo2->nStageCount-1) //���һ��ȡ dwClearScore
				? m_pTHRpyInfo2->dwClearScore : ppStageInfo[i+1]->dwScore,
			StrStageScore, TRUE, TRUE, 0);
		
		Num2Star(pCurrStage->dwPlayer, strPlayer, 9, pCurrStage->dwPlayerFragment);
		Num2Star(pCurrStage->dwBomb, strBomb, 9, pCurrStage->dwBombFragment);
		TH17FormatSpiritStock(pCurrStage->dwSpiritCount, pCurrStage->dwSpirits, strSpiritStock);

		StrFormat2.Format(
			_T("\r\nStage %s:\r\n")
			_T("       Player: %s\r\n")
			_T("         Bomb: %s\r\n")
			_T("     1UPCount:%12lu\r\n")
			_T("        Power:%9lu.%02lu\r\n")
			_T("        Graze:%12lu\r\n")
			_T("         �`��:  %s(%d/%d)\r\n")
			_T("     �����c:%9lu.%02lu\r\n")
			_T("         ����:%12d/%d(%d/%d)\r\n")
			
			, m_pTHRpyInfo2->stagenames[pCurrStage->hdr.wStageNumber-1]
			, (LPCTSTR)strPlayer
			, (LPCTSTR)strBomb
			, pCurrStage->dw1upCount
			, pCurrStage->dwPower/100, pCurrStage->dwPower%100
			, pCurrStage->dwGraze
			, (LPCTSTR)strSpiritStock, pCurrStage->nRoaringTime, pCurrStage->nMaxRoaringTime
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

		if (isScene)
			strfmt.Format(_T("Scene No. %d"), SCNum);
		else
			strfmt.Format(_T("SpellCard No. %d"), SCNum);
		m_info += strfmt;

		CString strSCName;
		CString strSCFileName(g_exefullpath);
		CString strSCFileBasePath(_T("sclists\\"));
		filepath_utils::ChangeFilename(strSCFileName, strSCFileBasePath+filename);
		if ( ::ReadUnicodeFileLine(strSCFileName, SCNum, strSCName) ) {
			strfmt.Format(_T(" %s"), strSCName);
			m_info += strfmt;
		}

		m_info += _T("\r\n");
	}
}

void CRPYAnalyzer::TH95GenInfo()
{
	AddSpellPracticeInfo(m_pTHRpyInfo2->nSpellPracticeNumber, _T("th95.sclist"), TRUE);
	AddGameOptionsInfo(m_pTHRpyInfo2->wFlags);

	THXAddExtraInfo2(0);
}

void CRPYAnalyzer::TH125GenInfo()
{
	AddSpellPracticeInfo(m_pTHRpyInfo2->nSpellPracticeNumber, _T("th125.sclist"), TRUE);
	THXAddExtraInfo2(0);
}

void CRPYAnalyzer::TH143GenInfo()
{
	AddSpellPracticeInfo(m_pTHRpyInfo2->nSpellPracticeNumber, _T("th143.sclist"), TRUE);
	AddGameOptionsInfo(m_pTHRpyInfo2->wFlags);

	LPCTSTR const item_names[] = {
		_T("�Ҥ�겼"), _T("�칷�Υȥ������"), _T("϶�g���ۤꤿ���߂�"),
		_T("��둤��ͤ����"), _T("Ѫ���|��������"), _T("�ĳߥޥ��å��ܥ�"),
		_T("��������i"), _T("�⤤�Υǥ�������"), _T("�������С鳣���ץꥫ��")
	};

	const DWORD   mainID    = m_pTHRpyInfo2->halfinfo.nMainItemID;
	const DWORD   subID     = m_pTHRpyInfo2->halfinfo.nSubItemID;
	LPCTSTR const mainName  = mainID < lengthof(item_names) ? item_names[mainID] : STR_ERROR;
	LPCTSTR const subName   = subID < lengthof(item_names) ? item_names[subID] : STR_ERROR;
	const DWORD   mainPower = m_pTHRpyInfo2->halfinfo.nMainItemPower;

	// ���� power
	CString strMainPower;
	switch (mainID)
	{
	case 0: case 2: case 6: case 7: // time
		strMainPower.Format(_T("(%.2fs)"), (double)mainPower / 60.0); break;
	case 1: // photography range
		strMainPower.Format(_T("(%lu%%)"), mainPower); break;
	case 5: // range
		strMainPower.Format(_T("(%.0f%%)"), (double)mainPower / 1.28); break;
	}

	CString strfmt;
	strfmt.Format(
		_T("������: %s(%lu)\r\n")
		_T("����ֵ: %lu%s\r\n")
		_T("������: %s(%lu)\r\n\r\n")
	
		, mainName, m_pTHRpyInfo2->halfinfo.nMainItemCount
		, mainPower, (LPCTSTR)strMainPower
		, subName, m_pTHRpyInfo2->halfinfo.nSubItemCount
	);
	m_info += strfmt;

	THXAddExtraInfo2(0);
}

void CRPYAnalyzer::TH165GenInfo()
{
	AddSpellPracticeInfo(m_pTHRpyInfo2->nSpellPracticeNumber, _T("th165.sclist"), TRUE);
	AddGameOptionsInfo(m_pTHRpyInfo2->wFlags);

	CString strfmt;
	strfmt.Format(
		_T("������LV: %lu\r\n")
		_T("�؁��^: %s\r\n\r\n")
	
		, m_pTHRpyInfo2->halfinfo.nSkillLevel
		, m_pTHRpyInfo2->halfinfo.bRetried ? _T("��") : _T("��")
	);
	m_info += strfmt;

	THXAddExtraInfo2(0);
}

void CRPYAnalyzer::THXAddExtraInfo2(int nIndex)
{
	THX_EXTRAINFO extrainfo = {0};
	DWORD dwFrameCounts[10] = {0};
	const int denominator = this->m_header == mgc95 ? 6 : 1; // th95 ��Ҫ�� keystatesize / 6 ������ȷ��֡��

	if (nIndex < 0) { // get total
		const int nStageCount = m_pTHRpyInfo2->nStageCount;
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

	extrainfo.pFPSInfo      = const_cast<TH_FPSINFO*>(&m_pTHRpyInfo2->fpsinfo);
	extrainfo.nIndex        = nIndex;
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
		strOptions.Insert(0, _T("�� "));
		strOptions.Delete(strOptions.GetLength()-2, 2); // delete the last ', '
		m_info += strOptions + _T("\r\n");
	}
}

///////////////////// ���� ///////////////////////

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
