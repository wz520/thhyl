#ifndef _RPYINFO874_H
#define _RPYINFO874_H

#include "RPYAnalyzer_common.h"
#include "RPYAnalyzer_userblock.h"

#define MINRPYSIZE						(0x60)

struct THX_EXTRAINFO{
    TH_FPSINFO* pFPSInfo;      // �������Ҫ��ʾ szSlowRate ���� TH6��������Ϊ NULL
    int         nIndex;        // С�� 0 ��ʾ��ʾ���йؿ����ۺ���Ϣ
    DWORD*      dwFrameCounts; // ָ��������ص���֡�������飬Ԫ�ظ����������� 20
    LPCTSTR     szPlayTime;    // PlayTime ��ʽ�ַ���������Ϊ NULL
    LPCTSTR     szSlowRate;    // SlowRateByFPS ��ʽ�ַ�����NULL ����Ը�����ʾ
};



class CRPYAnalyzer
{
public:
    // ��1.70 ����¡����ڹ��캯���ᱸ�� data ��ָ���ݣ����ù��캯������԰�ȫ�� free() �� delete 
    CRPYAnalyzer(
        BYTE* data,   // ָ��RPY�ļ�����������
        DWORD size,   // RPY�ļ���С
        UINT  DefCode // Ĭ�ϱ��롣����TH09�Ժ��RPY�ļ���
	);
	~CRPYAnalyzer();
    int Analyze(); // ����¼����Ϣ��ͬһ�� RPYAnalyzer ���󲻿ɵ������Σ�����ᵼ���ڴ�й¶
    CString GetFinalInfo() const {return m_info;} // ��ȡ���յ�¼����Ϣ
    CString GetComment(UINT codepage=0); // ��ȡע��

	// ��ȡ��������ݵ�ָ�롣�����ȵ��� Analyze()��ָ�����ݲ����޸ġ����� pOutDecodeSize ����Ϊ NULL
	const BYTE* GetDecodedDataPointer(DWORD* pOutDecodeSize) const;

	// ��ȡָ����ַ��������ָ�룬����ڽṹ�� RPYINFO_POINTERS �С�
	void GetResultPointers(RPYINFO_POINTERS* pointers) const;

	// ������������д���ļ��������ȵ��� Analyze()��
	BOOL DumpRPYData(LPCTSTR filename) const;

	RPYMGC m_header; // ��ʼΪ hdrnull ������ Analyze() ��Ϊ enum RPYMGC �е�ĳһֵ��ָʾ�� rpy ������Ϸ��
	
protected:
	int GenInfoWrapper(); // ��ȡ 6,7,8,9 ��������(6, 7 ����������Ϣ)�����Զ����� rpy �����ݵ������к���

    void TH6GenInfo(); // ׼��Ҫ��ʾ����Ϣ
	void TH7GenInfo();
	void TH8GenStageInfo();
	void TH9GenStageInfo();
	void THXAddExtraInfo(const THX_EXTRAINFO* pExtraInfo); //����ѡ������Ƿ���� PlayTime �� SlowRateByFPS
	void THXAddExtraInfo2(int nIndex); // for th95+ �İ汾

	int GenInfoWrapper2(); // ��ȡ������Ʒ�ĸ������ݣ���������� TH??GenStageInfo ����

	void THALGenStageInfo();
	void TH10GenStageInfo();
	void TH11GenStageInfo();
	void TH12GenStageInfo();
	void TH13GenStageInfo();
	void TH14GenStageInfo();
	void TH15GenStageInfo();
	void TH16GenStageInfo();
	void TH17GenStageInfo();
	void TH128GenStageInfo();

	// �޵�����Ϣ
	void TH95GenInfo();
	void TH125GenInfo();
	void TH143GenInfo();
	void TH165GenInfo();

    //����TH8�Ժ�ר��

	// m_info ��������Ϣ���ڴ��¼��ǰ������� ��/��/�� �� custom.exe �������õ�ѡ�
	// ������
	//	��ȡ�ֱ����벻ʹ�� DirectInput(TH95,10,11,12,128,13,14,143)
	//	��ס����������ƶ���ShotSlow��(��֧�� TH6, TH9, TH125����Ϸ��֧�ֻ� RPY ��û�б��������)
	//	�Ի��Ϸ���ʾ�������ٲ�(TH10 only)
	//
	//	@b6to9: Ϊ TRUE ��ʾ wFlags �� THRPYINFO::wFlags,
	//	        ����Ϊ THRPYINFO2::wFlags
	void AddGameOptionsInfo(WORD wFlags, BOOL b6to9 = FALSE);

	// m_info ��������Ϣ������� Spell Practice ¼����ʾ SpellCard �ı��
	// ��������ļ� filename ���ڣ���ȡSC��
	//   NOTE: ���� filename ʱ���Զ���ǰ����ϡ�EXE����Ŀ¼\sclists��
	//
	// Ŀǰ֧�� th13, th14, th16 ����ʽ�棬�Լ� th95, th125, th143
	// ��� isScene==TRUE������ "Scene" ���� "SpellCard" ��
	void AddSpellPracticeInfo(int SCNum, LPCTSTR filename, BOOL isScene=FALSE);
private:
    BYTE* m_pData;      // RPY���ݣ����ܣ��ǽ�ѹ���������Ҳ����ڴ˴�
    DWORD m_nDataSize;  // RPY��С
    CString m_info;     // ������ʾ������¼����Ϣ

    THRPYINFO  *m_pTHRpyInfo;  // ReplayDecode()
    THRPYINFO2 *m_pTHRpyInfo2; // ReplayDecode2()
    BYTE       *m_pDecompData; // ReplayDecode2()'s return value.

	//TH8�Ժ�ר��
    CString m_UBRpyInfo;   // ���� user block ��ת��UNICODE����ı���ʽ��¼����Ϣ
    UINT m_uDefCode;      // Ĭ�ϱ���
    
	RPYUserBlockMgr* m_pUBMgr;   // userblock ������
	CRpyInfoUserBlock* m_pInfoUB;   // �� userblock �л�ȡ�ı���ʽ��¼����Ϣ��¼��ע��

	CRPYAnalyzer& operator = (const CRPYAnalyzer& another);
	CRPYAnalyzer(const CRPYAnalyzer& another);
};


#endif //_RPYINFO874_H
