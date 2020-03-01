/*********************************
 * Touhou decode routine for     *
 * TH6, TH7, TH8, TH9            * 
 *                               *
 * written by wz520              *
 *********************************/

// ------------ Changelog:
//
// * THE NEWER CHANGELOG MAY NOT BE UPDATED HERE ANY MORE BECAUSE I AM LAZY.
//
// * 2014-06-03:
// 1. Refactored thdecode1_decode():
//      Reformatted the code.
//      Removed unnecessary variables, made the functions shorter.
// 2. Removed unnecessary <stdio.h> including.
// 3. 增加 isCNVersion() 函数以减少 TH?GetInfo() 函数的代码量。
// 4. moved RPHDR to thcommon.h as RPYMGC.
//
// * 2012-02-21:
// 1. RPYFLAG_CNVER 支持 花映冢，可以判断是否是花映冢的中文版录像（日文版不支持的录像）
//
// * 2012-02-06:
// 1. 更改 THRPYINFO::pGeneralInfo 的类型为 union ，这样就不用强制类型转换了，并且可以去掉一些 malloc() 和 free()
// 2. 增加 RPYFLAGS: RPYFLAG_SHOTSLOW, RPYFLAG_P1CHARGE, RPYFLAG_P2CHARGE 。详见下面的说明
// 3. 发现 TH6_STAGEINFO 结构里原本未知的 dwUnknown，其实是 wRandomSeed 和 wPoints 。 wRandomSeed 顾名思义；wPoints 是蓝点数，和后续作品一样是从游戏开始累计的。
// 4. 发现 TH7_STAGEINFO, TH8_STAGEINFO 和 TH9_STAGEINFO 结构里原本未知的 wUnknown 果然是 randomseed 无误，改名为 wRandomSeed。
// 5. TH9_STAGEINFO 结构新发现 cPlace 成员 ，对战场所，只在 Match 模式中有效。详见结构里的说明。
// 6. THRPYINFO 结构增加 dwStageSizes, dwFrameCounts 两个成员，详见该结构的说明
// 7. 才发现红魔乡 rpy 0x04 处原来和其他作品一样也是录像版本号……1.02h 版是 0x0102 以前可能是调试方法问题没有跟踪到，以为游戏根本没用到，进而认为这不是版本号……
// 8. 排版了一下代码，方便阅读
// ⑨ 是笨蛋
// ⑩ ...
//

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "thdecode1.h"
#include "thkeyinfo.h"

#define lengthof(arr)					(sizeof(arr)/sizeof(arr[0]))


THRPYINFO::THRPYINFO() {
	memset(this, 0, sizeof(*this));
	initStageNames();
}


