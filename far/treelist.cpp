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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "treelist.hpp"

// Internal:
#include "keyboard.hpp"
#include "farcolor.hpp"
#include "keys.hpp"
#include "filepanels.hpp"
#include "filelist.hpp"
#include "cmdline.hpp"
#include "scantree.hpp"
#include "copy.hpp"
#include "qview.hpp"
#include "ctrlobj.hpp"
#include "help.hpp"
#include "lockscrn.hpp"
#include "macroopcode.hpp"
#include "refreshwindowmanager.hpp"
#include "TPreRedrawFunc.hpp"
#include "taskbar.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "clipboard.hpp"
#include "config.hpp"
#include "delete.hpp"
#include "mkdir.hpp"
#include "setattr.hpp"
#include "execute.hpp"
#include "dirmix.hpp"
#include "pathmix.hpp"
#include "processname.hpp"
#include "filestr.hpp"
#include "wakeful.hpp"
#include "colormix.hpp"
#include "plugins.hpp"
#include "manager.hpp"
#include "lang.hpp"
#include "keybar.hpp"
#include "strmix.hpp"
#include "string_sort.hpp"
#include "cvtname.hpp"
#include "global.hpp"
#include "network.hpp"
#if defined(TREEFILE_PROJECT)
#include "drivemix.hpp"
#endif

// Platform:
#if defined(TREEFILE_PROJECT)
#include "platform.env.hpp"
#endif
#include "platform.fs.hpp"

// Common:
#include "common/enum_tokens.hpp"
#include "common/function_ref.hpp"
#include "common/scope_exit.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

static std::chrono::steady_clock::time_point TreeStartTime;
static int LastScrX = -1;
static int LastScrY = -1;


/*
  Global->Opt->Tree.LocalDisk          Хранить файл структуры папок для локальных дисков
  Global->Opt->Tree.NetDisk            Хранить файл структуры папок для сетевых дисков
  Global->Opt->Tree.NetPath            Хранить файл структуры папок для сетевых путей
  Global->Opt->Tree.RemovableDisk      Хранить файл структуры папок для сменных дисков
  Global->Opt->Tree.CDDisk             Хранить файл структуры папок для CD/DVD/BD/etc дисков

  Global->Opt->Tree.strLocalDisk;      шаблон имени файла-деревяхи для локальных дисков
     default = L"LD.%D.%SN.tree"
  Global->Opt->Tree.strNetDisk;        шаблон имени файла-деревяхи для сетевых дисков
     default = L"ND.%D.%SN.tree";
  Global->Opt->Tree.strNetPath;        шаблон имени файла-деревяхи для сетевых путей
     default = L"NP.%SR.%SH.tree";
  Global->Opt->Tree.strRemovableDisk;  шаблон имени файла-деревяхи для сменных дисков
     default = L"RD.%SN.tree";
  Global->Opt->Tree.strCDDisk;         шаблон имени файла-деревяхи для CD/DVD/BD/etc дисков
     default = L"CD.%L.%SN.tree";

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
string ConvertTemplateTreeName(string_view const strTemplate, string_view const D, DWORD SN, string_view const L, string_view const SR, string_view const SH)
{
	string Str(strTemplate);

	replace(Str, L"%D"sv, D);
	replace(Str, L"%SN"sv, format(FSTR(L"{0:04X}-{1:04X}"), HIWORD(SN), LOWORD(SN)));
	replace(Str, L"%L"sv, L);
	replace(Str, L"%SR"sv, SR);
	replace(Str, L"%SH"sv, SH);

	return Str;
}
#endif

static string CreateTreeFileName(string_view const Path)
{
#if defined(TREEFILE_PROJECT)
	const auto strRootDir = extract_root_directory(Path);
	string strTreeFileName;
	string strPath;

	for (const auto& i: enum_tokens_with_quotes(Global->Opt->Tree.strExceptPath.Get(), L",;"sv))
	{
		if (equal_icase(strRootDir,  i))
		{
			return {};
		}
	}

	const auto DriveType = os::fs::drive::get_type(strRootDir);
	const auto PathType = ParsePath(strRootDir);
	/*
	root_type::unknown,
	root_type::drive_letter,
	root_type::unc_drive_letter,
	root_type::remote,
	root_type::unc_remote,
	root_type::volume,
	PATH_PIPE,
	*/

	// получение инфы о томе
	string strVolumeName, strFileSystemName;
	DWORD MaxNameLength = 0, FileSystemFlags = 0, VolumeNumber = 0;

	if (os::fs::GetVolumeInformation(strRootDir, &strVolumeName,
		&VolumeNumber, &MaxNameLength, &FileSystemFlags,
		&strFileSystemName))
	{
		switch (DriveType)
		{
		case DRIVE_REMOVABLE:
			if (Global->Opt->Tree.RemovableDisk)
			{
				strTreeFileName = ConvertTemplateTreeName(Global->Opt->Tree.strRemovableDisk, strRootDir, VolumeNumber, strVolumeName, {}, {});
				// TODO: Global->Opt->ProfilePath / Global->Opt->LocalProfilePath
				strPath = Global->Opt->Tree.strSaveLocalPath;
			}
			break;

		case DRIVE_FIXED:
			if (Global->Opt->Tree.LocalDisk)
			{
				strTreeFileName = ConvertTemplateTreeName(Global->Opt->Tree.strLocalDisk, strRootDir, VolumeNumber, strVolumeName, {}, {});
				// TODO: Global->Opt->ProfilePath / Global->Opt->LocalProfilePath
				strPath = Global->Opt->Tree.strSaveLocalPath;
			}
			break;

		case DRIVE_REMOTE:
			if (Global->Opt->Tree.NetDisk || Global->Opt->Tree.NetPath)
			{
				string strServer, strShare;
				if (PathType == root_type::remote)
				{
					strServer = ExtractComputerName(strRootDir, &strShare);
					DeleteEndSlash(strShare);
				}

				strTreeFileName = ConvertTemplateTreeName(PathType == root_type::drive_letter? Global->Opt->Tree.strNetDisk : Global->Opt->Tree.strNetPath, strRootDir, VolumeNumber, strVolumeName, strServer, strShare);
				// TODO: Global->Opt->ProfilePath / Global->Opt->LocalProfilePath
				strPath = Global->Opt->Tree.strSaveNetPath;
			}
			break;

		case DRIVE_CDROM:
			if (Global->Opt->Tree.CDDisk)
			{
				strTreeFileName = ConvertTemplateTreeName(Global->Opt->Tree.strCDDisk, strRootDir, VolumeNumber, strVolumeName, {}, {});
				// TODO: Global->Opt->ProfilePath / Global->Opt->LocalProfilePath
				strPath = Global->Opt->Tree.strSaveLocalPath;
			}
			break;

		case DRIVE_RAMDISK:
			break;
		}

		return path::join(!strPath.empty()? os::env::expand(strPath) : Path, strTreeFileName);
	}
	else
	{
		return path::join(Path, L"tree3.far"sv);
	}

