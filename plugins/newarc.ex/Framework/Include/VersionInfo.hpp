#pragma once

struct LANGANDCODEPAGE {
	WORD wLanguage;
	WORD wCodePage;
};


class VersionInfo {

	string m_strFileName;
	void* m_pVersionBlock;

	WORD m_wLanguage;
	WORD m_wCodePage;

public:

	VersionInfo(const TCHAR* lpFileName)
	{
		m_strFileName = lpFileName;

		unsigned int uSize = GetFileVersionInfoSize(m_strFileName, NULL);

		if ( uSize != 0 )
		{
			m_pVersionBlock = malloc(uSize);

			GetFileVersionInfo(m_strFileName, NULL, uSize, m_pVersionBlock);

			unsigned int dwVersionInfoEntrySize;

			LANGANDCODEPAGE* pTranslate;

			if ( VerQueryValue(
					m_pVersionBlock,
					_T("\\VarFileInfo\\Translation"),
					(LPVOID*)&pTranslate,
					&dwVersionInfoEntrySize
					) )
			{
				m_wLanguage = pTranslate->wLanguage; // RAVE!!!
				m_wCodePage = pTranslate->wCodePage;
			}
		}
	}

	~VersionInfo()
	{
		free(m_pVersionBlock);
	}

	bool GetValue(
			const TCHAR* lpValueName, 
			const TCHAR* lpValueDescription,
			string& strResult
			)
	{
		bool bResult = false;

		unsigned int dwVersionInfoEntrySize;

		string strVerInfoSubBlock;
		TCHAR* lpValue;

		LANGANDCODEPAGE Langs[3];

		Langs[0].wLanguage = m_wLanguage;
		Langs[0].wCodePage = m_wCodePage;

		Langs[1].wLanguage = 0x0409;
		Langs[1].wCodePage = 0x04E4; //us ansi

		Langs[2].wLanguage = 0x0409;
		Langs[2].wCodePage = 0x04B0; //us unicode

		for (unsigned int i = 0; i < 3; i++)
		{
			strVerInfoSubBlock.Format(
					_T("\\StringFileInfo\\%04x%04x\\%s"),
					Langs[i].wLanguage,
					Langs[i].wCodePage,
					lpValueName
					);

			if ( VerQueryValue(
					m_pVersionBlock,
					strVerInfoSubBlock,
					(LPVOID*)&lpValue,
					&dwVersionInfoEntrySize
					) )
			{
				strResult = lpValue;

				bResult = true;
				break;
			}
		}

		return bResult;
	}
};
