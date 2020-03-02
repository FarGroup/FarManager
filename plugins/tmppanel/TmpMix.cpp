/*
TMPMIX.CPP

Temporary panel miscellaneous utility functions

*/

#include "plugin.hpp"

#include "TmpPanel.hpp"
#include <initguid.h>
#include "guid.hpp"

const wchar_t *GetMsg(int MsgId)
{
	return(Info.GetMsg(&MainGuid,MsgId));
}

void FreePanelItems(PluginPanelItem *Items, size_t Total)
{
	if (Items)
	{
		for (size_t I=0; I<Total; I++)
		{
			if (Items[I].Owner)
				free((void*)Items[I].Owner);

			if (Items[I].FileName)
				free((void*)Items[I].FileName);
		}

		free(Items);
	}
}

wchar_t *ParseParam(wchar_t *& str)
{
	wchar_t* p=str;
	wchar_t* parm=NULL;

	if (*p==L'|')
	{
		parm=++p;
		p=wcschr(p,L'|');

		if (p)
		{
			*p=L'\0';
			str=p+1;
			FSF.LTrim(str);
			return parm;
		}
	}

	return NULL;
}

void GoToFile(const wchar_t *Target, BOOL AnotherPanel)
{
	HANDLE  _PANEL_HANDLE = AnotherPanel?PANEL_PASSIVE:PANEL_ACTIVE;
	PanelRedrawInfo PRI = {sizeof(PanelRedrawInfo)};
	PanelInfo PInfo = {sizeof(PanelInfo)};
	int pathlen;
	const wchar_t *p = FSF.PointToName(const_cast<wchar_t*>(Target));
	StrBuf Name(lstrlen(p)+1);
	lstrcpy(Name,p);
	pathlen=(int)(p-Target);
	StrBuf Dir(pathlen+1);

	if (pathlen)
		memcpy(Dir.Ptr(),Target,pathlen*sizeof(wchar_t));

	Dir[pathlen]=L'\0';
	FSF.Trim(Name);
	FSF.Trim(Dir);
	FSF.Unquote(Name);
	FSF.Unquote(Dir);

	if (*Dir.Ptr())
	{
		FarPanelDirectory dirInfo = {sizeof(dirInfo), Dir, nullptr, {}, nullptr};
		Info.PanelControl(_PANEL_HANDLE, FCTL_SETPANELDIRECTORY, 0, &dirInfo);
	}

	Info.PanelControl(_PANEL_HANDLE,FCTL_GETPANELINFO,0,&PInfo);
	PRI.CurrentItem=PInfo.CurrentItem;
	PRI.TopPanelItem=PInfo.TopPanelItem;

	for (size_t J=0; J < PInfo.ItemsNumber; J++)
	{
		size_t Size=Info.PanelControl(_PANEL_HANDLE,FCTL_GETPANELITEM,J,0);
		PluginPanelItem* PPI=(PluginPanelItem*)malloc(Size);

		if (PPI)
		{
			FarGetPluginPanelItem gpi={sizeof(FarGetPluginPanelItem), Size, PPI};
			Info.PanelControl(_PANEL_HANDLE,FCTL_GETPANELITEM,J,&gpi);
		}


		if (!FSF.LStricmp(Name,FSF.PointToName((PPI?PPI->FileName:NULL))))
		{
			PRI.CurrentItem=J;
			PRI.TopPanelItem=J;
			free(PPI);
			break;
		}
		free(PPI);
	}

	Info.PanelControl(_PANEL_HANDLE,FCTL_REDRAWPANEL,0,&PRI);
}

void WFD2FFD(WIN32_FIND_DATA &wfd, PluginPanelItem &ffd)
{
	ffd.FileAttributes=wfd.dwFileAttributes;
	ffd.CreationTime=wfd.ftCreationTime;
	ffd.LastAccessTime=wfd.ftLastAccessTime;
	ffd.LastWriteTime=wfd.ftLastWriteTime;
	ffd.FileSize = wfd.nFileSizeHigh;
	ffd.FileSize <<= 32;
	ffd.FileSize |= wfd.nFileSizeLow;
	ffd.AllocationSize = 0;
	ffd.FileName = wcsdup(wfd.cFileName);
	ffd.AlternateFileName = NULL;
}

