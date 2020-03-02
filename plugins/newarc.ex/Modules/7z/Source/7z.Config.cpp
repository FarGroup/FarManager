#include "7z.h"

SevenZipCompressionConfig::SevenZipCompressionConfig(const CompressionFormatInfo* pFormat)
{
	Clear();
	m_pFormat = pFormat;
}

void SevenZipCompressionConfig::Clear()
{
	m_bOverride = false;

	m_uLevel = 5;
	m_uMethod = 0;
	m_uDictionarySize = 0;

	m_bFilter = false;
	m_bSolid = false;
	m_bMultithread = false;
	m_bSFX = false;
	m_bEncrypt = false;
	m_bEncryptFileNames = false;

	m_bCompressHeaders = false;
	m_bCompressHeadersFull = false;

	m_bVolumeMode = false;
}


void SevenZipCompressionConfig::ToString(string& strResult)
{
	strResult.Format(
			_T("CompressionFormat:%s|") \
			_T("Level:%u|") \
			_T("Override:%d|") \
			_T("Method:%u|") \
			_T("DictionarySize:%u|") \
			_T("Filter:%d|") \
			_T("Solid:%d|") \
			_T("Multithread:%d|") \
			_T("SFX:%d|") \
			_T("Encrypt:%d|") \
			_T("EncryptFileNames:%d|") \
			_T("CompressHeaders:%d|") \
			_T("CompressHeadersFull:%d|") \
			_T("Volumes:%d")
			, 
			m_pFormat->lpName, 
			m_uLevel, 
			m_bOverride, 
			m_uMethod,
			m_uDictionarySize, 
			m_bFilter, 
			m_bSolid, 
			m_bMultithread, 
			m_bSFX, 
			m_bEncrypt, 
			m_bEncryptFileNames, 
			m_bCompressHeaders, 
			m_bCompressHeadersFull, 
			m_bVolumeMode
			);
}

int StrToInt(const TCHAR* lpParam)
{
	return _tstoi(lpParam);
}

SevenZipCompressionConfig* SevenZipCompressionConfig::FromString(const CompressionFormatInfo* pFormat, const TCHAR* lpConfig)
{
	SevenZipCompressionConfig* pResult = new SevenZipCompressionConfig(pFormat);

	if ( lpConfig )
	{
		PointerArray<TCHAR*> Categories;

		TCHAR* lpParamsCopy = StrDuplicate(lpConfig);
	
		TCHAR* lpCategory = _tcstok(lpParamsCopy, _T("|"));

		while ( lpCategory )
		{
			Categories.add(StrDuplicate(lpCategory));
			lpCategory = _tcstok(nullptr, _T("|"));
		}

		StrFree(lpParamsCopy);

		for (unsigned int i = 0; i < Categories.count(); i++)
		{
			TCHAR* lpCategory = Categories[i];

			TCHAR* lpParam = _tcstok(lpCategory, _T(":"));

			string strName;
			string strValue;

			int nIndex = 0;

			while ( lpParam )
			{
				if ( nIndex == 0 )
					strName = lpParam;

				if ( nIndex == 1 )
					strValue = lpParam;

				if ( nIndex > 1 )
					break;

				nIndex++;

				lpParam = _tcstok(nullptr, _T(":"));
			}

			if ( strName == _T("CompressionFormat") ); //

			if ( strName == _T("Level") )
				pResult->m_uLevel = StrToInt(strValue);

			if ( strName == _T("Override") )
				pResult->m_bOverride = StrToInt(strValue);

			if ( strName == _T("Method") )
				pResult->m_uMethod = StrToInt(strValue);

			if ( strName == _T("DictionarySize") )
				pResult->m_uDictionarySize = StrToInt(strValue);

			if ( strName == _T("Filter") )
				pResult->m_bFilter = StrToInt(strValue);

			if ( strName == _T("Solid") )
				pResult->m_bSolid = StrToInt(strValue);

			if ( strName == _T("Multithread") )
				pResult->m_bMultithread = StrToInt(strValue);

			if ( strName == _T("SFX") )
				pResult->m_bSFX = StrToInt(strValue);

			if ( strName == _T("Encrypt") )
				pResult->m_bEncrypt = StrToInt(strValue);

			if ( strName == _T("EncryptFileNames") )
				pResult->m_bEncryptFileNames = StrToInt(strValue);

			if ( strName == _T("CompressHeaders") )
				pResult->m_bCompressHeaders = StrToInt(strValue);

			if ( strName == _T("CompressHeadersFull") )
				pResult->m_bCompressHeadersFull = StrToInt(strValue);

			if ( strName == _T("Volumes") )
				pResult->m_bVolumeMode = StrToInt(strValue);

		}
	}

	return pResult;
}


const CompressionFormatInfo* SevenZipCompressionConfig::GetFormat()
{
	return m_pFormat;
}

