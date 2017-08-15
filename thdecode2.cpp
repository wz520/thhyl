/***************************************************************
 * Touhou decode routine for                                   *
 * TH95, TH10, TH11, TH12, TH125, TH128, TH13, TH14tr, alcostg *
 *                                                             *
 * written by wz520                                            *
 ***************************************************************/

// Last Update: 2015-08-28

/////////////////////////////////////////////////
//
// Successfully compiled by:
//   MSVC6, (mingw 32bit, Linux 32bit)g++ 4.4.3, 4.6.1
// 
// NOTE:
//   g++ may give a warning about 'typedef enum tagRPMGC2' because this
//     enum has 'multi-character character constant'.
//   You can simply ignore this warning, or use '-Wno-multichar' option to
//   prevent the compiler from warning about this.
/////////////////////////////////////////////////

//
// --------- Changelog
//
// 2015-08-17:
// * 注意：本模块原本也可用 C 方式编译，现在已不再支持（只能用 C++ 方式编译）。
// * 增加：对绀珠传正式版的支持（不支持体验版）
// * 修正：如果解码失败，还会将 THRPYINFO2::dwDecodeSize 设为 0 。
// * ...
//
// 2014-06-03:
// * added: th143 support.
// * added: info struct for th95, th125, th143, see definition of THHALF_INFO.
// * added: szPlayerName, tClearTime, fSlowRate, halfinfo, fpsinfo in
//          THRPYINFO2 struct.
// * Refactored thdecode2_decompress() and thdecode2_decrypt():
//     Reformatted the code.
//     Removed unnecessary variables, made the functions shorter.
// * Removed unnecessary <stdio.h> including.
// * Defined CALL_DECRYPT macro to make thdecode2_decrypt() calls shorter.
// * THRPYINFO2::wFlags now available for th95 and th143, but still not
//     th125(seems not saved in RPY).
// * moved some typedefs to thcommon.h
// * moved RPYMGC2 to thcommon.h as RPYMGC.
//
// 2013-09-09:
// * moved RPYHDR2 enum definitions to the header file, renamed to RPYMGC2
//	 because the 'header' is the struct called THHEADER2, not just the
//	 4 leading bytes.
// * Use the THHEADER2 struct to increase readability.
//
// 2013-08-18:
// * added: th14 support.
// * fixed: Rename GetStageInfo() to _GetStageInfo().
//          The new GetStageInfo() can deal with th13/th14 rpy.
// 
// 2013-06-30:
// * fixed: More reliable th13/th14 distinguish method.
//
// 2013-05-27:
// * added: alcostg, th14tr support.
//
// 2012-02-21:
// * added: check if rpy size <= 0x28 in ReplayDecode2(). If so, return NULL.
//          This is impossible for a normal rpy.
// * fixed: Var 'header', 'decodesize', 'decompress_size', 'wVersion' in
//          ReplayDecode2() was initialized at the same time they were defined.
//          This is dangerous because at this moment, rpy size has not been
//            checked for validity.
//          Now assigning value to them has been deferred until after checking
//            rpy size.
//
// 2012-02-17:
// * fixed: g++ may give an 'operands to ?: have different types' error.
// * fixed: did not set clear score for th95,th125.
// * fixed: did not zeromemory(*pInfo) in ReplayDecode2().
// * added: set data for new members in THRPYINFO2 structures.

#include <stdlib.h>
#include <string.h>

#include "thdecode2.h"

#define BETWEEN(n,min,max)			((n)>=(min) && (n)<=(max))

// Both thdecode2_decompress() and thdecode2_decrypt() use
//   standard calling convention ('__stdcall') in original Touhou games.
//
// In this module, because these 2 functions are declared as 'static',
//   '__stdcall' keyword is totally dispensable.
// But here we still use __stdcall because it saves a few instructions per call.
// Most compilers(except __GNUC__) support __stdcall.
// So for the compilers that are not gcc/g++, __stdcall is used.
// For gcc/g++, the compiler's default calling convention is used.
#ifndef __GNUC__
# define STDCALL		__stdcall
#else
# define STDCALL
#endif
static unsigned char* STDCALL thdecode2_decompress(
        const unsigned char *pData, // compressed data
        int datasize,               // size of the compressed data
        unsigned char *pDecompData  // buffer to store decompressed data
);
static void STDCALL thdecode2_decrypt(
		char mask1,
		unsigned char *pData,
		signed int size1,
		char mask2,
		signed int key,
		signed int size2
);



