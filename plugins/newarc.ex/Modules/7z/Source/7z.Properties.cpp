#define countof(x) (sizeof(x)/sizeof(x[0]))

enum CompressionLevel
{
	kStore = 0,
	kFastest = 1,
	kFast = 3,
	kNormal = 5,
	kMaximum = 7,
	kUltra = 9
};

enum CompressionMethodID
{
	kCopy,
	kLZMA,
	kLZMA2,
	kPPMd,
	kBZip2,
	kDeflate,
	kDeflate64
};


static const TCHAR* MethodNames[] =
{
	_T("Copy"),
	_T("LZMA"),
	_T("LZMA2"),
	_T("PPMd"),
	_T("BZip2"),
	_T("Deflate"),
	_T("Deflate64")
};


static const TCHAR* LevelNames[] =
{
	_T("Store"),
	_T("Fastest"),
	NULL,
	_T("Fast"),
	NULL,
	_T("Normal"),
	NULL,
	_T("Maximum"),
	NULL,
	_T("Ultra")
};

static const CompressionMethodID SevenZipMethods[] = {kLZMA, kLZMA2, kPPMd, kBZip2};
static const CompressionMethodID SevenZipSfxMethods[] = {kCopy, kLZMA, kLZMA2, kPPMd};
static const CompressionMethodID ZipMethods[] = {kDeflate, kDeflate64, kBZip2, kLZMA};
static const CompressionMethodID GZipMethods[] = {kDeflate};
static const CompressionMethodID BZip2Methods[] = {kBZip2};
static const CompressionMethodID XzMethods[] = {kLZMA2};

struct CompressionFormatInfo
{
	const TCHAR* lpName;
	DWORD dwLevelMask;
	const CompressionMethodID* pMethodIDs;
	int nNumMethods;
	bool SupportFilter;
	bool SupportSolid;
	bool SupportMultiThread;
	bool SupportSFX;
	bool SupportEncrypt;
	bool SupportEncryptFileNames;
	bool SupportVolumes;
};

static const CompressionFormatInfo CompressionFormats[] =
{
	{	
  		_T("7z"), 
  		(1 << 0) | (1 << 1) | (1 << 3) | (1 << 5) | (1 << 7) | (1 << 9),
  		SevenZipMethods,
  		countof(SevenZipMethods),
  		true,
  		true,
  		true,
  		true,
  		true,
  		true,
		true
  	},
  	{
  		_T("7zSFX"),
  		(1 << 0) | (1 << 1) | (1 << 3) | (1 << 5) | (1 << 7) | (1 << 9),
  		SevenZipSfxMethods,
  		countof(SevenZipSfxMethods),
  		true,
  		true,
  		true,
  		true,
  		true,
  		true,
		true
	},
	{
		_T("Zip"),
		(1 << 0) | (1 << 1) | (1 << 3) | (1 << 5) | (1 << 7) | (1 << 9),
		ZipMethods,
		countof(ZipMethods),
    	false, 
		false, 
		true, 
		false, 
		true, 
		false,
		false
	},
	{
		_T("GZip"),
		(1 << 1) | (1 << 5) | (1 << 7) | (1 << 9),
		GZipMethods,
		countof(GZipMethods),
		false, 
		false, 
		false, 
		false, 
		false, 
		false,
		false
	},
	{
		_T("BZip2"),
		(1 << 1) | (1 << 3) | (1 << 5) | (1 << 7) | (1 << 9),
		BZip2Methods,
		countof(BZip2Methods),
		false, 
		false, 
		true, 
		false, 
		false, 
		false,
		false
	},
	{
		_T("xz"),
		(1 << 1) | (1 << 3) | (1 << 5) | (1 << 7) | (1 << 9),
		XzMethods,
		countof(XzMethods),
		false, 
		false, 
		true, 
		false, 
		false, 
		false,
		false
	},
	{
		_T("Tar"),
		(1 << 0),
		0, 
		0,
		false, 
		false, 
		false, 
		false, 
		false, 
		false,
		false
	}
};

struct CompressionMapEntry 
{
	const GUID& uid;
	const CompressionFormatInfo* Format;
};

