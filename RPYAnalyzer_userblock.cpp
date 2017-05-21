#include "StdAfx.h"
#include "resource.h"
#include "RPYAnalyzer_userblock.h"
#include "cpconv.h"
#include "RPYAnalyzer_common.h"
#include "RPYAnalyzer_formatter.h"
#include "global.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace RPYAnalyzer_formatter;

CRpyInfoUserBlock::CRpyInfoUserBlock(const RPYUserBlockMgr* pUBMgr, RPYMGC header, UINT codepage, BOOL isCNVer)
	: m_info()
{
	m_pUBMgr   = new RPYUserBlockMgr(*pUBMgr);
	m_header   = header;
	m_codepage = codepage;
	m_isCNVer  = isCNVer;
}

CRpyInfoUserBlock::~CRpyInfoUserBlock(){
	delete m_pUBMgr;
	m_pUBMgr = NULL;
}


int CRpyInfoUserBlock::analyze()
{
	int     ret              = RPYINFO_OK;
	BOOL    bAddZero         = TRUE;
	LPCWSTR cwszFindForScore = _T("\r\nScore ");      //“分数”前的字符串，用于之后的格式化分数

	// get text format rpy info
	switch ( m_header )
	{
	// TH6, TH7 没有文本格式的录像信息
	case mgc6:
	case mgc7:
		break;
	case mgc8: {
		m_codepage = m_isCNVer ? 936 : 932;
		ret = GetUBRpyInfo();
		if (ret!=RPYINFO_OK) break;

		if (m_isCNVer) {
			m_info.Insert(0, _T("游戏名\t\t东方永夜抄 中文版\r\n"));
			cwszFindForScore = _T("\r\n分数  \t\t");
		}
		else {
			m_info.Insert(0, _T("ゲーム\t\t東方永夜抄\r\n"));
			cwszFindForScore = _T("\r\nスコア\t\t");
		}
		bAddZero=FALSE;
		break;
		}
	case mgc9:
		ret = GetUBRpyInfo();

		break;
	//  alco,95,125,alco 无需补 0
	case mgc95:
	case mgc125:
	case mgcalco:
		bAddZero = FALSE;
		// no break
	case mgc10:
	case mgc11:
	case mgc12:
	case mgc128:
	case mgc13:
	case mgc15:
	case mgc16:
	case mgc143:
		ret = GetUBRpyInfo();
		if (ret != RPYINFO_OK) break;

		// 有部分作品 ZUN 都忘记（懒得？）改录像信息里的游戏名了……
		switch(m_header)
		{
		case mgc125:
			m_info.Replace(_T("東方文花帖"), _T("ダブルスポイラー～東方文花帖"));
			break;
		case mgc128:
			if (m_codepage==936) // 東方喝茶委员会 简体中文版
				m_info.Replace(_T("东方星莲船"), _T("妖精大戦争～東方三月精"));
			else
				m_info.Replace(_T("東方星蓮船"), _T("妖精大戦争～東方三月精"));
			break;
		case mgcalco: // 可怜的黄昏酒场，连游戏名都没……
			m_info.Insert(0, _T("黄昏酒場 "));
			break;
		}
		break;
	default:
		ret = RPYINFO_UNKNOWNFORMAT;
	}

	//查找注释 + 用FormatScore格式化文件末尾的文本录像信息中的分数（TH6，TH7除外）
	if (ret==RPYINFO_OK && m_header!=mgc6 && m_header!=mgc7) {
		//查找分数
		//查找 "Score " 在 m_info 中的位置
		int pos = m_info.Find(cwszFindForScore);
		if (pos > -1) {
			pos += 2;

			// 指向 "Score " 的指针
			LPCWSTR pScoreBuffer = (LPCWSTR)(m_info) + pos - 1;
			WCHAR chScore[16];  // 用于存储 m_UserData 中文本格式的分数。
			CopyInfoLine(chScore, pScoreBuffer);
			
			CString inScore(chScore), outScore;
			FormatScore(inScore, outScore, bAddZero);

			// 用格式化后的得分替换原有的得分
			CString strFindForScore(cwszFindForScore);
			m_info.Replace( strFindForScore + chScore, strFindForScore + outScore);
		}
	}

	return ret;
}

int CRpyInfoUserBlock::GetUBRpyInfo()
{
	// 获取 UserBlock 中的 info block
	const RPYUserBlock* const pUBInfo = m_pUBMgr->getInfo();

	if (pUBInfo && pUBInfo->isValid()) {
		// 转码
		LPWSTR const pUserDataW = Ansi2Unicode( (LPCSTR)pUBInfo->getData(), m_codepage, NULL, static_cast<int>(pUBInfo->getDataSize()) );
		
		if (pUserDataW) {
			m_info = pUserDataW; //CString=LPWSTR;
			delete []pUserDataW;
			return RPYINFO_OK;
		}
		else {
			LPCTSTR errmsg = GetErrorMessage(GetLastError());
			m_info.LoadString(IDS_CONVCODEERROR);
			m_info += errmsg;
			
			return RPYINFO_THHYL_CONVCODEERROR;
		}
	}
	else
		return RPYINFO_UNKNOWNFORMAT; // 不允许没有 info block 的 RPY
}

CString CRpyInfoUserBlock::getComment(UINT codepage)
{
	CString strComment;
	const RPYUserBlock* pUBComment = m_pUBMgr->getComment();
	
	if (pUBComment && pUBComment->isValid()) {
		LPWSTR const pCommentW = Ansi2Unicode( (LPCSTR)pUBComment->getData(), codepage, NULL, pUBComment->getDataSize() );
		if (pCommentW != NULL) {
			strComment = pCommentW;
			delete []pCommentW;
		}
	}

	return strComment;
}
