#include <stdlib.h>
#include <string.h>
#include "thuserblock.h"

#define USERBLOCKMARKER	((DWORD)'RESU')
#define SIZEOF_UBHEADER (sizeof(RPYUBHeader))

RPYUserBlock::RPYUserBlock(const BYTE* pData, size_t datasize, RPYUBID id)
{
	init(pData, datasize, id);
}

RPYUserBlock::RPYUserBlock(const RPYUBHeader* pHeader)
{
	if ( pHeader->marker == USERBLOCKMARKER ) {
		init( (const BYTE*)(pHeader) + SIZEOF_UBHEADER,
				pHeader->length - SIZEOF_UBHEADER, pHeader->id);
	}
	else {
		m_pHeader->marker = 0;
		m_pHeader->length = 0;
		m_pHeader->id = ubid_info;
	}
}

RPYUserBlock::~RPYUserBlock()
{
	free(m_pHeader);
}

void RPYUserBlock::init(const BYTE* pData, size_t datasize, RPYUBID id)
{
	const DWORD blocksize = SIZEOF_UBHEADER + datasize;
	
	// Allocate additional 1 byte.
	// We will fill this byte with '\0' to ensure that the block data
	//   can be safely copied by strcpy()
	m_pHeader = (RPYUBHeader*)malloc(blocksize+1);
	m_pHeader->marker = USERBLOCKMARKER;
	m_pHeader->length = blocksize;
	m_pHeader->id = id;

	BYTE* const pHeaderAsByte = (BYTE*)m_pHeader;
	memcpy(pHeaderAsByte + SIZEOF_UBHEADER, pData, datasize);
	pHeaderAsByte[blocksize] = '\0'; // fill '\0', as mentioned above
}


/////////////////////////////////////////////////////////////////


// Get pointer to first user block from rpy data
static const BYTE* getBegin(const BYTE* pData, size_t sz)
{
	const DWORD offset = *(DWORD*)(pData + 0x0C); // offset of the first user block
	const BYTE* pBegin = NULL;
	if ( offset <= (DWORD)sz ) {
		pBegin = pData + offset;
		
		if ( offset == (DWORD)sz )
			; // offset == sz, no user blocks, permitted.
		else if ( ((RPYUBHeader*)pBegin)->marker != USERBLOCKMARKER ) {
			pBegin = NULL; // offset != sz, there must be at least 1 user block.
		}
	}
	return pBegin;
}

RPYUserBlock* RPYUserBlockMgr::newuserblock(int i, const RPYUBHeader* pHeader)
{
	m_pUserBlocks[i] = new RPYUserBlock(pHeader);

	if ( !m_pUserBlocks[i]->isValid() ) {  // not a valid user block
		delete m_pUserBlocks[i];
		m_pUserBlocks[i] = NULL;
	}

	return m_pUserBlocks[i];
}

RPYUserBlockMgr::RPYUserBlockMgr()
{
	for (int i = 0; i < MAXUSERBLOCKCOUNT; ++i ) {
		m_pUserBlocks[i] = NULL;
	}
}

RPYUserBlockMgr::RPYUserBlockMgr(const BYTE* const pRPYData, size_t sz)
{
	const BYTE* const pEnd = pRPYData + sz;
	const BYTE* pCurrBlock = getBegin(pRPYData, sz);
	if ( pCurrBlock == NULL ) {
		// this lets all user block pointers will be initialized to NULL
		pCurrBlock = pEnd;
	}

	for (int i = 0; i < MAXUSERBLOCKCOUNT; ++i ) {
		RPYUBHeader* pHeader = (RPYUBHeader*)pCurrBlock;
		if ( pCurrBlock < pEnd ) {
			if ( !newuserblock(i, pHeader) )	{  // not a valid user block
				pCurrBlock = pEnd; // to prevent from constructing more RPYUserBlock objects
				continue;
			}

			// move pointer to the next header
			pCurrBlock += pHeader->length;
		}
		else
			m_pUserBlocks[i] = NULL;
	}
}

RPYUserBlockMgr::RPYUserBlockMgr(const RPYUserBlockMgr &other)
{
	for (int i = 0; i < MAXUSERBLOCKCOUNT; ++i ) {
		this->m_pUserBlocks[i] = NULL;
		
		const RPYUserBlock* const ub = other.get(i);
		if (ub != NULL)
			this->set( i, ub );
	}
}

