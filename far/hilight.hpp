#ifndef __HIGHLIGHTFILES_HPP__
#define __HIGHLIGHTFILES_HPP__
/*
hilight.hpp

Files highlighting

*/

#include "CFileMask.hpp"
#include "struct.hpp"
#include "filefilterparams.hpp"
#include "array.hpp"

class VMenu;
struct FileListItem;

class HighlightFiles
{
	private:
		TPointerArray<FileFilterParams> HiData;
		int FirstCount, UpperCount, LowerCount, LastCount;
		unsigned __int64 CurrentTime;

	private:
		void InitHighlightFiles();
		void ClearData();

		int  MenuPosToRealPos(int MenuPos, int **Count, bool Insert=false);
		void FillMenu(VMenu *HiMenu,int MenuPos);
		void ProcessGroups();

	public:
		HighlightFiles();
		~HighlightFiles();

	public:
		void UpdateCurrentTime();
		void GetHiColor(WIN32_FIND_DATA *fd,struct HighlightDataColor *Colors,bool UseAttrHighlighting=false);
		void GetHiColor(struct FileListItem *FileItem,int FileCount,bool UseAttrHighlighting=false);
		int  GetGroup(WIN32_FIND_DATA *fd);
		int  GetGroup(FileListItem *fli);
		void HiEdit(int MenuPos);

		void SaveHiData();
};

#endif  // __HIGHLIGHTFILES_HPP__
