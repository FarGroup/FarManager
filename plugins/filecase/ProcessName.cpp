#include "FileCase.hpp"

//   Process case conversion of single file\directory
//     ProcessName - convert case of given filename
//                   use all options from `Opt`
//                   Call recurse for subdirectories
void ProcessName(const wchar_t *OldFullName, DWORD FileAttributes)
{
	wchar_t *NewFullName=new wchar_t[lstrlen(OldFullName)+8];
	lstrcpy(NewFullName, OldFullName);

	wchar_t *ExtPtr;

	// Path
	ExtPtr = wcsrchr(NewFullName,L'\\');

	if (ExtPtr)
		ExtPtr[1] = 0;
	else
		NewFullName[0] = 0;

	//Name
	ExtPtr=const_cast<wchar_t*>(FSF.PointToName(OldFullName));
	wchar_t *NewName=new wchar_t[lstrlen(ExtPtr)+1];
	lstrcpy(NewName,ExtPtr);

	//Ext
	bool dynExt=true;
	ExtPtr = wcsrchr(NewName,L'.');
	wchar_t *NewExt=nullptr;

	if (ExtPtr)
	{
		ExtPtr[0] = 0; //delete extension from name
		NewExt=new wchar_t[lstrlen(ExtPtr+1)+1];
		lstrcpy(NewExt,ExtPtr+1);
	}
	else
	{
		static wchar_t dummy[1]={};
		NewExt = dummy;
		dynExt = false;
	}

	if (*NewExt==0 && (*NewName==0 || (*NewName==L'.' && NewName[1]==0) || (*NewName==L'.' && NewName[1]==L'.' && NewName[2]==0)))
	{
		if (dynExt)
			delete[] NewExt;
		delete[] NewName;
		delete[] NewFullName;
		return;
	}

	//Check need to convert
	int mN = Opt.ConvertMode!=MODE_NONE && (!Opt.SkipMixedCase || !IsCaseMixed(NewName));
	int mE = Opt.ConvertModeExt!=MODE_NONE && (!Opt.SkipMixedCase || !IsCaseMixed(NewExt));

	//Case single file\dir
	if ((mN || mE) && (Opt.ProcessDir || (FileAttributes&FILE_ATTRIBUTE_DIRECTORY) == 0))
	{
		if (mN)
			CaseWord(NewName,Opt.ConvertMode);

		if (mE)
			CaseWord(NewExt,Opt.ConvertModeExt);

		lstrcat(NewFullName,NewName);

		if (NewExt[0])
		{
			lstrcat(NewFullName,L".");
			lstrcat(NewFullName,NewExt);
		}

		MoveFile(OldFullName,NewFullName);
	}

	if (dynExt)
		delete[] NewExt;
	delete[] NewName;
	delete[] NewFullName;

	//Recurce to directories
	if (Opt.ProcessSubDir && (FileAttributes&FILE_ATTRIBUTE_DIRECTORY))
	{
		PluginPanelItem *Items;
		size_t ItemsNumber,DirList;
		DirList = PsInfo.GetDirList(OldFullName,&Items,&ItemsNumber);

		if (DirList && ItemsNumber)
		{
			for (size_t I=0; I < ItemsNumber; I++)
			{
				ProcessName(Items[I].FileName,(DWORD)Items[I].FileAttributes);
			}
		}

		PsInfo.FreeDirList(Items, ItemsNumber);
	}
}
