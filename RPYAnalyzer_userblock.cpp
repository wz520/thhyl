#include "StdAfx.h"
#include "resource.h"
#include "RPYAnalyzer_userblock.h"
#include "indep/cpconv.h"
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
	LPCWSTR cwszFindForScore = _T("\r\nScore ");      //��������ǰ���ַ���������֮��ĸ�ʽ������

	// get text format rpy info
	switch ( m_header )
	{
	// TH6, TH7 û���ı���ʽ��¼����Ϣ
	case mgc6:
	case mgc7:
		break;
	case mgc8: {
		m_codepage = m_isCNVer ? 936 : 932;
		ret = GetUBRpyInfo();
		if (ret!=RPYINFO_OK) break;

		if (m_isCNVer) {
			m_info.Insert(0, _T("��Ϸ��\t\t������ҹ�� ���İ�\r\n"));
			cwszFindForScore = _T("\r\n����  \t\t");
		}
		else {
			m_info.Insert(0, _T("���`��\t\t�|����ҹ��\r\n"));
			cwszFindForScore = _T("\r\n������\t\t");
		}
		bAddZero=FALSE;
		break;
		}
	case mgc9:
		ret = GetUBRpyInfo();

		break;
	//  alco,95,125,alco ���貹 0
	case mgc95:
	case mgc125:
	case mgcalco:
	case mgc165:
		bAddZero = FALSE;
		// no break
	case mgc10:
	case mgc11:
	case mgc12:
	case mgc128:
	case mgc13:
	case mgc15:
	case mgc16:
	case mgc17:
	case mgc143:
		ret = GetUBRpyInfo();
		if (ret != RPYINFO_OK) break;

		// �в�����Ʒ ZUN �����ǣ����ã�����¼����Ϣ�����Ϸ���ˡ���
		switch(m_header)
		{
		case mgc125:
			m_info.Replace(_T("�|���Ļ���"), _T("���֥륹�ݥ���`���|���Ļ���"));
			break;
		case mgc128:
			if (m_codepage==936) // �|���Ȳ�ίԱ�� �������İ�
				m_info.Replace(_T("����������"), _T("������������|�����¾�"));
			else
				m_info.Replace(_T("�|����ɏ��"), _T("������������|�����¾�"));
			break;
		case mgcalco: // �����Ļƻ�Ƴ�������Ϸ����û����
			m_info.Insert(0, _T("�ƻ�ƈ� "));
			break;
		}
		break;
	default:
		ret = RPYINFO_UNKNOWNFORMAT;
	}

	//����ע�� + ��FormatScore��ʽ���ļ�ĩβ���ı�¼����Ϣ�еķ�����TH6��TH7���⣩
	if (ret==RPYINFO_OK && m_header!=mgc6 && m_header!=mgc7) {
		//���ҷ���
		//���� "Score " �� m_info �е�λ��
		int pos = m_info.Find(cwszFindForScore);
		if (pos > -1) {
			pos += 2;

			// ָ�� "Score " ��ָ��
			LPCWSTR pScoreBuffer = (LPCWSTR)(m_info) + pos - 1;
			WCHAR chScore[16];  // ���ڴ洢 m_UserData ���ı���ʽ�ķ�����
			CopyInfoLine(chScore, pScoreBuffer);
			
			CString inScore(chScore), outScore;
			FormatScore(inScore, outScore, bAddZero);

			// �ø�ʽ����ĵ÷��滻ԭ�еĵ÷�
			CString strFindForScore(cwszFindForScore);
			m_info.Replace( strFindForScore + chScore, strFindForScore + outScore);
		}
	}

	return ret;
}

int CRpyInfoUserBlock::GetUBRpyInfo()
{
	// ��ȡ UserBlock �е� info block
	const RPYUserBlock* const pUBInfo = m_pUBMgr->getInfo();

	if (pUBInfo && pUBInfo->isValid()) {
		// ת��
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
		return RPYINFO_UNKNOWNFORMAT; // ������û�� info block �� RPY
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
