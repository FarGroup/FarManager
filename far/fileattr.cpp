/*
fileattr.cpp

Работа с атрибутами файлов
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
#include "fileattr.hpp"

// Internal:
#include "flink.hpp"
#include "lang.hpp"
#include "fileowner.hpp"
#include "exception.hpp"
#include "stddlg.hpp"

// Platform:
#include "platform.fs.hpp"

// Common:
#include "common/function_ref.hpp"
#include "common/scope_exit.hpp"

// External:

//----------------------------------------------------------------------------

static void retrievable_ui_operation(function_ref<bool()> const Action, string_view const Name, lng const ErrorDescription, bool& SkipErrors)
{
	while (!Action())
	{
		const auto ErrorState = error_state::fetch();

		switch (SkipErrors? operation::skip_all : OperationFailed(ErrorState, Name, lng::MError, msg(ErrorDescription)))
		{
		case operation::retry:
			break;

		case operation::skip_all:
			SkipErrors = true;
			[[fallthrough]];
		case operation::skip:
			return;

		case operation::cancel:
			cancel_operation();
		}
	}
}

static auto without_ro(string_view const Name, os::fs::attributes const Attributes, function_ref<bool()> const Action)
{
	// FILE_ATTRIBUTE_SYSTEM prevents encryption
	const auto Mask = FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM;

	return [=]
	{
		if ((Attributes & Mask) && !os::fs::set_file_attributes(Name, Attributes & ~Mask))
			return false;

		SCOPE_EXIT
		{
			SCOPED_ACTION(os::last_error_guard);

			if (Attributes & Mask)
				(void)os::fs::set_file_attributes(Name, Attributes); //BUGBUG
		};

		return Action();
	};
}

void ESetFileAttributes(string_view const Name, os::fs::attributes Attributes, bool& SkipErrors)
{
	if ((Attributes & FILE_ATTRIBUTE_DIRECTORY) && (Attributes & FILE_ATTRIBUTE_TEMPORARY))
		Attributes &= ~FILE_ATTRIBUTE_TEMPORARY;

	retrievable_ui_operation([&]{ return os::fs::set_file_attributes(Name, Attributes); }, Name, lng::MSetAttrCannotFor, SkipErrors);
}

static bool set_file_compression(string_view const Name, bool const State)
{
	const os::fs::file File(Name, FILE_READ_DATA | FILE_WRITE_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN);
	if (!File)
		return false;

	USHORT NewState = State? COMPRESSION_FORMAT_DEFAULT : COMPRESSION_FORMAT_NONE;
	return File.IoControl(FSCTL_SET_COMPRESSION, &NewState, sizeof(NewState), nullptr, 0);
}

void ESetFileCompression(string_view const Name, bool const State, os::fs::attributes const CurrentAttributes, bool& SkipErrors)
{
	if (!!(CurrentAttributes & FILE_ATTRIBUTE_COMPRESSED) == State)
		return;

	const auto Implementation = [&]
	{
		if (State && (CurrentAttributes & FILE_ATTRIBUTE_ENCRYPTED) && !os::fs::set_file_encryption(Name, false))
			return false;

		return set_file_compression(Name, State);
	};

	retrievable_ui_operation(
		without_ro(Name, CurrentAttributes, Implementation),
		Name, lng::MSetAttrCompressedCannotFor, SkipErrors);
}

void ESetFileEncryption(string_view const Name, bool const State, os::fs::attributes const CurrentAttributes, bool& SkipErrors)
{
	if (!!(CurrentAttributes & FILE_ATTRIBUTE_ENCRYPTED) == State)
		return;

	const auto Implementation = [&]
	{
		return os::fs::set_file_encryption(Name, State);
	};

	retrievable_ui_operation(
		without_ro(Name, CurrentAttributes, Implementation),
		Name, lng::MSetAttrEncryptedCannotFor, SkipErrors);
}


void ESetFileTime(
	string_view const Name,
	os::chrono::time_point const* const LastWriteTime,
	os::chrono::time_point const* const CreationTime,
	os::chrono::time_point const* const LastAccessTime,
	os::chrono::time_point const* const ChangeTime,
	os::fs::attributes const CurrentAttributes,
	bool& SkipErrors)
{
	if (!LastWriteTime && !CreationTime && !LastAccessTime && !ChangeTime)
		return;

	const auto Implementation = [&]
	{
		const os::fs::file File(Name, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT);
		if (!File)
			return false;

		return File.SetTime(CreationTime, LastAccessTime, LastWriteTime, ChangeTime);
	};

	retrievable_ui_operation(
		without_ro(Name, CurrentAttributes, Implementation),
		Name, lng::MSetAttrTimeCannotFor, SkipErrors);
}

static bool set_file_sparse(string_view const Name, bool const State)
{
	const os::fs::file File(Name, FILE_WRITE_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING);
	if (!File)
		return false;

	FILE_SET_SPARSE_BUFFER Buffer{ State };
	return File.IoControl(FSCTL_SET_SPARSE, &Buffer, sizeof(Buffer), nullptr, 0);
}

void ESetFileSparse(string_view const Name, bool const State, os::fs::attributes const CurrentAttributes, bool& SkipErrors)
{
	if ((CurrentAttributes & FILE_ATTRIBUTE_DIRECTORY) || !!(CurrentAttributes & FILE_ATTRIBUTE_SPARSE_FILE) == State)
		return;

	const auto Implementation = [&]
	{
		return set_file_sparse(Name, State);
	};

	retrievable_ui_operation(
		without_ro(Name, CurrentAttributes, Implementation),
		Name, lng::MSetAttrSparseCannotFor, SkipErrors);
}

void ESetFileOwner(string_view const Name, const string& Owner, bool& SkipErrors)
{
	retrievable_ui_operation([&]{ return SetFileOwner(Name, Owner); }, Name, lng::MSetAttrOwnerCannotFor, SkipErrors);
}

void EDeleteReparsePoint(string_view const Name, os::fs::attributes const CurrentAttributes, bool& SkipErrors)
{
	if (!(CurrentAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
		return;

	retrievable_ui_operation([&]{ return DeleteReparsePoint(Name); }, Name, lng::MSetAttrReparsePointCannotFor, SkipErrors);
}

void enum_attributes(function_ref<bool(os::fs::attributes, wchar_t)> const Pred)
{
	// The order and the symbols are (mostly) the same as in Windows UI
	static const std::pair<wchar_t, os::fs::attributes> AttrMap[]
	{
		{ L'N', FILE_ATTRIBUTE_NORMAL },
		{ L'R', FILE_ATTRIBUTE_READONLY },
		{ L'H', FILE_ATTRIBUTE_HIDDEN },
		{ L'S', FILE_ATTRIBUTE_SYSTEM },
		{ L'D', FILE_ATTRIBUTE_DIRECTORY },
		{ L'A', FILE_ATTRIBUTE_ARCHIVE },
		{ L'T', FILE_ATTRIBUTE_TEMPORARY },
		{ L'$', FILE_ATTRIBUTE_SPARSE_FILE },           // Used to be 'P' in Windows prior 10, which repurposed 'P' for 'Pinned'
		{ L'L', FILE_ATTRIBUTE_REPARSE_POINT },
		{ L'C', FILE_ATTRIBUTE_COMPRESSED },
		{ L'O', FILE_ATTRIBUTE_OFFLINE },
		{ L'I', FILE_ATTRIBUTE_NOT_CONTENT_INDEXED },
		{ L'E', FILE_ATTRIBUTE_ENCRYPTED },
		{ L'V', FILE_ATTRIBUTE_INTEGRITY_STREAM },
		{ L'?', FILE_ATTRIBUTE_VIRTUAL },               // Unknown symbol
		{ L'X', FILE_ATTRIBUTE_NO_SCRUB_DATA },
		{ L'P', FILE_ATTRIBUTE_PINNED },
		{ L'U', FILE_ATTRIBUTE_UNPINNED },
		{ L'‼', FILE_ATTRIBUTE_RECALL_ON_OPEN },        // Unknown symbol
		{ L'!', FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS }, // Unknown symbol
		{ L'B', FILE_ATTRIBUTE_STRICTLY_SEQUENTIAL },   // "SMR Blob" in attrib.exe
	};

	for (const auto& [Letter, Attr]: AttrMap)
	{
		if (!Pred(Attr, Letter))
			break;
	}
}
