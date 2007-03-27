/*
CFileMask.cpp

Основной класс для работы с масками файлов. Использовать нужно именно его.

*/

#include "headers.hpp"
#pragma hdrstop

#include "CFileMask.hpp"
#include "FileMasksProcessor.hpp"
#include "FileMasksWithExclude.hpp"
#include "fn.hpp"
#include "lang.hpp"

const int EXCLUDEMASKSEPARATOR=0x7C; // '|'

////////////////////////

CFileMaskW::CFileMaskW()
{
    FileMask=NULL;
}

void CFileMaskW::Free()
{
    if(FileMask)
       delete FileMask;
    FileMask=NULL;
}

/*
 Инициализирует список масок. Принимает список, разделенных запятой или точкой
 с запятой. Разрешается указывать маски исключения, отделив их от основных
 символом '|' Возвращает FALSE при неудаче (например, одна из длина одной из
 масок равна 0).
*/

BOOL CFileMaskW::Set(const wchar_t *Masks, DWORD Flags)
{
  Free();
  BOOL rc=FALSE;
  int Silent=Flags & FMF_SILENT;
  DWORD flags=0;
  if(Flags & FMF_ADDASTERISK) flags|=FMPF_ADDASTERISK;
  if (Masks && *Masks)
  {
    if(wcschr(Masks, EXCLUDEMASKSEPARATOR))
    {
      if(!(Flags&FMF_FORBIDEXCLUDE))
        FileMask=new FileMasksWithExcludeW;
    }
    else
      FileMask=new FileMasksProcessorW;

    if(FileMask)
       rc=FileMask->Set(Masks, flags);

    if(!rc)
      Free();
  }

  if(!Silent && !rc)
    Message(MSG_DOWN|MSG_WARNING,1,UMSG(MWarning),UMSG(MIncorrectMask), UMSG(MOk));

  return rc;
}

// Возвращает TRUE, если список масок пустой
BOOL CFileMaskW::IsEmpty(void)
{
  return FileMask?FileMask->IsEmpty():TRUE;
}

/* сравнить имя файла со списком масок
   Возвращает TRUE в случае успеха.
   Путь в имени файла игнорируется.
*/
BOOL CFileMaskW::Compare(const wchar_t *FileName)
{
  return FileMask?FileMask->Compare(PointToName((wchar_t*)FileName)):FALSE;
}