static CompressionMapEntry CompressionMap[] = 
{
	{CLSID_CFormat7z, &CompressionFormats[0]},
	{CLSID_CZipHandler, &CompressionFormats[2]},
	{CLSID_CGZipHandler, &CompressionFormats[3]},
	{CLSID_CBZip2Handler, &CompressionFormats[4]},
	{CLSID_CXzHandler, &CompressionFormats[5]},
	{CLSID_CTarHandler, &CompressionFormats[6]}
};

struct SevenZipCompressionConfig {
	const CompressionFormatInfo* pFormat;

	int nLevel;

	bool bOverride;

	int nMethod;
	unsigned int uDictionarySize;

	bool bFilter;
	bool bSolid;
	bool bMultithread;
	bool bSFX;
	bool bEncrypt;
	bool bEncryptFileNames;

	bool bCompressHeaders;
	bool bCompressHeadersFull;

	bool bVolumeMode;
};


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
	string strResult;
	GetSizeString(nSize, strResult);

	int index = D->ListAddStr(8, strResult);
	D->ListSetDataEx(8, index, (void*)nSize, sizeof(void*));
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
			pCfg->nLevel = (int)D->ListGetData(2, pos.SelectPos);

			D->ListGetCurrentPos(6, &pos);
			pCfg->nMethod = (int)D->ListGetData(6, pos.SelectPos);

			D->ListGetCurrentPos(8, &pos);
			pCfg->uDictionarySize = (unsigned int)D->ListGetData(8, pos.SelectPos);

			return TRUE;
		}
	}

	if ( nMsg == DN_INITDIALOG )
	{
		for (int i = 0; i < pCfg->pFormat->nNumMethods; i++)
		{
			int index = D->ListAddStr(6, MethodNames[pCfg->pFormat->pMethodIDs[i]]); //check for 7z SFX!!!
			D->ListSetDataEx(6, index, (void*)pCfg->pFormat->pMethodIDs[i], sizeof(void*));
		}

		FarListPos pos;

		pos.TopPos = -1;
		pos.SelectPos = 0;

		for (int nLevel = 0; nLevel < 32; nLevel++)
		{
		    DWORD dwMask = 1 << nLevel;
			
			if ( (pCfg->pFormat->dwLevelMask & dwMask) == dwMask )
			{
				int index = D->ListAddStr(2, LevelNames[nLevel]);
				D->ListSetDataEx(2, index, (void*)nLevel, sizeof(void*));

				if ( nLevel == 5 ) //normal
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

	const CompressionFormatInfo* pFormat = pCfg->pFormat;

	{
		FarDialog D;//(-1, -1, 60, 20);
		D.DoubleBox(3, 1, 0, 0, pFormat->lpName); //0

		int Y = 2;

		D.Text(5, Y, _T("Level:")); //1
		D.ComboBox (25, Y++, 20, NULL); //2
		D.SetFlags(DIF_DROPDOWNLIST);

		D.Separator(Y++); //3

		D.CheckBox(5, Y++, false, _T("Override params")); //4

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
			D.CheckBox(5, Y++, false, _T("Solid"));
		
		if ( pFormat->SupportMultiThread )
			D.CheckBox(5, Y++, false, _T("MultiThread"));
		
		if ( pFormat->SupportSFX )
			D.CheckBox(5, Y++, false, _T("SFX"));

		if ( pFormat->SupportEncrypt )
			D.CheckBox(5, Y++, false, _T("Encrypt"));
		
		if ( pFormat->SupportEncryptFileNames )
			D.CheckBox(5, Y++, false, _T("Encrypt file names"));

		if ( pFormat->SupportVolumes )
			D.CheckBox(5, Y++, false, _T("Volumes"));

		D.CheckBox(5, Y++, false, _T("Compress headers"));
		D.CheckBox(5, Y++, false, _T("Compress headers full"));

		D.Separator(Y++);

		D.Button (-1, Y, _T("Ok"));
		D.DefaultButton();

		D.Button (-1, Y, _T("Cancel"));

		D.SetPosition(-1, -1, 60, Y+3);
		D[0].X2 = 56;
		D[0].Y2 = Y+1;

		if ( D.Run(hndConfigureFormat, (void*)pCfg) == D.FirstButton() )
		{
			pCfg->bOverride = D.GetResultCheck(4);

			int id = 8;

			/*
			if ( pFormat->SupportFilter )
				pCfg->bFilter = D.GetResultCheck(id++);
			*/

			if ( pFormat->SupportSolid )
				pCfg->bSolid = D.GetResultCheck(id++);

			if ( pFormat->SupportMultiThread )
				pCfg->bSolid = D.GetResultCheck(id++);

			if ( pFormat->SupportSFX )
				pCfg->bSFX = D.GetResultCheck(id++);

			if ( pFormat->SupportEncrypt )
				pCfg->bEncrypt = D.GetResultCheck(id++);

			if ( pFormat->SupportEncryptFileNames )
				pCfg->bEncryptFileNames = D.GetResultCheck(id++);

			if ( pFormat->SupportVolumes )
				pCfg->bVolumeMode = D.GetResultCheck(id++);

			pCfg->bCompressHeaders = D.GetResultCheck(id++);
			pCfg->bCompressHeadersFull = D.GetResultCheck(id++);

			bResult = true;
		}
	}

	return bResult;
}

class SevenZipProperty {
private:

	wchar_t* lpName;
	CPropVariant vtValue;

public:


	SevenZipProperty(const wchar_t* name, const TCHAR* value)
	{
		lpName = _wcsdup(name);

		if ( *value )
		{
			vtValue.vt = VT_BSTR;
#ifdef UNICODE
			vtValue.bstrVal = SysAllocString(value);
#else
			wchar_t* pBuffer = AnsiToUnicode(value);
			vtValue.bstrVal = SysAllocString(pBuffer);
			free(pBuffer);
#endif
		}
		else
			vtValue.vt = VT_EMPTY;
	}


	SevenZipProperty(const wchar_t* name, unsigned int value)
	{
		lpName = _wcsdup(name);

		vtValue.vt = VT_UI4;
		vtValue.uintVal = value;
	}
	

	SevenZipProperty(const wchar_t* name, bool value)
	{
		lpName = _wcsdup(name);

		vtValue.vt = VT_BSTR;
		vtValue.bstrVal = SysAllocString(value ? L"ON" : L"OFF");
	}

	const wchar_t* GetName()
	{
		return lpName;
	}

	const PROPVARIANT& GetValue()
	{
		return vtValue;
	}

};

void CompressionConfigToProperties(
		bool bIs7z,
		const SevenZipCompressionConfig* pCfg,
		Array<SevenZipProperty*>& properties
		)
{
	const CompressionFormatInfo* pFormat = pCfg->pFormat;

	//level
	properties.add(new SevenZipProperty(L"X", (unsigned int)pCfg->nLevel));

	if ( pCfg->bOverride )
	{
		//method
		properties.add(new SevenZipProperty(bIs7z?L"0":L"M", MethodNames[pCfg->nMethod]));

		//dict
		wchar_t cTemp[40];

		_itow(pCfg->uDictionarySize, cTemp, 10);
		wcscat(cTemp, L"B");

		properties.add(new SevenZipProperty(bIs7z?L"0D":L"D", cTemp)); 
	}

	if ( pFormat->SupportVolumes )
		properties.add(new SevenZipProperty(L"V", pCfg->bVolumeMode));

	//if ( pFormat->bSupportEncrypt )
	//if ( pFormat->bSupportMultiThread )
	//if ( pFormat->bSupportSFX )

	if ( pFormat->SupportEncryptFileNames )
		properties.add(new SevenZipProperty(L"HE", pCfg->bEncryptFileNames));

	//properties.add(new SevenZipProperty(L"HC", pCfg->bCompressHeaders));
	//properties.add(new SevenZipProperty(L"HCF", pCfg->bCompressHeadersFull));
}

void SevenZipPlugin::Configure(const GUID& uid)
{
	const CompressionFormatInfo* pFormat = GetCompressionFormatInfo(uid);

	if ( pFormat )
	{
		SevenZipCompressionConfig* pCfg = new SevenZipCompressionConfig;
		memset(pCfg, 0, sizeof(SevenZipCompressionConfig));

		pCfg->pFormat = pFormat;

		dlgSevenZipPluginConfigure(pCfg);
	}
}