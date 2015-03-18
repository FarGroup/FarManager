/*
treelist.cpp

Tree panel
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
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

#include "treelist.hpp"
#include "flink.hpp"
#include "keyboard.hpp"
#include "colors.hpp"
#include "keys.hpp"
#include "filepanels.hpp"
#include "filelist.hpp"
#include "cmdline.hpp"
#include "chgprior.hpp"
#include "scantree.hpp"
#include "copy.hpp"
#include "qview.hpp"
#include "savescr.hpp"
#include "ctrlobj.hpp"
#include "help.hpp"
#include "lockscrn.hpp"
#include "macroopcode.hpp"
#include "refreshwindowmanager.hpp"
#include "scrbuf.hpp"
#include "TPreRedrawFunc.hpp"
#include "TaskBar.hpp"
#include "cddrv.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "clipboard.hpp"
#include "config.hpp"
#include "delete.hpp"
#include "mkdir.hpp"
#include "setattr.hpp"
#include "execute.hpp"
#include "shortcuts.hpp"
#include "dirmix.hpp"
#include "pathmix.hpp"
#include "processname.hpp"
#include "constitle.hpp"
#include "syslog.hpp"
#include "cache.hpp"
#include "filestr.hpp"
#include "wakeful.hpp"
#include "colormix.hpp"
#include "FarGuid.hpp"
#include "plugins.hpp"
#include "manager.hpp"
#if defined(TREEFILE_PROJECT)
#include "cddrv.hpp"
#include "drivemix.hpp"
#include "network.hpp"
#endif
#include "language.hpp"
#include "keybar.hpp"

static bool StaticSortNumeric;
static bool StaticSortCaseSensitive;
static clock_t TreeStartTime;
static int LastScrX = -1;
static int LastScrY = -1;


/*
  Global->Opt->Tree.LocalDisk          Хранить файл структуры папок для локальных дисков
  Global->Opt->Tree.NetDisk            Хранить файл структуры папок для сетевых дисков
  Global->Opt->Tree.NetPath            Хранить файл структуры папок для сетевых путей
  Global->Opt->Tree.RemovableDisk      Хранить файл структуры папок для сменных дисков
  Global->Opt->Tree.CDDisk             Хранить файл структуры папок для CD/DVD/BD/etc дисков

  Global->Opt->Tree.strLocalDisk;      шаблон имени файла-деревяхи для локальных дисков
     constLocalDiskTemplate=L"LD.%D.%SN.tree"
  Global->Opt->Tree.strNetDisk;        шаблон имени файла-деревяхи для сетевых дисков
     constNetDiskTemplate=L"ND.%D.%SN.tree";
  Global->Opt->Tree.strNetPath;        шаблон имени файла-деревяхи для сетевых путей
     constNetPathTemplate=L"NP.%SR.%SH.tree";
  Global->Opt->Tree.strRemovableDisk;  шаблон имени файла-деревяхи для сменных дисков
     constRemovableDiskTemplate=L"RD.%SN.tree";
  Global->Opt->Tree.strCDDisk;         шаблон имени файла-деревяхи для CD/DVD/BD/etc дисков
     constCDDiskTemplate=L"CD.%L.%SN.tree";

     %D    - буква диска
     %SN   - серийный номер
     %L    - метка диска
     %SR   - server name
     %SH   - share name

  Global->Opt->Tree.strExceptPath;     // для перечисленных здесь не хранить

  Global->Opt->Tree.strSaveLocalPath;  // сюда сохраняем локальные диски
  Global->Opt->Tree.strSaveNetPath;    // сюда сохраняем сетевые диски
*/

#if defined(TREEFILE_PROJECT)
string& ConvertTemplateTreeName(string &strDest, const string &strTemplate, const wchar_t *D, DWORD SN, const wchar_t *L, const wchar_t *SR, const wchar_t *SH)
{
	strDest=strTemplate;
	FormatString strDiskNumber;

	strDiskNumber <<
				fmt::MinWidth(4) << fmt::FillChar(L'0') << fmt::Radix(16) << HIWORD(SN) << L'-' <<
				fmt::MinWidth(4) << fmt::FillChar(L'0') << fmt::Radix(16) << LOWORD(SN);
	/*
    	 %D    - буква диска
	     %SN   - серийный номер
    	 %L    - метка диска
	     %SR   - server name
    	 %SH   - share name
	*/
	string strDiskLetter(D ? D : L"", 1);
	ReplaceStrings(strDest, L"%D", strDiskLetter);
	ReplaceStrings(strDest, L"%SN", strDiskNumber);
	ReplaceStrings(strDest, L"%L", L && *L? L : L"");
	ReplaceStrings(strDest, L"%SR", SR && *SR? SR : L"");
	ReplaceStrings(strDest, L"%SH", SH && *SH? SH : L"");

	return strDest;
}
#endif

string& CreateTreeFileName(const string& Path, string &strDest)
{
#if defined(TREEFILE_PROJECT)
	string strRootDir = ExtractPathRoot(Path);
	string strTreeFileName;
	string strPath;
	strDest = L"";

	std::vector<string> Strings;
	split(Strings, Global->Opt->Tree.strExceptPath, STLF_UNIQUE, L";");
	FOR(const auto& i, Strings)
	{
		if (strRootDir == i)
		{
			return strDest;
		}
	}

	UINT DriveType = FAR_GetDriveType(strRootDir, 0);
	const auto PathType = ParsePath(strRootDir);
	/*
	PATH_UNKNOWN,
	PATH_DRIVELETTER,
	PATH_DRIVELETTERUNC,
	PATH_REMOTE,
	PATH_REMOTEUNC,
	PATH_VOLUMEGUID,
	PATH_PIPE,
	*/

	// получение инфы о томе
	string strVolumeName, strFileSystemName;
	DWORD MaxNameLength = 0, FileSystemFlags = 0, VolumeNumber = 0;

	if (api::GetVolumeInformation(strRootDir, &strVolumeName,
		&VolumeNumber, &MaxNameLength, &FileSystemFlags,
		&strFileSystemName))
	{
		if (DriveType == DRIVE_SUBSTITUTE) // Разворачиваем и делаем подмену
		{
			DriveType = DRIVE_FIXED; //????
		}

		switch (DriveType)
		{
		case DRIVE_USBDRIVE:
		case DRIVE_REMOVABLE:
			if (Global->Opt->Tree.RemovableDisk)
			{
				ConvertTemplateTreeName(strTreeFileName, Global->Opt->Tree.strRemovableDisk, strRootDir.data(), VolumeNumber, strVolumeName.data(), nullptr, nullptr);
				// TODO: Global->Opt->ProfilePath / Global->Opt->LocalProfilePath
				strPath = Global->Opt->Tree.strSaveLocalPath;
			}
			break;
		case DRIVE_FIXED:
			if (Global->Opt->Tree.LocalDisk)
			{
				ConvertTemplateTreeName(strTreeFileName, Global->Opt->Tree.strLocalDisk, strRootDir.data(), VolumeNumber, strVolumeName.data(), nullptr, nullptr);
				// TODO: Global->Opt->ProfilePath / Global->Opt->LocalProfilePath
				strPath = Global->Opt->Tree.strSaveLocalPath;
			}
			break;
		case DRIVE_REMOTE:
			if (Global->Opt->Tree.NetDisk || Global->Opt->Tree.NetPath)
			{
				string strServer, strShare;
				if (PathType == PATH_REMOTE)
				{
					CurPath2ComputerName(strRootDir, strServer, strShare);
					DeleteEndSlash(strShare);
				}

				ConvertTemplateTreeName(strTreeFileName, PathType == PATH_DRIVELETTER ? Global->Opt->Tree.strNetDisk : Global->Opt->Tree.strNetPath, strRootDir.data(), VolumeNumber, strVolumeName.data(), strServer.data(), strShare.data());
				// TODO: Global->Opt->ProfilePath / Global->Opt->LocalProfilePath
				strPath = Global->Opt->Tree.strSaveNetPath;
			}
			break;
		case DRIVE_CD_RW:
		case DRIVE_CD_RWDVD:
		case DRIVE_DVD_ROM:
		case DRIVE_DVD_RW:
		case DRIVE_DVD_RAM:
		case DRIVE_BD_ROM:
		case DRIVE_BD_RW:
		case DRIVE_HDDVD_ROM:
		case DRIVE_HDDVD_RW:
		case DRIVE_CDROM:
			if (Global->Opt->Tree.CDDisk)
			{
				ConvertTemplateTreeName(strTreeFileName, Global->Opt->Tree.strCDDisk, strRootDir.data(), VolumeNumber, strVolumeName.data(), nullptr, nullptr);
				// TODO: Global->Opt->ProfilePath / Global->Opt->LocalProfilePath
				strPath = Global->Opt->Tree.strSaveLocalPath;
			}
			break;
		case DRIVE_VIRTUAL:
		case DRIVE_RAMDISK:
			break;
		case DRIVE_REMOTE_NOT_CONNECTED:
		case DRIVE_NOT_INIT:
			break;

		}
		if ( !strPath.empty() )
		{
			strDest = api::env::expand_strings(strPath);
		}
		else
		{
			strDest = Path;
		}
		AddEndSlash(strDest);
		strDest += strTreeFileName;
	}
	else
	{
		strDest = Path;
		AddEndSlash(strDest);
		strDest += L"tree3.far";
	}

#else
	strDest = Path;
	AddEndSlash(strDest);
	strDest += L"tree3.far";
#endif
	return strDest;
}

