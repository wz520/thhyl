// SaveRawDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "thhyl.h"
#include "thcommon.h"
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
	
	// 初始化与控件关联的变量
	m_chkExit      = HasConfigOption(CFG_SAVERAW_AUTOEXIT);
	m_chkOverwrite = HasConfigOption(CFG_SAVERAW_OVERWRITE);
	m_chkInv       = HasConfigOption(CFG_SAVERAW_INV);
	UpdateData(FALSE);

	m_tooltip.AddTool( GetDlgItem(IDOK), _T("将勾选的原始数据保存到指定的文件夹中") );
	m_tooltip.AddTool( GetDlgItem(IDC_SAVERAW_INV), _T("将右边两个按钮的效果倒置：勾选变未选，未选变勾选") );
	m_tooltip.AddTool( GetDlgItem(IDC_SAVERAW_SELALL), _T("若勾选“反相”，则将列表中所有项目的勾选状态倒置") );
	m_tooltip.AddTool( GetDlgItem(IDC_SAVERAW_SELHI), _T("若勾选“反相”，则将列表中所有高亮项目的勾选状态倒置") );

	CListCtrl* const pList = &m_list;
	pList->SetExtendedStyle(LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES);

	pList->InsertColumn(0, _T("文件名")  , LVCFMT_LEFT, 120);        //添加列标题
	pList->InsertColumn(1, _T("关卡")    , LVCFMT_LEFT, 70);
	pList->InsertColumn(2, _T("类型")    , LVCFMT_LEFT, 170);
	pList->InsertColumn(3, _T("起始偏移"), LVCFMT_LEFT, 100);
	pList->InsertColumn(4, _T("长度")    , LVCFMT_LEFT, 120);
	pList->InsertColumn(5, _T("保存结果"), LVCFMT_LEFT, 120);

	// 插入要导出的项目
	// 当 raw 文件大小为 0 时，有可能意味着是绀珠传体验版录像，目前无法导出（硬要导出也没用，全是 0）
	
	if (m_pRpyInfoPointers->dwDecompDataSize > 0) {
		int nListIndex = 0;
		CString fmt;
		CString stagename;
		CString filename, filename2;
		LPCTSTR const szWaiting = _T("少女发呆中 O_O ...");
		for (int player=0; player<2; player++) {
			for (int i=0; i<THRPYINFO_MAXSTAGES; i++) {
				const TH_STAGEPOINTER* const pCurr = &m_pRpyInfoBase->stagepointers[player][i];
				if ( pCurr->p != NULL ) {
					// 设置关卡名
					if (m_pRpyInfoBase->nStageNumberForHalf >= 0) {
						 // 95,125,143 使用 nStageNumberForHalf
						stagename.Format( _T("%d"), m_pRpyInfoBase->nStageNumberForHalf );
					}
					else {
						stagename.Format( _T("%s"), m_pRpyInfoBase->stagenames[i] );
					}

					// 文件名
					filepath_utils::GetBasename(m_strFileName, filename);
					filename2.Format(_T("%s_%s%s.bin"), (LPCTSTR)filename, (LPCTSTR)stagename,
						player == 0 ? _T("") : _T("(P2)")
						);
					pList->InsertItem(nListIndex, filename2);
					// 关卡
					pList->SetItemText(nListIndex, 1, stagename);
					// 类型
					pList->SetItemText(nListIndex, 2, player == 0 ? _T("关卡数据") : _T("关卡数据(P2)"));
					// 起始偏移
					fmt.Format( _T("%d(0x%X)"), pCurr->offset, pCurr->offset );
					pList->SetItemText(nListIndex, 3, fmt);
					// 长度
					fmt.Format( _T("%d(0x%X)"), pCurr->size, pCurr->size );
					pList->SetItemText(nListIndex, 4, fmt);
					// 保存结果
					pList->SetItemText(nListIndex, 5, szWaiting);
					nListIndex++;

					// --- FPS数据
					// 当 player 不是 0 时，因为已经显示过了，所以不用再显示了
					// 另外 TH6 没有 FPS 数据，也不需要（不能）显示
					if (player == 0 && m_pRpyInfoBase->fpspointers[i].p != NULL) {
						// 文件名
						filepath_utils::GetBasename(m_strFileName, filename);
						filename2.Format(_T("%s_%s_FPS.bin"), (LPCTSTR)filename, (LPCTSTR)stagename);
						pList->InsertItem(nListIndex, filename2);
						// 关卡
						pList->SetItemText(nListIndex, 1, stagename);
						// 类型
						pList->SetItemText(nListIndex, 2, _T("FPS数据"));
						// 起始偏移
						fmt.Format( _T("%d(0x%X)"), m_pRpyInfoBase->fpspointers[i].offset, m_pRpyInfoBase->fpspointers[i].offset );
						pList->SetItemText(nListIndex, 3, fmt);
						// 长度
						fmt.Format( _T("%d(0x%X)"), m_pRpyInfoBase->fpspointers[i].size, m_pRpyInfoBase->fpspointers[i].size );
						pList->SetItemText(nListIndex, 4, fmt);
						// 保存结果
						pList->SetItemText(nListIndex, 5, szWaiting);
						nListIndex++;
					}
				}
			}
		}
	}

	// 显示文件大小
	{
		CStatic* const pStatic = (CStatic*)(GetDlgItem(IDC_SAVERAWSIZE));
		CString fmt;
		fmt.Format(
				_T("原始数据大小 = %d(0x%X)\r\n")
				_T("录像文件大小 = %d(0x%X)"),
			m_pRpyInfoPointers->dwDecompDataSize, m_pRpyInfoPointers->dwDecompDataSize,
			m_dwRpySize, m_dwRpySize
		);
		pStatic->SetWindowText(fmt);
	}

	OnSaverawSelall(); // 全选
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
		MessageBox(_T("没有高亮的项目 poi"));
	else {
		while (pos) {
			int nItem = m_list.GetNextSelectedItem(pos);
			m_list.SetCheck( nItem, m_chkInv ? !m_list.GetCheck(nItem) : TRUE );
		}
	}
}


