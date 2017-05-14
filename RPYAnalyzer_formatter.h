#ifndef __RPYINFO_FORMATTER_H__55D15D62_
#     define __RPYINFO_FORMATTER_H__55D15D62_

namespace RPYAnalyzer_formatter {

//��ʽ������
void FormatScore(const CString &inScore, CString &outScore, BOOL bAddZero, BOOL bAddLeadingBlank=FALSE, DWORD dwContinuedTimes=0);
void FormatScore(UINT64 inScore64, CString &outScore, BOOL bAddZero, BOOL bAddLeadingBlank=FALSE, DWORD dwContinuedTimes=0);

//����ֵת���ɡ�
void Num2Star(
	int inNum,
	CString &outStr,
	int nMax,
	int inFragment=0 // TH10 �Ժ�� �л���Ƭ����������ʾ
);

//����TH8�Ժ�ר��

// ����¼����Ϣ�е�һ�����ݣ�UNICODE�����������������ơ�
void CopyInfoLine(LPWSTR lpszDest, LPCWSTR lpszSrc);

// ת����ʱ�̡�Ϊ��AM/PM xx:xx����ʽ������ strGameTime
CString& TH8FormatGameTime(int nGameTime, CString& strGameTime);
//ת��TH12��UFOStockΪ������ʾ
void TH12FormatUFOStock(DWORD UFOStock[], CString& outStrUFOStock);

// ת��TH16�ļ��ڲ�Ϊ �ӻ�+�۵İٷֱ���ʽ
// ���ظ�����ʾ��������
double TH16FormatSeasonGauge(DWORD dwSeasonGauge);

}

#endif /* __RPYINFO_FORMATTER_H__55D15D62_ */