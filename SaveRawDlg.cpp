// SaveRawDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "thhyl.h"
#include "SaveRawDlg.h"
#include "global.h"
#include "cfgfile.h"

#include "filepath_utils.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSaveRawDlg dialog


CSaveRawDlg::CSaveRawDlg(CWnd* pParent /*=NULL*/)
	: CDlgBaseWZ(CSaveRawDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSaveRawDlg)
	m_chkExit = FALSE;
	m_chkOverwrite = FALSE;
	m_chkInv = FALSE;
	//}}AFX_DATA_INIT
	m_dwRpySize = 0;
	m_pRpyInfoPointers = NULL;
	m_pRpyInfoBase = NULL;
}


void CSaveRawDlg::DoDataExchange(CDataExchange* pDX)
{
	CDlgBaseWZ::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSaveRawDlg)
	DDX_Control(pDX, IDC_SAVERAW_LIST, m_list);
	DDX_Check(pDX, IDC_SAVERAW_CHKEXIT, m_chkExit);
	DDX_Check(pDX, IDC_SAVERAW_OVERWRITE, m_chkOverwrite);
	DDX_Check(pDX, IDC_SAVERAW_INV, m_chkInv);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSaveRawDlg, CDlgBaseWZ)
	//{{AFX_MSG_MAP(CSaveRawDlg)
	ON_BN_CLICKED(IDC_SAVERAW_SELALL, OnSaverawSelall)
	ON_BN_CLICKED(IDC_SAVERAW_SELHI, OnSaverawSelhi)
	ON_BN_CLICKED(IDC_SAVERAW_CHKEXIT, OnSaverawChkexit)
	ON_BN_CLICKED(IDC_SAVERAW_OVERWRITE, OnSaverawChkexit)
	ON_BN_CLICKED(IDC_SAVERAW_INV, OnSaverawChkexit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSaveRawDlg message handlers

BOOL CSaveRawDlg::OnInitDialog() 
{
	CDlgBaseWZ::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	ASSERT(m_dwRpySize && m_pRpyInfoPointers && m_pRpyInfoBase);
	
	// ��ʼ����ؼ������ı���
	m_chkExit      = HasConfigOption(CFG_SAVERAW_AUTOEXIT);
	m_chkOverwrite = HasConfigOption(CFG_SAVERAW_OVERWRITE);
	m_chkInv       = HasConfigOption(CFG_SAVERAW_INV);
	UpdateData(FALSE);

	m_tooltip.AddTool( GetDlgItem(IDOK), _T("����ѡ��ԭʼ���ݱ��浽ָ�����ļ�����") );
	m_tooltip.AddTool( GetDlgItem(IDC_SAVERAW_INV), _T("���ұ�������ť��Ч�����ã���ѡ��δѡ��δѡ�乴ѡ") );
	m_tooltip.AddTool( GetDlgItem(IDC_SAVERAW_SELALL), _T("����ѡ�����ࡱ�����б���������Ŀ�Ĺ�ѡ״̬����") );
	m_tooltip.AddTool( GetDlgItem(IDC_SAVERAW_SELHI), _T("����ѡ�����ࡱ�����б������и�����Ŀ�Ĺ�ѡ״̬����") );

	CListCtrl* const pList = &m_list;
	pList->SetExtendedStyle(LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES);

	pList->InsertColumn(0, _T("�ļ���")  , LVCFMT_LEFT, 120);        //����б���
	pList->InsertColumn(1, _T("�ؿ�")    , LVCFMT_LEFT, 70);
	pList->InsertColumn(2, _T("����")    , LVCFMT_LEFT, 170);
	pList->InsertColumn(3, _T("��ʼƫ��"), LVCFMT_LEFT, 100);
	pList->InsertColumn(4, _T("����")    , LVCFMT_LEFT, 120);
	pList->InsertColumn(5, _T("������"), LVCFMT_LEFT, 120);

	// ����Ҫ��������Ŀ
	// �� raw �ļ���СΪ 0 ʱ���п�����ζ������鴫�����¼��Ŀǰ�޷�������ӲҪ����Ҳû�ã�ȫ�� 0��
	
	if (m_pRpyInfoPointers->dwDecompDataSize > 0) {
		int nListIndex = 0;
		CString fmt;
		CString stagename;
		CString filename, filename2;
		LPCTSTR const szWaiting = _T("��Ů������ O_O ...");
		for (int player=0; player<2; player++) {
			for (int i=0; i<THRPYINFO_MAXSTAGES; i++) {
				const TH_STAGEPOINTER* const pCurr = &m_pRpyInfoBase->stagepointers[player][i];
				if ( pCurr->p != NULL ) {
					// ���ùؿ���
					if (m_pRpyInfoBase->nStageNumberForHalf >= 0) {
						 // 95,125,143 ʹ�� nStageNumberForHalf
						stagename.Format( _T("%d"), m_pRpyInfoBase->nStageNumberForHalf );
					}
					else {
						stagename.Format( _T("%s"), m_pRpyInfoBase->stagenames[i] );
					}

					// �ļ���
					filepath_utils::GetBasename(m_strFileName, filename);
					filename2.Format(_T("%s_%s%s.bin"), (LPCTSTR)filename, (LPCTSTR)stagename,
						player == 0 ? _T("") : _T("(P2)")
						);
					pList->InsertItem(nListIndex, filename2);
					// �ؿ�
					pList->SetItemText(nListIndex, 1, stagename);
					// ����
					pList->SetItemText(nListIndex, 2, player == 0 ? _T("�ؿ�����") : _T("�ؿ�����(P2)"));
					// ��ʼƫ��
					fmt.Format( _T("%d(0x%X)"), pCurr->offset, pCurr->offset );
					pList->SetItemText(nListIndex, 3, fmt);
					// ����
					fmt.Format( _T("%d(0x%X)"), pCurr->size, pCurr->size );
					pList->SetItemText(nListIndex, 4, fmt);
					// ������
					pList->SetItemText(nListIndex, 5, szWaiting);
					nListIndex++;

					// --- FPS����
					// �� player ���� 0 ʱ����Ϊ�Ѿ���ʾ���ˣ����Բ�������ʾ��
					// ���� TH6 û�� FPS ���ݣ�Ҳ����Ҫ�����ܣ���ʾ
					if (player == 0 && m_pRpyInfoBase->fpspointers[i].p != NULL) {
						// �ļ���
						filepath_utils::GetBasename(m_strFileName, filename);
						filename2.Format(_T("%s_%s_FPS.bin"), (LPCTSTR)filename, (LPCTSTR)stagename);
						pList->InsertItem(nListIndex, filename2);
						// �ؿ�
						pList->SetItemText(nListIndex, 1, stagename);
						// ����
						pList->SetItemText(nListIndex, 2, _T("FPS����"));
						// ��ʼƫ��
						fmt.Format( _T("%d(0x%X)"), m_pRpyInfoBase->fpspointers[i].offset, m_pRpyInfoBase->fpspointers[i].offset );
						pList->SetItemText(nListIndex, 3, fmt);
						// ����
						fmt.Format( _T("%d(0x%X)"), m_pRpyInfoBase->fpspointers[i].size, m_pRpyInfoBase->fpspointers[i].size );
						pList->SetItemText(nListIndex, 4, fmt);
						// ������
						pList->SetItemText(nListIndex, 5, szWaiting);
						nListIndex++;
					}
				}
			}
		}
	}

	// ��ʾ�ļ���С
	{
		CStatic* const pStatic = (CStatic*)(GetDlgItem(IDC_SAVERAWSIZE));
		CString fmt;
		fmt.Format(
				_T("ԭʼ���ݴ�С = %d(0x%X)\r\n")
				_T("¼���ļ���С = %d(0x%X)"),
			m_pRpyInfoPointers->dwDecompDataSize, m_pRpyInfoPointers->dwDecompDataSize,
			m_dwRpySize, m_dwRpySize
		);
		pStatic->SetWindowText(fmt);
	}

	OnSaverawSelall(); // ȫѡ
	AutoSetColumnWidth();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSaveRawDlg::OnSaverawSelall() 
{
	// TODO: Add your control notification handler code here
	const int count = m_list.GetItemCount();

	for (int i=0; i<count; ++i) {
		m_list.SetCheck( i, m_chkInv ? !m_list.GetCheck(i) : TRUE );
	}
}

void CSaveRawDlg::OnSaverawSelhi() 
{
	// TODO: Add your control notification handler code here
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	
	if (pos == NULL)
		MessageBox(_T("û�и�������Ŀ poi"));
	else {
		while (pos) {
			int nItem = m_list.GetNextSelectedItem(pos);
			m_list.SetCheck( nItem, m_chkInv ? !m_list.GetCheck(nItem) : TRUE );
		}
	}
}


//ѡ���ļ��жԻ���ص�����
int CALLBACK BrowseCallBackFun(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    switch(uMsg)
    {
    case BFFM_INITIALIZED:  //ѡ���ļ��жԻ����ʼ��
		::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
		::SendMessage(hwnd, BFFM_SETOKTEXT, 0, (LPARAM)(_T("����(&S)")));
        break;
		/*
    case BFFM_SELCHANGED:   //ѡ���ļ��б��ʱ
        {
            TCHAR pszPath[MAX_PATH];
            //��ȡ��ǰѡ��·��
            SHGetPathFromIDList((LPCITEMIDLIST)lParam, pszPath);
            //��STATUSTEXT������ʾ��ǰ·��
            ::SendMessage(hwnd, BFFM_SETSTATUSTEXT, 0, (LPARAM)pszPath);
        }
        break;
		*/
    }
    return 0;
}

void CSaveRawDlg::OnOK()
{
	CString initpath;
	filepath_utils::GetDir(m_strFileName, initpath, false);

	BROWSEINFO bi;
	bi.hwndOwner      = this->GetSafeHwnd();
	bi.pidlRoot       = NULL;
	bi.pszDisplayName = NULL;
	bi.lpszTitle      = TEXT("(=�ɦء�=) ��ѡ�����ڱ���ѡ�е��ļ����ļ�������");
	bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT | BIF_USENEWUI | BIF_UAHINT;
	bi.lpfn           = BrowseCallBackFun;
	bi.lParam         = (LPARAM)((LPCTSTR)initpath);
	bi.iImage         = 0;
	
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	bool bSuccess = false;
	if (pidl != NULL) {
		TCHAR pszPath[MAX_PATH];
		if (SHGetPathFromIDList(pidl, pszPath)) {
			bSuccess = SaveCheckedFiles(pszPath);
		}
	}

	CoTaskMemFree(pidl);

	if (m_chkExit && bSuccess) {
		CDlgBaseWZ::OnOK();
	}
}

// ֻҪ��һ���ļ�����ʧ���򷵻� false
bool CSaveRawDlg::SaveCheckedFiles(LPCTSTR szDir)
{
	CString strDir(szDir);
	filepath_utils::AddDirBkSlash(strDir);
	
	const int itemcount = m_list.GetItemCount();
	bool      result    = true;
	
	for (int i=0; i<itemcount; ++i) {
		if (!m_list.GetCheck(i))
			continue;

		// ׼���ļ���
		CString strFilename( strDir + m_list.GetItemText(i, 0) );
		// ����ָ��ͳ���
		const int offset        = _ttoi( m_list.GetItemText(i, 3) );
		const BYTE* const pData = m_pRpyInfoPointers->pDecompData + offset;
		const int dwDataSize    = _ttoi( m_list.GetItemText(i, 4) );
		ASSERT(pData);
		ASSERT(offset > 0);
		ASSERT(dwDataSize > 0);

		// write
		{
			HANDLE const hFile = CreateFile(strFilename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL,
				m_chkOverwrite ? CREATE_ALWAYS : CREATE_NEW, // ���û�й�ѡ�������Ѵ��ڵ��ļ�����ȷ��ֻ�򿪲����ڵ��ļ�
				FILE_ATTRIBUTE_NORMAL, NULL);

			try {
				if (hFile == INVALID_HANDLE_VALUE)
					throw GetLastError();

				DWORD written;
				if (!WriteFile(hFile, pData, dwDataSize, &written, NULL)) {
					throw GetLastError();
				}

				m_list.SetItemText(i, 5, _T("�ɹ�����"));
			}
			catch (DWORD oserror) {
				CString errmsg;
				errmsg.Format(_T("ʧ�ܣ�%s"), ::GetErrorMessage(oserror));
				m_list.SetItemText(i, 5, errmsg);

				result = false;
			}

			if (hFile != INVALID_HANDLE_VALUE)
				CloseHandle(hFile);
		}
	}

	return result;
}


BOOL CSaveRawDlg::DestroyWindow() 
{
	// TODO: Add your specialized code here and/or call the base class
	
	cfg.set(CFG_SAVERAW_AUTOEXIT , m_chkExit);
	cfg.set(CFG_SAVERAW_OVERWRITE, m_chkOverwrite);
	cfg.set(CFG_SAVERAW_INV      , m_chkInv);
	return CDlgBaseWZ::DestroyWindow();
}


// also include other checks
void CSaveRawDlg::OnSaverawChkexit() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CSaveRawDlg::AutoSetColumnWidth()
{
	for (int i = 0; i < 6 ; i++) {
		m_list.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
	}
}