// ------ Core decode func ------
static RPYINFOERR thdecode1_decode(
    BYTE *pSrc,                // pointer to .rpy data
    DWORD dwMask,              // offset of decrypt mask
    DWORD dwStart,             // decrypt start offset
    DWORD dwEnd,               // decrypt end offset
    TH_DECOMPINFO *pDecompInfo // see declaration of this structure for details
)
{
	// format and decode method provided to Chikei Lee 2004/9/10
	// rewritten as php by piaip
	// rewritten as perl by dl
	// rerwitten as C++ by sue445
	// rewritten as C++ by wz520, now can decode all data

	// decrypt
	BYTE mask   = *(pSrc+dwMask);
	BYTE *p     = pSrc + dwStart;
	BYTE *end_p = pSrc + dwEnd;
	for (; p<end_p; ++p) {
		*p -= mask;
		mask += 7;
	}

	if (!pDecompInfo) //No need decompression, TH06
		return RPYINFO_OK;

	if (*(end_p-1)!=0 && *(end_p-1)!=0x80)
		return RPYINFO_DECRYPTERROR;

	// Get Size of decompressed data
	pDecompInfo->size = *( (DWORD *)(pSrc+pDecompInfo->offset) );
	pDecompInfo->size += pDecompInfo->ibase;
	pDecompInfo->pData = (BYTE *)malloc(pDecompInfo->size);
	if (!pDecompInfo->pData)
		return RPYINFO_MALLOCERROR;

	// decompress
	const int count = (int)pDecompInfo->size;
	const int ibase = pDecompInfo->ibase;

	// -- vars
	int v04 = 0, v1c = 0, v30 = 0;
	int v0c = 0, v24 = 0, v10 = 0;
	int v34 = 1;
	int v11 = 0x80;

	// ibase==0x54 if TH07
	const int v20 = *(int*)(pSrc + (ibase == 0x54 ? 0x14 : 0x10));

	BYTE workarea[0x2000];

	// not compressed until ibase
	int cnt1 = ibase;	// input
	BYTE* const pData = pDecompInfo->pData;
	memset(pData, 0, count); // copy the data that's no need to decompress
	memcpy(pData, pSrc, ibase);
	int cnt2 = ibase;	// output

	while (cnt2 < count) {
		while (cnt2 < count) {
			bool flFirstRun = true;
			do {
				if (v11 == 0x80) {
					v04 = *(pSrc+cnt1);
					if (cnt1-ibase < v20)
						cnt1++;
					else
						v04 = 0;
				}  // v11==0x80

				if (flFirstRun) {
					v1c = v04 & v11;
					v11 >>= 1;

					if (v11 == 0)
						v11 = 0x80;

					if (v1c == 0)
						goto exit_loop;

					v30 = 0x80;
					v1c = 0;
					flFirstRun = false;
				}
				else {
					if ((v11 & v04) != 0)
						v1c |= v30;

					v30 >>= 1;
					v11 >>= 1;

					if (v11==0)
						v11 = 0x80;
				}
			}
			while (v30 != 0);

			pData[cnt2++] = (BYTE)v1c;
			workarea[v34] = (BYTE)(v1c & 0xFF);
			v34 = (v34+1) & 0x1FFF;
		}	// 2nd. while(1)
exit_loop:
		if (cnt2 > count)
			break;

		for (v1c = 0, v30 = 0x1000; v30 != 0; ) {
			if (v11 == 0x80) {
				v04 = *(pSrc+cnt1);
				if (cnt1-ibase < v20)
					cnt1++;
				else
					v04 = 0;
			} // $v11 == 0x80

			if ((v11 & v04) != 0)
				v1c |= v30;

			v30 >>= 1;
			v11 >>= 1;

			if (v11 == 0)
				v11 = 0x80;
		}

		v0c = v1c;

		//if(v0c==0)
		//	break;

		for (v1c = 0, v30 = 8; v30 != 0; ) {
			if (v11 == 0x80) {
				v04 = *(pSrc+cnt1);
				if (cnt1-ibase < v20)
					cnt1++;
				else
					v04 = 0;
			} // v11 == 0x80
			if ((v11 & v04) != 0)
				v1c |= v30;
			v30 >>= 1;
			v11 >>= 1;
			if (v11 == 0)
				v11 = 0x80;
		} // v30 != 0
		v24 = v1c + 2;
		for (v10 = 0; v10<=v24 && cnt2<count && v0c<count; ++v10) {
			BYTE v2c = workarea[(v0c+v10)&0x1FFF];
			pData[cnt2++] = v2c;
			workarea[v34] = v2c;
			v34 = (v34+1) & 0x1FFF;
		}	// v10<=v24
	} // 1st. level while(1)

	return RPYINFO_OK;
}

// ----------------------------------
// ------ Core decode func END ------
// ----------------------------------

// ----------------------------------------------------------
// ------------- Common Stage Info Functions ----------------
// ----------------------------------------------------------

