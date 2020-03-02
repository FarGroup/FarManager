#include "7z.h"

void CompressionConfigToProperties(
		bool bIs7z,
		SevenZipCompressionConfig* pCfg,
		Array<SevenZipProperty*>& properties
		)
{
	const CompressionFormatInfo* pFormat = pCfg->GetFormat();

	//level
	properties.add(new SevenZipProperty(L"X", pCfg->GetLevel()));

	if ( pCfg->IsOverride() )
	{
		//method
		properties.add(new SevenZipProperty(bIs7z?L"0":L"M", MethodNames[pCfg->GetMethod()])); //BUGBUG

		//dict
		wchar_t cTemp[40];

		_itow(pCfg->GetDictionarySize(), cTemp, 10);
		wcscat(cTemp, L"B");

		properties.add(new SevenZipProperty(bIs7z?L"0D":L"D", cTemp)); 
	}

	if ( pFormat->SupportVolumes )
		properties.add(new SevenZipProperty(L"V", pCfg->IsVolumeMode()));

	//if ( pFormat->bSupportEncrypt )
	//if ( pFormat->bSupportMultiThread )
	//if ( pFormat->bSupportSFX )

	if ( pFormat->SupportEncryptFileNames )
		properties.add(new SevenZipProperty(L"HE", pCfg->IsEncryptFileNames()));

	//properties.add(new SevenZipProperty(L"HC", pCfg->bCompressHeaders));
	//properties.add(new SevenZipProperty(L"HCF", pCfg->bCompressHeadersFull));
}

