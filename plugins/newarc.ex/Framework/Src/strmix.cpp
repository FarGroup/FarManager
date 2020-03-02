#include "strmix.hpp"

void CutTo(string& str, TCHAR symbol, bool bInclude)
{
	TCHAR *pBuffer = str.GetBuffer();

	CutTo(pBuffer, symbol, bInclude);

	str.ReleaseBuffer();
}

void CutToSlash(string &str, bool bInclude)
{
	CutTo(str, _T('\\'), bInclude);
}

void AddEndSlash(string& str)
{
	if ( !str.IsEmpty() && (str[str.GetLength()-1] != _T('\\')) )
		str += _T('\\');
}

void ConvertSlashes(string& strStr)
{
	TCHAR* pBuffer = strStr.GetBuffer();

	while ( *pBuffer )
	{
		if ( *pBuffer == _T('/') )
			*pBuffer = _T('\\');

		pBuffer++;
	}

	strStr.ReleaseBuffer();
}


void Quote(string& strStr)
{
	string strResult;

	if ( strStr.At(0) != _T('\"') )
	{
		strResult += _T('\"');
		strResult += strStr;
	}

	if ( strResult.At(strResult.GetLength()) != _T('\"') )
		strResult += _T('\"');

	strStr = strResult;
}