// TODO: Файлы "Tree3.Far" для локальных дисков должны храниться в "Local AppData\Far Manager"
// TODO: Файлы "Tree3.Far" для сменных дисков должны храниться на самих "дисках"
// TODO: Файлы "Tree3.Far" для сетевых дисков должны храниться в "%HOMEDRIVE%\%HOMEPATH%",
//                        если эти переменные среды не определены, то "%APPDATA%\Far Manager"
string& MkTreeFileName(const string& RootDir, string &strDest)
{
	CreateTreeFileName(RootDir, strDest);
	return strDest;
}

// этому каталогу (Tree.Cache) место не в FarPath, а в "Local AppData\Far\"
string& MkTreeCacheFolderName(const string& RootDir, string &strDest)
{
#if defined(TREEFILE_PROJECT)
	// в проекте TREEFILE_PROJECT наличие каталога tree3.cache не предполагается
	CreateTreeFileName(RootDir, strDest);
#else
	strDest = RootDir;
	AddEndSlash(strDest);
	strDest += L"tree3.cache";
#endif
	return strDest;
}

int GetCacheTreeName(const string& Root, string& strName, int CreateDir)
{
	string strVolumeName, strFileSystemName;
	DWORD dwVolumeSerialNumber;

	if (!api::GetVolumeInformation(
		Root,
		&strVolumeName,
		&dwVolumeSerialNumber,
		nullptr,
		nullptr,
		&strFileSystemName
		))
		return FALSE;

	string strFolderName;
	MkTreeCacheFolderName(Global->Opt->LocalProfilePath, strFolderName);
#if defined(TREEFILE_PROJECT)
	if (strFolderName.empty())
		return FALSE;
#endif

	if (CreateDir)
	{
		api::CreateDirectory(strFolderName, nullptr);
		api::SetFileAttributes(strFolderName, Global->Opt->Tree.TreeFileAttr);
	}

	string strRemoteName;

	if (Root.front() == L'\\')
		strRemoteName = Root;
	else
	{
		string LocalName(L"?:");
		LocalName.front() = Root.front();
		api::WNetGetConnection(LocalName, strRemoteName);

		if (!strRemoteName.empty())
			AddEndSlash(strRemoteName);
	}

	std::replace(ALL_RANGE(strRemoteName), L'\\', L'_');
	strName = FormatString() << strFolderName << L"\\" << strVolumeName << L"." << fmt::Radix(16) << dwVolumeSerialNumber << L"." << strFileSystemName << L"." << strRemoteName;
	return TRUE;
}

static struct tree_less
{
	bool operator()(const string& a, const string& b, decltype(StrCmpNNI) comparer = StrCmpNNI) const
	{
		auto Str1 = a.data(), Str2 = b.data();

		if (*Str1 == L'\\' && *Str2 == L'\\')
		{
			Str1++;
			Str2++;
		}

		auto s1 = wcschr(Str1, L'\\'), s2 = wcschr(Str2, L'\\');

		while (s1 && s2)
		{
			int r = comparer(Str1, static_cast<int>(s1 - Str1), Str2, static_cast<int>(s2 - Str2));

			if (r)
				return r < 0;

			Str1 = s1 + 1;
			Str2 = s2 + 1;
			s1 = wcschr(Str1,L'\\');
			s2 = wcschr(Str2,L'\\');
		}

		if (s1 || s2)
		{
			int r = comparer(Str1, s1? static_cast<int>(s1 - Str1) : -1, Str2, s2? static_cast<int>(s2 - Str2) : -1);
			return r? r < 0 : !s1;
		}
		return comparer(Str1, -1, Str2, -1) < 0;
	}
} TreeLess;

static struct list_less
{
	bool operator()(const TreeList::TreeItem& a, const TreeList::TreeItem& b) const
	{
		const auto comparer = StaticSortNumeric? (StaticSortCaseSensitive ? NumStrCmpN : NumStrCmpNI) : (StaticSortCaseSensitive ? StrCmpNN : StrCmpNNI);

		return TreeLess(a.strName, b.strName, comparer);
	}
}
ListLess;

class TreeListCache: noncopyable
{
public:
	TreeListCache() {}
	TreeListCache(TreeListCache&& rhs) noexcept { *this = std::move(rhs); }

	MOVE_OPERATOR_BY_SWAP(TreeListCache);

	void swap(TreeListCache& rhs) noexcept
	{
		m_TreeName.swap(rhs.m_TreeName);
		m_Names.swap(rhs.m_Names);
	}

	FREE_SWAP(TreeListCache);

	void clear()
	{
		m_Names.clear();
		m_TreeName.clear();
	}

	bool empty() const { return m_Names.empty(); }

	void add(const wchar_t* Name) { m_Names.emplace(Name); }

	void add(string&& Name) { m_Names.emplace(std::move(Name)); }

	void remove(const wchar_t* Name)
	{
		size_t Length = wcslen(Name);

		FOR_RANGE(m_Names, i)
		{
			if (i->size() < Length)
				continue;

			if (!StrCmpNI(Name, i->data(), Length) && (i->size() == Length || IsSlash(i->at(Length))))
			{
				i = m_Names.erase(i);
				if (i == m_Names.end())
					break;
			}
		}
	}

	void rename(const wchar_t* OldName, const wchar_t* NewName)
	{
		size_t SrcLength = wcslen(OldName);
		FOR_RANGE(m_Names, i)
		{
			size_t iLen = i->size();
			if ((iLen == SrcLength || (iLen > SrcLength && IsSlash((*i)[SrcLength]))) && !StrCmpNI(OldName, i->data(), SrcLength))
			{
				string newName = string(NewName) + (i->data() + SrcLength);
				i = m_Names.erase(i);
				m_Names.insert(std::move(newName));
				if (i == m_Names.end())
					break;
			}
		}
	}

	const string& GetTreeName() const { return m_TreeName; }

	void SetTreeName(const string& Name) { m_TreeName = Name; }

private:
	struct cache_less
	{
		bool operator()(const string& a, const string& b) const { return TreeLess(a, b); }
	};

	typedef std::set<string, cache_less> cache_set;

public:
	typedef cache_set::const_iterator const_iterator;
	const_iterator begin() const { return m_Names.cbegin(); }
	const_iterator end() const { return m_Names.cend(); }

private:
	cache_set m_Names;
	string m_TreeName;
};

TreeListCache& TreeCache()
{
	static TreeListCache cache;
	return cache;
}

TreeListCache& tempTreeCache()
{
	static TreeListCache cache;
	return cache;
}

enum TREELIST_FLAGS
{
	FTREELIST_TREEISPREPARED = 0x00010000,
	FTREELIST_UPDATEREQUIRED = 0x00020000,
	FTREELIST_ISPANEL = 0x00040000,
};

TreeList::TreeList(window_ptr Owner, bool IsPanel):
	Panel(Owner),
	m_WorkDir(0),
	m_SavedWorkDir(0),
	m_GetSelPosition(0),
	m_ExitCode(1)
{
	m_Type=TREE_PANEL;
	m_CurFile=m_CurTopFile=0;
	m_Flags.Set(FTREELIST_UPDATEREQUIRED);
	m_Flags.Clear(FTREELIST_TREEISPREPARED);
	m_Flags.Change(FTREELIST_ISPANEL,IsPanel);
}

TreeList::~TreeList()
{
	tempTreeCache().clear();
	FlushCache();
}

void TreeList::SetRootDir(const string& NewRootDir)
{
	m_Root = NewRootDir;
	m_CurDir = NewRootDir;
}

