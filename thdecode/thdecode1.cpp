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
// 3. ���� isCNVersion() �����Լ��� TH?GetInfo() �����Ĵ�������
// 4. moved RPHDR to thcommon.h as RPYMGC.
//
// * 2012-02-21:
// 1. RPYFLAG_CNVER ֧�� ��ӳڣ�������ж��Ƿ��ǻ�ӳڣ�����İ�¼�����İ治֧�ֵ�¼��
//
// * 2012-02-06:
// 1. ���� THRPYINFO::pGeneralInfo ������Ϊ union �������Ͳ���ǿ������ת���ˣ����ҿ���ȥ��һЩ malloc() �� free()
// 2. ���� RPYFLAGS: RPYFLAG_SHOTSLOW, RPYFLAG_P1CHARGE, RPYFLAG_P2CHARGE ����������˵��
// 3. ���� TH6_STAGEINFO �ṹ��ԭ��δ֪�� dwUnknown����ʵ�� wRandomSeed �� wPoints �� wRandomSeed ����˼�壻wPoints �����������ͺ�����Ʒһ���Ǵ���Ϸ��ʼ�ۼƵġ�
// 4. ���� TH7_STAGEINFO, TH8_STAGEINFO �� TH9_STAGEINFO �ṹ��ԭ��δ֪�� wUnknown ��Ȼ�� randomseed ���󣬸���Ϊ wRandomSeed��
// 5. TH9_STAGEINFO �ṹ�·��� cPlace ��Ա ����ս������ֻ�� Match ģʽ����Ч������ṹ���˵����
// 6. THRPYINFO �ṹ���� dwStageSizes, dwFrameCounts ������Ա������ýṹ��˵��
// 7. �ŷ��ֺ�ħ�� rpy 0x04 ��ԭ����������Ʒһ��Ҳ��¼��汾�š���1.02h ���� 0x0102 ��ǰ�����ǵ��Է�������û�и��ٵ�����Ϊ��Ϸ����û�õ���������Ϊ�ⲻ�ǰ汾�š���
// 8. �Ű���һ�´��룬�����Ķ�
// �� �Ǳ���
// �� ...
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
// ��ȡ���ص���֡��
// ----------------------------------
static RPYINFOERR GetStageFrameCount(
		RPYMGC header,
        const void* const pStageData[], // ��������ָ�������
        const DWORD dwStageSizes[],     // �������ݳ��ȵ�����
        int nStageCount,                // �ؿ�����������Ԫ�ظ�����
        DWORD outStageFrameCounts[]     // ������������ո��ص���֡��
)
{
	int i;
    int size_of_keydata; // sizeof(TH?_KEYDATA)
    int offset_keydata;  // OFFSET_TH?KEYDATA

	switch (header)
	{
	case mgc6:
		{
			// TH6 ����֡�����ڵ����ڶ��� keydata �� frame ��Ա��
			const TH6_KEYDATA* pKeyData = NULL;
			for (i=0; i<nStageCount; ++i) {
				if (pStageData[i] != NULL) {
					// ���һ�� keydata �� frame ������ 9999999 �����ǵĻ�˵����������
					pKeyData = (const TH6_KEYDATA*)
						((BYTE*)pStageData[i] + dwStageSizes[i] - sizeof(TH6_KEYDATA));
					if (pKeyData->frame != 9999999)
						return RPYINFO_DECRYPTERROR;
					else
						// ȡ��ǰ���һ����Ҳ���ǵ����ڶ��� keydata �� frame
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

	// �ߵ����RPY �϶��� 7,8 �� 9 �ģ������Ѿ������� return ��
	for (i=0; i<nStageCount; ++i) {
		if (dwStageSizes[i] != 0) { // �ù�������
			// (�ܳ��� - ��һ�� keydata ƫ��) / keydata����
			outStageFrameCounts[i] = (dwStageSizes[i] - offset_keydata) / size_of_keydata;
		}
	}

	return RPYINFO_OK;
}



//��ȡָ��������ݵ�ָ��
static RPYINFOERR GetStageInfoOffsets(
        const BYTE* pData,       // ����/��ѹ�����ݵ�ָ��
        DWORD DataSize,          // ����/��ѹ�����ݵĴ�С
        int offsetCount,         // ƫ�Ƶ�ַ����
        int maxOffsetCount,      // ���ƫ�Ƶ�ַ���������ڼ��㳤��(outStageDataSize[])
        DWORD offsetStart,       // ָ���������pData��ƫ����
        void* outStageData[],    // ���������һ�����飬���ո������ݵ�ָ�롣����Ԫ�ر�����ָ�� TH?_STAGEINFO(?=6,7,8,9) �ṹ���ָ�롣
        DWORD outStageDataSize[] // ���������һ�����飬���ո������ݵĳ��ȡ�
)
{
	const DWORD* const StageScores=(DWORD*)(pData+offsetStart); // ����ƫ������
	int i;
	RPYINFOERR ret = RPYINFO_OK;

	// ��¼��һ����Ч�ؿ������������ڼ���ؿ����ݳ��ȡ�-1 ��ʾ��û��
	int prev_valid_stage = -1;
	
	for (i=0; i<offsetCount; ++i) {
		outStageData[i] = (void*)(pData + StageScores[i]);
		if (outStageData[i] == (void*)pData) { //���ƫ����Ϊ0��˵��û�д˹���Ϣ
			outStageData[i] = NULL;
			continue;
		}
		else if (outStageData[i] > (void*)(pData + DataSize)) { //�������ݷ�Χ����Чƫ����
			ret = RPYINFO_DECRYPTERROR;
			break;
		}
		
		// ���� 
		// ��ǰָ�� - ��һ����Чָ�� = ��һ����Чָ����ָ��Ĺؿ����ݵĳ���
		if (prev_valid_stage != -1) {
			outStageDataSize[prev_valid_stage] = (BYTE*)outStageData[i] - (BYTE*)outStageData[prev_valid_stage];
		}
		prev_valid_stage = i;
	}

	// �������һ����Ч�ؿ��ĳ��ȡ�
	// ��������������һ����Ϊ 0 ��ƫ�ƣ�ֱ�� maxOffsetCount ��
	// ������� maxOffsetCount ��û�ҵ�����ȡ DataSize
	const BYTE* lastpointer = NULL;
	for (; i<maxOffsetCount; ++i) {
		// ƫ�Ʋ�Ϊ 0�����µ�ַ��������
		// ������������һ��
		if (StageScores[i] != 0) {
			lastpointer = pData + StageScores[i];
			break;
		}
	}

	// ��������ѭ������ maxOffsetCount ��ʾ��Χ��
	// ȡ DataSize �������һ����Ч�ص����ݴ�С
	if (i >= maxOffsetCount) 
		lastpointer = pData + DataSize;

	// ��д�����Ч�ؿ��ĳ���
	if (prev_valid_stage != -1) {
		outStageDataSize[prev_valid_stage] = lastpointer - (const BYTE*)outStageData[prev_valid_stage];
	}

	return ret;
}

//��ȡ����ָ��FPS���ݵ�ָ��
static RPYINFOERR GetStageFPSOffsets(
        BYTE* pData,         // ����/��ѹ�����ݵ�ָ��
        DWORD DataSize,      // ����/��ѹ�����ݵĴ�С
        int offsetCount,     // ָ�����������ô��� 10
        DWORD offsetStart,   // ָ���������pData��ƫ����
		TH_FPSINFO* pFPSInfo // ������������� FPS ���ݺ���Ϣ��
)
{
	DWORD* const FPSs = (DWORD* const)(pData+offsetStart); //����FPS��Ϣƫ������
	int firstfpsindex = -1; // ��һ����Ч�ؿ�������
	int lastfpsindex  = -1;  // ���һ����Ч�ؿ�������
	RPYINFOERR ret    = RPYINFO_OK;

	pFPSInfo->total_size     = 0;
	pFPSInfo->total_slowrate = 0.0;

	for (int i=0; i<offsetCount; ++i) {
		pFPSInfo->pointers[i] = pData + FPSs[i]; //fps��Ϣָ��
		if (pFPSInfo->pointers[i] == pData) { //���ƫ����Ϊ0��˵��û�д˹���Ϣ
			pFPSInfo->pointers[i] = NULL;
			continue;
		}
		else if (pFPSInfo->pointers[i] > pData+DataSize) { //�������ݷ�Χ����Чƫ����
			return RPYINFO_DECRYPTERROR;
		}
		
		// previous fpsdata
		if (lastfpsindex>=0) {
			// ǰһ�ص� FPS ���ݳ��� = �ù� FPS ����ָ�� - ǰһ�� FPS ����ָ��
			pFPSInfo->sizes[lastfpsindex]     = (DWORD)(pFPSInfo->pointers[i] - pFPSInfo->pointers[lastfpsindex]);
			pFPSInfo->slowrates[lastfpsindex] = CalcSlowRate(pFPSInfo->pointers[lastfpsindex], pFPSInfo->sizes[lastfpsindex]);
		}

		// ��¼��ǰ�������������ǰ�������أ�������ѭ����
		// ���һ�� FPS ���ݳ��Ⱥ� slowrate �ļ������ѭ������
		lastfpsindex = i; 

		// ��¼��һ���ؿ����ݵĹؿ�����
		// ���ڼ���֮��� totalsize �� totalslowrate
		if (firstfpsindex == -1) firstfpsindex = i;
	}
	
	// last fpsdata
	// ���һ����Ч�ؿ��� FPS ��Ϣ
	if (lastfpsindex >= 0 && firstfpsindex >= 0) {
		const BYTE* const pDataEnd = pData + DataSize;

		// ��ĩ��Ч�� FPS ���ݳ��� = RPY ���ݽ�β��ָ�� - ��ĩ��Ч�� FPS ����ָ��
		pFPSInfo->sizes[lastfpsindex]     = (DWORD)(pDataEnd - pFPSInfo->pointers[lastfpsindex]);
		pFPSInfo->slowrates[lastfpsindex] = CalcSlowRate(pFPSInfo->pointers[lastfpsindex], pFPSInfo->sizes[lastfpsindex]);

		// �� FPS ���ݳ��� = RPY ���ݽ�β��ָ�� - �����Ч�� FPS ����ָ��
		pFPSInfo->total_size     = (DWORD)(pDataEnd - pFPSInfo->pointers[firstfpsindex]);
		pFPSInfo->total_slowrate = CalcSlowRate(pFPSInfo->pointers[firstfpsindex], pFPSInfo->total_size);
	}

	return ret;
}

// �ж��Ƿ������İ棨�泡�����棩��
static inline bool isCNVersion(
		const BYTE* pDecompData,
		DWORD offsetVersion,
		const char* pszVersion,  // ���ȱ����� 5
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

// ����� player ��Ϊ��ʾ������������ 0 ��ͷ�ģ�0 ���� player1
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

		// Ϊ�˱���һ�£����� DecompInfo û���õ���ҲҪ��һ�¡�
		pOutInfo->DecompInfo.pData = pData;
		pOutInfo->DecompInfo.size  = size;

		// Get pointers to stage data.
		const int nStageCount     = 7;
		const int nMaxOffsetCount = 7;
		pOutInfo->nStageCount = nStageCount;
		ret = GetStageInfoOffsets(pData, size, nStageCount, nMaxOffsetCount, 0x34, pOutInfo->pStageInfo.v, pOutInfo->dwStageSizes);

		// ��ȡ��֡��
		if (ret == RPYINFO_OK)
			ret = GetStageFrameCount(pOutInfo->header, pOutInfo->pStageInfo.v, pOutInfo->dwStageSizes, nStageCount, pOutInfo->dwFrameCounts);

		// ��д stagepointers
		if ( ret == RPYINFO_OK ) {
			fillStagePointers(pOutInfo, pOutInfo->pStageInfo.v, pOutInfo->dwStageSizes);
		}
	}

	return ret;
}

// �����汾����ʾ
// pOutput ���������� 7 ���ֽ�����
static void TH7GetVersionString(const BYTE* const pInput, char* const pOutput)
{
	// 1.00b ���ڲ��� "0100b"��Ҫ������ʾΪ "1.00b"
	// �������ʾ¼�񣬾��� "debug"
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
		// ������ debug
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
		// �����汾����ʾ
		TH7GetVersionString(pDecompData+0xE0, pInfo->szVersion);
		// Others
		pInfo->dwScore = *(DWORD*)(pDecompData+0x6C);
		pInfo->fDrop   = *(float*)(pDecompData+0xCC);

		// �ж��Ƿ������İ棨�泡�����棩��
		if ( isCNVersion(pDecompData, 0xE0, "0100b", 0x0009EE00, 0xAEC5445C) )
				pOutInfo->wFlags |= RPYFLAG_CNVER;

		// ����Ƿ����� ShotSlow
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

		// ��ȡ��֡��
		if (ret == RPYINFO_OK)
			ret = GetStageFrameCount(pOutInfo->header, pOutInfo->pStageInfo.v, pOutInfo->dwStageSizes, nStageCount, pOutInfo->dwFrameCounts);

		// ��д stagepointers
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

		// ��¼��ժҪ��Ϣ������
		memset(&pOutInfo->GeneralInfo, 0, sizeof(pOutInfo->GeneralInfo));

		// �ж��Ƿ������İ棨�泡�����棩��
		if ( isCNVersion(pDecompData, 0x12C, "0100d", 0x000CD400, 0xA26861B9) )
				pOutInfo->wFlags |= RPYFLAG_CNVER;

		// ����Ƿ����� ShotSlow
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

		// ��ȡ��֡��
		if (ret == RPYINFO_OK)
			ret = GetStageFrameCount(pOutInfo->header, pOutInfo->pStageInfo.v, pOutInfo->dwStageSizes, nStageCount, pOutInfo->dwFrameCounts);

		// ��д stagepointers
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

		// ��¼��ժҪ��Ϣ������
		memset(&pOutInfo->GeneralInfo, 0, sizeof(pOutInfo->GeneralInfo));

		// �ж��Ƿ������İ棨�泡�����棩��
		if ( isCNVersion(pDecompData, 0x1DC, "0150a", 0x000A7400, 0xABEE4C8F) )
				pOutInfo->wFlags |= RPYFLAG_CNVER;

		// ��� Charge Type
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

		// ��ȡ��֡��
		if (ret == RPYINFO_OK)
			ret = GetStageFrameCount(pOutInfo->header, pOutInfo->pStageInfo.v, pOutInfo->dwStageSizes, nStageCount*2, pOutInfo->dwFrameCounts); // P1 and P2

		// ��д stagepointers
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
	typedef struct tagRPYINFOFUNCMAP { //������
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
	pInfo->DecompInfo.pData = NULL; // �� pInfo->pDecodeData ָ��ͬһ���ط�

	// ����ָ��ָ�����Ϊ pDecodeData ������ڴ棬
	// ���� pDecodeData �Ѿ������汻 free()������ֱ���� NULL ���ɡ�
	for (i=0; i<10; ++i)
		pInfo->fpsinfo.pointers[i] = NULL;
	for (i=0; i<20; ++i)
		pInfo->pStageInfo.v[i] = NULL;
}
