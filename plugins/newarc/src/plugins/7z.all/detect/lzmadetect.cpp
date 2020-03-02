#include <windows.h>

#if defined(__BORLANDC__)
  #pragma option -a1
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack(1)
#else
  #pragma pack(push,1)
#endif

const size_t MIN_HEADER_LEN = 5 + 10;

struct CHeader
{
	unsigned __int64 UnpackSize;
	bool IsThereFilter;
	unsigned char LzmaProps[5];

	bool HasUnpackSize() const { return (UnpackSize != (unsigned __int64)(__int64)-1);  }
};

static bool CheckDictSize(DWORD dicSize)
{
	for (int i = 1; i <= 30; i++)
		if (dicSize == ((DWORD)2 << i) || dicSize == ((DWORD)3 << i))
			return true;
	return false;
}

int IsLzmaHeader(const unsigned char *Data,int DataSize)
{
	if ( (size_t)DataSize < MIN_HEADER_LEN )
		return -1;

	if (Data[0] > 5 * 5 * 9 - 1)
		return -1;

	CHeader block;

	for (int i = 0; i < 5; i++)
		block.LzmaProps[i] = Data[i];

	block.IsThereFilter = false;

	if (!CheckDictSize(*((DWORD *)(Data+1))))
	{
		if (Data[0] > 1 || Data[1] > 5 * 5 * 9 - 1)
			return -1;
		block.IsThereFilter = true;
		for (int i = 0; i < 5; i++)
			block.LzmaProps[i] = Data[i + 1];
		if (!CheckDictSize(*((DWORD *)(block.LzmaProps + 1))))
			return -1;
	}

	unsigned int unpOffset = 5 + (block.IsThereFilter ? 1 : 0);
	block.UnpackSize = *((unsigned __int64 *)(Data + unpOffset));
	if (block.HasUnpackSize() && block.UnpackSize >= ((unsigned __int64)1 << 56))
		return -1;

	if (Data[5 + 8 + (block.IsThereFilter ? 1 : 0)] != 0)
		return -1;

	return 0;
}
