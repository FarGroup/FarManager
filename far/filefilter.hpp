#ifndef __FILEFILTER_HPP__
#define __FILEFILTER_HPP__
/*
filefilter.hpp

Файловый фильтр

*/

/* Revision: 1.00 04.10.2003 $ */

/*
Modify:
  04.10.2003 KM
    ! Введение в строй фильтра операций. Начало новой эры :-)
*/

#include "plugin.hpp"
#include "struct.hpp"
#include "CFileMask.hpp"

#define SIZE_COUNT  2
#define DATE_COUNT  3

class FileFilter
{
  friend long WINAPI FilterDlgProc(HANDLE hDlg, int Msg,int Param1,long Param2);

  private:

    const char *FmtMask1;               // Маска даты для форматов DD.MM.YYYY и MM.DD.YYYY
    const char *FmtMask2;               // Маска даты для формата YYYY.MM.DD
    const char *FmtMask3;               // Маска времени
    const char *DigitMask;              // Маска для ввода размеров файла
    const char *FilterMasksHistoryName; // История для маски файлов

    FarList SizeList;                   // Лист для комбобокса: байты - килобайты
    FarListItem *TableItemSize;
    FarList DateList;                   // Лист для комбобокса времени файла
    FarListItem *TableItemDate;

    // Маски для диалога настройки
    char DateMask[16],DateStrAfter[16],DateStrBefore[16];
    char TimeMask[16],TimeStrAfter[16],TimeStrBefore[16];

    int DateSeparator;                  // Разделитель даты
    int TimeSeparator;                  // Разделитель времени
    int DateFormat;                     // Формат даты в системе

    FilterParams FF;                    // Внутреннее хранилище параметров используется
                                        // для того, чтобы не менять значение Opt.OpFilter
                                        // на "лету".

  private:

    // Диалоговая процедура
    static long WINAPI FilterDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2);
    void GetFileDateAndTime(const char *Src,unsigned *Dst,int Separator);

    // Пребразование строковых полей даты и времени в FILETIME
    FILETIME &StrToDateTime(const char *CDate,const char *CTime,FILETIME &ft);

  public:

    FileFilter();
    ~FileFilter();

    FilterParams GetParams(){return FF;};

    // Данный метод вызывается "снаружи" и служит для определения:
    // попадает ли файл fd под условие установленного фильтра.
    // Возвращает TRUE  - попадает;
    //            FALSE - не попадает.
    int FileInFilter(WIN32_FIND_DATA *fd);

    // Данный метод вызывается для настройки параметров фильтра.
    void Configure();
};

#endif  // __FINDFILES_HPP__