// ----------------------------------
// 获取各关的总帧数
// ----------------------------------
static RPYINFOERR GetStageFrameCount(
		RPYMGC header,
        const void* const pStageData[], // 各关数据指针的数组
        const DWORD dwStageSizes[],     // 各关数据长度的数组
        int nStageCount,                // 关卡个数（数组元素个数）
        DWORD outStageFrameCounts[]     // 输出参数，接收各关的总帧数
)
{
	int i;
    int size_of_keydata; // sizeof(TH?_KEYDATA)
    int offset_keydata;  // OFFSET_TH?KEYDATA

	switch (header)
	{
	case mgc6:
		{
			// TH6 的总帧数就在倒数第二个 keydata 的 frame 成员中
			const TH6_KEYDATA* pKeyData = NULL;
			for (i=0; i<nStageCount; ++i) {
				if (pStageData[i] != NULL) {
					// 最后一个 keydata 的 frame 必须是 9999999 ，不是的话说明数据有误
					pKeyData = (const TH6_KEYDATA*)
						((BYTE*)pStageData[i] + dwStageSizes[i] - sizeof(TH6_KEYDATA));
					if (pKeyData->frame != 9999999)
						return RPYINFO_DECRYPTERROR;
					else
						// 取更前面的一个，也就是倒数第二个 keydata 的 frame
						outStageFrameCounts[i] = (--pKeyData)->frame;
				}
			}

			return RPYINFO_OK;
		}
	case mgc7: size_of_keydata = sizeof(TH7_KEYDATA);
			   offset_keydata = OFFSET_TH7KEYDATA;
			   break;
	case mgc8: size_of_keydata = sizeof(TH8_KEYDATA);
			   offset_keydata = OFFSET_TH8KEYDATA;
			   break;
	case mgc9: size_of_keydata = sizeof(TH9_KEYDATA);
			   offset_keydata = OFFSET_TH9KEYDATA;
			   break;
	default:
		return RPYINFO_DECRYPTERROR;
	}

	// 走到这里，RPY 肯定是 7,8 或 9 的，否则已经在上面 return 了
	for (i=0; i<nStageCount; ++i) {
		if (dwStageSizes[i] != 0) { // 该关有数据
			// (总长度 - 第一个 keydata 偏移) / keydata长度
			outStageFrameCounts[i] = (dwStageSizes[i] - offset_keydata) / size_of_keydata;
		}
	}

	return RPYINFO_OK;
}



//获取指向各关数据的指针
static RPYINFOERR GetStageInfoOffsets(
        const BYTE* pData,       // 解密/解压后数据的指针
        DWORD DataSize,          // 解密/解压后数据的大小
        int offsetCount,         // 偏移地址数量
        int maxOffsetCount,      // 最大偏移地址数量，用于计算长度(outStageDataSize[])
        DWORD offsetStart,       // 指针数组相对pData的偏移量
        void* outStageData[],    // 输出参数，一个数组，接收各关数据的指针。数组元素必须是指向 TH?_STAGEINFO(?=6,7,8,9) 结构体的指针。
        DWORD outStageDataSize[] // 输出参数，一个数组，接收各关数据的长度。
)
{
	const DWORD* const StageScores=(DWORD*)(pData+offsetStart); // 各关偏移数组
	int i;
	RPYINFOERR ret = RPYINFO_OK;

	// 记录上一个有效关卡的索引，用于计算关卡数据长度。-1 表示还没有
	int prev_valid_stage = -1;
	
	for (i=0; i<offsetCount; ++i) {
		outStageData[i] = (void*)(pData + StageScores[i]);
		if (outStageData[i] == (void*)pData) { //如果偏移量为0，说明没有此关信息
			outStageData[i] = NULL;
			continue;
		}
		else if (outStageData[i] > (void*)(pData + DataSize)) { //超出数据范围，无效偏移量
			ret = RPYINFO_DECRYPTERROR;
			break;
		}
		
		// 长度 
		// 当前指针 - 上一个有效指针 = 上一个有效指针所指向的关卡数据的长度
		if (prev_valid_stage != -1) {
			outStageDataSize[prev_valid_stage] = (BYTE*)outStageData[i] - (BYTE*)outStageData[prev_valid_stage];
		}
		prev_valid_stage = i;
	}

	// 计算最后一个有效关卡的长度。
	// 继续往下搜索第一个不为 0 的偏移，直到 maxOffsetCount 。
	// 如果超出 maxOffsetCount 都没找到，则取 DataSize
	const BYTE* lastpointer = NULL;
	for (; i<maxOffsetCount; ++i) {
		// 偏移不为 0，记下地址，跳出；
		// 否则继续检查下一个
		if (StageScores[i] != 0) {
			lastpointer = pData + StageScores[i];
			break;
		}
	}

	// 如果上面的循环超出 maxOffsetCount 所示范围，
	// 取 DataSize 计算最后一个有效关的数据大小
	if (i >= maxOffsetCount) 
		lastpointer = pData + DataSize;

	// 填写最后有效关卡的长度
	if (prev_valid_stage != -1) {
		outStageDataSize[prev_valid_stage] = lastpointer - (const BYTE*)outStageData[prev_valid_stage];
	}

	return ret;
}

