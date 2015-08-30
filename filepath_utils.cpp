#include "StdAfx.h"
#include "filepath_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace filepath_utils {

// add/change the file extension part of a path
CString& ChangeFileExtension(CString& inoutFilePath, LPCTSTR Extension)
{
	const int pos_dot       = inoutFilePath.ReverseFind(_T('.'));
	const int pos_backslash = inoutFilePath.ReverseFind(_T('\\'));

	if (pos_dot != -1 && pos_backslash < pos_dot) // Found
		inoutFilePath = inoutFilePath.Left(pos_dot+1) + Extension;
	else
		inoutFilePath += CString(_T('.')) + Extension;

	return inoutFilePath;
}

// add/change the filename part(include ext) of a path
// If inoutFilePath doesn't include '\\',
// inoutFilePath will not be modified
CString& ChangeFilename(CString& inoutFilePath, LPCTSTR filename)
{
	const int pos_backslash = inoutFilePath.ReverseFind(_T('\\'));

	if (pos_backslash != -1) // Found
		inoutFilePath = inoutFilePath.Left(pos_backslash+1) + filename;

	return inoutFilePath;
}

// Get filename part(include ext) from a full path string
// if no filename part(e.g. not a full path),
// 'outFilename' will be set to the same string as 'inFilePath'
CString& GetFilename(const CString& inFilePath, CString& outFilename)
{
	const int pos_backslash = inFilePath.ReverseFind(_T('\\'));

	if (pos_backslash != -1) // Found
		outFilename = inFilePath.Right(inFilePath.GetLength()-(pos_backslash+1));
	else
		outFilename = inFilePath;

	return outFilename;
}

// Get Dir part from a full path string
// if no filename part(e.g. not a full path),
// 'outFilename' will be set to the same string as 'inFilePath'
CString& GetDirpart(const CString& inFilePath, CString& outFilename, bool bIncludeLastBackSlash)
{
	const int pos_backslash = inFilePath.ReverseFind(_T('\\'));
	
	if (pos_backslash != -1) // Found
		outFilename = inFilePath.Left( bIncludeLastBackSlash ? pos_backslash+1 : pos_backslash );
	else
		outFilename = inFilePath;
	
	return outFilename;
}



// Get filename part(WITHOUT ext) from a full path string
// if no filename part(e.g. not a full path),
// 'outFilename' will be set to the same string as 'inFilePath'
CString& GetBasename(const CString& inFilePath, CString& outFilename)
{
	CString filename( GetFilename(inFilePath, outFilename) );
	const int pos_dot = filename.ReverseFind(_T('.'));
	
	if (pos_dot != -1) // Found
		outFilename = filename.Left(pos_dot);
	
	return outFilename;
}

CString& AddDirBkSlash(CString& str)
{
	if (str.Right(1) != _T("\\"))
		str += _T('\\');

	return str;
}

}