#else
	return path::join(Path, L"tree3.far"sv);
#endif
}

// TODO: Файлы "Tree3.Far" для локальных дисков должны храниться в "Local AppData\Far Manager"
// TODO: Файлы "Tree3.Far" для сменных дисков должны храниться на самих "дисках"
// TODO: Файлы "Tree3.Far" для сетевых дисков должны храниться в "%HOMEDRIVE%\%HOMEPATH%",
//                        если эти переменные среды не определены, то "%APPDATA%\Far Manager"
static string MkTreeFileName(const string_view RootDir)
{
	return CreateTreeFileName(RootDir);
}

// этому каталогу (Tree.Cache) место не в FarPath, а в "Local AppData\Far\"
static string MkTreeCacheFolderName(const string_view RootDir)
{
#if defined(TREEFILE_PROJECT)
	// в проекте TREEFILE_PROJECT наличие каталога tree3.cache не предполагается
	return CreateTreeFileName(RootDir);
#else
	return path::join(RootDir, L"tree3.cache"sv);
#endif
}

static bool GetCacheTreeName(string_view const Root, string& strName, bool const CreateDir)
{
	string strVolumeName, strFileSystemName;
	DWORD dwVolumeSerialNumber;

	if (!os::fs::GetVolumeInformation(
		Root,
		&strVolumeName,
		&dwVolumeSerialNumber,
		nullptr,
		nullptr,
		&strFileSystemName
		))
		return false;

	const auto strFolderName = MkTreeCacheFolderName(Global->Opt->LocalProfilePath);
#if defined(TREEFILE_PROJECT)
	if (strFolderName.empty())
		return false;
#endif

	if (CreateDir)
	{
		// BUGBUG check result
		(void)os::fs::create_directory(strFolderName);
		// BUGBUG check result
		(void)os::fs::set_file_attributes(strFolderName, Global->Opt->Tree.TreeFileAttr);
	}

	string strRemoteName;

	const auto PathType = ParsePath(Root);
	if (PathType == root_type::remote || PathType == root_type::unc_remote)
	{
		strRemoteName = Root;
	}
	else if (PathType == root_type::drive_letter || PathType == root_type::win32nt_drive_letter)
	{
		if (DriveLocalToRemoteName(true, Root, strRemoteName))
			AddEndSlash(strRemoteName);
	}

	std::replace(ALL_RANGE(strRemoteName), L'\\', L'_');
	strName = format(FSTR(L"{0}\\{1}.{2:X}.{3}.{4}"), strFolderName, strVolumeName, dwVolumeSerialNumber, strFileSystemName, strRemoteName);
	return true;
}

class TreeListCache
{
public:
	NONCOPYABLE(TreeListCache);
	MOVABLE(TreeListCache);

	TreeListCache() = default;

	void clear()
	{
		m_Names.clear();
		m_TreeName.clear();
	}

	bool empty() const { return m_Names.empty(); }

	void add(const string_view Name) { m_Names.emplace(Name); }

	void remove(string_view const Name)
	{
		std::erase_if(m_Names, [&](const auto& i)
		{
			return starts_with_icase(i, Name) && (i.size() == Name.size() || (i.size() > Name.size() && IsSlash(i[Name.size()])));
		});
	}

	void rename(string_view const OldName, string_view const NewName)
	{
		const auto Count = m_Names.size();
		remove(OldName);
		if (m_Names.size() == Count)
			return;
		m_Names.emplace(NewName);
	}

	const string& GetTreeName() const { return m_TreeName; }

	void SetTreeName(string_view const Name) { m_TreeName = Name; }

private:
	using cache_set = std::set<string, string_sort::less_t>;

public:
	using const_iterator = cache_set::const_iterator;
	const_iterator begin() const { return m_Names.cbegin(); }
	const_iterator end() const { return m_Names.cend(); }

private:
	cache_set m_Names;
	string m_TreeName;
};

static TreeListCache& TreeCache()
{
	static TreeListCache cache;
	return cache;
}

static TreeListCache& tempTreeCache()
{
	static TreeListCache cache;
	return cache;
}

enum TREELIST_FLAGS
{
	FTREELIST_TREEISPREPARED = 16_bit,
	FTREELIST_UPDATEREQUIRED = 17_bit,
};

tree_panel_ptr TreeList::create(window_ptr Owner, int ModalMode)
{
	return std::make_shared<TreeList>(private_tag(), Owner, ModalMode);
}

TreeList::TreeList(private_tag, window_ptr Owner, int ModalMode):
	Panel(std::move(Owner)),
	m_WorkDir(0),
	m_SavedWorkDir(0),
	m_GetSelPosition(0),
	m_ExitCode(1)
{
	m_Type = panel_type::TREE_PANEL;
	m_CurFile=m_CurTopFile=0;
	m_Flags.Set(FTREELIST_UPDATEREQUIRED);
	m_Flags.Clear(FTREELIST_TREEISPREPARED);
	m_ModalMode = ModalMode;
}

TreeList::~TreeList()
{
	tempTreeCache().clear();
	FlushCache();
}

void TreeList::SetRootDir(string_view const NewRootDir)
{
	m_Root = NewRootDir;
	m_CurDir = NewRootDir;
}

void TreeList::DisplayObject()
{
	if (m_ReadingTree)
		return;

	if (m_Flags.Check(FSCROBJ_ISREDRAWING))
		return;

	m_Flags.Set(FSCROBJ_ISREDRAWING);

	if (m_Flags.Check(FTREELIST_UPDATEREQUIRED))
		Update(0);

	if (m_ExitCode)
		DisplayTree(false);

	m_Flags.Clear(FSCROBJ_ISREDRAWING);
}

string TreeList::GetTitle() const
{
	return msg(m_ModalMode? lng::MFindFolderTitle : lng::MTreeTitle);
}

