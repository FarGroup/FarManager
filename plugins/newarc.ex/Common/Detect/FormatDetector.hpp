class FormatDetector {

public:

	static bool Is7z(const unsigned char* pData, unsigned int uDataSize);
	static bool IsArj(const unsigned char* pData, unsigned int uDataSize);
	static bool IsCab(const unsigned char* pData, unsigned int uDataSize);
	static bool IsHfs(const unsigned char* pData, unsigned int uDataSize);
	static bool IsIso(const unsigned char* pData, unsigned int uDataSize);
	static bool IsLzh(const unsigned char* pData, unsigned int uDataSize);
	static bool IsLzma(const unsigned char* pData, unsigned int uDataSize);
	static bool IsMacho(const unsigned char* pData, unsigned int uDataSize);
	static bool IsNsis(const unsigned char* pData, unsigned int uDataSize);
	static bool IsPe(const unsigned char* pData, unsigned int uDataSize);
	static bool IsRar(const unsigned char* pData, unsigned int uDataSize);
	static bool IsTar(const unsigned char* pData, unsigned int uDataSize);
	static bool IsUdf(const unsigned char* pData, unsigned int uDataSize);
	static bool IsZip(const unsigned char* pData, unsigned int uDataSize);
	static bool IsAce(const unsigned char* pData, unsigned int uDataSize);

};