//获取各关指向FPS数据的指针
static RPYINFOERR GetStageFPSOffsets(
        BYTE* pData,         // 解密/解压后数据的指针
        DWORD DataSize,      // 解密/解压后数据的大小
        int offsetCount,     // 指针数量，不得大于 10
        DWORD offsetStart,   // 指针数组相对pData的偏移量
		TH_FPSINFO* pFPSInfo // 输出参数，接收 FPS 数据和信息。
)
{
	DWORD* const FPSs = (DWORD* const)(pData+offsetStart); //各关FPS信息偏移数组
	int firstfpsindex = -1; // 第一个有效关卡的索引
	int lastfpsindex  = -1;  // 最后一个有效关卡的索引
	RPYINFOERR ret    = RPYINFO_OK;

	pFPSInfo->total_size     = 0;
	pFPSInfo->total_slowrate = 0.0;

	for (int i=0; i<offsetCount; ++i) {
		pFPSInfo->pointers[i] = pData + FPSs[i]; //fps信息指针
		if (pFPSInfo->pointers[i] == pData) { //如果偏移量为0，说明没有此关信息
			pFPSInfo->pointers[i] = NULL;
			continue;
		}
		else if (pFPSInfo->pointers[i] > pData+DataSize) { //超出数据范围，无效偏移量
			return RPYINFO_DECRYPTERROR;
		}
		
		// previous fpsdata
		if (lastfpsindex>=0) {
			// 前一关的 FPS 数据长度 = 该关 FPS 数据指针 - 前一关 FPS 数据指针
			pFPSInfo->sizes[lastfpsindex]     = (DWORD)(pFPSInfo->pointers[i] - pFPSInfo->pointers[lastfpsindex]);
			pFPSInfo->slowrates[lastfpsindex] = CalcSlowRate(pFPSInfo->pointers[lastfpsindex], pFPSInfo->sizes[lastfpsindex]);
		}

		// 记录当前关索引，如果当前关是最后关，会跳出循环。
		// 最后一关 FPS 数据长度和 slowrate 的计算会在循环下面
		lastfpsindex = i; 

		// 记录第一个关卡数据的关卡索引
		// 用于计算之后的 totalsize 和 totalslowrate
		if (firstfpsindex == -1) firstfpsindex = i;
	}
	
	// last fpsdata
	// 最后一个有效关卡的 FPS 信息
	if (lastfpsindex >= 0 && firstfpsindex >= 0) {
		const BYTE* const pDataEnd = pData + DataSize;

		// 最末有效关 FPS 数据长度 = RPY 数据结尾处指针 - 最末有效关 FPS 数据指针
		pFPSInfo->sizes[lastfpsindex]     = (DWORD)(pDataEnd - pFPSInfo->pointers[lastfpsindex]);
		pFPSInfo->slowrates[lastfpsindex] = CalcSlowRate(pFPSInfo->pointers[lastfpsindex], pFPSInfo->sizes[lastfpsindex]);

		// 总 FPS 数据长度 = RPY 数据结尾处指针 - 最初有效关 FPS 数据指针
		pFPSInfo->total_size     = (DWORD)(pDataEnd - pFPSInfo->pointers[firstfpsindex]);
		pFPSInfo->total_slowrate = CalcSlowRate(pFPSInfo->pointers[firstfpsindex], pFPSInfo->total_size);
	}

	return ret;
}