// 记录相对于解码数据起始处的偏移
struct RPYOFFSETS {
	int firstStage;        // 第一关关卡数据结构
	int stageCount;        // 总关卡数
	int playerName;        // 玩家名
	int clearTime;         // 通关时间。UNIX时间戳
	bool isClearTime64bit; // [非偏移]，上面的时间戳是否是64位
	int slowRate;          // 处理落率
	int clearScore;        // 最终得分
	int ID;                // 自机角色ID，从 0 开始
	int equipID;           // 自机角色装备ID，从 0 开始。
	int difficulty;        // 难度
	int lastStage;         // 最终关卡
	int stageSizeFix;      // [非偏移]，关卡数据偏移修正值
	int flags;			   // 在 custom.exe 中设置的游戏选项

	// 如果 isALor10 不为 false，则表示是 黄昏酒场 或 风神录 的录像
	RPYOFFSETS(bool isALor10) {
		this->firstStage       = isALor10 ? 0x64 : 0x70;
		this->playerName       = 0x00;
		this->clearTime        = 0x0c;
		this->isClearTime64bit = isALor10 ? false : true;
		this->slowRate         = isALor10 ? 0x48 : 0x54;
		this->stageCount       = isALor10 ? 0x4c : 0x58;
		this->clearScore       = isALor10 ? 0x10 : 0x14;
		this->ID               = isALor10 ? 0x50 : 0x5c;
		this->equipID          = isALor10 ? 0x54 : 0x60;
		this->difficulty       = isALor10 ? 0x58 : 0x64;
		this->lastStage        = isALor10 ? 0x5c : 0x68;
		this->flags            = isALor10 ? 0x44 : 0x50;
	}
};

struct INTERNAL_DATA {
	int nMaxStageCount;
	int nMaxStageNumber;
	int nKeyStateElementSize;
	RPYOFFSETS o;

	INTERNAL_DATA(bool isALor10) : o(isALor10) {
		nMaxStageCount = 6;
		nMaxStageNumber = 7;
		nKeyStateElementSize = 6;
	}

	void setMaxValuesForTrial() {
		nMaxStageCount = 3;
		nMaxStageNumber = 3;
	}
};


THRPYINFO2::THRPYINFO2() {
	memset(this, 0, sizeof(*this));
	initStageNames();
}


static unsigned char *STDCALL thdecode2_decompress(const unsigned char * const pData, int datasize, unsigned char *pDecompData)
{
	char workarea[0x2000];
	unsigned char v4 = 0x80;
	unsigned int v13, v15, v18;

	int v5=0, v10, v11, v12;
	int v14, offset_p, v17, v21=0, v22=1, v23;
	int i;

	const unsigned char *p = pData;
	while ( 1 ) {
		if ( v4 == 0x80 ) {
			v21 = *p;
			if ( p - pData < datasize )
				++p;
			else
				v21 = v5;
		}
		v10 = v4;
		v4 >>= 1;
		if ( !v4 )
			v4 = 0x80;
		if ( (v21 & v10) != v5 ) {
			v11 = 0;
			v13 = 0x80;
			v12 = p - pData;
			do {
				if ( v4 == 0x80 ) {
					v21 = *p;
					if ( v12 < datasize ) {
						++p;
						++v12;
					}
					else {
						v21 = v5;
					}
				}
				if ( v4 & (unsigned char)v21 )
					v11 |= v13;
				v4 >>= 1;
				v13 >>= 1;
				if ( !v4 )
					v4 = 0x80;
			}
			while ( v13 != v5 );
			*(pDecompData++) = v11;
			workarea[v22] = v11;
			v22 = (v22 + 1) & 0x1FFF;
			continue;
		}
		v15 = 4096;
		v14 = p - pData;
		do {
			if ( v4 == 0x80 ) {
				v21 = *p;
				if ( v14 < datasize ) {
					++p;
					++v14;
				}
				else
					v21 = 0;
			}
			if ( v4 & (unsigned char)v21 )
				v5 |= v15;
			v4 >>= 1;
			v15 >>= 1;
			if ( !v4 )
				v4 = 0x80;
		}
		while ( v15 );
		v23 = v5;
		if ( !v5 )
			break;
		v17 = 0;
		v18 = 8;
		offset_p = p - pData;
		do {
			if ( v4 == 0x80 ) {
				v21 = *p;
				if ( offset_p < datasize ) {
					++p;
					++offset_p;
				}
				else
					v21 = 0;
			}
			if ( v4 & (unsigned char)v21 )
				v17 |= v18;
			v4 >>= 1;
			v18 >>= 1;
			if ( !v4 )
				v4 = 0x80;
		}
		while ( v18 );
		for ( i = 0; i <= v17 + 2; ++i ) {
			char v20 = workarea[(i + v23) & 0x1FFF];
			*(pDecompData++) = v20;
			workarea[v22] = v20;
			v22 = (v22 + 1) & 0x1FFF;
		}
		v5 = 0;
	}
	if ( v4 != 0x80 ) {
		do
			v4 >>= 1;
		while ( v4 && v4 != 0x80 );
	}
	return pDecompData;
}