void TreeList::DisplayTree(bool Fast)
{
	wchar_t TreeLineSymbol[4][3]=
	{
		{L' ',                  L' ',             0},
		{BoxSymbols[BS_V1],     L' ',             0},
		{BoxSymbols[BS_LB_H1V1],BoxSymbols[BS_H1],0},
		{BoxSymbols[BS_L_H1V1], BoxSymbols[BS_H1],0},
	};

	std::optional<LockScreen> LckScreen;

	if (!m_ModalMode && Parent()->GetAnotherPanel(this)->GetType() == panel_type::QVIEW_PANEL)
		LckScreen.emplace();

	CorrectPosition();

	if (!m_ListData.empty())
		m_CurDir = m_ListData[m_CurFile].strName; //BUGBUG

//    xstrncpy(CurDir,ListData[CurFile].Name,sizeof(CurDir));
	if (!Fast)
	{
		Box(m_Where, colors::PaletteColorToFarColor(COL_PANELBOX), DOUBLE_BOX);
		DrawSeparator(m_Where.bottom - 2 - (m_ModalMode != 0));

		const auto& strTitle = GetTitleForDisplay();
		if (!strTitle.empty())
		{
			SetColor((IsFocused() || m_ModalMode) ? COL_PANELSELECTEDTITLE:COL_PANELTITLE);
			GotoXY(m_Where.left + (m_Where.width() - static_cast<int>(strTitle.size())) / 2, m_Where.top);
			Text(strTitle);
		}
	}

	for (size_t I = m_Where.top + 1, J = m_CurTopFile; I < static_cast<size_t>(m_Where.bottom - 2 - (m_ModalMode != 0)); ++I, ++J)
	{
		GotoXY(m_Where.left + 1, static_cast<int>(I));
		SetColor(COL_PANELTEXT);
		Text(L' ');

		if (J < m_ListData.size() && m_Flags.Check(FTREELIST_TREEISPREPARED))
		{
			auto& CurPtr=m_ListData[J];

			if (!J)
			{
				DisplayTreeName(L"\\"sv, J);
			}
			else
			{
				string strOutStr;

				for (size_t i = 0; i < CurPtr.Depth - 1 && WhereX() + 3 * i < m_Where.right - 6u; ++i)
				{
					strOutStr+=TreeLineSymbol[CurPtr.Last[i]?0:1];
				}

				strOutStr+=TreeLineSymbol[CurPtr.Last[CurPtr.Depth-1]?2:3];
				BoxText(strOutStr);

				const auto pos = FindLastSlash(CurPtr.strName);
				if (pos != string::npos)
				{
					DisplayTreeName(string_view(CurPtr.strName).substr(pos + 1), J);
				}
			}
		}

		SetColor(COL_PANELTEXT);

		if (WhereX() < m_Where.right)
		{
			Text(string(m_Where.right - WhereX(), L' '));
		}
	}

	if (Global->Opt->ShowPanelScrollbar)
	{
		SetColor(COL_PANELSCROLLBAR);
		ScrollBar(m_Where.right, m_Where.top + 1, m_Where.height() - 4, m_CurTopFile, m_ListData.size());
	}

	SetColor(COL_PANELTEXT);
	SetScreen({ m_Where.left + 1, m_Where.bottom - (m_ModalMode? 2 : 1), m_Where.right - 1, m_Where.bottom - 1 }, L' ', colors::PaletteColorToFarColor(COL_PANELTEXT));

	if (!m_ListData.empty())
	{
		GotoXY(m_Where.left + 1, m_Where.bottom - 1);
		Text(fit_to_left(m_ListData[m_CurFile].strName, m_Where.width() - 2));
	}

	UpdateViewPanel();
	RefreshTitle(); // не забудем прорисовать заголовок
}

