#ifndef __FINDFILES_HPP__
#define __FINDFILES_HPP__
/*
findfile.hpp

Поиск (Alt-F7)

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

class FindFiles
{
  private:
    int FindFilesProcess();
    void SetPluginDirectory(char *FileName);
    SaveScreen *FindSaveScr;
  public:
    FindFiles();
    ~FindFiles();
};


#endif	// __FINDFILES_HPP__