static void STDCALL thdecode2_decrypt(char mask1, unsigned char *pData, signed int size1, char mask2, signed int key, signed int size2)
{
	unsigned char *pWorkArea, *v13;

	const int c1 = size1 % key & ((size1 % key >= key / 4) - 1);
	if (size2 > size1)
		size2 = size1;

	v13 = pWorkArea = (unsigned char *)malloc(size2);
	if ( pWorkArea ) {
		int v29 = size1 - (c1 + (size1 & 1));
		signed int v14 = v29;
		memcpy(pWorkArea, pData, size2);
		if ( v29 > 0 ) {
			while ( size2 > 0 ) {
				unsigned char *v17;
				int v16;
				if ( v14 < key )
					key = v14;
				pData += key;
				v16 = (key + 1) / 2;
				v17 = pData - 1;
				if ( v16 > 0 ) {
					do {
						*v17 = mask1 ^ *v13;
						v17 -= 2;
						mask1 += mask2;
						++v13;
					}
					while ( --v16 );
				}
				v17 = pData - 2;
				if ( key / 2 > 0 ) {
					v16 = key / 2;
					do {
						*v17 = mask1 ^ *v13;
						v17 -= 2;
						mask1 += mask2;
						++v13;
					}
					while ( --v16 );
				}
				v29 -= key;
				size2 -= key;
				if ( v29 <= 0 )
					break;
				v14 = v29;
			}
		}
		free(pWorkArea);
	}
}

static void SetRPYInfoFromOffsets( const BYTE* pData, const RPYOFFSETS* o, THRPYINFO2* pOutInfo )
{
	pOutInfo->dwClearScore = *(DWORD*)(pData+o->clearScore);
	pOutInfo->dwID         = *(DWORD*)(pData+o->ID);
	pOutInfo->dwEquipID    = *(DWORD*)(pData+o->equipID);
	pOutInfo->dwDifficulty = *(DWORD*)(pData+o->difficulty);
	pOutInfo->dwLastStage  = *(DWORD*)(pData+o->lastStage);

	memcpy(pOutInfo->szPlayerName, pData+o->playerName, sizeof(pOutInfo->szPlayerName));
	pOutInfo->szPlayerName[sizeof(pOutInfo->szPlayerName) - 1] = '\0';

	if ( o->isClearTime64bit ) {
		pOutInfo->tClearTime = *(time_t*)(pData+0x0c); // should be 64bit, or 32bit if time_t is 32bit
	}
	else {
		pOutInfo->tClearTime = (time_t)*(DWORD*)(pData+0x0c); // should be 32bit exactly
	}

	pOutInfo->fSlowRate = *(float*)(pData+o->slowRate);
}

static void SetFlags(
		RPYMGC magic,
		const BYTE* pData,
		int offsetFlags,
		WORD* pwFlags      // out param
)
{
	const DWORD dwFlags = *((DWORD*)(pData+offsetFlags));

	if (dwFlags & 0x8)
		*pwFlags |= RPYFLAG2_NODINPUT;
	if (dwFlags & 0x200)
		*pwFlags |= RPYFLAG2_SHOTSLOW;

	if (magic==mgc10) // th10 only
		if (dwFlags & 0x80)
			*pwFlags |= RPYFLAG2_FAITHGUAGE;
}