void TreeList::DisplayObject()
{
	if (m_Flags.Check(FSCROBJ_ISREDRAWING))
		return;

	m_Flags.Set(FSCROBJ_ISREDRAWING);

	if (m_Flags.Check(FTREELIST_UPDATEREQUIRED))
		Update(0);

	if (m_ExitCode)
	{
		Panel *RootPanel=GetRootPanel();

		if (RootPanel->GetType()==FILE_PANEL)
		{
			bool RootCaseSensitiveSort=RootPanel->GetCaseSensitiveSort() != 0;
			bool RootNumeric=RootPanel->GetNumericSort() != 0;

			if (RootNumeric != m_NumericSort || RootCaseSensitiveSort!=m_CaseSensitiveSort)
			{
				m_NumericSort=RootNumeric;
				m_CaseSensitiveSort=RootCaseSensitiveSort;
				StaticSortNumeric=m_NumericSort;
				StaticSortCaseSensitive=m_CaseSensitiveSort;
				std::sort(m_ListData.begin(), m_ListData.end(), ListLess);
				FillLastData();
				SyncDir();
			}
		}

		DisplayTree(FALSE);
	}

	m_Flags.Clear(FSCROBJ_ISREDRAWING);
}

string TreeList::GetTitle() const
{
	string strTitle = L" ";
	strTitle += m_ModalMode? MSG(MFindFolderTitle) : MSG(MTreeTitle);
	strTitle += L" ";
	TruncStr(strTitle,m_X2-m_X1-3);
	return strTitle;
}

void TreeList::DisplayTree(int Fast)
{
	wchar_t TreeLineSymbol[4][3]=
	{
		{L' ',                  L' ',             0},
		{BoxSymbols[BS_V1],     L' ',             0},
		{BoxSymbols[BS_LB_H1V1],BoxSymbols[BS_H1],0},
		{BoxSymbols[BS_L_H1V1], BoxSymbols[BS_H1],0},
	};

	string strTitle;
	std::unique_ptr<LockScreen> LckScreen;

	if (!m_ModalMode && Parent()->GetAnotherPanel(this)->GetType() == QVIEW_PANEL)
		LckScreen = std::make_unique<LockScreen>();

	CorrectPosition();

	if (!m_ListData.empty())
		m_CurDir = m_ListData[m_CurFile].strName; //BUGBUG

//    xstrncpy(CurDir,ListData[CurFile].Name,sizeof(CurDir));
	if (!Fast)
	{
		Box(m_X1,m_Y1,m_X2,m_Y2,colors::PaletteColorToFarColor(COL_PANELBOX),DOUBLE_BOX);
		DrawSeparator(m_Y2-2-(m_ModalMode!=0));
		strTitle = GetTitle();

		if (!strTitle.empty())
		{
			SetColor((m_Focus || m_ModalMode) ? COL_PANELSELECTEDTITLE:COL_PANELTITLE);
			GotoXY(m_X1+(m_X2-m_X1+1-(int)strTitle.size())/2,m_Y1);
			Text(strTitle);
		}
	}

	for (size_t I=m_Y1+1,J=m_CurTopFile; I<static_cast<size_t>(m_Y2-2-(m_ModalMode!=0)); I++,J++)
	{
		GotoXY(m_X1+1, static_cast<int>(I));
		SetColor(COL_PANELTEXT);
		Text(L' ');

		if (J < m_ListData.size() && m_Flags.Check(FTREELIST_TREEISPREPARED))
		{
			auto& CurPtr=m_ListData[J];

			if (!J)
			{
				DisplayTreeName(L"\\",J);
			}
			else
			{
				string strOutStr;

				for (size_t i=0; i<CurPtr.Depth-1 && WhereX() + 3 * i < m_X2 - 6u; i++)
				{
					strOutStr+=TreeLineSymbol[CurPtr.Last[i]?0:1];
				}

				strOutStr+=TreeLineSymbol[CurPtr.Last[CurPtr.Depth-1]?2:3];
				BoxText(strOutStr);
				const wchar_t *ChPtr=LastSlash(CurPtr.strName.data());

				if (ChPtr)
					DisplayTreeName(ChPtr+1,J);
			}
		}

		SetColor(COL_PANELTEXT);

		if (WhereX()<m_X2)
		{
			Global->FS << fmt::MinWidth(m_X2-WhereX())<<L"";
		}
	}

	if (Global->Opt->ShowPanelScrollbar)
	{
		SetColor(COL_PANELSCROLLBAR);
		ScrollBarEx(m_X2, m_Y1+1, m_Y2-m_Y1-3, m_CurTopFile, m_ListData.size());
	}

	SetColor(COL_PANELTEXT);
	SetScreen(m_X1+1,m_Y2-(m_ModalMode?2:1),m_X2-1,m_Y2-1,L' ',colors::PaletteColorToFarColor(COL_PANELTEXT));

	if (!m_ListData.empty())
	{
		GotoXY(m_X1+1,m_Y2-1);
		Global->FS << fmt::LeftAlign()<<fmt::ExactWidth(m_X2-m_X1-1)<<m_ListData[m_CurFile].strName;
	}

	UpdateViewPanel();
	SetTitle(); // не забудем прорисовать заголовок
}

void TreeList::DisplayTreeName(const wchar_t *Name, size_t Pos)
{
	if (WhereX()>m_X2-4)
		GotoXY(m_X2-4,WhereY());

	if (Pos==static_cast<size_t>(m_CurFile))
	{
		GotoXY(WhereX()-1,WhereY());

		if (m_Focus || m_ModalMode)
		{
			SetColor((Pos==m_WorkDir) ? COL_PANELSELECTEDCURSOR:COL_PANELCURSOR);
			Global->FS << L" "<<fmt::MaxWidth(m_X2-WhereX()-3)<<Name<<L" ";
		}
		else
		{
			SetColor((Pos==m_WorkDir) ? COL_PANELSELECTEDTEXT:COL_PANELTEXT);
			Global->FS << L"["<<fmt::MaxWidth(m_X2-WhereX()-3)<<Name<<L"]";
		}
	}
	else
	{
		SetColor((Pos==m_WorkDir) ? COL_PANELSELECTEDTEXT:COL_PANELTEXT);
		Global->FS << fmt::MaxWidth(m_X2-WhereX()-1)<<Name;
	}
}

void TreeList::Update(int Mode)
{
	if (!m_EnableUpdate)
		return;

	if (!IsVisible())
	{
		m_Flags.Set(FTREELIST_UPDATEREQUIRED);
		return;
	}

	m_Flags.Clear(FTREELIST_UPDATEREQUIRED);
	GetRoot();
	size_t LastTreeCount = m_ListData.size();
	int RetFromReadTree=TRUE;
	m_Flags.Clear(FTREELIST_TREEISPREPARED);
	int TreeFilePresent=ReadTreeFile();

	if (!TreeFilePresent)
		RetFromReadTree=ReadTree();

	m_Flags.Set(FTREELIST_TREEISPREPARED);

	if (!RetFromReadTree && !m_Flags.Check(FTREELIST_ISPANEL))
	{
		m_ExitCode=0;
		return;
	}

	if (RetFromReadTree && !m_ListData.empty() && (!(Mode & UPDATE_KEEP_SELECTION) || LastTreeCount != m_ListData.size()))
	{
		SyncDir();
		auto& CurPtr=m_ListData[m_CurFile];

		if (!api::fs::exists(CurPtr.strName))
		{
			DelTreeName(CurPtr.strName);
			Update(UPDATE_KEEP_SELECTION);
			Show();
		}
	}
	else if (!RetFromReadTree)
	{
		Show();

		if (!m_Flags.Check(FTREELIST_ISPANEL))
		{
			auto AnotherPanel = Parent()->GetAnotherPanel(this);
			AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
			AnotherPanel->Redraw();
		}
	}
}

static void PR_MsgReadTree();

struct TreePreRedrawItem: public PreRedrawItem
{
	TreePreRedrawItem():
		PreRedrawItem(PR_MsgReadTree),
		TreeCount()
	{}

	size_t TreeCount;
};

static int MsgReadTree(size_t TreeCount, int FirstCall)
{
	/* $ 24.09.2001 VVM
	! Писать сообщение о чтении дерева только, если это заняло более 500 мсек. */
	BOOL IsChangeConsole = LastScrX != ScrX || LastScrY != ScrY;

	if (IsChangeConsole)
	{
		LastScrX = ScrX;
		LastScrY = ScrY;
	}

	if (IsChangeConsole || (clock() - TreeStartTime) > CLOCKS_PER_SEC)
	{
		Message((FirstCall? 0 : MSG_KEEPBACKGROUND), 0, MSG(MTreeTitle), MSG(MReadingTree), std::to_wstring(TreeCount).data());
		if (!PreRedrawStack().empty())
		{
			auto item = dynamic_cast<TreePreRedrawItem*>(PreRedrawStack().top());
			item->TreeCount = TreeCount;
		}
		TreeStartTime = clock();
	}

	return 1;
}