// 判断是否是中文版（渔场汉化版）。
static inline bool isCNVersion(
		const BYTE* pDecompData,
		DWORD offsetVersion,
		const char* pszVersion,  // 长度必须是 5
		DWORD checksum1,
		DWORD checksum2
)
{
	const BYTE* const pVersion = pDecompData + offsetVersion;
	if ( memcmp(pVersion, pszVersion, 5) == 0 ) {
		if ( *(DWORD*)(pVersion-8) != checksum1 || *(DWORD*)(pVersion-4) != checksum2)
			return true;
	}
	return false;
}

// 这里的 player 因为表示索引，所以是 0 开头的，0 代表 player1
void fillStagePointers(THRPYINFO* pOutInfo, const void* const * stagepointers, const DWORD * stagesizes, int player=0)
{
	for ( int i = 0; i < pOutInfo->nStageCount; ++i ) {
		pOutInfo->stagepointers[player][i].p      = (BYTE*)(stagepointers[i]);
		pOutInfo->stagepointers[player][i].offset = (BYTE*)(stagepointers[i]) - pOutInfo->pDecodeData;
		pOutInfo->stagepointers[player][i].size   = stagesizes[i];

		pOutInfo->fpspointers[i].p      = pOutInfo->fpsinfo.pointers[i];
		pOutInfo->fpspointers[i].offset = pOutInfo->fpsinfo.pointers[i] - pOutInfo->pDecodeData;
		pOutInfo->fpspointers[i].size   = pOutInfo->fpsinfo.sizes[i];
	}
}


//----- Common Stage Info Functions END -----

static RPYINFOERR TH6GetInfo(BYTE* pData, DWORD size, THRPYINFO* pOutInfo)
{
	RPYINFOERR ret = thdecode1_decode(pData, 0x0E, 0x0F, size, NULL);

	if (ret == RPYINFO_OK) {
		TH6_RPYINFO* const pInfo = &pOutInfo->GeneralInfo.th6;

		// copy summary info
		memset(pInfo, 0, sizeof(TH6_RPYINFO));
		memcpy(pInfo->szDate  , pData+0x10, sizeof(pInfo->szDate));
		memcpy(pInfo->szPlayer, pData+0x19, sizeof(pInfo->szPlayer));
		pInfo->cGameMinorVersion = pData[0x04];
		pInfo->cGameMajorVersion = pData[0x05];
		pInfo->cChara            = pData[0x06];
		pInfo->cRank             = pData[0x07];
		pInfo->dwScore           = *(DWORD*)(pData+0x24);
		pInfo->fDrop             = *(float*)(pData+0x2C);

		// since TH6 doesn't need decompression,
		// here we need to copy the decrypted data to pOutInfo->pDecodeData to keep consistency
		// for TH7~TH9, just copy the pointer pOutInfo->DecompInfo.pData to pOutInfo->pDecodeData
		pOutInfo->pDecodeData = (BYTE*)malloc(size);
		if (!pOutInfo->pDecodeData)
			return RPYINFO_MALLOCERROR;
		memcpy(pOutInfo->pDecodeData, pData, size);
		pData = pOutInfo->pDecodeData;

		// 为了保持一致，尽管 DecompInfo 没有用到，也要填一下。
		pOutInfo->DecompInfo.pData = pData;
		pOutInfo->DecompInfo.size  = size;

		// Get pointers to stage data.
		const int nStageCount     = 7;
		const int nMaxOffsetCount = 7;
		pOutInfo->nStageCount = nStageCount;
		ret = GetStageInfoOffsets(pData, size, nStageCount, nMaxOffsetCount, 0x34, pOutInfo->pStageInfo.v, pOutInfo->dwStageSizes);

		// 获取总帧数
		if (ret == RPYINFO_OK)
			ret = GetStageFrameCount(pOutInfo->header, pOutInfo->pStageInfo.v, pOutInfo->dwStageSizes, nStageCount, pOutInfo->dwFrameCounts);

		// 填写 stagepointers
		if ( ret == RPYINFO_OK ) {
			fillStagePointers(pOutInfo, pOutInfo->pStageInfo.v, pOutInfo->dwStageSizes);
		}
	}

	return ret;
}

