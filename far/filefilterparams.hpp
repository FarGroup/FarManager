#ifndef __FILEFILTERPARAMS_HPP__
#define __FILEFILTERPARAMS_HPP__
/*
filefilterparams.hpp

Параметры Файлового фильтра

*/

#include "plugin.hpp"
#include "struct.hpp"
#include "CFileMask.hpp"
#include "bitflags.hpp"

struct FileListItem;

#define FILEFILTER_MASK_SIZE 2048
#define FILEFILTER_SIZE_SIZE 32

enum FileFilterFlags
{
  FFF_RPANELINCLUDE = 1,
  FFF_RPANELEXCLUDE = 2,
  FFF_LPANELINCLUDE = 4,
  FFF_LPANELEXCLUDE = 8,
  FFF_FINDFILEINCLUDE = 16,
  FFF_FINDFILEEXCLUDE = 32,
  FFF_COPYINCLUDE = 64,
  FFF_COPYEXCLUDE = 128,
  FFF_SELECTINCLUDE = 256,
  FFF_SELECTEXCLUDE = 512,
};

enum FDateType
{
  FDATE_MODIFIED=0,
  FDATE_CREATED,
  FDATE_OPENED,

  FDATE_COUNT, // всегда последний !!!
};

class FileFilterParams
{
  private:

    char m_Title[512];

    struct
    {
      DWORD Used;
      char Mask[FILEFILTER_MASK_SIZE];
      CFileMask FilterMask; // Хранилище скомпилированной маски.
    } FMask;

    struct
    {
      DWORD Used;
      FDateType DateType;
      FILETIME DateAfter;
      FILETIME DateBefore;
      bool bRelative;
    } FDate;

    struct
    {
      DWORD Used;
      char SizeAbove[FILEFILTER_SIZE_SIZE]; // Здесь всегда будет размер как его ввёл юзер
      char SizeBelow[FILEFILTER_SIZE_SIZE]; // Здесь всегда будет размер как его ввёл юзер
      unsigned __int64 SizeAboveReal; // Здесь всегда будет размер в байтах
      unsigned __int64 SizeBelowReal; // Здесь всегда будет размер в байтах
    } FSize;

    struct
    {
      DWORD Used;
      DWORD AttrSet;
      DWORD AttrClear;
    } FAttr;

    struct
    {
      HighlightDataColor Colors;
      int SortGroup;
      bool bContinueProcessing;
    } FHighlight;

  public:

    BitFlags Flags; // Флаги фильтра

  public:

    FileFilterParams();

    const FileFilterParams &operator=(const FileFilterParams &FF);

    void SetTitle(const char *Title);
    void SetMask(DWORD Used, const char *Mask);
    void SetDate(DWORD Used, DWORD DateType, FILETIME DateAfter, FILETIME DateBefore, bool bRelative);
    void SetSize(DWORD Used, const char *SizeAbove, const char *SizeBelow);
    void SetAttr(DWORD Used, DWORD AttrSet, DWORD AttrClear);
    void SetColors(HighlightDataColor *Colors);
    void SetSortGroup(int SortGroup) { FHighlight.SortGroup = SortGroup; }
    void SetContinueProcessing(bool bContinueProcessing) { FHighlight.bContinueProcessing = bContinueProcessing; }

    const char *GetTitle() const;
    DWORD GetMask(const char **Mask) const;
    DWORD GetDate(DWORD *DateType, FILETIME *DateAfter, FILETIME *DateBefore, bool *bRelative) const;
    DWORD GetSize(const char **SizeAbove, const char **SizeBelow) const;
    DWORD GetAttr(DWORD *AttrSet, DWORD *AttrClear) const;
    void  GetColors(HighlightDataColor *Colors) const;
    int   GetMarkChar() const;
    int   GetSortGroup() const { return FHighlight.SortGroup; }
    bool  GetContinueProcessing() const { return FHighlight.bContinueProcessing; }

    // Данный метод вызывается "снаружи" и служит для определения:
    // попадает ли файл fd под условие установленного фильтра.
    // Возвращает true  - попадает;
    //            false - не попадает.
    bool FileInFilter(WIN32_FIND_DATA *fd, unsigned __int64 CurrentTime);
    bool FileInFilter(FileListItem *fli, unsigned __int64 CurrentTime);
};

bool FileFilterConfig(FileFilterParams *FF, bool ColorConfig=false);

//Централизованная функция для создания строк меню различных фильтров.
void MenuString(char *dest, FileFilterParams *FF, bool bHighightType=false, bool bPanelType=false, const char *FMask=NULL, const char *Title=NULL);

#endif //__FILEFILTERPARAMS_HPP__
