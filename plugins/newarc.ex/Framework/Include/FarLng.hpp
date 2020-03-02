#pragma once
#include "FarPluginBase.hpp"

extern bool LoadLanguageFile (
		const TCHAR *lpFileName, 
		const TCHAR *lpLanguage,
		TCHAR **&Strings, 
		int &StringsCount
		);

extern bool SearchAndLoadLanguageFile (
		const TCHAR *lpPath,
		const TCHAR *lpLanguage,
		TCHAR **&pStrings, 
		int &nStringsCount
		);

extern void FinalizeLanguageStrings (
		TCHAR **pStrings,
		int nStringsCount
		);
