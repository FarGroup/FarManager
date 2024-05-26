// validator: no-self-include
/*
testing.cpp

Testing framework wrapper

*/
/*
Copyright © 2019 Far Group
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

#ifdef ENABLE_TESTS

#define CATCH_CONFIG_RUNNER

// Self:
#include "testing.hpp"

// Internal:
#include "components.hpp"
#include "locale.hpp"
#include "log.hpp"

// Platform:

// Common:

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

constexpr auto DebugTests = false;


generic_exception_matcher::generic_exception_matcher(std::function<bool(std::any const&)> Matcher):
	m_Matcher(std::move(Matcher))
{}

bool generic_exception_matcher::match(std::any const& e) const
{
	return m_Matcher(e);
}

std::string generic_exception_matcher::describe() const
{
	return "Generic matcher"s;
}

static bool is_ui_test_run(std::span<wchar_t const* const> const Args)
{
	// Heuristics to make it work with various VS test adapters
	for (auto It = Args.begin(); It != Args.end(); ++It)
	{
		if (
			*It == L"--libidentify"sv ||
			*It == L"--list-test-names"sv ||
			*It == L"--list-test-names-only"sv ||
			(
				(*It == L"-r"sv || *It == L"--reporter"sv) &&
				It + 1 != Args.end() && *(It + 1) == L"xml"sv
			)
		)
			return true;
	}

	return false;
}

std::optional<int> testing_main(std::span<wchar_t const* const> const Args)
{
	if (is_ui_test_run(Args.subspan(1)))
		return Catch::Session().run(static_cast<int>(Args.size()), Args.data());

	const auto ServiceTestIterator = std::ranges::find(Args, L"/service:test"sv);
	const auto IsBuildStep = ServiceTestIterator != Args.end();

	if constexpr (DebugTests)
	{
		if (IsBuildStep)
		{
			std::wcout << L"Unit tests skipped"sv << std::endl;
			return EXIT_SUCCESS;
		}

		if (Args.size() == 3 && logging::is_log_argument(Args[1]))
			return {};
	}
	else
	{
		if (!IsBuildStep)
			return {};
	}

	std::vector<const wchar_t*> NewArgs;

	if (IsBuildStep)
	{
		NewArgs.reserve(Args.size() - 1 + 2);
		NewArgs.emplace_back(Args[0]);
		NewArgs.insert(NewArgs.end(), ServiceTestIterator + 1, Args.end());
	}
	else
	{
		NewArgs.reserve(Args.size() + 1 + DebugTests + 2);
		NewArgs.assign(ALL_CONST_RANGE(Args));
		NewArgs.emplace_back(L"--break");

		if (DebugTests)
			NewArgs.emplace_back(L"--wait-for-keypress exit");
	}

	NewArgs.emplace_back(L"--warn");
	NewArgs.emplace_back(L"NoAssertions");

	locale = invariant_locale();

	return Catch::Session().run(static_cast<int>(NewArgs.size()), NewArgs.data());
}

namespace
{
	SCOPED_ACTION(components::component)([]
	{
		return components::info{ L"Catch2"sv, far::format(L"{}.{}.{}"sv, CATCH_VERSION_MAJOR, CATCH_VERSION_MINOR, CATCH_VERSION_PATCH) };
	});
}

#endif
