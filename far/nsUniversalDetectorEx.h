#include "ucd/nscore.h"
#include "ucd/nsUniversalDetector.h"

class nsUniversalDetectorEx : public nsUniversalDetector {
public:

	nsUniversalDetectorEx() : nsUniversalDetector(NS_FILTER_NON_CJK) 
	{
		m_codepage = -1;
	}

	int getCodePage()
	{
		return m_codepage;
	}

protected:

	virtual void Report(const char* aCharset)
	{
		if ( !strcmp(aCharset, "windows-1251") )
			m_codepage = 1251;
		else

		if ( !strcmp(aCharset, "windows-1252") )
			m_codepage = 1252;
		else

		if ( !strcmp(aCharset, "windows-1253") )
			m_codepage = 1253;
		else

		if ( !strcmp(aCharset, "UTF16-LE") )
			m_codepage = 1200;
		else

		if ( !strcmp(aCharset, "IBM866") )
			m_codepage = 866;
		else

		if ( !strcmp(aCharset, "KOI8-R") )
			m_codepage = 20866;
		else

		if ( !strcmp(aCharset, "windows-1255") )
			m_codepage = 1255;
		else

		if ( !strcmp(aCharset, "UTF-8") )
			m_codepage = CP_UTF8;

	}

private:

	int m_codepage;
};
