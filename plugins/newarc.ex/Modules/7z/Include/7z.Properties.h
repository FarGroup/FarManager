#pragma once
#include "7z.h"

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

extern void CompressionConfigToProperties(
		bool bIs7z,
		SevenZipCompressionConfig* pCfg,
		Array<SevenZipProperty*>& properties
		);