void SevenZipCompressionConfig::SetFormat(CompressionFormatInfo* pFormat)
{
	m_pFormat = pFormat;
}

unsigned int SevenZipCompressionConfig::GetLevel() const
{
	return m_uLevel;
}

void SevenZipCompressionConfig::SetLevel(unsigned int uLevel)
{
	m_uLevel = uLevel;
}

unsigned int SevenZipCompressionConfig::GetMethod() const
{
	return m_uMethod;
}

void SevenZipCompressionConfig::SetMethod(unsigned int uMethod)
{
	m_uMethod = uMethod;
}

unsigned int SevenZipCompressionConfig::GetDictionarySize() const
{
	return m_uDictionarySize;
}

void SevenZipCompressionConfig::SetDictionarySize(unsigned int uDictionarySize)
{
	m_uDictionarySize = uDictionarySize;
}

bool SevenZipCompressionConfig::IsOverride() const
{
	return m_bOverride;
}

void SevenZipCompressionConfig::SetOverride(bool bOverride)
{
	m_bOverride = bOverride;
}

bool SevenZipCompressionConfig::IsFilter() const
{
	return m_bFilter;
}

void SevenZipCompressionConfig::SetFilter(bool bFilter)
{
	m_bFilter = bFilter;
}

bool SevenZipCompressionConfig::IsSolid() const
{
	return m_bSolid;
}

void SevenZipCompressionConfig::SetSolid(bool bSolid)
{
	m_bSolid = bSolid;
}

bool SevenZipCompressionConfig::IsMultithread() const
{
	return m_bMultithread;
}

void SevenZipCompressionConfig::SetMultithread(bool bMultithread)
{
	m_bMultithread = bMultithread;
}

bool SevenZipCompressionConfig::IsSFX() const
{
	return m_bSFX;
}

void SevenZipCompressionConfig::SetSFX(bool bSFX)
{
	m_bSFX = bSFX;
}

bool SevenZipCompressionConfig::IsEncrypt() const
{
	return m_bEncrypt;
}

void SevenZipCompressionConfig::SetEncrypt(bool bEncrypt)
{
	m_bEncrypt = bEncrypt;
}

bool SevenZipCompressionConfig::IsEncryptFileNames() const
{
	return m_bEncryptFileNames;
}

void SevenZipCompressionConfig::SetEncryptFileNames(bool bEncryptFileNames)
{
	m_bEncryptFileNames = bEncryptFileNames;
}

bool SevenZipCompressionConfig::IsCompressHeaders() const
{
	return m_bCompressHeaders;
}

void SevenZipCompressionConfig::SetCompressHeaders(bool bCompressHeaders)
{
	m_bCompressHeaders = bCompressHeaders;
}

bool SevenZipCompressionConfig::IsCompressHeadersFull() const
{
	return m_bCompressHeadersFull;
}

void SevenZipCompressionConfig::SetCompressHeadersFull(bool bCompressHeadersFull)
{
	m_bCompressHeadersFull = bCompressHeadersFull;
}

bool SevenZipCompressionConfig::IsVolumeMode() const
{
	return m_bVolumeMode;
}

void SevenZipCompressionConfig::SetVolumeMode(bool bVolumeMode)
{
	m_bVolumeMode = bVolumeMode;
}


const CompressionFormatInfo* GetCompressionFormatInfo(const GUID& uid)
{
	for (int i = 0; i < countof(CompressionMap); i++)
	{
		if ( CompressionMap[i].uid == uid )
			return CompressionMap[i].Format;
	}

	return NULL;
}

const TCHAR* Modifiers[] = {_T("b"), _T("Kb"), _T("Mb"), _T("Gb"), _T("Tb"), _T("Pb"), _T("Eb")}; //??

void GetSizeString(int nSizeInBytes, string& strResult)
{
	int nModifier = 0;

	while ( nSizeInBytes >= 1024 )
	{
		nSizeInBytes /= 1024;
		nModifier++; 
	}

	TCHAR pBuffer[32];

	_itot(nSizeInBytes, pBuffer, 10);

	strResult = pBuffer;
	strResult += Modifiers[nModifier];
}

void AddDictionarySize(FarDialog* D, int nSize)
{
	SevenZipCompressionConfig* pCfg = (SevenZipCompressionConfig*)D->GetDlgData();

	string strResult;
	GetSizeString(nSize, strResult);

	int index = D->ListAddStr(8, strResult);
	D->ListSetDataEx(8, index, (void*)nSize, sizeof(void*));

	if ( nSize == pCfg->GetDictionarySize() )
	{
		FarListPos pos;

		pos.TopPos = -1;
		pos.SelectPos = index;

		D->ListSetCurrentPos(8, &pos);
	}
}