// 修正版本号显示
// pOutput 必须至少有 7 个字节容量
static void TH7GetVersionString(const BYTE* const pInput, char* const pOutput)
{
	// 1.00b 在内部是 "0100b"，要让它显示为 "1.00b"
	// 如果是演示录像，就是 "debug"
	if ( isdigit(pInput[1]) ) {
		char myver[7];
		memcpy(myver  , pInput, 2);
		myver[2]='.';
		memcpy(myver+3, pInput+2, 3);
		if (myver[5]=='_')	myver[5]='\0';
		if (myver[0]=='0')
			memcpy(pOutput, myver+1, 5);
		else
			memcpy(pOutput, myver  , 6);
	}
	else {
		// 可能是 debug
		memcpy(pOutput, pInput, 7);
	}
}

static RPYINFOERR TH7GetInfo(BYTE* pData, DWORD size, THRPYINFO* pOutInfo)
{
	// stagenames
	pOutInfo->stagenames[6] = _T("EX/PH");

	pOutInfo->DecompInfo.ibase  = 0x54;
	pOutInfo->DecompInfo.offset = 0x18;
	RPYINFOERR ret = thdecode1_decode(pData, 0x0D, 0x10, size, &pOutInfo->DecompInfo);

	if (ret == RPYINFO_OK) {
		BYTE* const pDecompData  = pOutInfo->DecompInfo.pData;
		DWORD const dwDecompSize = pOutInfo->DecompInfo.size;
		TH7_RPYINFO* const pInfo = &pOutInfo->GeneralInfo.th7;

		// copy summary info
		memset(pInfo, 0, sizeof(TH7_RPYINFO));
		memcpy(pInfo->szDate  , pDecompData+0x58, sizeof(pInfo->szDate));
		memcpy(pInfo->szPlayer, pDecompData+0x5E, sizeof(pInfo->szPlayer));
		pInfo->cChara = pDecompData[0x56];
		pInfo->cRank  = pDecompData[0x57];
		// 修正版本号显示
		TH7GetVersionString(pDecompData+0xE0, pInfo->szVersion);
		// Others
		pInfo->dwScore = *(DWORD*)(pDecompData+0x6C);
		pInfo->fDrop   = *(float*)(pDecompData+0xCC);

		// 判断是否是中文版（渔场汉化版）。
		if ( isCNVersion(pDecompData, 0xE0, "0100b", 0x0009EE00, 0xAEC5445C) )
				pOutInfo->wFlags |= RPYFLAG_CNVER;

		// 检测是否开启了 ShotSlow
		if (pDecompData[0x96] != 0)
			pOutInfo->wFlags |= RPYFLAG_SHOTSLOW;

		// for TH7~TH9, just copy the pointer pOutInfo->DecompInfo.pData to pOutInfo->pDecodeData
		pOutInfo->pDecodeData = pOutInfo->DecompInfo.pData;

		// Get pointers to stage data
		const int nStageCount     = 7;
		const int nMaxOffsetCount = 14; // 7 stage data + 7 FPS data
		pOutInfo->nStageCount = nStageCount;
		ret = GetStageInfoOffsets(pDecompData, dwDecompSize, nStageCount, nMaxOffsetCount, 0x1C, pOutInfo->pStageInfo.v, pOutInfo->dwStageSizes);

		if (ret == RPYINFO_OK)
			ret = GetStageFPSOffsets(pDecompData, dwDecompSize, nStageCount, 0x38, &pOutInfo->fpsinfo);

		// 获取总帧数
		if (ret == RPYINFO_OK)
			ret = GetStageFrameCount(pOutInfo->header, pOutInfo->pStageInfo.v, pOutInfo->dwStageSizes, nStageCount, pOutInfo->dwFrameCounts);

		// 填写 stagepointers
		if ( ret == RPYINFO_OK ) {
			fillStagePointers(pOutInfo, pOutInfo->pStageInfo.v, pOutInfo->dwStageSizes);
		}
	}
	
	return ret;
}