// 根据 pInOutFPSInfo->pointers 和 pInOutFPSInfo->sizes 填写其他属性
// 与 thdecode1 不同，thdecode2 的 fps 数据是各关独立的，计算综合掉帧率需要遍历每一关的指针。
static void GetFPSInfo(int nStageCount, TH_FPSINFO* pInOutFPSInfo)
{
	DWORD total_size = 0;
	DWORD total_fpssum = 0;
	for ( int currStage = 0; currStage < nStageCount; ++currStage ) {
		const DWORD fpsDataSize  = pInOutFPSInfo->sizes[currStage];
		const DWORD normalfpssum = fpsDataSize * 60;                 //正常情况应该每个FPS都是60。
		DWORD fpssum = 0;
		for (DWORD i=0; i<fpsDataSize; ++i)
			fpssum += (DWORD)pInOutFPSInfo->pointers[currStage][i];
		pInOutFPSInfo->slowrates[currStage] = 100.00 - ((double)fpssum) / ((double)normalfpssum) * 100.00;
		
		total_size += fpsDataSize;
		total_fpssum += fpssum;
	}
	
	// total
	{
		const DWORD total_normalfpssum = total_size * 60;
		pInOutFPSInfo->total_size = total_size;
		pInOutFPSInfo->total_slowrate = 100.00 - ((double)total_fpssum) / ((double)total_normalfpssum) * 100.00;
	}
}

// also get fpsinfo
static bool _GetStagePointers(
		const BYTE* pData,
		DWORD size,
		const INTERNAL_DATA& idata,
		THRPYINFO2* pOutInfo
)
{
	// o->stageCount 小于 0 则不使用该 offset，假设只有 1 关
	const int nStageCount = idata.o.stageCount < 0 ? 1 : *(int*)(pData + idata.o.stageCount);
	// Stage count should between 1 and nMaxStageCount
	if ( !BETWEEN(nStageCount, 1, idata.nMaxStageCount) )
		return false;
	else {
		// Get the stage structure pointers
		const BYTE* const pBegin = pData + idata.o.firstStage;
		const BYTE* pCurr = pBegin;
		const BYTE* const pEnd = pData+size;
		
		pOutInfo->nStageCount = nStageCount;
		for (int i=0; i<nStageCount; i++) {
			const THHDR_STAGEINFO* const pCurrHdr = (const THHDR_STAGEINFO*)pCurr;

			// 数据指针有效性检查
			// 若 o->stageCount <= 0 则不检查 stagenumber
			if ( pCurr >= pEnd || pCurr < pBegin ||
				(idata.o.stageCount > 0 && !BETWEEN(pCurrHdr->wStageNumber, 1, idata.nMaxStageNumber)) )
				return false;
			
			pOutInfo->pStageInfo.v[i] = (void*)(pCurr);
			
			// 跳过 keystate 数据便是 fps 数据，一组 keystate 的长度由 nKeyStateElementSize 指定
			pOutInfo->fpsinfo.pointers[i] = (BYTE*)pCurr + pCurrHdr->dwhdrKeyStateSize*idata.nKeyStateElementSize + idata.o.stageSizeFix;


			const int nStageNumberIndex = pCurrHdr->wStageNumber-1;
			// 填写 stagepointers
			if (i == 0 && idata.o.stageCount < 0) {
				// 95, 125, 143 强制填写在索引 0 上
				pOutInfo->nStageNumberForHalf = pOutInfo->nSpellPracticeNumber;
				pOutInfo->stagepointers[0][0].p = (BYTE*)pCurr;
				pOutInfo->stagepointers[0][0].offset = pCurr - pData;
				pOutInfo->stagepointers[0][0].size = pOutInfo->fpsinfo.pointers[i] - pCurr;
			}
			else {
				// 由于 wStageNumber 从 1 开始，作为索引使用必须 -1
				pOutInfo->stagepointers[0][nStageNumberIndex].p = (BYTE*)pCurr;
				pOutInfo->stagepointers[0][nStageNumberIndex].offset = pCurr - pData;
				pOutInfo->stagepointers[0][nStageNumberIndex].size = pOutInfo->fpsinfo.pointers[i] - pCurr;
			}
 
			// datasize 应该大于 keystatesize，
			// 如果小于，则把它直接视作 fps 数据的长度，th95 便是如此
			// 等于？应该不可能……
			if (pCurrHdr->dwhdrDataSize < pCurrHdr->dwhdrKeyStateSize) {
				pOutInfo->fpsinfo.sizes[i] = pCurrHdr->dwhdrDataSize;
			}
			else {
				// 指向下一关起始位置
				pCurr += pCurrHdr->dwhdrDataSize + idata.o.stageSizeFix;
				// 下一关起始位置 - fps 数据起始位置 = fps 数据长度
				pOutInfo->fpsinfo.sizes[i] = pCurr - pOutInfo->fpsinfo.pointers[i];
			}

			// 填写 fpspointers
			// 如果是 95,125,143 的 REP，则填写 pOutInfo->nStageNumberForHalf，且将 fpspointers
			if (i == 0 && idata.o.stageCount < 0) {
				pOutInfo->fpspointers[0].p = pOutInfo->fpsinfo.pointers[i];
				pOutInfo->fpspointers[0].offset = pOutInfo->fpsinfo.pointers[i] - pData;
				pOutInfo->fpspointers[0].size = pOutInfo->fpsinfo.sizes[i];				
			}
			else {
				pOutInfo->fpspointers[nStageNumberIndex].p = pOutInfo->fpsinfo.pointers[i];
				pOutInfo->fpspointers[nStageNumberIndex].offset = pOutInfo->fpsinfo.pointers[i] - pData;
				pOutInfo->fpspointers[nStageNumberIndex].size = pOutInfo->fpsinfo.sizes[i];
			}
		}
		
		GetFPSInfo(nStageCount, &pOutInfo->fpsinfo);
	}
	
	return true;
}


