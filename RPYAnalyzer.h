#ifndef _RPYINFO874_H
#define _RPYINFO874_H

#include "RPYAnalyzer_common.h"

#define MINRPYSIZE						(0x60)

struct THX_EXTRAINFO{
    TH_FPSINFO* pFPSInfo;      // 如果不需要显示 szSlowRate （如 TH6），可以为 NULL
    int         nIndex;        // 小于 0 表示显示所有关卡的综合信息
    DWORD*      dwFrameCounts; // 指向包含各关的总帧数的数组，元素个数不得少于 20
    LPCTSTR     szPlayTime;    // PlayTime 格式字符串，不可为 NULL
    LPCTSTR     szSlowRate;    // SlowRateByFPS 格式字符串，NULL 则忽略该项显示
};



class CRPYAnalyzer
{
public:
    // 【1.70 版更新】现在构造函数会备份 data 所指内容，调用构造函数后可以安全地 free() 或 delete 
    CRPYAnalyzer(
        BYTE* data,   // 指向RPY文件的所有数据
        DWORD size,   // RPY文件大小
        UINT  DefCode // 默认编码。用于TH09以后的RPY文件。
	);
	~CRPYAnalyzer();
    int Analyze(); // 分析录像信息，同一个 RPYAnalyzer 对象不可调用两次，否则会导致内存泄露
    CString GetFinalInfo() const {return m_info;} // 获取最终的录像信息
    CString GetComment(UINT codepage=0); // 获取注释

	// 获取解码后数据的指针。必须先调用 Analyze()。指针内容不可修改。参数 pOutDecodeSize 不可为 NULL
	const BYTE* GetDecodedDataPointer(DWORD* pOutDecodeSize) const;

	// 获取指向各种分析结果的指针，存放于结构体 RPYINFO_POINTERS 中。
	void GetResultPointers(RPYINFO_POINTERS* pointers) const;

	// 将解码后的数据写入文件。必须先调用 Analyze()。
	BOOL DumpRPYData(LPCTSTR filename) const;

	RPYMGC m_header; // 初始为 hdrnull 。调用 Analyze() 后即为 enum RPYMGC 中的某一值，指示该 rpy 所属游戏。
	
protected:
	int GenInfoWrapper(); // 获取 6,7,8,9 各关数据(6, 7 包括整体信息)，会自动根据 rpy 的内容调用下列函数

    void TH6GenInfo(); // 准备要显示的信息
	void TH7GenInfo();
	void TH8GenStageInfo();
	void TH9GenStageInfo();
	void THXAddExtraInfo(const THX_EXTRAINFO* pExtraInfo); //根据选项决定是否添加 PlayTime 和 SlowRateByFPS
	void THXAddExtraInfo2(int nIndex); // for th95+ 的版本

	int GenInfoWrapper2(); // 获取其他作品的各关数据，会调用下列 TH??GenStageInfo 函数

	void THALGenStageInfo();
	void TH10GenStageInfo();
	void TH11GenStageInfo();
	void TH12GenStageInfo();
	void TH13GenStageInfo();
	void TH14GenStageInfo();
	void TH15GenStageInfo();
	void TH16GenStageInfo();
	void TH128GenStageInfo();

	// 无单关信息
	void TH95GenInfo();
	void TH125GenInfo();
	void TH143GenInfo();

    //以下TH8以后专用

	// m_info 中增加信息：在打该录像前，玩家在 他/她/它 的 custom.exe 中所设置的选项。
	// 包括：
	//	获取手柄输入不使用 DirectInput(TH95,10,11,12,128,13,14,143)
	//	按住射击键低速移动（ShotSlow）(不支持 TH6, TH9, TH125，游戏不支持或 RPY 中没有保存该设置)
	//	自机上方显示信仰减少槽(TH10 only)
	//
	//	@b6to9: 为 TRUE 表示 wFlags 是 THRPYINFO::wFlags,
	//	        否则为 THRPYINFO2::wFlags
	void AddGameOptionsInfo(WORD wFlags, BOOL b6to9 = FALSE);

	// m_info 中增加信息：如果是 Spell Practice 录像，显示 SpellCard 的编号
	// 并且如果文件 filename 存在，读取SC名
	//
	// 目前支持 th13, th14 的正式版，以及 th95, th125, th143
	void AddSpellPracticeInfo(int SCNum, LPCTSTR filename, BOOL isScene=FALSE);
private:
    BYTE* m_pData;      // RPY数据，解密（非解压）后的内容也存放于此处
    DWORD m_nDataSize;  // RPY大小
    CString m_info;     // 用于显示的最终录像信息

    THRPYINFO  *m_pTHRpyInfo;  // ReplayDecode()
    THRPYINFO2 *m_pTHRpyInfo2; // ReplayDecode2()
    BYTE       *m_pDecompData; // ReplayDecode2()'s return value.

	//TH8以后专用
    CString m_UBRpyInfo;   // 来自 user block 的转成UNICODE后的文本格式的录像信息
    UINT m_uDefCode;      // 默认编码
    
	RPYUserBlockMgr* m_pUBMgr;   // userblock 管理器
	CRpyInfoUserBlock* m_pInfoUB;   // 从 userblock 中获取文本格式的录像信息和录像注释
};


#endif //_RPYINFO874_H
