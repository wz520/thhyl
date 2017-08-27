#ifndef __RPYINFO_USERBLOCK_H__55D1958B_
#     define __RPYINFO_USERBLOCK_H__55D1958B_

// 获取 userblock 中的 REP 摘要信息 和 REP 注释。
//
// 和 thuserblock 不同的是，本模块带转码，用于最终的输出(thuserblock 属于底层模块）
//
// 注意类里的方法名首字母都是小写的


#include "StdAfx.h"

#include "thdecode/thcommon.h"
#include "thdecode/thuserblock.h"


class CRpyInfoUserBlock{
public:
	// isCNVer 只用于 th8
	CRpyInfoUserBlock(const RPYUserBlockMgr* pUBMgr, RPYMGC header, UINT codepage, BOOL isCNVer);
	~CRpyInfoUserBlock();

	int analyze();

	// 如果失败，返回空字符串
	CString getComment(UINT codepage);
	CString getSummary() {return m_info;}
	
private:
	int GetUBRpyInfo();

	RPYUserBlockMgr* m_pUBMgr;
	RPYMGC m_header;
	UINT m_codepage;
	BOOL m_isCNVer;
	CString m_info;

	CRpyInfoUserBlock& operator = (const CRpyInfoUserBlock& another);
	CRpyInfoUserBlock(const CRpyInfoUserBlock& another);
};

#endif /* __RPYINFO_USERBLOCK_H__55D1958B_ */