DWORD GetDefaultDictionarySize(int nMethod, int nLevel)
{
	DWORD dwDefaultSize = 0; 

	switch (nMethod) { 

	case kLZMA:
	case kLZMA2:

		if ( nLevel >= 9 )
			dwDefaultSize = (1 << 26);
		else
		
		if ( nLevel >= 7 )
			dwDefaultSize = (1 << 25);
		else

		if ( nLevel >= 5 )
			dwDefaultSize = (1 << 24);
		else
				
		if ( nLevel >= 3 )
			dwDefaultSize = (1 << 20);
		else
			dwDefaultSize = (1 << 16);

		break;

	case kPPMd:

		if ( nLevel >= 9 )
			dwDefaultSize = (192 << 20);
		else 
			
		if ( nLevel >= 7 ) 
			dwDefaultSize = (64 << 20);
		else 
			
		if ( nLevel >= 5 ) 
			dwDefaultSize = (16 << 20);
		else
			dwDefaultSize = (4 << 20);

		break;

	case kDeflate:
		dwDefaultSize = (32 << 10);
		break;
	
	case kDeflate64:
		dwDefaultSize = (64 << 10);
		break;

	case kBZip2:
		if ( nLevel >= 5 )
			dwDefaultSize = (900 << 10);
		else 
		
		if ( nLevel >= 3 )
			dwDefaultSize = (500 << 10);
		else
			dwDefaultSize = (100 << 10);
	
		break;
	}

	return dwDefaultSize;
}

void UpdateDictionarySize(FarDialog *D, int nMethod)
{
	switch (nMethod) 
	{
		case kLZMA:
		case kLZMA2:
		{
 			AddDictionarySize(D, 1 << 16);

		  	for (int i = 20; i <= 30; i++)
			{
				for (int j = 0; j < 2; j++)
				{
					if (i == 20 && j > 0)
						continue;

					DWORD dictionary = (1 << i) + (j << (i - 1));

					if (dictionary >
#ifdef _WIN64
						(1 << 30)
#else
						(1 << 26)
#endif
						)
						continue;
				
					AddDictionarySize(D, dictionary);
					///check max ram usage as 7z
				}
			}

			break;
		}

		case kPPMd:
		{
			for (int i = 20; i < 31; i++)
			{
				for (int j = 0; j < 2; j++)
				{
					if (i == 20 && j > 0)
						continue;

					DWORD dictionary = (1 << i) + (j << (i - 1));
				
					if (dictionary >= (1 << 31))
						continue;
				
					AddDictionarySize(D, dictionary);
				}
			}

			break;
		}

		case kDeflate:
		{
			AddDictionarySize(D, 32 << 10);
			break;
		}

		case kDeflate64:
		{
			AddDictionarySize(D, 64 << 10);
			break;
		}

		case kBZip2:
		{
			for (int i = 1; i <= 9; i++)
			{
				DWORD dictionary = (i * 100) << 10;
				AddDictionarySize(D, dictionary);
			}
			break;
		}
	}
}

LONG_PTR __stdcall hndConfigureFormat(FarDialog* D, int nMsg, int Param1, LONG_PTR Param2)
{
	SevenZipCompressionConfig* pCfg = (SevenZipCompressionConfig*)D->GetDlgData();

	if ( nMsg == DN_LISTCHANGE )
	{
		if ( Param1 == 6 )
		{
			FarListPos pos;

			D->ListGetCurrentPos(6, &pos);
			int nMethod = (int)D->ListGetData(6, pos.SelectPos);

			D->ListDelete(8, NULL);
			UpdateDictionarySize(D, nMethod); 

			return TRUE;
		}
	}

	if ( nMsg == DN_CLOSE )
	{
		if ( Param1 == D->FirstButton() )
		{
			FarListPos pos;

			D->ListGetCurrentPos(2, &pos);
			pCfg->SetLevel((unsigned int)D->ListGetData(2, pos.SelectPos));

			D->ListGetCurrentPos(6, &pos);
			pCfg->SetMethod((unsigned int)D->ListGetData(6, pos.SelectPos));

			D->ListGetCurrentPos(8, &pos);
			pCfg->SetDictionarySize((unsigned int)D->ListGetData(8, pos.SelectPos));

			return TRUE;
		}
	}

	if ( nMsg == DN_INITDIALOG )
	{
		const CompressionFormatInfo* pFormat = pCfg->GetFormat();

		FarListPos pos;

		pos.TopPos = -1;
		pos.SelectPos = 0;

		for (unsigned int uMethod = 0; uMethod < pFormat->nNumMethods; uMethod++)
		{
			int index = D->ListAddStr(6, MethodNames[pFormat->pMethodIDs[uMethod]]); //check for 7z SFX!!!
			D->ListSetDataEx(6, index, (void*)pFormat->pMethodIDs[uMethod], sizeof(void*));

			if ( uMethod == pCfg->GetMethod()-1 )
				pos.SelectPos = index;
		}

		D->ListSetCurrentPos(6, &pos);

		pos.TopPos = -1;
		pos.SelectPos = 0;

		for (unsigned int uLevel = 0; uLevel < 32; uLevel++)
		{
			DWORD dwMask = 1 << uLevel;
			
			if ( (pFormat->dwLevelMask & dwMask) == dwMask )
			{
				int index = D->ListAddStr(2, LevelNames[uLevel]);
				D->ListSetDataEx(2, index, (void*)uLevel, sizeof(void*));

				if ( uLevel == pCfg->GetLevel() )
					pos.SelectPos = index;
			}
		}

		D->ListSetCurrentPos(2, &pos);

		UpdateDictionarySize(D, (int)D->ListGetData(6, 0)); 
	}

	return D->DefDlgProc (nMsg, Param1, Param2);
}


