#ifndef __THUSERBLOCK_H__537E773A_
#     define __THUSERBLOCK_H__537E773A_

#include "thcommon.h"

// Touhou replay file's userblock management class for
// TH8, TH9, TH95, TH10, TH11, TH12, TH125, TH128, TH13, TH14, alcostg, TH143
//
// written by wz520
// Last Update: 2015-08-17

enum RPYUBID {
	ubid_info,
	ubid_comment
};

struct RPYUBHeader {
	DWORD marker; // must be 'USER'
	DWORD length;

	// 0=rpyinfo, 1=comment
	// NOTE: only the first byte(8 bits) is used.
	// RPYUBID is defined as enum to ensure that sizeof(RPYUBHeader) == 12
	RPYUBID id : 8;
};

// 代表一个 UserBlock
class RPYUserBlock {
public:
	// pData 会被备份
	RPYUserBlock(const BYTE* pData, size_t datasize, RPYUBID id);
	RPYUserBlock(const RPYUBHeader* pHeader);
	~RPYUserBlock();

	const RPYUBHeader* getHeader() const { return m_pHeader; }
	const BYTE* getRaw() const { return (const BYTE*)m_pHeader; }
	// to get rawsize, use getHeader()->length;
	const BYTE* getData() const { return this->getRaw() + sizeof(RPYUBHeader); }
	DWORD getDataSize() const { return m_pHeader->length - sizeof(RPYUBHeader); }
	bool isValid() const { return m_pHeader->length >= sizeof(RPYUBHeader); }

private:
	void init(const BYTE* pData, size_t datasize, RPYUBID id);

	RPYUBHeader* m_pHeader;
};


// 从 rpy 文件获取 UserBlock 数据并进行管理，可以添加、删除、遍历等等...
// NOTE: 目前最多只能处理 MAXUSERBLOCKCOUNT 个 UserBlock 。
// 所有方法的 index 参数必须符合： 0 <= index < MAXUSERBLOCKCOUNT 。
#define MAXUSERBLOCKCOUNT 10

class RPYUserBlockMgr {
public:
	RPYUserBlockMgr();

	// pData 必须指向 rpy 文件数据。
	// 内部只存储 UserBlock 相关数据，不会存储其他内容。
	RPYUserBlockMgr(const BYTE* pRPYData, size_t sz);

	// 拷贝构造
	RPYUserBlockMgr(const RPYUserBlockMgr &other);

	~RPYUserBlockMgr();
	
	// 保存所有 block 到一个 rpy 文件数据块中。
	// 【注意】：rpy 文件中原有的 user block 会被全部覆盖！
	// 返回使用 malloc() 分配的新内存，调用者需要使用 free() 释放返回的指针。
	// 如果 pRPYData 里的数据不是合法的 TH8+ rpy，返回 NULL。
	BYTE* saveToRPY(const BYTE* pRPYData, size_t sz, size_t* pNewSize);
	
	bool append(const RPYUserBlock* pBlock);
	bool remove(int index);

	// 返回的指针无需释放，但若其所属的 RPYUserBlockMgr 对象被释放，则不可再使用。
	// 若无法获取，返回 NULL
	const RPYUserBlock* get(int index) const;
	
	// 设置新的 userblock 到指定的位置上。
	// 如果无法设置（比如 index 超出范围），返回 false
	// NOTE: pBlock 必须不为 NULL
	bool set(int index, const RPYUserBlock* pBlock);

	const RPYUserBlock* getFirstBlockByID(RPYUBID id) const;
	// get first info block
	const RPYUserBlock* getInfo() const { return getFirstBlockByID(ubid_info); }
	// get first comment block
	const RPYUserBlock* getComment() const { return getFirstBlockByID(ubid_comment); }

	int getFirstIndexByID(RPYUBID id) const;
	// get index of first info block, -1 if not found
	int getInfoIndex() const { return getFirstIndexByID(ubid_info); }
	// get index of first comment block, -1 if not found
	int getCommentIndex() const { return getFirstIndexByID(ubid_comment); }

	size_t size() const; // 获取所有 User Block 的合计长度，包括头部。以字节为单位
	DWORD count() const;  // 获取 user blocks 的个数

	// 返回 true 继续遍历，否则返回 false
	// typedef bool (*Traverser)(RPYUBID id, const BYTE* pBlock, size_t sz, void* pCustom);
	
	// int traverse(Traverser traverser, void* pCustom); // 返回最后遍历的 index

private:
	RPYUserBlock* newuserblock(int i, const RPYUBHeader* pHeader);
	RPYUserBlock* m_pUserBlocks[MAXUSERBLOCKCOUNT];
};

#endif /* __THUSERBLOCK_H__537E773A_ */