wchar_t* FormNtPath(const wchar_t* path, StrBuf& buf)
{
	int l = lstrlen(path);

	if (l > 4 && path[0] == L'\\' && path[1] == L'\\')
	{
		if ((path[2] == L'?' || path[2] == L'.') && path[3] == L'\\')
		{
			buf.Grow(l + 1);
			lstrcpy(buf, path);
		}
		else
		{
			buf.Grow(6 + l + 1);
			lstrcpy(buf, L"\\\\?\\UNC\\");
			lstrcat(buf, path + 2);
		}
	}
	else
	{
		buf.Grow(4 + l + 1);
		lstrcpy(buf, L"\\\\?\\");
		lstrcat(buf, path);
	}

	// slash -> backslash
	for (wchar_t* ch = buf; *ch; ch++)
		if (*ch == L'/')
			*ch = L'\\';

	return buf;
}

wchar_t* ExpandEnvStrs(const wchar_t* input, StrBuf& output)
{
	output.Grow(NT_MAX_PATH);
	size_t size = ExpandEnvironmentStrings(input, output, (DWORD)output.Size());

	if (size > output.Size())
	{
		output.Grow(size);
		size = ExpandEnvironmentStrings(input, output, (DWORD)output.Size());
	}

	if ((size == 0) || (size > output.Size()))
	{
		output.Grow(lstrlen(input) + 1);
		lstrcpy(output, input);
	}

	return output;
}

bool FindListFile(const wchar_t *FileName, StrBuf &output)
{
	StrBuf Path;
	DWORD dwSize;
	StrBuf FullPath;
	GetFullPath(FileName, FullPath);
	StrBuf NtPath;
	FormNtPath(FullPath, NtPath);
	const wchar_t *final=NULL;

	if (GetFileAttributes(NtPath) != INVALID_FILE_ATTRIBUTES)
	{
		output.Grow(FullPath.Size());
		lstrcpy(output, FullPath);
		return true;
	}

	{
		const wchar_t *tmp = FSF.PointToName(Info.ModuleName);
		Path.Grow(tmp-Info.ModuleName+1);
		lstrcpyn(Path,Info.ModuleName,(int)(tmp-Info.ModuleName+1));
		dwSize=SearchPath(Path,FileName,NULL,0,NULL,NULL);

		if (dwSize)
		{
			final = Path;
			goto success;
		}
	}

	ExpandEnvStrs(L"%FARHOME%;%PATH%",Path);

	for (wchar_t *str=Path, *p=wcschr(Path,L';'); *str; p=wcschr(str,L';'))
	{
		if (p)
			*p = 0;

		FSF.Unquote(str);
		FSF.Trim(str);

		if (*str)
		{
			dwSize=SearchPath(str,FileName,NULL,0,NULL,NULL);

			if (dwSize)
			{
				final = str;
				goto success;
			}
		}

		if (p)
			str = p+1;
		else
			break;
	}

	return false;
success:
	output.Grow(dwSize);
	SearchPath(final,FileName,NULL,dwSize,output,NULL);
	return true;
}

wchar_t* GetFullPath(const wchar_t* input, StrBuf& output)
{
	output.Grow(MAX_PATH);
	size_t size = FSF.ConvertPath(CPM_FULL, input, output, output.Size());

	if (size > output.Size())
	{
		output.Grow(size);
		FSF.ConvertPath(CPM_FULL, input, output, output.Size());
	}

	return output;
}

bool IsTextUTF8(const char* Buffer,size_t Length)
{
	bool Ascii=true;
	size_t Octets=0;
	size_t LastOctetsPos = 0;
	const size_t MaxCharSize = 4;

	for (size_t i=0; i<Length; i++)
	{
		BYTE c=Buffer[i];

		if (c&0x80)
			Ascii=false;

		if (Octets)
		{
			if ((c&0xC0)!=0x80)
				return false;

			Octets--;
		}
		else
		{
			LastOctetsPos = i;

			if (c&0x80)
			{
				while (c&0x80)
				{
					c <<= 1;
					Octets++;
				}

				Octets--;

				if (!Octets)
					return false;
			}
		}
	}

	return (!Octets || Length - LastOctetsPos < MaxCharSize) && !Ascii;
}
