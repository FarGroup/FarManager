#pragma once
#include "d5d.h"

struct LanguageEntry {
	char ID[6];
	char* lpAnsiString;
};

class D5DLanguage {

private:

	ObjectArray<LanguageEntry*> m_pStrings;

public:

	D5DLanguage();
	~D5DLanguage();

	bool Load(const TCHAR* lpFileName);
	const char* GetMessage(const char* lpID);

private:

	void ReadWideString(HANDLE hFile, string& strData);
};
