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

void TH12FormatUFOStock(const DWORD UFOStock[], CString& outStrUFOStock)
{
	TCHAR   szUFOHZ[4] = _T("�տտ�");
	LPCTSTR szUFOHZMap = _T("�պ�����");
	for (int i=0; i<3; ++i)
		szUFOHZ[i] = (UFOStock[i] > 3)
			? _T('��')
			: szUFOHZMap[UFOStock[i]];

	outStrUFOStock = szUFOHZ;
}

double TH16FormatSeasonGauge(DWORD dwSeasonGauge, DWORD& dwOutNorm)
{
	// �����ӻ�����ֵ�б���������棬��֪��ʽ���Ƿ��б仯��
	DWORD arrSubWeapon[] = { 0, 100, 230, 390, 590, 840, 1140 };
	double result = -1.00;
	DWORD const dwLastNorm = arrSubWeapon[lengthof(arrSubWeapon)-1];

	dwOutNorm = 0;
	if ( dwSeasonGauge == dwLastNorm ) {
		result = 6.00;  // ���ֵ��Ҫ���⴦��
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
CString& TH17FormatSpiritStock(DWORD dwSpiritCount, const DWORD spirits[], CString& outStrSpiritStock)
{
	const DWORD stock_count = 5;
	const DWORD stock_type_count = 14;

	TCHAR szSpiritHZ[stock_count+1] = {0};  // +1 is for '\0'
	LPCTSTR szSpiritHZMap = _T("����̡�գ����е�ˮţ����������");
	for ( int i = 0; i < stock_count; ++i ) {
		szSpiritHZ[i] = ((DWORD)(i) >= dwSpiritCount)
			? szSpiritHZMap[0]
			: (spirits[i] > stock_type_count)
				? _T('��')
				: szSpiritHZMap[ spirits[i] ];
	}

	outStrSpiritStock = szSpiritHZ;
	return outStrSpiritStock;
}


void FormatScore(const CString &inScore, CString &outScore, BOOL bAddZero, BOOL bAddLeadingBlank, DWORD dwContinuedTimes)
{
	const UINT64 u64Score = _ttoi64(inScore);
	FormatScore(u64Score, outScore, bAddZero, bAddLeadingBlank, dwContinuedTimes);
}

void FormatScore(UINT64 inScore64, CString &outScore, BOOL bAddZero, BOOL bAddLeadingBlank, DWORD dwContinuedTimes)
{
	if (bAddZero) inScore64 *= 10;

	DWORD e = (DWORD)(inScore64/100000000);
	DWORD w = (DWORD)(inScore64%100000000/10000);
	DWORD d = (DWORD)(inScore64%10000);
	d += dwContinuedTimes;

	if (inScore64<10000)
		outScore.Format(_T("0�|0�f%04lu�c"), d);
	else
		outScore.Format(_T("%lu�|%04lu�f%04lu�c"), e, w, d);

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
				? _T('��')
				: (i==inNum && inFragment>0)
					? (_T('��') + inFragment)
					: _T('��');

		if (inNum == nMax && inFragment > 0) {
			szOut[nMax] = _T('��') + inFragment;
			szOut[nMax+1] = _T('\0');
		}
		else
			szOut[nMax] = _T('\0');

		outStr = szOut;
		delete []szOut;
	}
}

// ����¼����Ϣ�е�һ�����ݣ�UNICODE�����������������ơ�
// ���¹��̣�
// �Ȳ��� lpszSrc �е�һ�����հ�(�ո�TAB)���ַ���ֱ����β���ַ�������
//		����ҵ�һ����������ҡ��ǿհס��ַ���ֱ��...
//			����ҵ����ǿհס��ַ�����ʣ�����ݸ��Ƶ� lpszDest��ֱ��...
// ������Σ���󶼻��� lpszDest[0] ���� 0 ��
void CopyInfoLine(LPWSTR lpszDest, LPCWSTR lpszSrc)
{
	__asm { // �����ϰ :) ���ɵĻ����������� C++ ����С 30 ����ֽڣ�����һ��
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
	bool bSpaceFound = false; // �ҵ��� �ո� �� Tab
	bool bLineEnded  = false; // �ҵ��� �س� �� \0

	for (; 1; lpszSrc++) {
		if (*lpszSrc!=0x09 && *lpszSrc!=0x20) { //�������TAB��ո�
			if (*lpszSrc==0x0d || *lpszSrc==0) { //����ǻس���\0���˳�
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
