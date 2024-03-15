/*
print.cpp

Печать (Alt-F5)
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
#include "print.hpp"

// Internal:
#include "panel.hpp"
#include "vmenu.hpp"
#include "vmenu2.hpp"
#include "filelist.hpp"
#include "savescr.hpp"
#include "ctrlobj.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "delete.hpp"
#include "mix.hpp"
#include "lang.hpp"
#include "plugins.hpp"
#include "strmix.hpp"
#include "global.hpp"
#include "log.hpp"
#include "stddlg.hpp"
#include "datetime.hpp"
#include "exception.hpp"

// Platform:
#include "platform.hpp"
#include "platform.fs.hpp"

// Common:
#include "common/io.hpp"
#include "common/scope_exit.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

static void AddToPrintersMenu(VMenu2 *PrinterList, std::span<PRINTER_INFO_4W const> const Printers)
{
	string strDefaultPrinter;
	// BUGBUG check result
	if (!os::GetDefaultPrinter(strDefaultPrinter))
	{
		LOGWARNING(L"GetDefaultPrinter(): {}"sv, os::last_error());
	}

	bool bDefaultPrinterFound = false;

	for (const auto& printer: Printers)
	{
		MenuItemEx Item(printer.pPrinterName);

		if (!bDefaultPrinterFound && printer.pPrinterName == strDefaultPrinter)
		{
			bDefaultPrinterFound = true;
			Item.SetCheck();
			Item.SetSelect(true);
		}
		Item.ComplexUserData = Item.Name;
		PrinterList->AddItem(Item);
	}

	if (!bDefaultPrinterFound)
		PrinterList->SetSelectPos(0, 1);
}

void PrintFiles(FileList* SrcPanel)
{
	try
	{
		const auto SelCount = SrcPanel->GetSelCount();
		if (!SelCount)
			return;

		const auto Enumerator = SrcPanel->enum_selected();
		const auto DirsCount = std::ranges::fold_left(Enumerator, 0uz, [](size_t Count, const os::fs::find_data& i)
		{
			return Count + os::fs::is_directory(i);
		});

		if (DirsCount == SelCount)
			return;

		block_ptr<PRINTER_INFO_4W, os::default_buffer_size> pi(os::default_buffer_size);

		DWORD Needed = 0, PrintersCount = 0;

		while (!EnumPrinters(
			PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
			nullptr,
			4,
			std::bit_cast<BYTE*>(pi.data()),
			static_cast<DWORD>(pi.size()),
			&Needed,
			&PrintersCount
		))
		{
			if (Needed > pi.size())
			{
				pi.reset(Needed);
				continue;
			}

			throw far_exception(msg(lng::MCannotEnumeratePrinters));
		}

		if (!PrintersCount)
			return;

		string strPrinterName;

		{
			string strTitle;
			if (SelCount == 1)
			{
				os::fs::find_data Data;
				if (!SrcPanel->get_first_selected(Data))
					return;

				strTitle = far::vformat(msg(lng::MPrintTo), quote_unconditional(truncate_left(Data.FileName, 50)));
			}
			else
			{
				strTitle = far::vformat(msg(lng::MPrintFilesTo), SelCount - DirsCount);
			}

			const auto PrinterList = VMenu2::create(strTitle, {}, ScrY - 4);
			PrinterList->SetMenuFlags(VMENU_WRAPMODE | VMENU_SHOWAMPERSAND);
			PrinterList->SetPosition({ -1, -1, 0, 0 });
			AddToPrintersMenu(PrinterList.get(), { pi.data(), PrintersCount });

			if (PrinterList->Run() < 0)
				return;

			strPrinterName = *PrinterList->GetComplexUserDataPtr<string>();
		}

		os::printer_handle Printer;

		if (!OpenPrinter(UNSAFE_CSTR(strPrinterName), &ptr_setter(Printer), nullptr))
			throw far_exception(msg(lng::MCannotOpenPrinter));

		SCOPED_ACTION(SaveScreen);

		single_progress const Progress(msg(lng::MPrintTitle), {}, 0);
		time_check const TimeCheck;

		HideCursor();

		const auto hPlugin = SrcPanel->GetPluginHandle();

		const auto UseInternalCommand = [&]
		{
			OpenPanelInfo Info;
			Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin, &Info);
			return PluginManager::UseInternalCommand(hPlugin, PLUGIN_FARGETFILE, Info);
		};

		const auto PluginMode = SrcPanel->GetMode() == panel_mode::PLUGIN_PANEL && !UseInternalCommand();

		size_t PrintIndex{};

		for (const auto& i: SrcPanel->enum_selected())
		{
			if (i.Attributes & FILE_ATTRIBUTE_DIRECTORY)
				continue;

			if (TimeCheck)
			{
				if (CheckForEscAndConfirmAbort())
					break;

				Progress.update(far::vformat(msg(lng::MPrintingFile), i.FileName));
				Progress.update(ToPercent(PrintIndex, SelCount - DirsCount));
			}

			++PrintIndex;

			delayed_deleter Deleter(true);

			string FileName;

			if (PluginMode)
			{
				const auto strTempDir = MakeTemp();
				if (!os::fs::create_directory(strTempDir))
					throw far_exception(L"create_directory error"sv);

				const auto ListItem = SrcPanel->GetLastSelectedItem();
				if (!ListItem)
					throw far_exception(L"GetLastSelectedItem error"sv);

				PluginPanelItemHolderHeap PanelItem;
				SrcPanel->FileListToPluginItem(*ListItem, PanelItem);

				if (!Global->CtrlObject->Plugins->GetFile(hPlugin, &PanelItem.Item, strTempDir, FileName, OPM_SILENT))
				{
					// BUGBUG check result
					if (!os::fs::remove_directory(strTempDir))
					{
						LOGWARNING(L"remove_directory({}): {}"sv, strTempDir, os::last_error());
					}

					throw far_exception(L"GetFile error"sv);
				}

				Deleter.add(FileName);
			}
			else
			{
				FileName = i.FileName;
			}

			try
			{
				const os::fs::file SrcFile(FileName, FILE_READ_DATA, os::fs::file_share_all, nullptr, OPEN_EXISTING);
				if (!SrcFile)
					throw far_exception(L"Cannot open the file"sv);

				os::fs::filebuf StreamBuffer(SrcFile, std::ios::in);
				std::istream Stream(&StreamBuffer);
				Stream.exceptions(Stream.badbit | Stream.failbit);

				DOC_INFO_1 di1{ UNSAFE_CSTR(FileName) };

				if (!StartDocPrinter(Printer.native_handle(), 1, std::bit_cast<BYTE*>(&di1)))
					throw far_exception(L"StartDocPrinter error"sv);

				SCOPE_EXIT{ EndDocPrinter(Printer.native_handle()); };

				for (;;)
				{
					std::byte Buffer[8192];
					const auto Read = io::read(Stream, Buffer);
					if (!Read)
						break;

					DWORD Written;
					if (!WritePrinter(Printer.native_handle(), Buffer, static_cast<DWORD>(Read), &Written))
						throw far_exception(L"WritePrinter error"sv);
				}

				SrcPanel->ClearLastGetSelection();
			}
			catch (far_exception const& e)
			{
				if (Message(MSG_WARNING, e,
					msg(lng::MPrintTitle),
					{
						msg(lng::MCannotPrint),
						i.FileName
					},
					{ lng::MSkip, lng::MCancel }) != message_result::first_button)
					break;
			}
		}
	}
	catch (far_exception const& e)
	{
		Message(MSG_WARNING, e,
			msg(lng::MPrintTitle),
			{},
			{ lng::MOk });
	}

	SrcPanel->Redraw();
}
