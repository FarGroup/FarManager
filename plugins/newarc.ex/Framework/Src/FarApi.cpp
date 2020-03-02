#include "FarPluginBase.hpp"
#include "SystemApi.hpp"
#include "strmix.hpp"

DWORD farGetFullPathName(const TCHAR* lpFileName, string& strResult)
{
	string strFileName = lpFileName;

#ifdef UNICODE
	DWORD dwSize = FSF.ConvertPath(CPM_FULL, strFileName, NULL, 0);

	TCHAR* pBuffer = strResult.GetBuffer(dwSize+1);

	FSF.ConvertPath(CPM_FULL, strFileName, pBuffer, dwSize);

	strResult.ReleaseBuffer();

	return dwSize;
#else
	return apiGetFullPathName(strFileName, strResult);
#endif		
}

void farQuoteSpaceOnly(string& strStr)
{
	TCHAR* pBuffer = strStr.GetBuffer(strStr.GetLength()+2);

	FSF.QuoteSpaceOnly(pBuffer);

	strStr.ReleaseBuffer();
}

void farUnquote(string& strStr)
{
	TCHAR* pBuffer = strStr.GetBuffer();

	FSF.Unquote(pBuffer);

	strStr.ReleaseBuffer();
}

void farTrim(string& strStr)
{
	TCHAR* pBuffer = strStr.GetBuffer();

	FSF.Trim(pBuffer);

	strStr.ReleaseBuffer();
}


void farPrepareFileName(string& strFileName)
{
	apiExpandEnvironmentStrings(strFileName, strFileName);

	farUnquote(strFileName);
	farTrim(strFileName);

	if ( strFileName.IsEmpty() )
		strFileName = _T(".");

	farGetFullPathName(strFileName, strFileName);
}

void farTruncPathStr(string& strFileName, int nLength)
{
	TCHAR* pBuffer = strFileName.GetBuffer();

	FSF.TruncPathStr(pBuffer, nLength);

	strFileName.ReleaseBuffer();
}
