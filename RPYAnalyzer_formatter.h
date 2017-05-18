#ifndef __RPYINFO_FORMATTER_H__55D15D62_
#     define __RPYINFO_FORMATTER_H__55D15D62_

namespace RPYAnalyzer_formatter {

//格式化分数
void FormatScore(const CString &inScore, CString &outScore, BOOL bAddZero, BOOL bAddLeadingBlank=FALSE, DWORD dwContinuedTimes=0);
void FormatScore(UINT64 inScore64, CString &outScore, BOOL bAddZero, BOOL bAddLeadingBlank=FALSE, DWORD dwContinuedTimes=0);

//将数值转换成★
void Num2Star(
	int inNum,
	CString &outStr,
	int nMax,
	int inFragment=0 // TH10 以后的 残机碎片，以数字显示
);

//以下TH8以后专用

// 拷贝录像信息中的一行内容（UNICODE），不包括内容名称。
void CopyInfoLine(LPWSTR lpszDest, LPCWSTR lpszSrc);

// 转换“时刻”为“AM/PM xx:xx”格式，返回 strGameTime
CString& TH8FormatGameTime(int nGameTime, CString& strGameTime);
//转换TH12的UFOStock为汉字显示
void TH12FormatUFOStock(DWORD UFOStock[], CString& outStrUFOStock);

// 转换TH16的季节槽为 子机+槽的百分比形式
// dwOutNorm：输出参数，返回下一次增加子机所需季节槽数值
// 返回负数表示输入有误
double TH16FormatSeasonGauge(DWORD dwSeasonGauge, DWORD& dwOutNorm);

}

#endif /* __RPYINFO_FORMATTER_H__55D15D62_ */