//选择文件夹对话框回调函数
int CALLBACK BrowseCallBackFun(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    switch(uMsg)
    {
    case BFFM_INITIALIZED:  //选择文件夹对话框初始化
		::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
		::SendMessage(hwnd, BFFM_SETOKTEXT, 0, (LPARAM)(_T("保存(&S)")));
        break;
		/*
    case BFFM_SELCHANGED:   //选择文件夹变更时
        {
            TCHAR pszPath[MAX_PATH];
            //获取当前选择路径
            SHGetPathFromIDList((LPCITEMIDLIST)lParam, pszPath);
            //在STATUSTEXT区域显示当前路径
            ::SendMessage(hwnd, BFFM_SETSTATUSTEXT, 0, (LPARAM)pszPath);
        }
        break;
		*/
    }
    return 0;
}

void CSaveRawDlg::OnOK()
{
	TCHAR pszPath[MAX_PATH];
	CString initpath;
	filepath_utils::GetDir(m_strFileName, initpath, false);

	BROWSEINFO bi;
	bi.hwndOwner      = this->GetSafeHwnd();
	bi.pidlRoot       = NULL;
	bi.pszDisplayName = NULL;
	bi.lpszTitle      = TEXT("(=∩ω∩=) 请选择用于保存选中的文件的文件夹喵～");
	bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT | BIF_USENEWUI | BIF_UAHINT;
	bi.lpfn           = BrowseCallBackFun;
	bi.lParam         = (LPARAM)((LPCTSTR)initpath);
	bi.iImage         = 0;
	
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	bool bSuccess = false;
	if (pidl != NULL) {
		if (SHGetPathFromIDList(pidl, pszPath)) {
			bSuccess = SaveCheckedFiles(pszPath);
		}
	}

	CoTaskMemFree(pidl);

	if (m_chkExit && bSuccess) {
		CDlgBaseWZ::OnOK();
	}
}

// 只要有一个文件保存失败则返回 false
bool CSaveRawDlg::SaveCheckedFiles(LPCTSTR szDir)
{
	CString strDir(szDir);
	filepath_utils::AddDirBkSlash(strDir);
	
	const int itemcount = m_list.GetItemCount();
	bool      result    = true;
	
	for (int i=0; i<itemcount; ++i) {
		if (!m_list.GetCheck(i))
			continue;

		// 准备文件名
		CString strFilename( strDir + m_list.GetItemText(i, 0) );
		// 数据指针和长度
		const int offset        = _ttoi( m_list.GetItemText(i, 3) );
		const BYTE* const pData = m_pRpyInfoPointers->pDecompData + offset;
		const int dwDataSize    = _ttoi( m_list.GetItemText(i, 4) );
		ASSERT(pData);
		ASSERT(offset > 0);
		ASSERT(dwDataSize > 0);

		// write
		{
			HANDLE const hFile = CreateFile(strFilename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL,
				m_chkOverwrite ? CREATE_ALWAYS : CREATE_NEW, // 如果没有勾选“覆盖已存在的文件”，确保只打开不存在的文件
				FILE_ATTRIBUTE_NORMAL, NULL);

			try {
				if (hFile == INVALID_HANDLE_VALUE)
					throw GetLastError();

				DWORD written;
				if (!WriteFile(hFile, pData, dwDataSize, &written, NULL)) {
					throw GetLastError();
				}

				m_list.SetItemText(i, 5, _T("成功喵～"));
			}
			catch (DWORD oserror) {
				CString errmsg;
				errmsg.Format(_T("失败：%s"), ::GetErrorMessage(oserror));
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