static RPYINFOERR TH8GetInfo(BYTE* pData, DWORD size, THRPYINFO* pOutInfo)
{
	// stagenames
	LPCTSTR const  newstagenames[] = {
		_T("4A"), _T("4B"), _T("5"), _T("6A"), _T("6B"), _T("EX")
	};
	pOutInfo->setStageNames(newstagenames, 3, 6);

	const DWORD decode_size = *(DWORD*)(pData+0x0C);
	pOutInfo->DecompInfo.ibase  = 0x68;
	pOutInfo->DecompInfo.offset = 0x1C;
	RPYINFOERR ret = thdecode1_decode(pData, 0x15, 0x18, decode_size, &pOutInfo->DecompInfo);

	if (ret == RPYINFO_OK) {
		BYTE* const pDecompData  = pOutInfo->DecompInfo.pData;
		DWORD const dwDecompSize = pOutInfo->DecompInfo.size;

		// 无录像摘要信息，清零
		memset(&pOutInfo->GeneralInfo, 0, sizeof(pOutInfo->GeneralInfo));

		// 判断是否是中文版（渔场汉化版）。
		if ( isCNVersion(pDecompData, 0x12C, "0100d", 0x000CD400, 0xA26861B9) )
				pOutInfo->wFlags |= RPYFLAG_CNVER;

		// 检测是否开启了 ShotSlow
		if (pDecompData[0xDA] != 0)
			pOutInfo->wFlags |= RPYFLAG_SHOTSLOW;

		// for TH7~TH9, just copy the pointer pOutInfo->DecompInfo.pData to pOutInfo->pDecodeData
		pOutInfo->pDecodeData = pOutInfo->DecompInfo.pData;

		// Get pointers to stage data
		const int nStageCount     = 9;
		const int nMaxOffsetCount = 18; // 9 stage data + 9 FPS data
		pOutInfo->nStageCount = nStageCount;
		ret = GetStageInfoOffsets(pDecompData, dwDecompSize, nStageCount, nMaxOffsetCount, 0x20, pOutInfo->pStageInfo.v, pOutInfo->dwStageSizes);

		if (ret == RPYINFO_OK)
			ret = GetStageFPSOffsets(pDecompData, dwDecompSize, nStageCount, 0x44, &pOutInfo->fpsinfo);

		// 获取总帧数
		if (ret == RPYINFO_OK)
			ret = GetStageFrameCount(pOutInfo->header, pOutInfo->pStageInfo.v, pOutInfo->dwStageSizes, nStageCount, pOutInfo->dwFrameCounts);

		// 填写 stagepointers
		if ( ret == RPYINFO_OK ) {
			fillStagePointers(pOutInfo, pOutInfo->pStageInfo.v, pOutInfo->dwStageSizes);
		}
	}

	return ret;
}

