#ifndef REGISTRY_H_THHYL_
#define REGISTRY_H_THHYL_

LONG WriteKey(const void* lpInData,
			  HKEY hKey,
			  LPCTSTR lpSubKey,
			  LPCTSTR lpValue,
			  DWORD dwType=REG_SZ,
			  DWORD cbData=0);

LONG ReadKey(HKEY hKey,
			 LPCTSTR lpSubKey,
			 LPCTSTR lpValue,
			 void* lpOutData);

/***
*DeleteSubKeyTree(HKEY hKey, LPCTSTR lpSubKey)
* 功能: 递归删除注册表子键
* 入口：
*      hKey :  一个HEKY类型的参数，比如HKEY_CLASSES_ROOT
*      lpSubKey:　比如".exe"
* 出口: 成功删除，返回ERROR_SUCCESS,失败则返回一个非0值。
*
* 实例:        LPCTSTR lpSubKey= "shile";
HKEY hKey = HKEY_CLASSES_ROOT;
long ret;
ret = DeleteSubKeyTree(hKey, lpSubKey);
if (ret == ERROR_SUCCESS)
{
SetDlgItemText(IDC_STATIC, "删除子键HKEY_CLASSES_ROOT\\shile成功");
}
*     
**************************************************/
LONG DeleteSubKeyTree(HKEY hKey, LPCTSTR lpSubKey);

LONG SetAssociation(LPCTSTR FileExt,      // 扩展名, 比如 ".foo"
					LPCTSTR FileTypeKey,  // 文件类型键名，随便起一个，不冲突就好，比如 "my_foofile"
					LPCTSTR FileTypeName, // 文件类型名，用于描述该文件类型，在资源管理器里可以看到，比如“我的某某文件类型”
					LPCTSTR ExePath,      // 关联目标，一般是 EXE 文件的完整路径
					LPCTSTR Parameters,   // 运行 EXE 文件所需的参数
					int nDefaultIcon=0,   // 文件类型图标ID，ExePath 里的序号，从 0 开始，
					BOOL bForUser=FALSE   // TRUE: For Current User, FALSE: For All Users
);

LONG DeleteAssociation(LPCTSTR FileExt, BOOL bForUser=FALSE);

#endif // REGISTRY_H_THHYL_
