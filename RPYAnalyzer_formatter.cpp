#include "StdAfx.h"
#include "RPYAnalyzer_formatter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define lengthof(arr) (sizeof(arr)/sizeof(arr[0]))

namespace RPYAnalyzer_formatter {

CString& TH8FormatGameTime(int nGameTime, CString& strGameTime)
{
	bool isAM   = true;
	int  hour   = nGameTime / 2 - 1;
	int  minute = nGameTime % 2 * 30;
	if (hour < 0) hour += 12, isAM = false;

	strGameTime.Format(_T("%s %02d:%02d"),
			isAM ? _T("AM") : _T("PM"),
			hour, minute);
	
	return strGameTime;
}

void TH12FormatUFOStock(DWORD UFOStock[], CString& outStrUFOStock)
{
	TCHAR   szUFOHZ[4] = _T("空空空");
	LPCTSTR szUFOHZMap = _T("空红蓝绿");
	for (int i=0; i<3; i++)
		szUFOHZ[i] = (UFOStock[i] > 3)
			? _T('误')
			: szUFOHZMap[UFOStock[i]];

	outStrUFOStock = szUFOHZ;
}

double TH16FormatSeasonGauge(DWORD dwSeasonGauge, DWORD& dwOutNorm)
{
	// 增加子机所需值列表（用于体验版，不知正式版是否有变化）
	DWORD arrSubWeapon[] = {
		0, 100, 230, 390, 590, 840, 1140
	};
	double result = -1.00;
	DWORD const dwLastNorm = arrSubWeapon[lengthof(arrSubWeapon)-1];

	dwOutNorm = 0;
	if ( dwSeasonGauge == dwLastNorm ) {
		result = 6.00;  // 最大值需要特殊处理
		dwOutNorm = dwLastNorm;
	}
	else {
		for ( int i = 1; i < lengthof(arrSubWeapon); ++i ) {
			DWORD const dwNextNorm = arrSubWeapon[i];
			DWORD const dwPrevNorm = arrSubWeapon[i-1];
			if ( dwSeasonGauge < dwNextNorm ) {
				DWORD diff = dwNextNorm - dwPrevNorm;
				DWORD diff_curr = dwSeasonGauge - dwPrevNorm;
				result = ((double)diff_curr / (double)diff) + (double)(i-1);
				dwOutNorm = dwNextNorm;
				break;
			}
		}
	}

	return result;
}

void FormatScore(const CString &inScore, CString &outScore, BOOL bAddZero,
						  BOOL bAddLeadingBlank, DWORD dwContinuedTimes
)
{
	const UINT64 u64Score = _ttoi64(inScore);
	FormatScore(u64Score, outScore, bAddZero, bAddLeadingBlank, dwContinuedTimes);
}

void FormatScore(UINT64 inScore64, CString &outScore, BOOL bAddZero,
						  BOOL bAddLeadingBlank, DWORD dwContinuedTimes
)
{
	if (bAddZero) inScore64 *= 10;

	DWORD e = (DWORD)(inScore64/100000000);
	DWORD w = (DWORD)(inScore64%100000000/10000);
	DWORD d = (DWORD)(inScore64%10000);
	d += dwContinuedTimes;

	if (inScore64<10000)
		outScore.Format(_T("0億0萬%04u點"), d);
	else
		outScore.Format(_T("%u億%04u萬%04u點"), e, w, d);

	if (bAddLeadingBlank && e<10)
		outScore.Insert(0, _T(' '));
}

void Num2Star(int inNum, CString &outStr, int nMax, int inFragment)
{
	if (inNum > nMax || inFragment > 9)
		outStr = _T("error");
	else {
		LPTSTR szOut = new TCHAR[nMax + 2];
		for (int i=0; i<nMax; i++)
			szOut[i] = i < inNum
				? _T('★')
				: (i==inNum && inFragment>0)
					? (_T('０') + inFragment)
					: _T('☆');

		if (inNum == nMax && inFragment > 0) {
			szOut[nMax] = _T('０') + inFragment;
			szOut[nMax+1] = _T('\0');
		}
		else
			szOut[nMax] = _T('\0');

		outStr = szOut;
		delete []szOut;
	}
}

// 拷贝录像信息中的一行内容（UNICODE），不包括内容名称。
// 大致过程：
// 先查找 lpszSrc 中第一个“空白(空格、TAB)”字符，直到行尾或字符串结束
//		如果找到一个，则继续找“非空白”字符，直到...
//			如果找到“非空白”字符，则将剩余内容复制到 lpszDest，直到...
// 无论如何，最后都会在 lpszDest[0] 里填 0 。
void CopyInfoLine(LPWSTR lpszDest, LPCWSTR lpszSrc)
{
	__asm { // 汇编练习 :) 生成的机器码比下面的 C++ 代码小 30 多个字节，功能一样
		mov esi, lpszSrc;
		xor edi, edi; // edi=spacefound
loopread:
		movzx eax, word ptr [esi];
		cmp ax, 0x09;
		je	has_blank;
		cmp ax, 0x20;
		je	has_blank;
check_line_end:
		cmp ax, 0x0d;
		je	has_line_end;
		test eax, eax;
		je	has_line_end;
no_line_end:
		test edi, edi;
		jz for3rd;
		jmp loopcopystart;
has_line_end:
		mov edi, lpszDest; //edi=lpszDest
		jmp finally;
has_blank:
		inc edi;
for3rd:
		add esi, 2;
		jmp loopread;

loopcopystart:
		mov edi, lpszDest; //edi=lpszDest
		pushfd;
		cld;
loopcopy:
		movsw;
		movzx eax, word ptr [esi];
		cmp ax, 0x0d;
		je loopcopyend;
		test eax,eax;
		jnz loopcopy;
loopcopyend:
		popfd;
		
finally:
		and word ptr [edi], 0;
	}

#if 0
	bool bSpaceFound = false; // 找到了 空格 或 Tab
	bool bLineEnded  = false; // 找到了 回车 或 \0

	for (; 1; lpszSrc++) {
		if (*lpszSrc!=0x09 && *lpszSrc!=0x20) { //如果不是TAB或空格
			if (*lpszSrc==0x0d || *lpszSrc==0) { //如果是回车或\0则退出
				bLineEnded=true;
				break;
			}

			if (bSpaceFound)
				break;
		}
		else
			bSpaceFound=true;
	}

	if (!bLineEnded) {
		while (*lpszSrc!=0x0d && *lpszSrc!=0)
			*(lpszDest++) = *(lpszSrc++);
	}

	*lpszDest = 0;
#endif

}

}
