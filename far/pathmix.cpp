/*
pathmix.cpp

Misc functions for processing of path names
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "headers.hpp"
#pragma hdrstop

#include "pathmix.hpp"
#include "config.hpp"
#include "treelist.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "filelist.hpp"
#include "panel.hpp"
#include "filepanels.hpp"

NTPath::NTPath(LPCWSTR Src)
{
	Str=Src;
	if(!PathPrefix(Src))
	{
		ConvertNameToFull(Str,Str);
		if(!PathPrefix(Str))
		{
			if(IsLocalPath(Str))
				Str=string(L"\\\\?\\")+Str;
			else
				Str=string(L"\\\\?\\UNC\\")+&Str[2];
		}
	}
}

void CreatePath(string &strPath)
{
  wchar_t *ChPtr = strPath.GetBuffer ();
  wchar_t *DirPart = ChPtr;

  BOOL bEnd = FALSE;

  while ( TRUE )
  {
		if ( (*ChPtr == 0) || IsSlash(*ChPtr) )
    {
      if ( *ChPtr == 0 )
        bEnd = TRUE;

      *ChPtr = 0;

			if ( Opt.CreateUppercaseFolders && !IsCaseMixed(DirPart) && apiGetFileAttributes(strPath) == INVALID_FILE_ATTRIBUTES) //BUGBUG
        CharUpperW (DirPart);

			if ( apiCreateDirectory(strPath, NULL) )
        TreeList::AddTreeName(strPath);

      if ( bEnd )
        break;

      *ChPtr = L'\\';
      DirPart = ChPtr+1;
    }

    ChPtr++;
  }
}

int PathMayBeAbsolute(const wchar_t *Path)
{
	return (Path && ((IsAlpha(*Path) && Path[1]==L':' && IsSlash(Path[2])) || (Path[0]==L'\\' && Path[1]==L'\\')));
}

BOOL IsNetworkPath(const wchar_t *Path)
{
  return (Path && Path[0] == L'\\' && Path[1] == L'\\' && Path[2] != L'\\' && wcsrchr(Path+2,L'\\'));
}

BOOL IsLocalPath(const wchar_t *Path)
{
	return (Path && *Path && Path[1]==L':');
}

BOOL IsLocalRootPath(const wchar_t *Path)
{
	return (Path && *Path && Path[1]==L':' && IsSlash(Path[2]) && !Path[3]);
}

bool PathPrefix(const wchar_t *Path)
{
/*
	\\?\
	\\.\
	\??\
*/
	return Path && Path[0] == L'\\' && (Path[1] == L'\\' || Path[1] == L'?') && (Path[2] == L'?' || Path[2] == L'.') && Path[3] == L'\\';
}

BOOL IsLocalPrefixPath(const wchar_t *Path)
{
	return PathPrefix(Path) && Path[4] && Path[5] == L':' && Path[6] == L'\\';
}

BOOL IsLocalVolumePath(const wchar_t *Path)
{
	return PathPrefix(Path) && !wcsnicmp(&Path[4],L"Volume{",7) && Path[47] == L'}';
}

BOOL IsLocalVolumeRootPath(const wchar_t *Path)
{
	return IsLocalVolumePath(Path) && !Path[48];
}

// Косметические преобразования строки пути.
// CheckFullPath используется в FCTL_SET[ANOTHER]PANELDIR
string& PrepareDiskPath(string &strPath,BOOL CheckFullPath)
{
	if( !strPath.IsEmpty() )
	{
		if(((IsAlpha(strPath.At(0)) && strPath.At(1)==L':') || (strPath.At(0)==L'\\' && strPath.At(1)==L'\\')))
		{
			if(CheckFullPath)
			{
				ConvertNameToFull(strPath,strPath);
				wchar_t *lpwszPath=strPath.GetBuffer(),*Src=lpwszPath,*Dst=lpwszPath;
				if(IsLocalPath(lpwszPath))
				{
					Src+=2;
					if(IsSlash(*Src))
						Src++;
					Dst+=2;
					if(IsSlash(*Dst))
						Dst++;
				}
				if(*Src)
				{
					for(wchar_t c=*Src;;Src++,c=*Src)
					{
						if (!c||IsSlash(c))
						{
							*Src=0;
							FAR_FIND_DATA_EX fd;
							BOOL find=apiGetFindDataEx(lpwszPath,&fd,false);
							*Src=c;
							if(find)
							{
								size_t n=fd.strFileName.GetLength();
								size_t n1 = n-(Src-Dst);
								if((int)n1>0)
								{
									size_t dSrc=Src-lpwszPath,dDst=Dst-lpwszPath;
									strPath.ReleaseBuffer();
									lpwszPath=strPath.GetBuffer(int(strPath.GetLength()+n1));
									Src=lpwszPath+dSrc;
									Dst=lpwszPath+dDst;
									wmemmove(Src+n1,Src,StrLength(Src)+1);
									Src+=n1;
								}
								wcsncpy(Dst,fd.strFileName,n);
								Dst+=n;
								wcscpy(Dst,Src);
								if(c)
								{
									Dst++;
									Src=Dst;
								}
							}
							else
							{
								if(c)
								{
									Src++;
									Dst=Src;
								}
							}
						}
						if(!*Src)
							break;
					}
				}
				strPath.ReleaseBuffer();
			}

			wchar_t *lpwszPath = strPath.GetBuffer ();

			if (lpwszPath[0]==L'\\' && lpwszPath[1]==L'\\')
			{
				if(IsLocalPrefixPath(lpwszPath))
				{
					lpwszPath[4] = Upper(lpwszPath[4]);
				}
				else
				{
					wchar_t *ptr=&lpwszPath[2];
					while (*ptr && !IsSlash(*ptr))
					{
						*ptr=Upper(*ptr);
						ptr++;
					}
				}
			}
			else
			{
				lpwszPath[0]=Upper(lpwszPath[0]);
			}

			strPath.ReleaseBuffer ();
		}
	}
	return strPath;
}