static void PR_MsgReadTree()
{
	if (!PreRedrawStack().empty())
	{
		int FirstCall = 1;
		auto item = dynamic_cast<const TreePreRedrawItem*>(PreRedrawStack().top());
		MsgReadTree(item->TreeCount, FirstCall);
	}
}

static api::fs::file OpenTreeFile(const string& Name, bool Writable)
{
	api::fs::file Result;
	Result.Open(Name, Writable? FILE_WRITE_DATA : FILE_READ_DATA, FILE_SHARE_READ, nullptr, Writable? OPEN_ALWAYS : OPEN_EXISTING);
	return std::move(Result);
}

static bool MustBeCached(const string& Root)
{
	auto type = FAR_GetDriveType(Root);

	if (type==DRIVE_UNKNOWN || type==DRIVE_NO_ROOT_DIR || type==DRIVE_REMOVABLE || IsDriveTypeCDROM(type))
	{
		// кешируются CD, removable и неизвестно что :)
		return true;
	}

	/* остались
	    DRIVE_REMOTE
	    DRIVE_RAMDISK
	    DRIVE_FIXED
	*/
	return false;
}

static api::fs::file OpenCacheableTreeFile(const string& Root, string& Name, bool Writable)
{
	api::fs::file Result;
	if (!MustBeCached(Root))
		Result = OpenTreeFile(Name, Writable);

	if (!Result.Opened())
	{
		if (GetCacheTreeName(Root, Name, Writable))
		{
			Result = OpenTreeFile(Name, Writable);
		}
	}
	return std::move(Result);
}

static void ReadLines(api::fs::file& TreeFile, const std::function<void(string&)>& Inserter)
{
	GetFileString GetStr(TreeFile, CP_UNICODE);
	string Record;
	while (GetStr.GetString(Record))
	{
		if (Record.empty() || !IsSlash(Record.front()))
			continue;

		size_t pos = Record.find(L'\n');

		if (pos != string::npos)
			Record.resize(pos);

		Inserter(Record);
	}
}

template<class string_type, class container_type, class opener_type>
static inline void WriteTree(string_type& Name, const container_type& Container, const opener_type& Opener, size_t offset)
{
	// получим и сразу сбросим атрибуты (если получится)
	DWORD SavedAttributes = api::GetFileAttributes(Name);

	if (SavedAttributes != INVALID_FILE_ATTRIBUTES)
		api::SetFileAttributes(Name, FILE_ATTRIBUTE_NORMAL);

	api::fs::file TreeFile = Opener(Name);

	bool Result = false;

	if (TreeFile.Opened())
	{
		CachedWrite Cache(TreeFile);

		const auto WriteLine = [&](const string& str)
		{
			return Cache.Write(str.data() + offset, (str.size() - offset) * sizeof(wchar_t)) && Cache.Write(L"\n", 1 * sizeof(wchar_t));
		};

		Result = std::all_of(ALL_RANGE(Container), WriteLine) && Cache.Flush();

		if (!Result)
			Global->CatchError();

		TreeFile.SetEnd();
		TreeFile.Close();
	}
	else
	{
		Global->CatchError();
	}

	if (Result)
	{
		if (SavedAttributes != INVALID_FILE_ATTRIBUTES) // вернем атрибуты (если получится :-)
			api::SetFileAttributes(Name, SavedAttributes);
	}
	else
	{
		api::DeleteFile(TreeCache().GetTreeName());
		if (!Global->WindowManager->ManagerIsDown())
			Message(MSG_WARNING | MSG_ERRORTYPE, 1, MSG(MError), MSG(MCannotSaveTree), Name.data(), MSG(MOk));
	}
}

