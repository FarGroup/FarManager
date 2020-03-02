#include "FormatDetector.hpp"

#include "7zdetect.cpp"
#include "arjdetect.cpp"
#include "cabdetect.cpp"
#include "hfsdetect.cpp"
#include "isodetect.cpp"
#include "lzhdetect.cpp"
#include "lzmadetect.cpp"
#include "machodetect.cpp"
#include "nsisdetect.cpp"
#include "pedetect.cpp"
#include "rardetect.cpp"
#include "tardetect.cpp"
#include "udfdetect.cpp"
#include "zipdetect.cpp"
#include "acedetect.cpp"


bool FormatDetector::Is7z(const unsigned char* pData, unsigned int uDataSize)
{
	return Is7zHeader(pData, uDataSize);
}

bool FormatDetector::IsArj(const unsigned char* pData, unsigned int uDataSize)
{
	return IsArjHeader(pData, uDataSize);
}

bool FormatDetector::IsCab(const unsigned char* pData, unsigned int uDataSize)
{
	return IsCabHeader(pData, uDataSize);
}

bool FormatDetector::IsHfs(const unsigned char* pData, unsigned int uDataSize)
{
	return IsHfsHeader(pData, uDataSize);
}

bool FormatDetector::IsIso(const unsigned char* pData, unsigned int uDataSize)
{
	return IsIsoHeader(pData, uDataSize);
}

bool FormatDetector::IsLzh(const unsigned char* pData, unsigned int uDataSize)
{
	return IsLzhHeader(pData, uDataSize);
}

bool FormatDetector::IsLzma(const unsigned char* pData, unsigned int uDataSize)
{
	return IsLzmaHeader(pData, uDataSize);
}

bool FormatDetector::IsMacho(const unsigned char* pData, unsigned int uDataSize)
{
	return IsMachoHeader(pData, uDataSize);
}

bool FormatDetector::IsNsis(const unsigned char* pData, unsigned int uDataSize)
{
	return IsNsisHeader(pData, uDataSize);
}

bool FormatDetector::IsPe(const unsigned char* pData, unsigned int uDataSize)
{
	return IsPeHeader(pData, uDataSize);
}

bool FormatDetector::IsRar(const unsigned char* pData, unsigned int uDataSize)
{
	return IsRarHeader(pData, uDataSize);
}

bool FormatDetector::IsTar(const unsigned char* pData, unsigned int uDataSize)
{
	return IsTarHeader(pData, uDataSize);
}

bool FormatDetector::IsUdf(const unsigned char* pData, unsigned int uDataSize)
{
	return IsUdfHeader(pData, uDataSize);
}

bool FormatDetector::IsZip(const unsigned char* pData, unsigned int uDataSize)
{
	return IsZipHeader(pData, uDataSize);
}

bool FormatDetector::IsAce(const unsigned char* pData, unsigned int uDataSize)
{
	return IsAceHeader(pData, uDataSize);
}