int _MakePath1(DWORD Key, string &strPathName, const wchar_t *Param2,int ShortNameAsIs)
{
  int RetCode=FALSE;
  int NeedRealName=FALSE;

  strPathName = L"";
  switch(Key)
  {
    case KEY_CTRLALTBRACKET:       // Вставить сетевое (UNC) путь из левой панели
    case KEY_CTRLALTBACKBRACKET:   // Вставить сетевое (UNC) путь из правой панели
    case KEY_ALTSHIFTBRACKET:      // Вставить сетевое (UNC) путь из активной панели
    case KEY_ALTSHIFTBACKBRACKET:  // Вставить сетевое (UNC) путь из пассивной панели
      NeedRealName=TRUE;
    case KEY_CTRLBRACKET:          // Вставить путь из левой панели
    case KEY_CTRLBACKBRACKET:      // Вставить путь из правой панели
    case KEY_CTRLSHIFTBRACKET:     // Вставить путь из активной панели
    case KEY_CTRLSHIFTBACKBRACKET: // Вставить путь из пассивной панели

    case KEY_CTRLSHIFTNUMENTER:       // Текущий файл с пасс.панели
    case KEY_SHIFTNUMENTER:           // Текущий файл с актив.панели
    case KEY_CTRLSHIFTENTER:       // Текущий файл с пасс.панели
    case KEY_SHIFTENTER:           // Текущий файл с актив.панели
    {
      Panel *SrcPanel=NULL;
      FilePanels *Cp=CtrlObject->Cp();
      switch(Key)
      {
        case KEY_CTRLALTBRACKET:
        case KEY_CTRLBRACKET:
          SrcPanel=Cp->LeftPanel;
          break;
        case KEY_CTRLALTBACKBRACKET:
        case KEY_CTRLBACKBRACKET:
          SrcPanel=Cp->RightPanel;
          break;
        case KEY_SHIFTNUMENTER:
        case KEY_SHIFTENTER:
        case KEY_ALTSHIFTBRACKET:
        case KEY_CTRLSHIFTBRACKET:
          SrcPanel=Cp->ActivePanel;
          break;
        case KEY_CTRLSHIFTNUMENTER:
        case KEY_CTRLSHIFTENTER:
        case KEY_ALTSHIFTBACKBRACKET:
        case KEY_CTRLSHIFTBACKBRACKET:
          SrcPanel=Cp->GetAnotherPanel(Cp->ActivePanel);
          break;
      }

      if (SrcPanel!=NULL)
      {
        if(Key == KEY_SHIFTENTER || Key == KEY_CTRLSHIFTENTER || Key == KEY_SHIFTNUMENTER || Key == KEY_CTRLSHIFTNUMENTER)
        {
          string strShortFileName;
          SrcPanel->GetCurName(strPathName,strShortFileName);
          if(SrcPanel->GetShowShortNamesMode()) // учтем короткость имен :-)
            strPathName = strShortFileName;
        }
        else
        {
          /* TODO: Здесь нужно учесть, что у TreeList тоже есть путь :-) */
          if (!(SrcPanel->GetType()==FILE_PANEL || SrcPanel->GetType()==TREE_PANEL))
            return(FALSE);

          SrcPanel->GetCurDir(strPathName);
          if (SrcPanel->GetMode()!=PLUGIN_PANEL)
          {
            FileList *SrcFilePanel=(FileList *)SrcPanel;
            SrcFilePanel->GetCurDir(strPathName);

            {
                if(NeedRealName)
                    SrcFilePanel->CreateFullPathName(strPathName, strPathName,FILE_ATTRIBUTE_DIRECTORY, strPathName,TRUE,ShortNameAsIs);
            }


            if (SrcFilePanel->GetShowShortNamesMode() && ShortNameAsIs)
              ConvertNameToShort(strPathName,strPathName);
          }
          else
          {
            FileList *SrcFilePanel=(FileList *)SrcPanel;
            struct OpenPluginInfo Info;

            CtrlObject->Plugins.GetOpenPluginInfo(SrcFilePanel->GetPluginHandle(),&Info);
            FileList::AddPluginPrefix(SrcFilePanel,strPathName);

            strPathName += Info.CurDir;

          }
          AddEndSlash(strPathName);
        }

        if(Opt.QuotedName&QUOTEDNAME_INSERT)
          QuoteSpace(strPathName);

        if ( Param2 )
            strPathName += Param2;

        RetCode=TRUE;
      }
    }
    break;
  }
  return RetCode;
}