// get info for TH95, TH125, TH143
static bool GetHalfInfo(
		RPYMGC magic,
		const BYTE* pData,
		DWORD size,
		INTERNAL_DATA& idata,
		THRPYINFO2* pOutInfo
)
{
	RPYOFFSETS* const o = &idata.o;
	idata.nKeyStateElementSize = 6;
	o->flags = 0;  // 若该值为 0 将不调用 SetFlags()
	o->clearScore = 0x14;
	o->stageCount = -1;

	if (pOutInfo->wVersion != 1) return false;

	switch (magic)
	{
	case mgc95:
		idata.nKeyStateElementSize = 1;
		o->firstStage        = 0xec;
		o->stageSizeFix      = 0x0c;
		o->playerName        = 0x07;
		o->isClearTime64bit  = false;
		o->clearTime         = 0x10;
		o->slowRate          = 0xe0;
		o->flags             = 0xdc;
		pOutInfo->nSpellPracticeNumber = *(WORD*)(pData) + 1;
		break;
	case mgc125:
		idata.nKeyStateElementSize = 3;
		o->firstStage        = 0x68;
		o->stageSizeFix      = 0xa0;
		pOutInfo->nSpellPracticeNumber = *(WORD*)(pData+0x68) + 1;
		// th125 seems don't save flags into replay files
		break;
	case mgc143:
		o->clearTime    = 0x14;
		o->clearScore   = 0x1c;
		o->slowRate     = 0x7c;
		o->firstStage   = 0xa8;
		o->stageSizeFix = 0x10c;
		o->flags        = 0x58;
		pOutInfo->halfinfo.nMainItemID = *(DWORD*)(pData+0x94);
		// same as 0xe4, 0x100?
		pOutInfo->halfinfo.nDayID = *(int*)(pData+0x88);
		// same as 0xe8, 0x104?
		pOutInfo->halfinfo.nSceneID = *(int*)(pData+0x8c);
		// same as 0xec?
		pOutInfo->halfinfo.nMainItemCount = *(DWORD*)(pData+0xa0);
		// same as 0xf0?
		pOutInfo->halfinfo.nMainItemPower = *(DWORD*)(pData+0xa4);
		pOutInfo->halfinfo.nSubItemID = *(DWORD*)(pData+0x98);
		pOutInfo->halfinfo.nSubItemCount = *(DWORD*)(pData+0x9c);
		
		pOutInfo->nSpellPracticeNumber = *(DWORD*)(pData+0x90);
		break;
	default:
		// we should never arrive here
		return false;
	}

	if ( o->flags != 0 )
		SetFlags(magic, pData, o->flags, &pOutInfo->wFlags);

	idata.nMaxStageNumber = 255;
	idata.nMaxStageCount = 1;
	if (!_GetStagePointers(pData, size, idata, pOutInfo))
		return false;

	SetRPYInfoFromOffsets(pData, o, pOutInfo);
	return true;
}