void TreeList::DisplayTreeName(const string_view Name, const size_t Pos) const
{
	if (WhereX() > m_Where.right - 4)
		GotoXY(m_Where.right - 4, WhereY());

	if (Pos==static_cast<size_t>(m_CurFile))
	{
		GotoXY(WhereX()-1,WhereY());

		if (IsFocused() || m_ModalMode)
		{
			SetColor(Pos == m_WorkDir? COL_PANELSELECTEDCURSOR : COL_PANELCURSOR);
			Text(concat(L' ', cut_right(Name, m_Where.right - WhereX() - 3), L' '));
		}
		else
		{
			SetColor(Pos == m_WorkDir? COL_PANELSELECTEDTEXT : COL_PANELTEXT);
			Text(concat(L'[', cut_right(Name, m_Where.right - WhereX() - 3), L']'));
		}
	}
	else
	{
		SetColor(Pos == m_WorkDir? COL_PANELSELECTEDTEXT : COL_PANELTEXT);
		Text(cut_right(Name, m_Where.right - WhereX() - 1));
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
	const auto LastTreeCount = m_ListData.size();
	int RetFromReadTree=TRUE;
	m_Flags.Clear(FTREELIST_TREEISPREPARED);
	const auto TreeFilePresent = ReadTreeFile();

	if (!TreeFilePresent)
		RetFromReadTree=ReadTree();

	m_Flags.Set(FTREELIST_TREEISPREPARED);

	if (!RetFromReadTree && m_ModalMode)
	{
		m_ExitCode=0;
		return;
	}

	if (RetFromReadTree && !m_ListData.empty() && (!(Mode & UPDATE_KEEP_SELECTION) || LastTreeCount != m_ListData.size()))
	{
		SyncDir();
		auto& CurPtr=m_ListData[m_CurFile];

		if (!os::fs::exists(CurPtr.strName))
		{
			DelTreeName(CurPtr.strName);
			Update(UPDATE_KEEP_SELECTION);
			Show();
		}
	}
	else if (!RetFromReadTree)
	{
		Show();

		if (!m_ModalMode)
		{
			const auto AnotherPanel = Parent()->GetAnotherPanel(this);
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

static void MsgReadTreeImpl(size_t TreeCount)
{
	/* $ 24.09.2001 VVM
	! Писать сообщение о чтении дерева только, если это заняло более 500 мсек. */
	const auto IsChangeConsole = LastScrX != ScrX || LastScrY != ScrY;

	if (IsChangeConsole)
	{
		LastScrX = ScrX;
		LastScrY = ScrY;
	}

	if (IsChangeConsole || std::chrono::steady_clock::now() - TreeStartTime > 1s)
	{
		Message(0,
			msg(lng::MTreeTitle),
			{
				msg(lng::MReadingTree),
				str(TreeCount)
			},
			{});

		TreeStartTime = std::chrono::steady_clock::now();
	}
}

static void MsgReadTree(size_t TreeCount)
{
	MsgReadTreeImpl(TreeCount);

	TPreRedrawFunc::instance()([&](TreePreRedrawItem& Item)
	{
		Item.TreeCount = TreeCount;
	});
}

static void PR_MsgReadTree()
{
	TPreRedrawFunc::instance()([](const TreePreRedrawItem& Item)
	{
		MsgReadTreeImpl(Item.TreeCount);
	});
}

static os::fs::file OpenTreeFile(string_view const Name, bool const Writable)
{
	return os::fs::file(Name, Writable? FILE_WRITE_DATA : FILE_READ_DATA, FILE_SHARE_READ, nullptr, Writable? OPEN_ALWAYS : OPEN_EXISTING);
}

static bool MustBeCached(string_view const Root)
{
	const auto type = os::fs::drive::get_type(Root);

	if (type == DRIVE_UNKNOWN || type == DRIVE_NO_ROOT_DIR || type == DRIVE_REMOVABLE || type == DRIVE_CDROM)
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

static os::fs::file OpenCacheableTreeFile(string_view const Root, string& Name, bool Writable)
{
	os::fs::file Result;
	if (!MustBeCached(Root))
		Result = OpenTreeFile(Name, Writable);

	if (!Result)
	{
		if (GetCacheTreeName(Root, Name, Writable))
		{
			Result = OpenTreeFile(Name, Writable);
		}
	}
	return Result;
}

static void ReadLines(const os::fs::file& TreeFile, function_ref<void(string_view)> const Inserter)
{
	os::fs::filebuf StreamBuffer(TreeFile, std::ios::in);
	std::istream Stream(&StreamBuffer);
	Stream.exceptions(Stream.badbit | Stream.failbit);

	for (const auto& i: enum_lines(Stream, CP_UNICODE))
	{
		if (i.Str.empty() || !IsSlash(i.Str.front()))
			continue;

		Inserter(i.Str);
	}
}

template<class string_type, class container_type, class opener_type>
static void WriteTree(string_type& Name, const container_type& Container, const opener_type& Opener, size_t offset)
{
	// получим и сразу сбросим атрибуты (если получится)
	const auto SavedAttributes = os::fs::get_file_attributes(Name);

	if (SavedAttributes != INVALID_FILE_ATTRIBUTES)
		(void)os::fs::set_file_attributes(Name, FILE_ATTRIBUTE_NORMAL); //BUGBUG

	std::optional<error_state_ex> ErrorState;

	if (auto TreeFile = Opener(Name))
	{
		try
		{
			os::fs::filebuf StreamBuffer(TreeFile, std::ios::out);
			std::ostream Stream(&StreamBuffer);
			Stream.exceptions(Stream.badbit | Stream.failbit);
			encoding::writer Writer(Stream, CP_UNICODE, false);
			const auto Eol = eol::system.str();

			for (const auto& i: Container)
			{
				Writer.write(string_view(i).substr(offset));
				Writer.write(Eol);
			}

			Stream.flush();

			if (SavedAttributes != INVALID_FILE_ATTRIBUTES) // вернем атрибуты (если получится :-)
				(void)os::fs::set_file_attributes(Name, SavedAttributes); //BUGBUG
		}
		catch (const far_exception& e)
		{
			ErrorState = e;
		}

		TreeFile.SetEnd();
		TreeFile.Close();
	}
	else
	{
		ErrorState = error_state::fetch();
	}

	if (ErrorState)
	{
		(void)os::fs::delete_file(TreeCache().GetTreeName()); // BUGBUG
		if (!Global->WindowManager->ManagerIsDown())
			Message(MSG_WARNING, *ErrorState,
				msg(lng::MError),
				{
					msg(lng::MCannotSaveTree),
					Name
				},
				{ lng::MOk });
	}
}

bool TreeList::ReadTree()
{
	m_ReadingTree = true;
	SCOPE_EXIT{ m_ReadingTree = false; };

	SCOPED_ACTION(TPreRedrawFuncGuard)(std::make_unique<TreePreRedrawItem>());
	ScanTree ScTree(false, true, -1, false);
	os::fs::find_data fdata;
	string strFullName;
	FlushCache();
	SaveState();
	GetRoot();

	const auto PrevCurFile = std::exchange(m_CurFile, 0);

	m_ListData.clear();
	m_ListData.reserve(4096);

	m_ListData.emplace_back(m_Root);

	auto AscAbort = false;
	TreeStartTime = std::chrono::steady_clock::now();
	SCOPED_ACTION(RefreshWindowManager)(ScrX, ScrY);
	ScTree.SetFindPath(m_Root, L"*"sv);
	LastScrX = ScrX;
	LastScrY = ScrY;
	SCOPED_ACTION(taskbar::indeterminate);
	SCOPED_ACTION(wakeful);
	while (ScTree.GetNextName(fdata,strFullName))
	{
		MsgReadTree(m_ListData.size());

		if (CheckForEscSilent())
		{
			// BUGBUG, Dialog calls Commit, TreeList redraws and crashes.
			AscAbort=ConfirmAbortOp();
		}

		if (AscAbort)
			break;

		if (!(fdata.Attributes & FILE_ATTRIBUTE_DIRECTORY))
			continue;

		m_ListData.emplace_back(strFullName);
	}

	if (AscAbort && m_ModalMode)
	{
		m_ListData.clear();
		RestoreState();
		return false;
	}

	std::sort(ALL_RANGE(m_ListData), string_sort::less);

	if (!FillLastData())
		return false;

	if (!AscAbort)
		SaveTreeFile();

	m_CurFile = PrevCurFile;
	return true;
}

void TreeList::SaveTreeFile()
{
	if (m_ListData.size() < static_cast<size_t>(Global->Opt->Tree.MinTreeCount))
		return;

	const auto RootLength = m_Root.empty()? 0 : m_Root.size() - 1;
	auto strName = MkTreeFileName(m_Root);
#if defined(TREEFILE_PROJECT)
	if (strName.empty())
		return;
#endif

	const auto Opener = [&](string& Name) { return OpenCacheableTreeFile(m_Root, Name, true); };

	WriteTree(strName, m_ListData, Opener, RootLength);

#if defined(TREEFILE_PROJECT)
	os::fs::set_file_attributes(strName, Global->Opt->Tree.TreeFileAttr);
#endif

}

void TreeList::GetRoot()
{
	m_Root = extract_root_directory(GetRootPanel()->GetCurDir());
}

panel_ptr TreeList::GetRootPanel()
{
	panel_ptr RootPanel;

	if (m_ModalMode) // watch out, Parent() in nullptr
	{
		if (m_ModalMode==MODALTREE_ACTIVE)
			RootPanel = Global->CtrlObject->Cp()->ActivePanel();
		else if (m_ModalMode==MODALTREE_FREE)
			RootPanel = shared_from_this();
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
	const auto AnotherPanel = GetRootPanel();
	const auto strPanelDir = AnotherPanel->GetCurDir();

	if (!strPanelDir.empty())
	{
		if (AnotherPanel->GetType() == panel_type::FILE_PANEL)
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
	const auto CountSlash = [](const string& Str, size_t Offset)
	{
		return static_cast<size_t>(std::count_if(Str.cbegin() + Offset, Str.cend(), IsSlash));
	};

	const auto RootLength = m_Root.empty()? 0 : m_Root.size()-1;
	const range Range(m_ListData.begin() + 1, m_ListData.end());
	FOR_RANGE(Range, i)
	{
		const auto Pos = i->strName.rfind(L'\\');
		const auto PathLength = Pos != string::npos? static_cast<int>(Pos) + 1 : 0;
		const auto Depth = i->Depth = CountSlash(i->strName, RootLength);

		if (!Depth)
			return false;

		auto SubDirPos = i;
		int Last = 1;

		const range SubRange(i + 1, Range.end());
		FOR_RANGE(SubRange, j)
		{
			if (CountSlash(j->strName, RootLength) > Depth)
			{
				SubDirPos = j;
				continue;
			}
			else
			{
				if (starts_with_icase(j->strName, string_view(i->strName).substr(0, PathLength)))
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

long long TreeList::VMProcess(int OpCode, void* vParam, long long iParam)
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

bool TreeList::ProcessKey(const Manager::Key& Key)
{
	const auto LocalKey = Key();
	if (!IsVisible())
		return false;

	if (m_ListData.empty() && none_of(LocalKey, KEY_CTRLR, KEY_RCTRLR))
		return false;

	if (
		in_closed_range(KEY_CTRLSHIFT0, LocalKey, KEY_CTRLSHIFT9) ||
		in_closed_range(KEY_RCTRLSHIFT0, LocalKey, KEY_RCTRLSHIFT9)
	)
	{
		SaveShortcutFolder((LocalKey&(~(KEY_CTRL | KEY_RCTRL | KEY_SHIFT | KEY_RSHIFT))) - L'0');
		return true;
	}

	if (in_closed_range(KEY_RCTRL0, LocalKey, KEY_RCTRL9))
	{
		ExecShortcutFolder(LocalKey-KEY_RCTRL0);
		return true;
	}

	switch (LocalKey)
	{
		case KEY_F1:
			help::show(L"TreePanel"sv);
			return true;

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

			if (any_of(LocalKey, KEY_CTRLALTINS, KEY_RCTRLRALTINS, KEY_CTRLRALTINS, KEY_RCTRLALTINS, KEY_CTRLALTNUMPAD0, KEY_RCTRLRALTNUMPAD0, KEY_CTRLRALTNUMPAD0, KEY_RCTRLALTNUMPAD0))
			{
				SetClipboardText(strQuotedName);
			}
			else
			{
				if (any_of(LocalKey, KEY_SHIFTENTER, KEY_SHIFTNUMENTER))
				{
					OpenFolderInShell(strQuotedName);
				}
				else
				{
					strQuotedName += L' ';
					Parent()->GetCmdLine()->InsertString(strQuotedName);
				}
			}

			return true;
		}
		case KEY_CTRLBACKSLASH:
		case KEY_RCTRLBACKSLASH:
		{
			m_CurFile=0;
			ProcessEnter();
			return true;
		}
		case KEY_NUMENTER:
		case KEY_ENTER:
		{
			if (!m_ModalMode && !Parent()->GetCmdLine()->GetString().empty())
				break;

			ProcessEnter();
			return true;
		}
		case KEY_F4:
		case KEY_CTRLA:
		case KEY_RCTRLA:
		{
			if (SetCurPath())
				ShellSetFileAttributes(this);

			return true;
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
				int ToPlugin = 0;
				Copy(shared_from_this(), LocalKey == KEY_SHIFTF6, false, true, true, ToPlugin, nullptr);
			}

			return true;
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
				const auto AnotherPanel = Parent()->GetAnotherPanel(this);
				const auto Ask = none_of(LocalKey, KEY_DRAGCOPY, KEY_DRAGMOVE) || Global->Opt->Confirm.Drag;
				const auto Move = any_of(LocalKey, KEY_F6, KEY_DRAGMOVE);

				const auto UseInternalCommand = [&]
				{
					const auto Handle = AnotherPanel->GetPluginHandle();
					OpenPanelInfo Info;
					Global->CtrlObject->Plugins->GetOpenPanelInfo(Handle, &Info);
					return PluginManager::UseInternalCommand(Handle, PLUGIN_FARPUTFILES, Info);
				};

				int ToPlugin = AnotherPanel->GetMode() == panel_mode::PLUGIN_PANEL && AnotherPanel->IsVisible() && !UseInternalCommand();
				const auto IsAltF6 = any_of(LocalKey, KEY_ALTF6, KEY_RALTF6);
				const auto Link = IsAltF6 && !ToPlugin;

				if (IsAltF6 && !Link) // молча отвалим :-)
					return true;

				{
					Copy(shared_from_this(), Move, Link, false, Ask, ToPlugin, nullptr);
				}

				if (ToPlugin)
				{
					PluginPanelItemHolder Item;
					const auto hAnotherPlugin = AnotherPanel->GetPluginHandle();
					if (FileList::FileNameToPluginItem(m_ListData[m_CurFile].strName, Item))
					{
						const auto PutCode = Global->CtrlObject->Plugins->PutFiles(hAnotherPlugin, { &Item.Item, 1 }, Move, 0);

						if (PutCode == 1 || PutCode == 2)
							AnotherPanel->SetPluginModified();
						if (Move)
							ReadSubTree(m_ListData[m_CurFile].strName);

						Update(0);
						Redraw();
						AnotherPanel->Update(UPDATE_KEEP_SELECTION);
						AnotherPanel->Redraw();
					}
				}
			}

			return true;
		}
		case KEY_F7:
		{
			if (SetCurPath())
				ShellMakeDir(this);

			return true;
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
				Delete(shared_from_this(),
					any_of(LocalKey, KEY_SHIFTDEL, KEY_SHIFTNUMDEL, KEY_SHIFTDECIMAL)? delete_type::remove :
					any_of(LocalKey, KEY_ALTDEL, KEY_RALTDEL, KEY_ALTNUMDEL, KEY_RALTNUMDEL, KEY_ALTDECIMAL, KEY_RALTDECIMAL)? delete_type::erase :
					Global->Opt->DeleteToRecycleBin? delete_type::recycle : delete_type::remove);
				// Надобно не забыть обновить противоположную панель...
				const auto AnotherPanel = Parent()->GetAnotherPanel(this);
				AnotherPanel->Update(UPDATE_KEEP_SELECTION);
				AnotherPanel->Redraw();

				if (Global->Opt->Tree.AutoChangeFolder && !m_ModalMode)
					ProcessKey(Manager::Key(KEY_ENTER));
			}

			return true;
		}
		case KEY_MSWHEEL_UP:
		case(KEY_MSWHEEL_UP | KEY_ALT):
		case(KEY_MSWHEEL_UP | KEY_RALT):
		{
			Scroll(LocalKey & (KEY_ALT | KEY_RALT)? -1 : static_cast<int>(-Global->Opt->MsWheelDelta));
			return true;
		}
		case KEY_MSWHEEL_DOWN:
		case(KEY_MSWHEEL_DOWN | KEY_ALT):
		case(KEY_MSWHEEL_DOWN | KEY_RALT):
		{
			Scroll(LocalKey& (KEY_ALT | KEY_RALT)? 1 : static_cast<int>(Global->Opt->MsWheelDelta));
			return true;
		}
		case KEY_MSWHEEL_LEFT:
		case(KEY_MSWHEEL_LEFT | KEY_ALT):
		case(KEY_MSWHEEL_LEFT | KEY_RALT):
		{
			const auto Roll = LocalKey & (KEY_ALT | KEY_RALT)? 1 : static_cast<int>(Global->Opt->MsHWheelDelta);

			for (int i=0; i<Roll; i++)
				ProcessKey(Manager::Key(KEY_LEFT));

			return true;
		}
		case KEY_MSWHEEL_RIGHT:
		case(KEY_MSWHEEL_RIGHT | KEY_ALT):
		case(KEY_MSWHEEL_RIGHT | KEY_RALT):
		{
			const auto Roll = LocalKey & (KEY_ALT | KEY_RALT)? 1 : static_cast<int>(Global->Opt->MsHWheelDelta);

			for (int i=0; i<Roll; i++)
				ProcessKey(Manager::Key(KEY_RIGHT));

			return true;
		}
		case KEY_HOME:        case KEY_NUMPAD7:
		{
			ToBegin();

			if (Global->Opt->Tree.AutoChangeFolder && !m_ModalMode)
				ProcessKey(Manager::Key(KEY_ENTER));

			return true;
		}
		case KEY_ADD: // OFM: Gray+/Gray- navigation
		{
			m_CurFile=GetNextNavPos();

			if (Global->Opt->Tree.AutoChangeFolder && !m_ModalMode)
				ProcessKey(Manager::Key(KEY_ENTER));
			else
				DisplayTree(true);

			return true;
		}
		case KEY_SUBTRACT: // OFM: Gray+/Gray- navigation
		{
			m_CurFile=GetPrevNavPos();

			if (Global->Opt->Tree.AutoChangeFolder && !m_ModalMode)
				ProcessKey(Manager::Key(KEY_ENTER));
			else
				DisplayTree(true);

			return true;
		}
		case KEY_END:         case KEY_NUMPAD1:
		{
			ToEnd();

			if (Global->Opt->Tree.AutoChangeFolder && !m_ModalMode)
				ProcessKey(Manager::Key(KEY_ENTER));

			return true;
		}
		case KEY_UP:          case KEY_NUMPAD8:
		{
			Up(1);

			if (Global->Opt->Tree.AutoChangeFolder && !m_ModalMode)
				ProcessKey(Manager::Key(KEY_ENTER));

			return true;
		}
		case KEY_DOWN:        case KEY_NUMPAD2:
		{
			Down(1);

			if (Global->Opt->Tree.AutoChangeFolder && !m_ModalMode)
				ProcessKey(Manager::Key(KEY_ENTER));

			return true;
		}
		case KEY_PGUP:        case KEY_NUMPAD9:
		{
			m_CurTopFile -= m_Where.height() - 4 - (m_ModalMode != 0);
			m_CurFile -= m_Where.height() - 4 - (m_ModalMode != 0);
			DisplayTree(true);

			if (Global->Opt->Tree.AutoChangeFolder && !m_ModalMode)
				ProcessKey(Manager::Key(KEY_ENTER));

			return true;
		}
		case KEY_PGDN:        case KEY_NUMPAD3:
		{
			m_CurTopFile += m_Where.height() - 4 - (m_ModalMode != 0);
			m_CurFile += m_Where.height() - 4 - (m_ModalMode != 0);
			DisplayTree(true);

			if (Global->Opt->Tree.AutoChangeFolder && !m_ModalMode)
				ProcessKey(Manager::Key(KEY_ENTER));

			return true;
		}

		case KEY_APPS:
		case KEY_SHIFTAPPS:
		{
			//вызовем EMenu если он есть
			if (Global->CtrlObject->Plugins->FindPlugin(Global->Opt->KnownIDs.Emenu.Id))
			{
				Global->CtrlObject->Plugins->CallPlugin(Global->Opt->KnownIDs.Emenu.Id, OPEN_FILEPANEL, ToPtr(1)); // EMenu Plugin :-)
			}
			return true;
		}

		default:
			if ((LocalKey>=KEY_ALT_BASE+0x01 && LocalKey<=KEY_ALT_BASE+65535) || (LocalKey>=KEY_RALT_BASE+0x01 && LocalKey<=KEY_RALT_BASE+65535) ||
			        (LocalKey>=KEY_ALTSHIFT_BASE+0x01 && LocalKey<=KEY_ALTSHIFT_BASE+65535) || (LocalKey>=KEY_RALTSHIFT_BASE+0x01 && LocalKey<=KEY_RALTSHIFT_BASE+65535))
			{
				FastFind(Key);

				if (Global->Opt->Tree.AutoChangeFolder && !m_ModalMode)
					ProcessKey(Manager::Key(KEY_ENTER));
			}
			else
				break;

			return true;
	}

	return false;
}

int TreeList::GetNextNavPos() const
{
	int NextPos=m_CurFile;

	if (static_cast<size_t>(m_CurFile) + 1 < m_ListData.size())
	{
		const auto CurDepth = m_ListData[m_CurFile].Depth;

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
		const auto CurDepth = m_ListData[m_CurFile].Depth;

		for (int I=m_CurFile-1; I > 0; --I)
			if (m_ListData[I].Depth == CurDepth)
			{
				PrevPos=I;
				break;
			}
	}

	return PrevPos;
}

void TreeList::ToBegin()
{
	m_CurFile = 0;
	DisplayTree(true);
}

void TreeList::ToEnd()
{
	m_CurFile = m_ListData.empty()? 0 : static_cast<int>(m_ListData.size() - 1);
	DisplayTree(true);
}

void TreeList::Up(int Count)
{
	m_CurFile-=Count;
	DisplayTree(true);
}

void TreeList::Down(int Count)
{
	m_CurFile+=Count;
	DisplayTree(true);
}

void TreeList::Scroll(int Count)
{
	m_CurFile+=Count;
	m_CurTopFile+=Count;
	DisplayTree(true);
}

void TreeList::CorrectPosition()
{
	if (m_ListData.empty())
	{
		m_CurFile=m_CurTopFile=0;
		return;
	}

	const auto Height = m_Where.height() - 4 - (m_ModalMode != 0);

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

bool TreeList::SetCurDir(string_view const NewDir, bool const ClosePanel, bool const IsUpdated, bool const Silent)
{
	if (m_ListData.empty())
		Update(0);

	if (!m_ListData.empty() && !SetDirPosition(NewDir))
	{
		Update(0);
		SetDirPosition(NewDir);
	}

	if (IsFocused())
	{
		Parent()->GetCmdLine()->SetCurDir(NewDir);
		Parent()->GetCmdLine()->Show();
	}

	return true; //???
}

bool TreeList::SetDirPosition(string_view const NewDir)
{
	for (size_t i = 0; i < m_ListData.size(); ++i)
	{
		if (equal_icase(NewDir, m_ListData[i].strName))
		{
			m_WorkDir = i;
			m_CurFile = static_cast<int>(i);
			m_CurTopFile = m_CurFile - (m_Where.height() - 2) / 2;
			CorrectPosition();
			return true;
		}
	}

	return false;
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

bool TreeList::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	if (!IsMouseInClientArea(MouseEvent))
		return false;

	if (Panel::ProcessMouseDrag(MouseEvent))
		return true;

	if (!(MouseEvent->dwButtonState & MOUSE_ANY_BUTTON_PRESSED))
		return false;

	const auto OldFile = m_CurFile;

	if (Global->Opt->ShowPanelScrollbar && IntKeyState.MousePos.x == m_Where.right &&
	        (MouseEvent->dwButtonState & 1) && !IsDragging())
	{
		const auto ScrollY = m_Where.top + 1;
		const auto Height = m_Where.height() - 4;

		if (IntKeyState.MousePos.y == ScrollY)
		{
			// Press and hold the [▲] button
			while_mouse_button_pressed([&](DWORD const Button)
			{
				if (Button != FROM_LEFT_1ST_BUTTON_PRESSED)
					return false;

				ProcessKey(Manager::Key(KEY_UP));
				return true;
			});

			if (!m_ModalMode)
				Parent()->SetActivePanel(this);

			return true;
		}

		if (IntKeyState.MousePos.y == ScrollY + Height - 1)
		{
			// Press and hold the [▼] button
			while_mouse_button_pressed([&](DWORD const Button)
			{
				if (Button != FROM_LEFT_1ST_BUTTON_PRESSED)
					return false;

				ProcessKey(Manager::Key(KEY_DOWN));
				return true;
			});

			if (!m_ModalMode)
				Parent()->SetActivePanel(this);

			return true;
		}

		if (IntKeyState.MousePos.y > ScrollY && IntKeyState.MousePos.y < ScrollY + Height - 1 && Height>2)
		{
			m_CurFile = static_cast<int>(m_ListData.size() - 1) * (IntKeyState.MousePos.y - ScrollY) / (Height - 2);
			DisplayTree(true);

			if (!m_ModalMode)
				Parent()->SetActivePanel(this);

			return true;
		}
	}

	// BUGBUG
	if (!(MouseEvent->dwButtonState & 3))
		return false;

	if (MouseEvent->dwMousePosition.Y > m_Where.top && MouseEvent->dwMousePosition.Y < m_Where.bottom - 2)
	{
		if (!m_ModalMode)
			Parent()->SetActivePanel(this);

		MoveToMouse(MouseEvent);
		DisplayTree(true);

		if (m_ListData.empty())
			return true;

		if (((MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) &&
		        MouseEvent->dwEventFlags==DOUBLE_CLICK) ||
		        ((MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED) &&
		         !MouseEvent->dwEventFlags) ||
		        (OldFile!=m_CurFile && Global->Opt->Tree.AutoChangeFolder && !m_ModalMode))
		{
			const auto control = MouseEvent->dwControlKeyState & (SHIFT_PRESSED | LEFT_ALT_PRESSED | LEFT_CTRL_PRESSED | RIGHT_ALT_PRESSED | RIGHT_CTRL_PRESSED);

			//вызовем EMenu если он есть
			if (!Global->Opt->RightClickSelect && MouseEvent->dwButtonState == RIGHTMOST_BUTTON_PRESSED && (control == 0 || control == SHIFT_PRESSED) && Global->CtrlObject->Plugins->FindPlugin(Global->Opt->KnownIDs.Emenu.Id))
			{
				Global->CtrlObject->Plugins->CallPlugin(Global->Opt->KnownIDs.Emenu.Id, OPEN_FILEPANEL, nullptr); // EMenu Plugin :-)
				return true;
			}

			ProcessEnter();
			return true;
		}

		return true;
	}

	if (MouseEvent->dwMousePosition.Y <= m_Where.top + 1 || MouseEvent->dwMousePosition.Y >= m_Where.bottom - 2)
	{
		if (!m_ModalMode)
			Parent()->SetActivePanel(this);

		if (m_ListData.empty())
			return true;

		while_mouse_button_pressed([&](DWORD)
		{
			if (IntKeyState.MousePos.y <= m_Where.top + 1)
				Up(1);
			else if (IntKeyState.MousePos.y >= m_Where.bottom - 2)
				Down(1);

			return true;
		});

		if (Global->Opt->Tree.AutoChangeFolder && !m_ModalMode)
			ProcessKey(Manager::Key(KEY_ENTER));

		return true;
	}

	return false;
}

void TreeList::MoveToMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	m_CurFile = m_CurTopFile + MouseEvent->dwMousePosition.Y - m_Where.top - 1;
	CorrectPosition();
}

void TreeList::ProcessEnter()
{
	auto& CurPtr=m_ListData[m_CurFile];
	if (os::fs::is_directory(CurPtr.strName))
	{
		if (!m_ModalMode && FarChDir(CurPtr.strName))
		{
			const auto AnotherPanel=GetRootPanel();
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

bool TreeList::ReadTreeFile()
{
	m_ReadingTree = true;
	SCOPE_EXIT{ m_ReadingTree = false; };

	size_t RootLength=m_Root.empty()?0:m_Root.size()-1;
	//SaveState();
	FlushCache();
	auto strName = MkTreeFileName(m_Root);
#if defined(TREEFILE_PROJECT)
	if (strName.empty())
		return false;
#endif

	auto TreeFile = OpenCacheableTreeFile(m_Root, strName, false);
	if (!TreeFile)
	{
		//RestoreState();
		return false;
	}

	m_ListData.clear();

	ReadLines(TreeFile, [&](const string_view Name) { m_ListData.emplace_back(string_view(m_Root).substr(0, RootLength) + Name); });

	TreeFile.Close();

	if (m_ListData.empty())
		return false;

	return FillLastData();
}

bool TreeList::GetPlainString(string& Dest, int ListPos) const
{
	Dest.clear();

	if constexpr (features::mantis_698)
	{
		if (static_cast<size_t>(ListPos) < m_ListData.size())
		{
			Dest = m_ListData[ListPos].strName;
			return true;
		}
	}

	return false;
}

bool TreeList::FindPartName(string_view const Name,int Next,int Direct)
{
	const auto strMask = exclude_sets(Name + L'*');

	for (int i=m_CurFile+(Next?Direct:0); i >= 0 && static_cast<size_t>(i) < m_ListData.size(); i+=Direct)
	{
		if (CmpName(strMask, m_ListData[i].strName, true, i == m_CurFile))
		{
			m_CurFile=i;
			m_CurTopFile = m_CurFile - (m_Where.height() - 2) / 2;
			DisplayTree(true);
			return true;
		}
	}

	for (size_t i=(Direct > 0)?0:m_ListData.size()-1; (Direct > 0) ? i < static_cast<size_t>(m_CurFile):i > static_cast<size_t>(m_CurFile); i+=Direct)
	{
		if (CmpName(strMask, m_ListData[i].strName, true))
		{
			m_CurFile=static_cast<int>(i);
			m_CurTopFile = m_CurFile - (m_Where.height() - 2) / 2;
			DisplayTree(true);
			return true;
		}
	}

	return false;
}

size_t TreeList::GetSelCount() const
{
	return 1;
}

bool TreeList::GetSelName(string *Name, string *ShortName, os::fs::find_data *fd)
{
	if (!Name)
	{
		m_GetSelPosition=0;
		return true;
	}

	if (!m_GetSelPosition)
	{
		*Name = GetCurDir();

		if (ShortName )
			*ShortName = *Name;

		if (fd)
			fd->Attributes = FILE_ATTRIBUTE_DIRECTORY;

		m_GetSelPosition++;
		return true;
	}

	m_GetSelPosition=0;
	return false;
}

bool TreeList::GetCurName(string &strName, string &strShortName) const
{
	if (m_ListData.empty())
		return false;

	strName = m_ListData[m_CurFile].strName;
	strShortName = strName;
	return true;
}

void TreeList::AddTreeName(const string_view Name)
{
	if (Global->Opt->Tree.TurnOffCompletely)
		return;
	if (Name.empty())
		return;

	const auto strFullName = ConvertNameToFull(Name);
	const auto strRoot = extract_root_directory(strFullName);
	string NamePart = strFullName.substr(strRoot.size() - 1);

	if (!ContainsSlash(NamePart))
	{
		return;
	}

	ReadCache(strRoot);
	TreeCache().add(std::move(NamePart));
}

void TreeList::DelTreeName(const string_view Name)
{
	if (Global->Opt->Tree.TurnOffCompletely)
		return;
	if (Name.empty())
		return;

	const auto strFullName = ConvertNameToFull(Name);
	const auto strRoot = extract_root_directory(strFullName);
	const auto NamePart = string_view(strFullName).substr(strRoot.size() - 1);

	ReadCache(strRoot);
	TreeCache().remove(NamePart);
}

void TreeList::RenTreeName(string_view const SrcName, string_view const DestName)
{
	if (Global->Opt->Tree.TurnOffCompletely)
		return;

	const auto strSrcRoot = extract_root_directory(ConvertNameToFull(SrcName));
	const auto strDestRoot = extract_root_directory(ConvertNameToFull(DestName));

	if (!equal_icase(strSrcRoot, strDestRoot))
	{
		DelTreeName(SrcName);
		ReadSubTree(SrcName);
	}

	const auto SrcNamePart = SrcName.substr(strSrcRoot.size() - 1);
	const auto DestNamePart = DestName.substr(strDestRoot.size() - 1);
	ReadCache(strSrcRoot);

	TreeCache().rename(SrcNamePart, DestNamePart);
}

void TreeList::ReadSubTree(string_view const Path)
{
	if (!os::fs::is_directory(Path))
		return;

	SCOPED_ACTION(TPreRedrawFuncGuard)(std::make_unique<TreePreRedrawItem>());
	ScanTree ScTree(false, true, -1, false);

	const auto strDirName = ConvertNameToFull(Path);
	AddTreeName(strDirName);
	auto AscAbort = false;
	ScTree.SetFindPath(strDirName, L"*"sv);
	LastScrX = ScrX;
	LastScrY = ScrY;

	os::fs::find_data fdata;
	string strFullName;
	int Count = 0;
	while (ScTree.GetNextName(fdata, strFullName))
	{
		if (fdata.Attributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			MsgReadTree(Count + 1);

			if (CheckForEscSilent())
			{
				AscAbort=ConfirmAbortOp();
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

void TreeList::ReadCache(string_view const TreeRoot)
{
	if (Global->Opt->Tree.TurnOffCompletely)
		return;

	auto strTreeName = MkTreeFileName(TreeRoot);
	if (strTreeName == TreeCache().GetTreeName())
		return;

	if (!TreeCache().empty())
		FlushCache();

	const auto TreeFile = OpenCacheableTreeFile(TreeRoot, strTreeName, false);
	if (!TreeFile)
	{
		ClearCache();
		return;
	}

	TreeCache().SetTreeName(TreeFile.GetName());

	ReadLines(TreeFile, [](const string_view Name){ TreeCache().add(Name); });
}

void TreeList::FlushCache()
{
	if (Global->Opt->Tree.TurnOffCompletely)
		return;

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
		const auto AnotherPanel = std::dynamic_pointer_cast<QuickView>(GetRootPanel());
		if (AnotherPanel && SetCurPath())
		{
			AnotherPanel->ShowFile(GetCurDir(), nullptr, false, nullptr);
		}
	}
}

bool TreeList::GoToFile(long idxItem)
{
	if (static_cast<size_t>(idxItem) < m_ListData.size())
	{
		m_CurFile=idxItem;
		CorrectPosition();
		return true;
	}

	return false;
}

bool TreeList::GoToFile(const string_view Name, const bool OnlyPartName)
{
	return GoToFile(FindFile(Name,OnlyPartName));
}

long TreeList::FindFile(const string_view Name, const bool OnlyPartName)
{
	const auto ItemIterator = std::find_if(CONST_RANGE(m_ListData, i)
	{
		return equal_icase(Name, OnlyPartName? PointToName(i.strName) : i.strName);
	});

	return ItemIterator == m_ListData.cend()? -1 : ItemIterator - m_ListData.cbegin();
}

long TreeList::FindFirst(string_view const Name)
{
	return FindNext(0, Name);
}

long TreeList::FindNext(int StartPos, string_view const Name)
{
	if (static_cast<size_t>(StartPos) < m_ListData.size())
	{
		const auto ItemIterator = std::find_if(CONST_RANGE(m_ListData, i)
		{
			return CmpName(Name, i.strName, true) && !IsParentDirectory(i.strName);
		});

		if (ItemIterator != m_ListData.cend())
			return static_cast<long>(ItemIterator - m_ListData.cbegin());
	}

	return -1;
}

bool TreeList::GetFileName(string &strName, int const Pos, os::fs::attributes& FileAttr) const
{
	if (Pos < 0 || static_cast<size_t>(Pos) >= m_ListData.size())
		return false;

	strName = m_ListData[Pos].strName;
	FileAttr = FILE_ATTRIBUTE_DIRECTORY | os::fs::get_file_attributes(m_ListData[Pos].strName);
	return true;
}

void TreeList::OnFocusChange(bool Get)
{
	if (!Get && static_cast<size_t>(m_CurFile) < m_ListData.size())
	{
		if (!os::fs::exists(m_ListData[m_CurFile].strName))
		{
			DelTreeName(m_ListData[m_CurFile].strName);
			Update(UPDATE_KEEP_SELECTION);
		}
	}

	Panel::OnFocusChange(Get);
}

void TreeList::UpdateKeyBar()
{
	auto& Keybar = Parent()->GetKeybar();
	Keybar.SetLabels(lng::MKBTreeF1);
	Keybar.SetCustomLabels(KBA_TREE);
}

void TreeList::RefreshTitle()
{
	m_Title = L'{';
	if (!m_ListData.empty())
	{
		append(m_Title, m_ListData[m_CurFile].strName, L" - "sv);
	}
	append(m_Title, msg(m_ModalMode? lng::MFindFolderTitle : lng::MTreeTitle), L'}');
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
