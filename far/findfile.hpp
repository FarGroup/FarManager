#ifndef __FINDFILES_HPP__
#define __FINDFILES_HPP__
/*
findfile.hpp

Поиск (Alt-F7)

*/

/* Revision: 1.02 05.06.2001 $ */

/*
Modify:
  05.06.2001 SVS
    + Обработчик диалога (без него нажатие на пимпу "[View]" заваливает ФАР)
  06.05.2001 DJ
    ! перетрях #include
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#undef SEARCH_ALL
enum {
  SEARCH_ALL=0,
  SEARCH_ROOT,
  SEARCH_FROM_CURRENT,
  SEARCH_CURRENT_ONLY,
  SEARCH_SELECTED
};

class SaveScreen;

class FindFiles
{
  private:
    SaveScreen *FindSaveScr;

  private:
    int FindFilesProcess();
    void SetPluginDirectory(char *FileName);
    static long WINAPI FindDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2);

  public:
    FindFiles();
    ~FindFiles();

};


#endif	// __FINDFILES_HPP__
