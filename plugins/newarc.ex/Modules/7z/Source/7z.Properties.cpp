#include "7z.h"

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