int TreeList::ReadTree()
{
	SCOPED_ACTION(ChangePriority)(THREAD_PRIORITY_NORMAL);
	//SaveScreen SaveScr;
	SCOPED_ACTION(TPreRedrawFuncGuard)(std::make_unique<TreePreRedrawItem>());
	ScanTree ScTree(false);
	api::FAR_FIND_DATA fdata;
	string strFullName;
	FlushCache();
	SaveState();
	GetRoot();

	m_ListData.clear();

	m_ListData.reserve(4096);

	m_ListData.emplace_back(m_Root);
	SaveScreen SaveScrTree;
	UndoGlobalSaveScrPtr UndSaveScr(&SaveScrTree);
	/* Т.к. мы можем вызвать диалог подтверждения (который не перерисовывает панельки,
	   а восстанавливает сохраненный образ экрана, то нарисуем чистую панель */
	//Redraw();
	int FirstCall=TRUE, AscAbort=FALSE;
	TreeStartTime = clock();
	RefreshWindowManager frref(ScrX,ScrY);
	ScTree.SetFindPath(m_Root, L"*", 0);
	LastScrX = ScrX;
	LastScrY = ScrY;
	SCOPED_ACTION(IndeterminateTaskBar);
	SCOPED_ACTION(wakeful);
	while (ScTree.GetNextName(&fdata,strFullName))
	{
		MsgReadTree(m_ListData.size(), FirstCall);

		if (CheckForEscSilent())
		{
			// BUGBUG, Dialog calls Commit, TreeList redraws and crashes.
			auto f = Global->WindowManager->GetCurrentWindow();
			if (f)
				f->Lock();

			AscAbort=ConfirmAbortOp();

			if (f)
				f->Unlock();

			FirstCall=TRUE;
		}

		if (AscAbort)
			break;

		if (!(fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			continue;

		m_ListData.emplace_back(strFullName);
	}

	if (AscAbort && !m_Flags.Check(FTREELIST_ISPANEL))
	{
		m_ListData.clear();
		RestoreState();
		return FALSE;
	}

	StaticSortNumeric = m_NumericSort = StaticSortCaseSensitive = m_CaseSensitiveSort = false;
	std::sort(m_ListData.begin(), m_ListData.end(), ListLess);

	if (!FillLastData())
		return FALSE;

	if (!AscAbort)
		SaveTreeFile();

	if (!FirstCall && !m_Flags.Check(FTREELIST_ISPANEL))
	{
		// Перерисуем другую панель - удалим следы сообщений :)
		Parent()->GetAnotherPanel(this)->Redraw();
	}

	return TRUE;
}

void TreeList::SaveTreeFile()
{
	if (m_ListData.size() < static_cast<size_t>(Global->Opt->Tree.MinTreeCount))
		return;

	string strName;

	size_t RootLength=m_Root.empty()?0:m_Root.size()-1;
	MkTreeFileName(m_Root, strName);
#if defined(TREEFILE_PROJECT)
	if (strName.empty())
		return;
#endif

	const auto Opener = [&](string& Name) { return OpenCacheableTreeFile(m_Root, Name, true); };

	WriteTree(strName, m_ListData, Opener, RootLength);

#if defined(TREEFILE_PROJECT)
	api::SetFileAttributes(strName, Global->Opt->Tree.TreeFileAttr);
#endif

}

void TreeList::GetRoot()
{
	m_Root = ExtractPathRoot(GetRootPanel()->GetCurDir());
}

Panel* TreeList::GetRootPanel()
{
	Panel *RootPanel;

	if (m_ModalMode) // watch out, Parent() in nullptr
	{
		if (m_ModalMode==MODALTREE_ACTIVE)
			RootPanel = Global->CtrlObject->Cp()->ActivePanel();
		else if (m_ModalMode==MODALTREE_FREE)
			RootPanel=this;
		else
		{
			RootPanel = Global->CtrlObject->Cp()->PassivePanel();

			if (!RootPanel->IsVisible())
				RootPanel = Global->CtrlObject->Cp()->ActivePanel();
		}
	}
	else
		RootPanel = Parent()->GetAnotherPanel(this);

	return RootPanel;
}

void TreeList::SyncDir()
{
	Panel *AnotherPanel=GetRootPanel();
	string strPanelDir(AnotherPanel->GetCurDir());

	if (!strPanelDir.empty())
	{
		if (AnotherPanel->GetType()==FILE_PANEL)
		{
			if (!SetDirPosition(strPanelDir))
			{
				ReadSubTree(strPanelDir);
				ReadTreeFile();
				SetDirPosition(strPanelDir);
			}
		}
		else
			SetDirPosition(strPanelDir);
	}
}

bool TreeList::FillLastData()
{
	const auto CountSlash = [](const wchar_t *Str) -> size_t
	{
		auto str = as_string(Str);
		return std::count_if(ALL_CONST_RANGE(str), IsSlash);
	};

	size_t RootLength = m_Root.empty()? 0 : m_Root.size()-1;
	auto Range = make_range(m_ListData.begin() + 1, m_ListData.end());
	FOR_RANGE(Range, i)
	{
		size_t Pos = i->strName.rfind(L'\\');
		int PathLength = Pos != string::npos? (int)Pos+1 : 0;

		size_t Depth = i->Depth=CountSlash(i->strName.data()+RootLength);

		if (!Depth)
			return false;

		auto SubDirPos = i;
		int Last = 1;

		auto SubRange = make_range(i + 1, Range.end());
		FOR_RANGE(SubRange, j)
		{
			if (CountSlash(j->strName.data()+RootLength)>Depth)
			{
				SubDirPos = j;
				continue;
			}
			else
			{
				if (!StrCmpNI(i->strName.data(), j->strName.data(), PathLength))
					Last=0;
				break;
			}
		}

		for (auto j = i; j != SubDirPos + 1; ++j)
		{
 			if (Depth > j->Last.size())
			{
				j->Last.resize(j->Last.size() + MAX_PATH, 0);
			}
			j->Last[Depth-1]=Last;
		}
	}
	return true;
}

__int64 TreeList::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	switch (OpCode)
	{
		case MCODE_C_EMPTY:
			return m_ListData.empty();
		case MCODE_C_EOF:
			return static_cast<size_t>(m_CurFile) == m_ListData.size() - 1;
		case MCODE_C_BOF:
			return !m_CurFile;
		case MCODE_C_SELECTED:
			return 0;
		case MCODE_V_ITEMCOUNT:
			return m_ListData.size();
		case MCODE_V_CURPOS:
			return m_CurFile+1;
	}

	return 0;
}

int TreeList::ProcessKey(const Manager::Key& Key)
{
    int LocalKey=Key.FarKey();
	if (!IsVisible())
		return FALSE;

	if (m_ListData.empty() && LocalKey!=KEY_CTRLR && LocalKey!=KEY_RCTRLR)
		return FALSE;

	if ((LocalKey>=KEY_CTRLSHIFT0 && LocalKey<=KEY_CTRLSHIFT9) || (LocalKey>=KEY_CTRLALT0 && LocalKey<=KEY_CTRLALT9))
	{
		bool Add = (LocalKey>=KEY_CTRLALT0 && LocalKey<=KEY_CTRLALT9);
		SaveShortcutFolder(LocalKey-(Add?KEY_CTRLALT0:KEY_CTRLSHIFT0), Add);
		return TRUE;
	}

	if (LocalKey>=KEY_RCTRL0 && LocalKey<=KEY_RCTRL9)
	{
		ExecShortcutFolder(LocalKey-KEY_RCTRL0);
		return TRUE;
	}

	switch (LocalKey)
	{
		case KEY_F1:
		{
			Help::create(L"TreePanel");
			return TRUE;
		}
		case KEY_SHIFTNUMENTER:
		case KEY_CTRLNUMENTER:
		case KEY_RCTRLNUMENTER:
		case KEY_SHIFTENTER:
		case KEY_CTRLENTER:
		case KEY_RCTRLENTER:
		case KEY_CTRLF:
		case KEY_RCTRLF:
		case KEY_CTRLALTINS:
		case KEY_RCTRLRALTINS:
		case KEY_CTRLRALTINS:
		case KEY_RCTRLALTINS:
		case KEY_CTRLALTNUMPAD0:
		case KEY_RCTRLRALTNUMPAD0:
		case KEY_CTRLRALTNUMPAD0:
		case KEY_RCTRLALTNUMPAD0:
		{
			string strQuotedName=m_ListData[m_CurFile].strName;
			QuoteSpace(strQuotedName);

			if (LocalKey==KEY_CTRLALTINS||LocalKey==KEY_RCTRLRALTINS||LocalKey==KEY_CTRLRALTINS||LocalKey==KEY_RCTRLALTINS||
				LocalKey==KEY_CTRLALTNUMPAD0||LocalKey==KEY_RCTRLRALTNUMPAD0||LocalKey==KEY_CTRLRALTNUMPAD0||LocalKey==KEY_RCTRLALTNUMPAD0)
			{
				SetClipboard(strQuotedName);
			}
			else
			{
				if (LocalKey == KEY_SHIFTENTER||LocalKey == KEY_SHIFTNUMENTER)
				{
					Execute(strQuotedName, false, true, true, true);
				}
				else
				{
					strQuotedName+=L" ";
					Parent()->GetCmdLine()->InsertString(strQuotedName);
				}
			}

			return TRUE;
		}
		case KEY_CTRLBACKSLASH:
		case KEY_RCTRLBACKSLASH:
		{
			m_CurFile=0;
			ProcessEnter();
			return TRUE;
		}
		case KEY_NUMENTER:
		case KEY_ENTER:
		{
			if (!m_ModalMode && Parent()->GetCmdLine()->GetLength()>0)
				break;

			ProcessEnter();
			return TRUE;
		}
		case KEY_F4:
		case KEY_CTRLA:
		case KEY_RCTRLA:
		{
			if (SetCurPath())
				ShellSetFileAttributes(this);

			return TRUE;
		}
		case KEY_CTRLR:
		case KEY_RCTRLR:
		{
			ReadTree();

			if (!m_ListData.empty())
				SyncDir();

			Redraw();
			break;
		}
		case KEY_SHIFTF5:
		case KEY_SHIFTF6:
		{
			if (SetCurPath())
			{
				int ToPlugin=0;
				ShellCopy ShCopy(this,LocalKey==KEY_SHIFTF6,FALSE,TRUE,TRUE,ToPlugin,nullptr);
			}

			return TRUE;
		}
		case KEY_F5:
		case KEY_DRAGCOPY:
		case KEY_F6:
		case KEY_ALTF6:
		case KEY_RALTF6:
		case KEY_DRAGMOVE:
		{
			if (!m_ListData.empty() && SetCurPath())
			{
				auto AnotherPanel = Parent()->GetAnotherPanel(this);
				int Ask=((LocalKey!=KEY_DRAGCOPY && LocalKey!=KEY_DRAGMOVE) || Global->Opt->Confirm.Drag);
				int Move=(LocalKey==KEY_F6 || LocalKey==KEY_DRAGMOVE);
				int ToPlugin=AnotherPanel->GetMode()==PLUGIN_PANEL &&
				             AnotherPanel->IsVisible() &&
				             !Global->CtrlObject->Plugins->UseFarCommand(AnotherPanel->GetPluginHandle(),PLUGIN_FARPUTFILES);
				int Link=((LocalKey==KEY_ALTF6||LocalKey==KEY_RALTF6) && !ToPlugin);

				if ((LocalKey==KEY_ALTF6||LocalKey==KEY_RALTF6) && !Link) // молча отвалим :-)
					return TRUE;

				{
					ShellCopy ShCopy(this,Move,Link,FALSE,Ask,ToPlugin,nullptr);
				}

				if (ToPlugin==1)
				{
					PluginPanelItem Item;
					int ItemNumber=1;
					auto hAnotherPlugin=AnotherPanel->GetPluginHandle();
					FileList::FileNameToPluginItem(m_ListData[m_CurFile].strName, &Item);
					int PutCode=Global->CtrlObject->Plugins->PutFiles(hAnotherPlugin, &Item, ItemNumber, Move != 0, 0);

					if (PutCode==1 || PutCode==2)
						AnotherPanel->SetPluginModified();

					if (Move)
						ReadSubTree(m_ListData[m_CurFile].strName);

					Update(0);
					Redraw();
					AnotherPanel->Update(UPDATE_KEEP_SELECTION);
					AnotherPanel->Redraw();
				}
			}

			return TRUE;
		}
		case KEY_F7:
		{
			if (SetCurPath())
				ShellMakeDir(this);

			return TRUE;
		}
		/*
		  Удаление                                   Shift-Del, Shift-F8, F8

		  Удаление файлов и папок. F8 и Shift-Del удаляют все выбранные
		 файлы, Shift-F8 - только файл под курсором. Shift-Del всегда удаляет
		 файлы, не используя Корзину (Recycle Bin). Использование Корзины
		 командами F8 и Shift-F8 зависит от конфигурации.

		  Уничтожение файлов и папок                                 Alt-Del
		*/
		case KEY_F8:
		case KEY_SHIFTDEL:
		case KEY_SHIFTNUMDEL:
		case KEY_SHIFTDECIMAL:
		case KEY_ALTNUMDEL:
		case KEY_RALTNUMDEL:
		case KEY_ALTDECIMAL:
		case KEY_RALTDECIMAL:
		case KEY_ALTDEL:
		case KEY_RALTDEL:
		{
			if (SetCurPath())
			{
				bool SaveOpt=Global->Opt->DeleteToRecycleBin;

				if (LocalKey==KEY_SHIFTDEL||LocalKey==KEY_SHIFTNUMDEL||LocalKey==KEY_SHIFTDECIMAL)
					Global->Opt->DeleteToRecycleBin=0;

				ShellDelete(this,LocalKey==KEY_ALTDEL||LocalKey==KEY_RALTDEL||LocalKey==KEY_ALTNUMDEL||LocalKey==KEY_RALTNUMDEL||LocalKey==KEY_ALTDECIMAL||LocalKey==KEY_RALTDECIMAL);
				// Надобно не забыть обновить противоположную панель...
				auto AnotherPanel = Parent()->GetAnotherPanel(this);
				AnotherPanel->Update(UPDATE_KEEP_SELECTION);
				AnotherPanel->Redraw();
				Global->Opt->DeleteToRecycleBin=SaveOpt;

				if (Global->Opt->Tree.AutoChangeFolder && !m_ModalMode)
					ProcessKey(Manager::Key(KEY_ENTER));
			}

			return TRUE;
		}
		case KEY_MSWHEEL_UP:
		case(KEY_MSWHEEL_UP | KEY_ALT):
		case(KEY_MSWHEEL_UP | KEY_RALT):
		{
			Scroll(LocalKey & (KEY_ALT|KEY_RALT)?-1:(int)-Global->Opt->MsWheelDelta);
			return TRUE;
		}
		case KEY_MSWHEEL_DOWN:
		case(KEY_MSWHEEL_DOWN | KEY_ALT):
		case(KEY_MSWHEEL_DOWN | KEY_RALT):
		{
			Scroll(LocalKey & (KEY_ALT|KEY_RALT)?1:(int)Global->Opt->MsWheelDelta);
			return TRUE;
		}
		case KEY_MSWHEEL_LEFT:
		case(KEY_MSWHEEL_LEFT | KEY_ALT):
		case(KEY_MSWHEEL_LEFT | KEY_RALT):
		{
			int Roll = LocalKey & (KEY_ALT|KEY_RALT)?1:(int)Global->Opt->MsHWheelDelta;

			for (int i=0; i<Roll; i++)
				ProcessKey(Manager::Key(KEY_LEFT));

			return TRUE;
		}
		case KEY_MSWHEEL_RIGHT:
		case(KEY_MSWHEEL_RIGHT | KEY_ALT):
		case(KEY_MSWHEEL_RIGHT | KEY_RALT):
		{
			int Roll = LocalKey & (KEY_ALT|KEY_RALT)?1:(int)Global->Opt->MsHWheelDelta;

			for (int i=0; i<Roll; i++)
				ProcessKey(Manager::Key(KEY_RIGHT));

			return TRUE;
		}
		case KEY_HOME:        case KEY_NUMPAD7:
		{
			Up(0x7fffff);

			if (Global->Opt->Tree.AutoChangeFolder && !m_ModalMode)
				ProcessKey(Manager::Key(KEY_ENTER));

			return TRUE;
		}
		case KEY_ADD: // OFM: Gray+/Gray- navigation
		{
			m_CurFile=GetNextNavPos();

			if (Global->Opt->Tree.AutoChangeFolder && !m_ModalMode)
				ProcessKey(Manager::Key(KEY_ENTER));
			else
				DisplayTree(TRUE);

			return TRUE;
		}
		case KEY_SUBTRACT: // OFM: Gray+/Gray- navigation
		{
			m_CurFile=GetPrevNavPos();

			if (Global->Opt->Tree.AutoChangeFolder && !m_ModalMode)
				ProcessKey(Manager::Key(KEY_ENTER));
			else
				DisplayTree(TRUE);

			return TRUE;
		}
		case KEY_END:         case KEY_NUMPAD1:
		{
			Down(0x7fffff);

			if (Global->Opt->Tree.AutoChangeFolder && !m_ModalMode)
				ProcessKey(Manager::Key(KEY_ENTER));

			return TRUE;
		}
		case KEY_UP:          case KEY_NUMPAD8:
		{
			Up(1);

			if (Global->Opt->Tree.AutoChangeFolder && !m_ModalMode)
				ProcessKey(Manager::Key(KEY_ENTER));

			return TRUE;
		}
		case KEY_DOWN:        case KEY_NUMPAD2:
		{
			Down(1);

			if (Global->Opt->Tree.AutoChangeFolder && !m_ModalMode)
				ProcessKey(Manager::Key(KEY_ENTER));

			return TRUE;
		}
		case KEY_PGUP:        case KEY_NUMPAD9:
		{
			m_CurTopFile-=m_Y2-m_Y1-3-(m_ModalMode!=0);
			m_CurFile-=m_Y2-m_Y1-3-(m_ModalMode!=0);
			DisplayTree(TRUE);

			if (Global->Opt->Tree.AutoChangeFolder && !m_ModalMode)
				ProcessKey(Manager::Key(KEY_ENTER));

			return TRUE;
		}
		case KEY_PGDN:        case KEY_NUMPAD3:
		{
			m_CurTopFile+=m_Y2-m_Y1-3-(m_ModalMode!=0);
			m_CurFile+=m_Y2-m_Y1-3-(m_ModalMode!=0);
			DisplayTree(TRUE);

			if (Global->Opt->Tree.AutoChangeFolder && !m_ModalMode)
				ProcessKey(Manager::Key(KEY_ENTER));

			return TRUE;
		}

		case KEY_APPS:
		case KEY_SHIFTAPPS:
		{
			//вызовем EMenu если он есть
			if (Global->CtrlObject->Plugins->FindPlugin(Global->Opt->KnownIDs.Emenu.Id))
			{
				Global->CtrlObject->Plugins->CallPlugin(Global->Opt->KnownIDs.Emenu.Id, OPEN_FILEPANEL, ToPtr(1)); // EMenu Plugin :-)
			}
			return TRUE;
		}

		default:
			if ((LocalKey>=KEY_ALT_BASE+0x01 && LocalKey<=KEY_ALT_BASE+65535) || (LocalKey>=KEY_RALT_BASE+0x01 && LocalKey<=KEY_RALT_BASE+65535) ||
			        (LocalKey>=KEY_ALTSHIFT_BASE+0x01 && LocalKey<=KEY_ALTSHIFT_BASE+65535) || (LocalKey>=KEY_RALTSHIFT_BASE+0x01 && LocalKey<=KEY_RALTSHIFT_BASE+65535))
			{
				FastFind(LocalKey);

				if (Global->Opt->Tree.AutoChangeFolder && !m_ModalMode)
					ProcessKey(Manager::Key(KEY_ENTER));
			}
			else
				break;

			return TRUE;
	}

	return FALSE;
}

int TreeList::GetNextNavPos() const
{
	int NextPos=m_CurFile;

	if (static_cast<size_t>(m_CurFile+1) < m_ListData.size())
	{
		auto CurDepth=m_ListData[m_CurFile].Depth;

		for (size_t I=m_CurFile+1; I < m_ListData.size(); ++I)
			if (m_ListData[I].Depth == CurDepth)
			{
				NextPos=static_cast<int>(I);
				break;
			}
	}

	return NextPos;
}

int TreeList::GetPrevNavPos() const
{
	int PrevPos=m_CurFile;

	if (m_CurFile-1 > 0)
	{
		auto CurDepth=m_ListData[m_CurFile].Depth;

		for (int I=m_CurFile-1; I > 0; --I)
			if (m_ListData[I].Depth == CurDepth)
			{
				PrevPos=I;
				break;
			}
	}

	return PrevPos;
}

void TreeList::Up(int Count)
{
	m_CurFile-=Count;
	DisplayTree(TRUE);
}

void TreeList::Down(int Count)
{
	m_CurFile+=Count;
	DisplayTree(TRUE);
}

void TreeList::Scroll(int Count)
{
	m_CurFile+=Count;
	m_CurTopFile+=Count;
	DisplayTree(TRUE);
}

void TreeList::CorrectPosition()
{
	if (m_ListData.empty())
	{
		m_CurFile=m_CurTopFile=0;
		return;
	}

	int Height=m_Y2-m_Y1-3-(m_ModalMode!=0);

	if (m_CurTopFile+Height > static_cast<int>(m_ListData.size()))
		m_CurTopFile = static_cast<int>(m_ListData.size() - Height);

	if (m_CurFile<0)
		m_CurFile=0;

	if (m_CurFile > static_cast<int>(m_ListData.size() - 1))
		m_CurFile = static_cast<int>(m_ListData.size() - 1);

	if (m_CurTopFile<0)
		m_CurTopFile=0;

	if (m_CurTopFile > static_cast<int>(m_ListData.size() - 1))
		m_CurTopFile = static_cast<int>(m_ListData.size() - 1);

	if (m_CurFile<m_CurTopFile)
		m_CurTopFile=m_CurFile;

	if (m_CurFile>m_CurTopFile+Height-1)
		m_CurTopFile=m_CurFile-(Height-1);
}

bool TreeList::SetCurDir(const string& NewDir,bool ClosePanel,bool /*IsUpdated*/)
{
	if (m_ListData.empty())
		Update(0);

	if (!m_ListData.empty() && !SetDirPosition(NewDir))
	{
		Update(0);
		SetDirPosition(NewDir);
	}

	if (GetFocus())
	{
		Parent()->GetCmdLine()->SetCurDir(NewDir);
		Parent()->GetCmdLine()->Show();
	}

	return true; //???
}

int TreeList::SetDirPosition(const string& NewDir)
{
	for (size_t i = 0; i < m_ListData.size(); ++i)
	{
		if (!StrCmpI(NewDir, m_ListData[i].strName))
		{
			m_WorkDir = i;
			m_CurFile = static_cast<int>(i);
			m_CurTopFile=m_CurFile-(m_Y2-m_Y1-1)/2;
			CorrectPosition();
			return TRUE;
		}
	}

	return FALSE;
}

const string& TreeList::GetCurDir() const
{
	if (m_ListData.empty())
	{
		if (m_ModalMode == MODALTREE_FREE)
			return m_Root;
		else
			return m_Empty;
	}
	else
		return m_ListData[m_CurFile].strName; //BUGBUG
}

int TreeList::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	int OldFile=m_CurFile;
	int RetCode;

	if (Global->Opt->ShowPanelScrollbar && IntKeyState.MouseX==m_X2 &&
	        (MouseEvent->dwButtonState & 1) && !IsDragging())
	{
		int ScrollY=m_Y1+1;
		int Height=m_Y2-m_Y1-3;

		if (IntKeyState.MouseY==ScrollY)
		{
			while (IsMouseButtonPressed())
				ProcessKey(Manager::Key(KEY_UP));

			if (!m_ModalMode)
				SetFocus();

			return TRUE;
		}

		if (IntKeyState.MouseY==ScrollY+Height-1)
		{
			while (IsMouseButtonPressed())
				ProcessKey(Manager::Key(KEY_DOWN));

			if (!m_ModalMode)
				SetFocus();

			return TRUE;
		}

		if (IntKeyState.MouseY>ScrollY && IntKeyState.MouseY<ScrollY+Height-1 && Height>2)
		{
			m_CurFile = static_cast<int>(m_ListData.size() - 1) * (IntKeyState.MouseY - ScrollY) / (Height - 2);
			DisplayTree(TRUE);

			if (!m_ModalMode)
				SetFocus();

			return TRUE;
		}
	}

	if (Panel::PanelProcessMouse(MouseEvent,RetCode))
		return RetCode;

	if (MouseEvent->dwMousePosition.Y>m_Y1 && MouseEvent->dwMousePosition.Y<m_Y2-2)
	{
		if (!m_ModalMode)
			SetFocus();

		MoveToMouse(MouseEvent);
		DisplayTree(TRUE);

		if (m_ListData.empty())
			return TRUE;

		if (((MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) &&
		        MouseEvent->dwEventFlags==DOUBLE_CLICK) ||
		        ((MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED) &&
		         !MouseEvent->dwEventFlags) ||
		        (OldFile!=m_CurFile && Global->Opt->Tree.AutoChangeFolder && !m_ModalMode))
		{
			DWORD control=MouseEvent->dwControlKeyState&(SHIFT_PRESSED|LEFT_ALT_PRESSED|LEFT_CTRL_PRESSED|RIGHT_ALT_PRESSED|RIGHT_CTRL_PRESSED);

			//вызовем EMenu если он есть
			if (!Global->Opt->RightClickSelect && MouseEvent->dwButtonState == RIGHTMOST_BUTTON_PRESSED && (control == 0 || control == SHIFT_PRESSED) && Global->CtrlObject->Plugins->FindPlugin(Global->Opt->KnownIDs.Emenu.Id))
			{
				Global->CtrlObject->Plugins->CallPlugin(Global->Opt->KnownIDs.Emenu.Id, OPEN_FILEPANEL, nullptr); // EMenu Plugin :-)
				return TRUE;
			}

			ProcessEnter();
			return TRUE;
		}

		return TRUE;
	}

	if (MouseEvent->dwMousePosition.Y<=m_Y1+1)
	{
		if (!m_ModalMode)
			SetFocus();

		if (m_ListData.empty())
			return TRUE;

		while (IsMouseButtonPressed() && IntKeyState.MouseY<=m_Y1+1)
			Up(1);

		if (Global->Opt->Tree.AutoChangeFolder && !m_ModalMode)
			ProcessKey(Manager::Key(KEY_ENTER));

		return TRUE;
	}

	if (MouseEvent->dwMousePosition.Y>=m_Y2-2)
	{
		if (!m_ModalMode)
			SetFocus();

		if (m_ListData.empty())
			return TRUE;

		while (IsMouseButtonPressed() && IntKeyState.MouseY>=m_Y2-2)
			Down(1);

		if (Global->Opt->Tree.AutoChangeFolder && !m_ModalMode)
			ProcessKey(Manager::Key(KEY_ENTER));

		return TRUE;
	}

	return FALSE;
}