static bool _GetStageInfo(
		RPYMGC magic,
		const BYTE* pData,
		DWORD size,
		THRPYINFO2* pOutInfo,
		bool forceTH14        // force to treat mgc13 as th14 release ver rpy
)
{
	// is alcostg or th10?
	const bool isALor10 = magic==mgc10 || magic==mgcalco;
	const WORD wVersion = pOutInfo->wVersion;
	INTERNAL_DATA idata(isALor10);

	switch (magic)
	{
		case mgcalco:
			idata.setMaxValuesForTrial();
			idata.nKeyStateElementSize = 8;
			idata.o.firstStage         = 0x60;
			idata.o.stageSizeFix       = 0x18;
			if (wVersion != 3) // unknown version number, not allowed
				return false;
			break;
		case mgc10:
			idata.o.stageSizeFix = 0x1c4;
			// 部分奇葩的中文版录像貌似每一组 keystate 的长度变成了 3 。
			// 播放这种录像，会看到右下角 FPS 大多数时候都是 0，这是因为 keystate 长度不对导致 fps 数据取错了。
			// 这种录像在日文版上播放可能出错，所以没有支持的打算（也可能导致本模块出错）。
			// nKeyStateElementSize = 3;
			break;
		case mgc11:
			idata.o.stageSizeFix = 0x90; break;
		case mgc128:
			{
				idata.nMaxStageCount  = 3;
				idata.nMaxStageNumber = 16;
				idata.o.stageSizeFix  = 0x90;

				LPCTSTR const newstagenames[]={
					_T("A1-1"), _T("A1-2"), _T("A1-3"), _T("A2-2"), _T("A2-3"),
					_T("B1-1"), _T("B1-2"), _T("B1-3"), _T("B2-2"), _T("B2-3"),
					_T("C1-1"), _T("C1-2"), _T("C1-3"), _T("C2-2"), _T("C2-3"),
					_T("EX")		
				};
				pOutInfo->setStageNames(newstagenames, 0, 16);
			}
			break;
		case mgc12:
			idata.o.stageSizeFix = 0xa0; break;
		case mgc13:
			{
				const bool isTH14Release = (wVersion == 2) &&
					((pOutInfo->dwGameVersion == 0x100) ||  // th14 release
					pOutInfo->isDebugVersion());  // th14 demo by ZUN
				const bool isTH14Trial = pOutInfo->isTrialVersion(); // th14 trial
				if ( forceTH14 && ( isTH14Release || isTH14Trial ) ) { 
					// treat as release version if forceTH14 is true
					pOutInfo->wFlags |= forceTH14 ? RPYFLAG2_TH14RELEASE : RPYFLAG2_TH14TRIAL;

					if ( isTH14Trial )
						idata.setMaxValuesForTrial();
					idata.o.slowRate      = 0x74;
					idata.o.stageCount    = 0x78;
					idata.o.ID            = 0x7c;
					idata.o.equipID       = 0x80;
					idata.o.difficulty    = 0x84;
					idata.o.lastStage     = 0x88;
					idata.o.firstStage    = 0x94;
					idata.o.stageSizeFix  = 0xdc;

					// get (Spell Practice Number), release version only
					if (forceTH14)
						pOutInfo->nSpellPracticeNumber = *((int*)(pData+0x90)) + 1;
				}
				else if (wVersion == 2 && (pOutInfo->dwGameVersion == 0x100 // th13 release
						|| pOutInfo->dwGameVersion == 0)  // th13 demo by ZUN
						) {
					idata.o.stageSizeFix = 0xc4;
					idata.o.firstStage   = 0x74;

					// get (Spell Practice Number)
					pOutInfo->nSpellPracticeNumber = *((int*)(pData+0x70)) + 1;
				}
				else if (wVersion == 1 && pOutInfo->dwGameVersion == 0x1) { // th13 trial
					idata.o.stageSizeFix = 0xc0;
				}
				else
					return false; // not supported
			}
			break;
		case mgc15:
			idata.o.firstStage = 0xa4;

			// 比其他作品多偏移了 0x30 bytes
			idata.o.flags        = 0x40;
			idata.o.slowRate     = 0x84;
			idata.o.stageCount   = 0x88;
			idata.o.ID           = 0x8c;
			idata.o.equipID      = 0x90;
			idata.o.difficulty   = 0x94;
			idata.o.lastStage    = 0x98;
			idata.o.stageSizeFix = 0x238;
			break;
		case mgc16:
			if ( pOutInfo->isTrialVersion() )
				idata.setMaxValuesForTrial();
			idata.o.firstStage    = 0xa0;
			idata.o.flags         = 0x40;

			// 大部分数据的偏移都比 TH15 少 8 bytes
			idata.o.slowRate     = 0x84-8;
			idata.o.stageCount   = 0x88-8;
			idata.o.ID           = 0x8c-8;
			idata.o.equipID      = 0x9c;
			idata.o.difficulty   = 0x94-8;
			idata.o.lastStage    = 0x98-8;
			idata.o.stageSizeFix = pOutInfo->isTrialVersion() ? 0x284 : 0x294;
			pOutInfo->nSpellPracticeNumber = *((int*)(pData+0x98)) + 1;
			break;
		// no stage info
		case mgc95:
		case mgc125:
		case mgc143:
			return GetHalfInfo(magic, pData, size, idata, pOutInfo);
		default:
			return false;
	}

	if (!_GetStagePointers(pData, size, idata, pOutInfo))
		return false;

	SetFlags(magic, pData, idata.o.flags, &pOutInfo->wFlags);
	SetRPYInfoFromOffsets(pData, &idata.o, pOutInfo);

	return true;
}