bool dlgSevenZipPluginConfigure(SevenZipCompressionConfig* pCfg)
{
	bool bResult = false;

	const CompressionFormatInfo* pFormat = pCfg->GetFormat();

	{
		FarDialog D;//(-1, -1, 60, 20);
		D.DoubleBox(3, 1, 0, 0, pFormat->lpName); //0

		int Y = 2;

		D.Text(5, Y, _T("Level:")); //1
		D.ComboBox (25, Y++, 20, NULL); //2
		D.SetFlags(DIF_DROPDOWNLIST);

		D.Separator(Y++); //3

		D.CheckBox(5, Y++, pCfg->IsOverride(), _T("Override params")); //4

		D.Text(8, Y, _T("Method:")); //5
		D.ComboBox (25, Y++, 20, NULL); //6
		D.SetFlags(DIF_DROPDOWNLIST);

		D.Text(8, Y, _T("Dict. size:")); //7
		D.ComboBox(25, Y++, 20, NULL); //8
		D.SetFlags(DIF_DROPDOWNLIST);


		D.Separator(Y++); //9

		/*
		if ( pFormat->SupportFilter )
			D.CheckBox(5, Y++, false, _T("Filter"));
		*/

		if ( pFormat->SupportSolid )
			D.CheckBox(5, Y++, pCfg->IsSolid(), _T("Solid"));
		
		if ( pFormat->SupportMultiThread )
			D.CheckBox(5, Y++, pCfg->IsMultithread(), _T("MultiThread"));
		
		if ( pFormat->SupportSFX )
			D.CheckBox(5, Y++, pCfg->IsSFX(), _T("SFX"));

		if ( pFormat->SupportEncrypt )
			D.CheckBox(5, Y++, pCfg->IsEncrypt(), _T("Encrypt"));
		
		if ( pFormat->SupportEncryptFileNames )
			D.CheckBox(5, Y++, pCfg->IsEncryptFileNames(), _T("Encrypt file names"));

		if ( pFormat->SupportVolumes )
			D.CheckBox(5, Y++, pCfg->IsVolumeMode(), _T("Volumes"));

		D.CheckBox(5, Y++, pCfg->IsCompressHeaders(), _T("Compress headers"));
		D.CheckBox(5, Y++, pCfg->IsCompressHeadersFull(), _T("Compress headers full"));

		D.Separator(Y++);

		D.Button (-1, Y, _T("Ok"));
		D.DefaultButton();

		D.Button (-1, Y, _T("Cancel"));

		D.SetPosition(-1, -1, 60, Y+3);

		D[0].X2 = 56;
		D[0].Y2 = Y+1;

		if ( D.Run(hndConfigureFormat, (void*)pCfg) == D.FirstButton() )
		{
			pCfg->SetOverride(D.GetResultCheck(4));

			int id = 10;

			/*
			if ( pFormat->SupportFilter )
				pCfg->bFilter = D.GetResultCheck(id++);
			*/

			if ( pFormat->SupportSolid )
				pCfg->SetSolid(D.GetResultCheck(id++));

			if ( pFormat->SupportMultiThread )
				pCfg->SetMultithread(D.GetResultCheck(id++));

			if ( pFormat->SupportSFX )
				pCfg->SetSFX(D.GetResultCheck(id++));

			if ( pFormat->SupportEncrypt )
				pCfg->SetEncrypt(D.GetResultCheck(id++));

			if ( pFormat->SupportEncryptFileNames )
				pCfg->SetEncryptFileNames(D.GetResultCheck(id++));

			if ( pFormat->SupportVolumes )
				pCfg->SetVolumeMode(D.GetResultCheck(id++));

			pCfg->SetCompressHeaders(D.GetResultCheck(id++));
			pCfg->SetCompressHeadersFull(D.GetResultCheck(id++));

			bResult = true;
		}
	}

	return bResult;
}
