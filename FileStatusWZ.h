#ifndef __CFILESTATUSWZ_H_51A572A9_
#     define __CFILESTATUSWZ_H_51A572A9_

// written by wz520. need MFC

struct CFileStatusWZ : public CFileStatus
{
	CFileStatusWZ();

	void Update(const CFileStatus& other);
	BOOL IsValid() const { return (BOOL)(this->m_szFullName[0] != 0); }
	void Clear();
	BOOL IsSameDateAndSize(const CFileStatus& other) const;
};

#endif // __CFILESTATUSWZ_H_51A572A9_
