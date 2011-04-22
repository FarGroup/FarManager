#include "FarPluginBase.hpp"

class Language {

private:

	string m_strLanguage;
	Array<string> strings;

public:

	Language();

	bool LoadFromBuffer(char* pBuffer, int nCodePage = CP_OEMCP);
	bool LoadFromFile(const TCHAR* lpFileName);

	void AddString(const TCHAR* lpStr);
	const TCHAR* GetMsg(unsigned int uID);

	const TCHAR* GetLanguage();

private:

	void ParseString(const TCHAR* lpStr);
};

