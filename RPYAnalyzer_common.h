#ifndef __RPYINFO_COMMON_H__55D17A39_
#     define __RPYINFO_COMMON_H__55D17A39_

#include "thdecode/thdecode1.h"
#include "thdecode/thdecode2.h"

// Error value
#define RPYINFO_THHYL_CONVCODEERROR		10	// ANSI->UNICODE error

// RPYINFO::GetResultPointers() 的返回结果
// 用于获取RPY分析后的数据
// 为了防止意外更改内容，指针指向的类型都是 const ，不建议转成非 const 。
struct RPYINFO_POINTERS {
	const THRPYINFO  *pRPYInfo1;
	const THRPYINFO2 *pRPYInfo2;
	const BYTE       *pDecompData;
	DWORD             dwDecompDataSize;
};

#endif /* __RPYINFO_COMMON_H__55D17A39_ */


