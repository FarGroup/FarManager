#ifndef __FILEPOSITIONCACHE_HPP__
#define __FILEPOSITIONCACHE_HPP__
/*
poscache.hpp

Кэш позиций в файлах для viewer/editor

*/

#define MAX_POSITIONS 64

enum
{
	FPOSCACHE_32,
	FPOSCACHE_64,
};

struct TPosCache32
{
	/*
	Param:
		Editor:
			Param[0] = Line
			Param[1] = ScreenLine
			Param[2] = LinePos
			Param[3] = LeftPos
			Param[4] = CodePage or 0
		Viewer:
			Param[0] = FilePos
			Param[1] = LeftPos
			Param[2] = Hex?
			Param[3] = 0
			Param[4] = CodePage
	*/
	DWORD Param[5];

	/*
	Position
		Editor:
			Position[0] = [BOOKMARK_COUNT] Line
			Position[1] = [BOOKMARK_COUNT] Cursor
			Position[2] = [BOOKMARK_COUNT] ScreenLine
			Position[3] = [BOOKMARK_COUNT] LeftPos
		Viewer:
			Position[0] = [BOOKMARK_COUNT] SavePosAddr
			Position[1] = [BOOKMARK_COUNT] SavePosLeft
			Position[2] = [BOOKMARK_COUNT] 0
			Position[3] = [BOOKMARK_COUNT] 0
	*/
	DWORD *Position[4];
};

struct TPosCache64
{
	__int64 Param[5];
	__int64 *Position[4];
};

class FilePositionCache
{
	private:
		int IsMemory;
		char *Names;
		int SizeValue;
		int CurPos;

		BYTE *Param;
		BYTE *Position;
		static char SubKeyItem[16] ,*PtrSubKeyItem;
		static char SubKeyShort[16],*PtrSubKeyShort;

	private:
		int FindPosition(const char *FullName);

	public:
		FilePositionCache(int TypeCache);
		~FilePositionCache();

	public:
		void AddPosition(const char *Name,void *PosCache);
		BOOL GetPosition(const char *Name,void *PosCache);

		BOOL Read(const char *Key);
		BOOL Save(const char *Key);
};


#endif  // __FILEPOSITIONCACHE_HPP__
