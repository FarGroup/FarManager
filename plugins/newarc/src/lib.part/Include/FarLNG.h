#define _CRT_SECURE_NO_WARNINGS


extern bool LoadLanguageFile (
		const char *lpFileName, 
		const char *lpLanguage,
		char **&pStrings, 
		int &nStringsCount
		);

extern bool SearchAndLoadLanguageFile (
		const char *lpPath,
		const char *lpLanguage,
		char **&pStrings, 
		int &nStringsCount
		);

extern void FinalizeLanguageStrings (
		char **pStrings,
		int nStringsCount
		);
