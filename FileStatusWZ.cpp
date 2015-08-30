// written by wz520. need MFC

#include "stdafx.h"
#include "FileStatusWZ.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CFileStatusWZ::CFileStatusWZ()
{
	Clear();
}

void CFileStatusWZ::Update(const CFileStatus& other)
{
	ASSERT(sizeof(CFileStatus) == sizeof(*this));

	memcpy(this, &other, sizeof(*this));
}

void CFileStatusWZ::Clear()
{
	memset(this, 0, sizeof(*this));
}

BOOL CFileStatusWZ::IsSameDateAndSize(const CFileStatus& other) const
{
	return (this->m_mtime == other.m_mtime && this->m_size == other.m_size) ? TRUE : FALSE;
}