void TreeList::MoveToMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	m_CurFile=m_CurTopFile+MouseEvent->dwMousePosition.Y-m_Y1-1;
	CorrectPosition();
}

void TreeList::ProcessEnter()
{
	auto& CurPtr=m_ListData[m_CurFile];
	if (api::fs::is_directory(CurPtr.strName))
	{
		if (!m_ModalMode && FarChDir(CurPtr.strName))
		{
			Panel *AnotherPanel=GetRootPanel();
			SetCurDir(CurPtr.strName,true);
			Show();
			AnotherPanel->SetCurDir(CurPtr.strName,true);
			AnotherPanel->Redraw();
		}
	}
	else
	{
		DelTreeName(CurPtr.strName);
		Update(UPDATE_KEEP_SELECTION);
		Show();
	}
}

int TreeList::ReadTreeFile()
{
	size_t RootLength=m_Root.empty()?0:m_Root.size()-1;
	string strName;
	//SaveState();
	FlushCache();
	MkTreeFileName(m_Root,strName);
#if defined(TREEFILE_PROJECT)
	if (strName.empty())
		return FALSE;
#endif

	auto TreeFile = OpenCacheableTreeFile(m_Root, strName, false);
	if (!TreeFile.Opened())
	{
		//RestoreState();
		return FALSE;
	}

	m_ListData.clear();

	ReadLines(TreeFile, [&](string& Name) { m_ListData.emplace_back(string(m_Root.data(), RootLength) + Name); });

	TreeFile.Close();

	if (m_ListData.empty())
		return FALSE;

	m_NumericSort = false;
	m_CaseSensitiveSort = false;
	return FillLastData();
}

