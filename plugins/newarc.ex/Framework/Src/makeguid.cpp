#include "makeguid.h"

unsigned long CRC32(
		unsigned long crc,
		const char *buf,
		unsigned int len
		)
{
	static unsigned long crc_table[256];

	if (!crc_table[1])
	{
		unsigned long c;
		int n, k;

		for (n = 0; n < 256; n++)
		{
			c = (unsigned long)n;
			for (k = 0; k < 8; k++) c = (c >> 1) ^ (c & 1 ? 0xedb88320L : 0);
				crc_table[n] = c;
		}
	}

	crc = crc ^ 0xffffffffL;
	while (len-- > 0) {
		crc = crc_table[(crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
	}

	return crc ^ 0xffffffffL;
}

GUID CreatePluginUID(const GUID& ModuleUID, const TCHAR* lpFileName)
{
#ifdef UNICODE
	char* lpFileNameA = UnicodeToAnsi(lpFileName);
#else
	const char* lpFileNameA = lpFileName;
#endif

	GUID uid;

	DWORD dwCRC32 = CRC32(0, lpFileNameA, strlen(lpFileNameA));

	memcpy (&uid, &ModuleUID, sizeof (GUID));

	memcpy ((unsigned char*)&uid+8, &dwCRC32, 4);
	//memcpy ((unsigned char*)puid+14, &wFormat, 2);

#ifdef UNICODE
	free(lpFileNameA);
#endif

	return uid;
}

GUID CreateFormatUID(const GUID& PluginUID, const TCHAR* lpFormatName)
{
#ifdef UNICODE
	char* lpFormatNameA = UnicodeToAnsi(lpFormatName);
#else
	const char* lpFormatNameA = lpFormatName;
#endif

	GUID uid;

	DWORD dwCRC32 = CRC32(0, lpFormatNameA, strlen(lpFormatNameA));

	memcpy (&uid, &PluginUID, sizeof(GUID));

	//memcpy ((unsigned char*)&uid+10, &dwCRC32, 4);
	memcpy ((unsigned char*)&uid+12, &dwCRC32, 4);

#ifdef UNICODE
	free(lpFormatNameA);
#endif

	return uid;

}

GUID CreateFormatUID(const GUID& PluginUID, int nModuleNumber)
{
	GUID uid;

	memcpy (&uid, &PluginUID, sizeof(GUID));

	//memcpy ((unsigned char*)&uid+10, &dwCRC32, 4);
	memcpy ((unsigned char*)&uid+12, &nModuleNumber, 4);

	return uid;
}