static bool GetStageInfo(
		RPYMGC magic,
		const BYTE* pData,
		DWORD size,
		THRPYINFO2* pOutInfo
)
{
	// first try
	// if magic == mgc13, treat it as th13 release rpy
	bool result = _GetStageInfo(magic, pData, size, pOutInfo, false);

	if (!result)
		// failed, perhaps it's not a th13 release rpy
		// so try to treat it as th14 release rpy
		result = _GetStageInfo(magic, pData, size, pOutInfo, true);

	return result;
}

BYTE* ReplayDecode2(
		BYTE* pRpyData,
		DWORD rpysize,
		THRPYINFO2* pInfo
)
{
	DWORD      decodesize;
	DWORD      decompress_size;
	BYTE      *pDecompBuf      = NULL;
	THHEADER2 *pHeader         = (THHEADER2 *)pRpyData;

	// size is too small, wrong replay data
	if (rpysize <= sizeof(THHEADER2) + 4)
		return NULL;

	// obtain header info
	decodesize      = pHeader->datasize;
	decompress_size = pHeader->decoded_datasize;

	// impossible sizes, wrong replay data
	if (decodesize > rpysize || decodesize > decompress_size)
		return NULL;

	// fill some values
	pInfo->wVersion             = pHeader->rpy_version;
	pInfo->dwGameVersion        = pHeader->game_version;
	pInfo->nSpellPracticeNumber = 0;

	pRpyData += sizeof(THHEADER2); // skip the header

#define CALL_DECRYPT(m1,m2,k) (thdecode2_decrypt((char)m1,pRpyData,decodesize,(char)m2,k,decodesize))
	switch (pHeader->magic_number)
	{
		case mgc95:
		case mgc10:
		case mgcalco:
			CALL_DECRYPT(0xaa, 0xe1, 0x400); CALL_DECRYPT(0x3d, 0x7a, 0x80);
			break;
		case mgc11:
			CALL_DECRYPT(0xaa, 0xe1, 0x800); CALL_DECRYPT(0x3d, 0x7a, 0x40);
			break;
		case mgc12:
		case mgc125:
			CALL_DECRYPT(0x5e, 0xe1, 0x800); CALL_DECRYPT(0x7d, 0x3a, 0x40);
			break;
		case mgc128:
			CALL_DECRYPT(0x5e, 0xe7, 0x800); CALL_DECRYPT(0x7d, 0x36, 0x80);
			break;
		case mgc13: // includes th14
		case mgc143:
		case mgc15:
		case mgc16:
			CALL_DECRYPT(0x5c, 0xe1, 0x400); CALL_DECRYPT(0x7d, 0x3a, 0x100);
			break;
		default:
			return NULL;
	}
#undef CALL_DECRYPT

	pDecompBuf = (BYTE*)malloc(decompress_size);
	thdecode2_decompress(pRpyData, decodesize, pDecompBuf);

	pInfo->dwDecodeSize = decompress_size;

	// Explicit casting NULL to 'BYTE*' is used to prevent g++ 4.6.1 from
	// giving an error 'operands to ?: have different types'
	return GetStageInfo(pHeader->magic_number, pDecompBuf, decompress_size, pInfo)
		? pDecompBuf
		: (free(pDecompBuf), pInfo->dwDecodeSize=0, (BYTE*)NULL);
}