bool TreeList::GetPlainString(string& Dest, int ListPos) const
{
	Dest.clear();
#if defined(Mantis_698)
	if (ListPos<TreeCount)
	{
		Dest=m_ListData[ListPos].strName;
		return true;
	}
#endif
	return false;
}

int TreeList::FindPartName(const string& Name,int Next,int Direct)
{
	string strMask;
	strMask = Name;
	strMask += L"*";

	Panel::exclude_sets(strMask);

	for (int i=m_CurFile+(Next?Direct:0); i >= 0 && static_cast<size_t>(i) < m_ListData.size(); i+=Direct)
	{
		if (CmpName(strMask.data(),m_ListData[i].strName.data(),true,(i==m_CurFile)))
		{
			m_CurFile=i;
			m_CurTopFile=m_CurFile-(m_Y2-m_Y1-1)/2;
			DisplayTree(TRUE);
			return TRUE;
		}
	}

	for (size_t i=(Direct > 0)?0:m_ListData.size()-1; (Direct > 0) ? i < static_cast<size_t>(m_CurFile):i > static_cast<size_t>(m_CurFile); i+=Direct)
	{
		if (CmpName(strMask.data(),m_ListData[i].strName.data(),true))
		{
			m_CurFile=static_cast<int>(i);
			m_CurTopFile=m_CurFile-(m_Y2-m_Y1-1)/2;
			DisplayTree(TRUE);
			return TRUE;
		}
	}

	return FALSE;
}

size_t TreeList::GetSelCount() const
{
	return 1;
}

int TreeList::GetSelName(string *strName, DWORD &FileAttr, string *strShortName, api::FAR_FIND_DATA *fd)
{
	if (!strName)
	{
		m_GetSelPosition=0;
		return TRUE;
	}

	if (!m_GetSelPosition)
	{
		*strName = GetCurDir();

		if (strShortName )
			*strShortName = *strName;

		FileAttr=FILE_ATTRIBUTE_DIRECTORY;
		m_GetSelPosition++;
		return TRUE;
	}

	m_GetSelPosition=0;
	return FALSE;
}