RPYUserBlockMgr::~RPYUserBlockMgr()
{
	for (int i = 0; i < MAXUSERBLOCKCOUNT; ++i ) {
		delete m_pUserBlocks[i];
		m_pUserBlocks[i] = NULL;
	}
}

DWORD RPYUserBlockMgr::count() const
{
	int i;
	for (i = 0; i < MAXUSERBLOCKCOUNT; ++i ) {
		if ( m_pUserBlocks[i] == NULL )
			break;
	}
	return i;
}

bool RPYUserBlockMgr::append(const RPYUserBlock* pBlock)
{
	const DWORD i = count();
	if ( i >= MAXUSERBLOCKCOUNT ) return false;

	return newuserblock(i, pBlock->getHeader()) != NULL;
}

bool RPYUserBlockMgr::remove(int index)
{
	const DWORD i = (DWORD)index;
	if ( i >= MAXUSERBLOCKCOUNT ) return false;

	delete m_pUserBlocks[i];
	m_pUserBlocks[i] = NULL;

	return true;
}

const RPYUserBlock* RPYUserBlockMgr::get(int index) const
{
	const DWORD i = (DWORD)index;
	if ( i >= MAXUSERBLOCKCOUNT || m_pUserBlocks[i] == NULL ) return NULL;

	return m_pUserBlocks[i];
}

bool RPYUserBlockMgr::set(int index, const RPYUserBlock* pBlock)
{
	const DWORD i = (DWORD)index;
	if ( i >= MAXUSERBLOCKCOUNT) return false;

	if ( m_pUserBlocks[i] != NULL ) {
		remove(index);
	}

	return newuserblock(i, pBlock->getHeader()) != NULL;
}

const RPYUserBlock* RPYUserBlockMgr::getFirstBlockByID(RPYUBID id) const
{
	const RPYUserBlock* pBlock;
	for ( int i = 0; i < MAXUSERBLOCKCOUNT; ++i ) {
		pBlock = this->get(i);
		if ( pBlock && pBlock->getHeader()->id == id ) {
			return pBlock;
		}
	}
	return NULL;
}




int RPYUserBlockMgr::getFirstIndexByID(RPYUBID id) const
{
	const RPYUserBlock* pBlock;
	for ( int i = 0; i < MAXUSERBLOCKCOUNT; ++i ) {
		pBlock = this->get(i);
		if ( pBlock && pBlock->getHeader()->id == id ) {
			return i;
		}
	}
	return -1;
}



size_t RPYUserBlockMgr::size() const
{
	DWORD allsize = 0;
	const RPYUserBlock* pBlock;
	for ( int i = 0; i < MAXUSERBLOCKCOUNT; ++i ) {
		pBlock = this->get(i);
		if ( pBlock ) {
			allsize += pBlock->getHeader()->length;
		}
	}

	return allsize;
}

BYTE* RPYUserBlockMgr::saveToRPY(const BYTE* pRPYData, size_t sz, size_t* pNewSize)
{
	const BYTE* pCurrBlockRaw = getBegin(pRPYData, sz);
	if (pCurrBlockRaw == NULL)
		return NULL;

	const size_t szWithoutUB = (size_t)(pCurrBlockRaw - pRPYData);
	*pNewSize = szWithoutUB + this->size();
	BYTE* pNewRPYData = (BYTE*)malloc(*pNewSize);

	// copy all data before user blocks
	memcpy(pNewRPYData, pRPYData, szWithoutUB);

	BYTE* pCurrOutBlockRaw = pNewRPYData + szWithoutUB;
	for ( int i = 0; i < MAXUSERBLOCKCOUNT; ++i ) {
		const RPYUserBlock* pCurrBlock = this->get(i);
		if (pCurrBlock != NULL) {
			const DWORD length = pCurrBlock->getHeader()->length;
			memcpy(pCurrOutBlockRaw, pCurrBlock->getRaw(), length);
			// move the pointer to the next position where the next user block would be stored
			pCurrOutBlockRaw += length;
		}
	}

	return pNewRPYData;
}