static RPYINFOERR TH9GetInfo(BYTE* pData, DWORD size, THRPYINFO* pOutInfo)
{
	// stagenames
	LPCTSTR const newstagenames[] = { _T("7"), _T("8"), _T("9"), _T("Match") };
	pOutInfo->setStageNames(newstagenames, 6, 4);

	const DWORD decode_size = *((DWORD*)(pData+0x0C));

	pOutInfo->DecompInfo.ibase  = 0xC0;
	pOutInfo->DecompInfo.offset = 0x1C;
	RPYINFOERR ret = thdecode1_decode(pData, 0x15, 0x18, decode_size, &pOutInfo->DecompInfo);

	if (ret == RPYINFO_OK) {
		BYTE* const pDecompData  = pOutInfo->DecompInfo.pData;
		DWORD const dwDecompSize = pOutInfo->DecompInfo.size;

		// 无录像摘要信息，清零
		memset(&pOutInfo->GeneralInfo, 0, sizeof(pOutInfo->GeneralInfo));

		// 判断是否是中文版（渔场汉化版）。
		if ( isCNVersion(pDecompData, 0x1DC, "0150a", 0x000A7400, 0xABEE4C8F) )
				pOutInfo->wFlags |= RPYFLAG_CNVER;

		// 检测 Charge Type
		if (pDecompData[0x190] != 0)
			pOutInfo->wFlags |= RPYFLAG_P1CHARGE;
		if (pDecompData[0x191] != 0)
			pOutInfo->wFlags |= RPYFLAG_P2CHARGE;

		// for TH7~TH9, just copy the pointer pOutInfo->DecompInfo.pData to pOutInfo->pDecodeData
		pOutInfo->pDecodeData = pOutInfo->DecompInfo.pData;

		// Get pointers to stage data.
		const int nStageCount     = 10;
		const int nMaxOffsetCount = 40; // 10 stage data*3(p1+p2+maybe checksum) + 10 fps data
		pOutInfo->nStageCount = nStageCount;
		ret = GetStageInfoOffsets(pDecompData, dwDecompSize, nStageCount*2, nMaxOffsetCount, 0x20, pOutInfo->pStageInfo.v, pOutInfo->dwStageSizes); // nStageCount*2: player 1 and player 2

		if (ret == RPYINFO_OK)
			ret = GetStageFPSOffsets(pDecompData, dwDecompSize, nStageCount, 0x98, &pOutInfo->fpsinfo);

		// 获取总帧数
		if (ret == RPYINFO_OK)
			ret = GetStageFrameCount(pOutInfo->header, pOutInfo->pStageInfo.v, pOutInfo->dwStageSizes, nStageCount*2, pOutInfo->dwFrameCounts); // P1 and P2

		// 填写 stagepointers
		if ( ret == RPYINFO_OK ) {
			fillStagePointers(pOutInfo, &pOutInfo->pStageInfo.v[0], &pOutInfo->dwStageSizes[0]);      // P1
			fillStagePointers(pOutInfo, &pOutInfo->pStageInfo.v[10], &pOutInfo->dwStageSizes[10], 1); // P2
		}
	}
	
	return ret;
}

RPYINFOERR ReplayDecode(BYTE* pRpyData, DWORD rpysize, THRPYINFO* pInfo)
{
	RPYINFOERR ri_err   = RPYINFO_UNKNOWNFORMAT;
	RPYMGC     header   = *((RPYMGC *)pRpyData);
	WORD       wVersion = *((WORD *)(pRpyData+4));
	typedef struct tagRPYINFOFUNCMAP { //函数表
		RPYMGC header;
		RPYINFOERR (*pFunc)(BYTE*, DWORD, THRPYINFO*);
	}RPYINFOFUNCMAP;
	RPYINFOFUNCMAP RPYInfoFuncMap[]={
		{mgc6,  &TH6GetInfo}, {mgc7,  &TH7GetInfo}, {mgc8,  &TH8GetInfo}, {mgc9,  &TH9GetInfo}
	};

	pInfo->header   = header;
	pInfo->wVersion = wVersion;

	for (int i=0; i<lengthof(RPYInfoFuncMap); ++i) {
		if (header == RPYInfoFuncMap[i].header) {
			ri_err = (*RPYInfoFuncMap[i].pFunc)(pRpyData, rpysize, pInfo);
			if ( ri_err == RPYINFO_OK) {
				pInfo->dwDecodeSize = pInfo->DecompInfo.size;
				break;
			}
			else if ( ri_err != RPYINFO_UNKNOWNFORMAT ) {
				ReplayDecode_clean(pInfo); // free memory on error
				break;
			}
		}
	}

	return ri_err;
}

void ReplayDecode_clean(THRPYINFO* pInfo)
{
	int i;

	free(pInfo->pDecodeData);
	pInfo->pDecodeData      = NULL;
	pInfo->DecompInfo.pData = NULL; // 和 pInfo->pDecodeData 指向同一个地方

	// 以下指针指向的是为 pDecodeData 分配的内存，
	// 由于 pDecodeData 已经在上面被 free()，所以直接置 NULL 即可。
	for (i=0; i<10; ++i)
		pInfo->fpsinfo.pointers[i] = NULL;
	for (i=0; i<20; ++i)
		pInfo->pStageInfo.v[i] = NULL;
}
