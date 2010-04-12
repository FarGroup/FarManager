#ifndef __FILEFILTER_HPP__
#define __FILEFILTER_HPP__
/*
filefilter.hpp

Файловый фильтр

*/

#include "plugin.hpp"
#include "struct.hpp"
#include "filefilterparams.hpp"

class VMenu;
class Panel;

enum enumFileFilterType
{
	FFT_PANEL = 0,           // отображалка файловых объектов на панели
	FFT_FINDFILE,            // фильтр для поисковика
	FFT_COPY,                // фильтр в копире
	FFT_SELECT,              // select файловых объекто на панели
};

// почему FileInFilter вернул true или false
enum enumFileInFilterType
{
	FIFT_NOTINTFILTER = 0,   // файловый объект не попал ни в один из фильтров
	FIFT_INCLUDE,            // файловый объект попал в Include
	FIFT_EXCLUDE,            // файловый объект попал в Exclude
};

class FileFilter
{
	private:
		Panel *m_HostPanel;
		enumFileFilterType m_FilterType;
		unsigned __int64 CurrentTime;

		int  ParseAndAddMasks(char **ExtPtr,const char *FileName,DWORD FileAttr,int& ExtCount,int Check);
		void ProcessSelection(VMenu *FilterList);
		enumFileFilterFlagsType GetFFFT();
		int  GetCheck(FileFilterParams *FFP);
		static void SwapPanelFlags(FileFilterParams *CurFilterData);

	public:
		FileFilter(Panel *HostPanel, enumFileFilterType FilterType);
		~FileFilter();

		bool FilterEdit();
		void UpdateCurrentTime();
		bool FileInFilter(FileListItem *fli,enumFileInFilterType *foundType=NULL);
		bool FileInFilter(WIN32_FIND_DATA *fd,enumFileInFilterType *foundType=NULL);
		bool IsEnabledOnPanel();

		static void InitFilter();
		static void CloseFilter();
		static void SwapFilter();
		static void SaveFilters();
};

#endif  // __FILEFILTER_HPP__
