#include "d5d.h"
#include "zlib/deflate.h"

D5DLanguage::D5DLanguage()
{
}

bool D5DLanguage::Load(const TCHAR* lpFileName)
{
	bool bResult = false;

	HANDLE hFile = CreateFile(
			lpFileName,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			0,
			NULL
			);

	if ( hFile != INVALID_HANDLE_VALUE )
	{
		DWORD dwRead;
		DLNGHeader Hdr;

		ReadFile(hFile, &Hdr, sizeof(DLNGHeader), &dwRead, NULL);

		if ( !memcmp(Hdr.ID, "DLNG\x1A", sizeof(Hdr.ID)) )
		{
			if ( (Hdr.Version == 1) || (Hdr.Version == 3) || (Hdr.Version == 4) )
			{
				if ( Hdr.Version == 4 )
				{
					DLNGHeaderV4 Hdr4;

					SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
					ReadFile(hFile, &Hdr4, sizeof(DLNGHeaderV4), &dwRead, NULL);

					/*
					string strTemp;

					ReadWideString(hFile, strTemp); //name
					ReadWideString(hFile, strTemp); //author
					ReadWideString(hFile, strTemp); //url
					ReadWideString(hFile, strTemp); //email
					ReadWideString(hFile, strTemp); //fontname
					*/

					SetFilePointer(hFile, Hdr4.DataOffset, NULL, FILE_BEGIN);

					unsigned char* pBuffer = new unsigned char[Hdr4.DataSize];
	
					if ( Hdr4.Compression == 0 )
						ReadFile(hFile, pBuffer, Hdr4.DataSize, &dwRead, NULL);
					else

					if ( Hdr4.Compression == 99 )
					{
						unsigned char* pCompressedBuffer = new unsigned char[Hdr4.DataCSize];

						ReadFile(hFile, pCompressedBuffer, Hdr4.DataCSize, &dwRead, NULL);

						uncompress(pBuffer, (uLongf*)&Hdr4.DataSize, pCompressedBuffer, Hdr4.DataCSize);
					
						delete [] pCompressedBuffer;
					}

					if ( (Hdr4.Compression == 0) || (Hdr4.Compression == 99) )
					{
						SetFilePointer(hFile, Hdr4.IndexOffset, NULL, FILE_BEGIN);

						DLNGIndexEntryV4 Idx;

						m_pStrings.reset();

						for (int i = 0; i < Hdr4.IndexNum; i++)
						{
							ReadFile(hFile, &Idx, sizeof(DLNGIndexEntryV4), &dwRead, NULL);

							unsigned int uPos = Idx.Offset*sizeof(wchar_t);

							wchar_t* lpString = new wchar_t[Idx.Length+1];
							lpString[Idx.Length] = 0;

							memcpy(lpString, &pBuffer[uPos], Idx.Length*sizeof(wchar_t));
						                                                    
							LanguageEntry* pEntry = new LanguageEntry;

							memcpy(&pEntry->ID, &Idx.ID, sizeof(Idx.ID));
							pEntry->lpAnsiString = UnicodeToAnsi(lpString, CP_ACP);

							m_pStrings.add(pEntry);

							delete [] lpString;
						}
		
						delete [] pBuffer;

						return true;
					}
				}
				else

				if ( Hdr.Version == 3 )
				{
					__debug(_T("Are you nutz?"));
				}
				else

				if ( Hdr.Version == 1 )
				{
					__debug(_T("Are you nutz?"));
				}
			}
		}


		CloseHandle(hFile);
	}

	return bResult;
}

void D5DLanguage::ReadWideString(HANDLE hFile, string& strData)
{
	DWORD dwRead;
	unsigned char cLength;

	ReadFile(hFile, &cLength, 1, &dwRead, NULL);

	wchar_t* lpData = new wchar_t[cLength+1];

	lpData[cLength] = 0;
	ReadFile(hFile, lpData, cLength*sizeof(wchar_t), &dwRead, NULL);

	strData.SetData(lpData, CP_OEMCP);

	delete [] lpData;
}

const char* D5DLanguage::GetMessage(const char* lpID)
{
	for (int i = 0; i < m_pStrings.count(); i++)
	{
		if ( !memcmp(m_pStrings[i]->ID, lpID, 6) )
			return m_pStrings[i]->lpAnsiString;
	}

	return "--UNKNOWN--";
}

D5DLanguage::~D5DLanguage()
{
}