int TreeList::GetCurName(string &strName, string &strShortName) const
{
	if (m_ListData.empty())
	{
		strName.clear();
		strShortName.clear();
		return FALSE;
	}

	strName = m_ListData[m_CurFile].strName;
	strShortName = strName;
	return TRUE;
}

void TreeList::AddTreeName(const string& Name)
{
	if (Name.empty())
		return;

	string strFullName;
	ConvertNameToFull(Name, strFullName);
	string strRoot = ExtractPathRoot(strFullName);
	const wchar_t* NamePtr = strFullName.data();
	NamePtr += strRoot.size() - 1;

	if (!LastSlash(NamePtr))
		return;

	ReadCache(strRoot);

	TreeCache().add(NamePtr);
}

void TreeList::DelTreeName(const string& Name)
{
	if (Name.empty())
		return;

	string strFullName;
	ConvertNameToFull(Name, strFullName);
	string strRoot = ExtractPathRoot(strFullName);
	const wchar_t* NamePtr = strFullName.data();
	NamePtr += strRoot.size() - 1;
	ReadCache(strRoot);

	TreeCache().remove(NamePtr);
}

void TreeList::RenTreeName(const string& strSrcName,const string& strDestName)
{
	string SrcNameFull, DestNameFull;
	ConvertNameToFull(strSrcName, SrcNameFull);
	ConvertNameToFull(strDestName, DestNameFull);
	string strSrcRoot = ExtractPathRoot(SrcNameFull);
	string strDestRoot = ExtractPathRoot(DestNameFull);

	if (StrCmpI(strSrcRoot, strDestRoot))
	{
		DelTreeName(strSrcName);
		ReadSubTree(strSrcName);
	}

	const wchar_t* SrcName = strSrcName.data();
	SrcName += strSrcRoot.size() - 1;
	const wchar_t* DestName = strDestName.data();
	DestName += strDestRoot.size() - 1;
	ReadCache(strSrcRoot);

	TreeCache().rename(SrcName, DestName);
}

void TreeList::ReadSubTree(const string& Path)
{
	SCOPED_ACTION(ChangePriority)(THREAD_PRIORITY_NORMAL);
	//SaveScreen SaveScr;
	SCOPED_ACTION(TPreRedrawFuncGuard)(std::make_unique<TreePreRedrawItem>());
	ScanTree ScTree(false);
	api::FAR_FIND_DATA fdata;
	string strDirName;
	string strFullName;
	int Count=0;

	if (!api::fs::is_directory(Path))
		return;

	ConvertNameToFull(Path, strDirName);
	AddTreeName(strDirName);
	int FirstCall=TRUE, AscAbort=FALSE;
	ScTree.SetFindPath(strDirName,L"*",0);
	LastScrX = ScrX;
	LastScrY = ScrY;

	while (ScTree.GetNextName(&fdata, strFullName))
	{
		if (fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			MsgReadTree(Count+1,FirstCall);

			if (CheckForEscSilent())
			{
				AscAbort=ConfirmAbortOp();
				FirstCall=TRUE;
			}

			if (AscAbort)
				break;

			AddTreeName(strFullName);
			++Count;
		}
	}
}

void TreeList::ClearCache()
{
	TreeCache().clear();
}

void TreeList::ReadCache(const string& TreeRoot)
{
	string strTreeName;
	if (MkTreeFileName(TreeRoot, strTreeName) == TreeCache().GetTreeName())
		return;

	if (!TreeCache().empty())
		FlushCache();

	auto TreeFile = OpenCacheableTreeFile(TreeRoot, strTreeName, false);
	if (!TreeFile.Opened())
	{
		ClearCache();
		return;
	}

	TreeCache().SetTreeName(TreeFile.GetName());

	ReadLines(TreeFile, [](string& Name){ TreeCache().add(std::move(Name)); });
}

void TreeList::FlushCache()
{
	if (!TreeCache().GetTreeName().empty())
	{
		const auto Opener = [&](const string& Name) { return OpenTreeFile(Name, true); };

		WriteTree(TreeCache().GetTreeName(), TreeCache(), Opener, 0);
	}
	ClearCache();
}

void TreeList::UpdateViewPanel()
{
	if (!m_ModalMode)
	{
		auto AnotherPanel = dynamic_cast<QuickView*>(GetRootPanel());
		if (AnotherPanel && SetCurPath())
		{
			AnotherPanel->ShowFile(GetCurDir(), false, nullptr);
		}
	}
}

int TreeList::GoToFile(long idxItem)
{
	if (static_cast<size_t>(idxItem) < m_ListData.size())
	{
		m_CurFile=idxItem;
		CorrectPosition();
		return TRUE;
	}

	return FALSE;
}

int TreeList::GoToFile(const string& Name,BOOL OnlyPartName)
{
	return GoToFile(FindFile(Name,OnlyPartName));
}

long TreeList::FindFile(const string& Name,BOOL OnlyPartName)
{
	for (size_t i=0; i < m_ListData.size(); ++i)
	{
		const wchar_t* CurPtrName = OnlyPartName? PointToName(m_ListData[i].strName) : m_ListData[i].strName.data();

		if (Name == CurPtrName)
			return static_cast<long>(i);

		if (!StrCmpI(Name.data(),CurPtrName))
			return static_cast<long>(i);
	}

	return -1;
}

long TreeList::FindFirst(const string& Name)
{
	return FindNext(0,Name);
}

long TreeList::FindNext(int StartPos, const string& Name)
{
	if (static_cast<size_t>(StartPos) < m_ListData.size())
	{
		auto ItemIterator = std::find_if(CONST_RANGE(m_ListData, i)
		{
			return CmpName(Name.data(), i.strName.data(), true) && !TestParentFolderName(i.strName);
		});

		if (ItemIterator != m_ListData.cend())
			return static_cast<long>(ItemIterator - m_ListData.cbegin());
	}

	return -1;
}

int TreeList::GetFileName(string &strName, int Pos, DWORD &FileAttr) const
{
	if (Pos < 0 || static_cast<size_t>(Pos) >= m_ListData.size())
		return FALSE;

	strName = m_ListData[Pos].strName;
	FileAttr=FILE_ATTRIBUTE_DIRECTORY|api::GetFileAttributes(m_ListData[Pos].strName);
	return TRUE;
}

void TreeList::SetFocus()
{
	Panel::SetFocus();
	SetTitle();
}

void TreeList::KillFocus()
{
	if (static_cast<size_t>(m_CurFile) < m_ListData.size())
	{
		if (!api::fs::exists(m_ListData[m_CurFile].strName))
		{
			DelTreeName(m_ListData[m_CurFile].strName);
			Update(UPDATE_KEEP_SELECTION);
		}
	}

	Panel::KillFocus();
}

void TreeList::UpdateKeyBar()
{
	auto& Keybar = Parent()->GetKeybar();
	Keybar.SetLabels(MKBTreeF1);
	Keybar.SetCustomLabels(KBA_TREE);
}

void TreeList::SetTitle()
{
	if (GetFocus())
	{
		string strTitleDir(L"{");

		const wchar_t *Ptr=m_ListData.empty()? L"" : m_ListData[m_CurFile].strName.data();

		if (*Ptr)
		{
			strTitleDir += Ptr;
			strTitleDir += L" - ";
		}

		strTitleDir += L"Tree}";

		ConsoleTitle::SetFarTitle(strTitleDir);
	}
}

const TreeList::TreeItem* TreeList::GetItem(size_t Index) const
{
	if (static_cast<int>(Index) == -1 || static_cast<int>(Index) == -2)
		Index=GetCurrentPos();

	if (Index>= m_ListData.size())
		return nullptr;

	return &m_ListData[Index];
}

int TreeList::GetCurrentPos() const
{
	return m_CurFile;
}

bool TreeList::SaveState()
{
	m_SavedListData.clear();
	m_SavedWorkDir=0;

	if (!m_ListData.empty())
	{
		m_SavedListData = std::move(m_ListData);
		tempTreeCache() = std::move(TreeCache());
		m_SavedWorkDir=m_WorkDir;
		return true;
	}

	return false;
}

bool TreeList::RestoreState()
{
	m_ListData.clear();

	m_WorkDir=0;

	if (!m_SavedListData.empty())
	{
		m_ListData = std::move(m_SavedListData);
		TreeCache() = std::move(tempTreeCache());
		tempTreeCache().clear();
		m_WorkDir=m_SavedWorkDir;
		return true;
	}

	return false;
}
