#ifndef __FILEFILTER_HPP__
#define __FILEFILTER_HPP__
/*
filefilter.hpp

Файловый фильтр

*/

#include "plugin.hpp"
#include "struct.hpp"
#include "CFileMask.hpp"

#define DATE_COUNT  3

class FileFilter
{
  friend LONG_PTR WINAPI FilterDlgProc(HANDLE hDlg, int Msg,int Param1,LONG_PTR Param2);

  private:

    const wchar_t *FmtMask1;               // Маска даты для форматов DD.MM.YYYY и MM.DD.YYYY
    const wchar_t *FmtMask2;               // Маска даты для формата YYYY.MM.DD
    const wchar_t *FmtMask3;               // Маска времени
    const wchar_t *DigitMask;              // Маска для ввода размеров файла
    const wchar_t *FilterMasksHistoryName; // История для маски файлов

    FarList SizeList;                   // Лист для комбобокса: байты - килобайты
    FarListItem *TableItemSize;
    FarList DateList;                   // Лист для комбобокса времени файла
    FarListItem *TableItemDate;

    // Маски для диалога настройки
    string strDateMask, strDateStrAfter, strDateStrBefore;
    string strTimeMask, strTimeStrAfter, strTimeStrBefore;

    int DateSeparator;                  // Разделитель даты
    int TimeSeparator;                  // Разделитель времени
    int DateFormat;                     // Формат даты в системе

    FilterParams FF;                    // Внутреннее хранилище параметров используется
                                        // для того, чтобы не менять значение Opt.OpFilter
                                        // на "лету".

    CFileMaskW FilterMask;               // Хранилище скомпилированной маски.

  private:

    // Диалоговая процедура
    static LONG_PTR WINAPI FilterDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2);
    void GetFileDateAndTime(const wchar_t *Src,unsigned *Dst,int Separator);

    // Пребразование строковых полей даты и времени в FILETIME
    FILETIME &StrToDateTime(const wchar_t *CDate,const wchar_t *CTime,FILETIME &ft);

  public:

    FileFilter(int DisableDirAttr=FALSE);
    ~FileFilter();

    // Получить текущие настройки фильтра.
    FilterParams *GetParams(){return &FF;};

    // Данный метод вызывается "снаружи" и служит для определения:
    // попадает ли файл fd под условие установленного фильтра.
    // Возвращает TRUE  - попадает;
    //            FALSE - не попадает.
    int FileInFilter(const FAR_FIND_DATA_EX *fd);
    int FileInFilter(const FAR_FIND_DATA *fd);

    // Данный метод вызывается для настройки параметров фильтра.
    void Configure();
};

#endif  // __FINDFILES_HPP__
