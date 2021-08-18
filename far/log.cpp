/*
log.cpp
*/
/*
Copyright © 2021 Far Group
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
#include "log.hpp"

// Internal:
#include "colormix.hpp"
#include "console.hpp"
#include "datetime.hpp"
#include "encoding.hpp"
#include "eol.hpp"
#include "exception.hpp"
#include "exception_handler.hpp"
#include "farversion.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "palette.hpp"
#include "pathmix.hpp"
#include "pipe.hpp"
#include "strmix.hpp"
#include "tracer.hpp"

// Platform:
#include "platform.concurrency.hpp"
#include "platform.env.hpp"
#include "platform.fs.hpp"
#include "platform.version.hpp"

// Common:
#include "common/from_string.hpp"
#include "common/lazy.hpp"
#include "common/scope_exit.hpp"

// External:

//----------------------------------------------------------------------------

namespace
{
	static const auto log_argument = L"/service:log_viewer"sv;

	static string get_parameter(string_view const Name)
	{
		return os::env::get(concat(L"far.log."sv, Name));
	}

	template<typename sink_type>
	static string get_sink_parameter(string_view const Name)
	{
		return get_parameter(concat(L"sink."sv, sink_type::name, L'.', Name));
	}

	static auto parse_level(string_view const Str, logging::level const Default)
	{
		if (Str.empty())
			return Default;

		using level = logging::level;

		static std::unordered_map<string_view, level, hash_icase_t, equal_icase_t> LevelMap
		{
#define STRLEVEL(x) { WSTRVIEW(x), level::x }
			STRLEVEL(off),
			STRLEVEL(fatal),
			STRLEVEL(error),
			STRLEVEL(warning),
			STRLEVEL(notice),
			STRLEVEL(info),
			STRLEVEL(debug),
			STRLEVEL(trace),
			STRLEVEL(all),
#undef STRLEVEL
		};

		const auto ItemIterator = LevelMap.find(Str);
		return ItemIterator == LevelMap.cend()? Default : ItemIterator->second;
	}

	static string_view level_to_string(logging::level const Level)
	{
		using level = logging::level;

		switch (Level)
		{
#define LEVELTOSTR(x) case level::x: return WSTRVIEW(x)
		LEVELTOSTR(off);
		LEVELTOSTR(fatal);
		LEVELTOSTR(error);
		LEVELTOSTR(warning);
		LEVELTOSTR(notice);
		LEVELTOSTR(info);
		LEVELTOSTR(debug);
		LEVELTOSTR(trace);
		LEVELTOSTR(all);
#undef LEVELTOSTR
		default:
			return L"???"sv;
		}
	}

	static WORD level_to_color(logging::level const Level)
	{
		using level = logging::level;

		if (Level <= level::fatal)   return F_WHITE | B_LIGHTRED;
		if (Level <= level::error)   return F_LIGHTRED;
		if (Level <= level::warning) return F_YELLOW;
		if (Level <= level::notice)  return F_LIGHTCYAN;
		if (Level <= level::info)    return F_LIGHTGRAY;
		if (Level <= level::debug)   return F_CYAN;
		if (Level <= level::trace)   return F_DARKGRAY;

		return F_DARKGRAY;
	}

	static string get_location(std::string_view const Function, std::string_view const File, int const Line)
	{
		return format(FSTR(L"{}, {}({})"sv), encoding::utf8::get_chars(Function), encoding::utf8::get_chars(File), Line);
	}

	static string get_thread_id()
	{
		return str(GetCurrentThreadId());
	}

	struct message
	{
		message(string_view const Str, logging::level const Level, std::string_view const Function, std::string_view const File, int const Line, size_t const TraceDepth):
			m_ThreadId(get_thread_id()),
			m_LevelString(level_to_string(Level)),
			m_Data(Str),
			m_Location(get_location(Function, File, Line)),
			m_Level(Level)
		{
			std::tie(m_Date, m_Time) = get_time();

			if (TraceDepth)
			{
				m_Data += L"\nLog stack:\n"sv;

				const auto FramesToSkip = 4; // log -> engine.log -> submit -> this ctor
				tracer.get_symbols({}, os::debug::current_stack(FramesToSkip, TraceDepth), [&](string_view const TraceLine)
				{
					append(m_Data, TraceLine, L'\n');
				});
			}
		}

		message() = default;

		string m_Date;
		string m_Time;
		string m_ThreadId;
		string m_LevelString;
		string m_Data;
		string m_Location;
		logging::level m_Level{ logging::level::off };
	};

	class sink
	{
	public:
		NONCOPYABLE(sink);

		sink() = default;
		virtual ~sink() = default;
		virtual void configure(string_view const Parameters) {}
		virtual void handle(message Message) = 0;
		virtual string_view get_name() = 0;
	};

	template<typename sink_type>
	class sink_boilerplate: public sink
	{
	public:
		string_view get_name() override
		{
			return sink_type::name;
		}
	};

	template<bool Value>
	struct discardable
	{
		static constexpr auto is_discardable = Value;
	};

	class sink_null: public discardable<true>, public sink_boilerplate<sink_null>
	{
	public:
		void handle(message Message) override
		{
		}

		static constexpr auto name = L"null"sv;
	};

	class sink_debug: public discardable<false>, public sink_boilerplate<sink_debug>
	{
	public:
		void handle(message Message) override
		{
			os::debug::print(concat(
				L'[', Message.m_Time, L']',
				L'[', Message.m_ThreadId, L']',
				L'[', Message.m_LevelString, L']',
				L' ', Message.m_Data, L' ',
				L'[', Message.m_Location, L']',
				L'\n'
			));
		}

		static constexpr auto name = L"debug"sv;
	};

	class sink_console: public discardable<true>, public sink_boilerplate<sink_console>
	{
	public:
		sink_console():
			m_Buffer(CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, {}, CONSOLE_TEXTMODE_BUFFER, {}))
		{
			const auto Size = GetLargestConsoleWindowSize(m_Buffer.native_handle());
			SetConsoleScreenBufferSize(m_Buffer.native_handle(), { Size.X, 9999 });
		}

		static void process(HANDLE Buffer, message const& Message)
		{
			class console_color
			{
				NONCOPYABLE(console_color);

			public:
				explicit console_color(HANDLE Buffer, WORD const Color):
					m_Buffer(Buffer)
				{
					CONSOLE_SCREEN_BUFFER_INFO csbi;
					if (!GetConsoleScreenBufferInfo(Buffer, &csbi))
						return;

					m_SavedAttributes = csbi.wAttributes;
					SetConsoleTextAttribute(Buffer, Color);
				}

				~console_color()
				{
					if (m_SavedAttributes)
						SetConsoleTextAttribute(m_Buffer, *m_SavedAttributes);
				}

			private:
				std::optional<WORD> m_SavedAttributes;
				HANDLE m_Buffer;
			};

			const auto write_console = [Buffer](string_view Str)
			{
				DWORD Written;
				WriteConsole(Buffer, Str.data(), static_cast<DWORD>(Str.size()), &Written, {});
			};

			const auto write = [&](string_view const Borders, WORD const Color, string_view const Str)
			{
				SCOPED_ACTION(console_color)(Buffer, Color);

				write_console(Borders.substr(0, 1));
				write_console(Str);
				write_console(Borders.substr(1, 1));
			};

			write(L"[]"sv, F_DARKGRAY, Message.m_Time);
			write(L"[]"sv, F_DARKGRAY, Message.m_ThreadId);

			const auto Color = level_to_color(Message.m_Level);
			write(L"[]"sv, Color, Message.m_LevelString);
			write(L"  "sv, Color, Message.m_Data);

			write(L"[]"sv, F_DARKGRAY, Message.m_Location);

			write_console(L"\n");
		}

		void handle(message Message) override
		{
			process(m_Buffer.native_handle(), Message);
		}

		void configure(string_view const Parameters) override
		{
			if (Parameters.empty())
			{
				console.SetActiveScreenBuffer(m_Buffer.native_handle());

				for (;;)
				{
					os::handle::wait_all({ console.GetInputHandle() });

					if (CheckForEscSilent())
					{
						console.SetActiveScreenBuffer(console.GetOutputHandle());
						return;
					}
				}
			}
		}

		static constexpr auto name = L"console"sv;

	private:
		os::handle m_Buffer;
	};

	class sink_file: public discardable<false>, public sink_boilerplate<sink_file>
	{
	public:
		explicit sink_file():
			m_File(open_file()),
			m_StreamBuffer(m_File, std::ios::out),
			m_Stream(&m_StreamBuffer),
			m_Writer(m_Stream, CP_UTF8)
		{
			m_Stream.exceptions(m_Stream.badbit | m_Stream.failbit);
		}

		void handle(message Message) override
		{
			if (!m_File)
				return;

			try
			{
				m_Writer.write(L"["sv);
				m_Writer.write(Message.m_Date);
				m_Writer.write(L" "sv);
				m_Writer.write(Message.m_Time);
				m_Writer.write(L"]["sv);
				m_Writer.write(Message.m_ThreadId);
				m_Writer.write(L"]["sv);
				m_Writer.write(Message.m_LevelString);
				m_Writer.write(L"] "sv);
				m_Writer.write(Message.m_Data);
				m_Writer.write(L" "sv);
				m_Writer.write(L"["sv);
				m_Writer.write(Message.m_Location);
				m_Writer.write(L"]"sv);
				m_Writer.write(m_Eol);

				m_Stream.flush();
			}
			catch (std::exception const& e)
			{
				m_File.Close();

				LOGERROR(L"{}"sv, e);
			}
		}

		static constexpr auto name = L"file"sv;

	private:
		static string make_filename()
		{
			auto [Date, Time] = get_time();
			std::replace(ALL_RANGE(Date), L'/', L'.');
			std::replace(ALL_RANGE(Time), L':', L'.');

			return path::join
			(
				get_sink_parameter<sink_file>(L"path"sv),
				format(L"Far.{}_{}_{}.txt"sv, Date, Time, GetCurrentProcessId())
			);
		}

		static os::fs::file open_file()
		{
			os::fs::file File(make_filename(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS);
			if (!File)
				throw MAKE_FAR_EXCEPTION(L"Can't create a log file"sv);

			LOGINFO(L"Logging to {}"sv, File.GetName());

			File.SetPointer(0, {}, FILE_END);
			return File;
		}

		os::fs::file m_File;
		os::fs::filebuf m_StreamBuffer;
		std::ostream m_Stream;
		encoding::writer m_Writer;
		string_view m_Eol{ eol::win.str() };
	};

	class sink_pipe: public discardable<true>, public sink_boilerplate<sink_pipe>
	{
	public:
		void connect()
		{
			if (!PeekNamedPipe(m_Pipe.native_handle(), {}, 0, {}, {}, {}))
				disconnect();

			if (m_Connected)
				return;

			STARTUPINFO si{ sizeof(si) };
			PROCESS_INFORMATION pi{};

			if (!CreateProcess(m_ThisModule.c_str(), UNSAFE_CSTR(format(FSTR(L"\"{}\" {} {}"sv), m_ThisModule, log_argument, m_PipeName)), {}, {}, false, CREATE_NEW_CONSOLE, {}, {}, &si, &pi))
			{
				LOGERROR(L"{}"sv, last_error());
				return;
			}

			os::handle(pi.hThread);
			os::handle(pi.hProcess);

			while (!ConnectNamedPipe(m_Pipe.native_handle(), {}) && GetLastError() != ERROR_PIPE_CONNECTED)
			{
				LOGWARNING(L"ConnectNamedPipe({}): {}"sv, m_PipeName, last_error());
			}

			m_Connected = true;
		}

		void disconnect()
		{
			if (!m_Connected)
				return;

			DisconnectNamedPipe(m_Pipe.native_handle());
			m_Connected = false;
		}

		void handle(message Message) override
		{
			connect();

			if (!m_Connected)
				return;

			try
			{
				pipe::write(
					m_Pipe,
					Message.m_Level,
					Message.m_Time,
					Message.m_ThreadId,
					Message.m_LevelString,
					Message.m_Data,
					Message.m_Location
				);
			}
			catch (far_exception const& e)
			{
				disconnect();

				LOGERROR(L"{}"sv, e);
			}
		}

		static constexpr auto name = L"pipe"sv;

	private:
		string m_PipeName{ format(FSTR(L"\\\\.\\pipe\\far_{}.log"sv), GetCurrentProcessId()) };
		os::handle m_Pipe{ CreateNamedPipe(m_PipeName.c_str(), PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 1, 0, 0, 0, {}) };
		string m_ThisModule{ os::fs::get_current_process_file_name() };
		bool m_Connected{};
	};

	class async_impl
	{
	protected:
		template<typename... args>
		explicit async_impl(bool const IsDiscardable):
			m_IsDiscardable(IsDiscardable)
		{
			m_Thread = os::thread(os::thread::mode::join, &async_impl::poll, this);
		}

		virtual ~async_impl()
		{
			m_FinishEvent.set();
		}

		void in(message Message)
		{
			// The receiver takes all available messages in one go so this shouldn't happen.
			// However, should it fail to do so, this will prevent us from eating all the RAM:
			if (m_Messages.size() > QueueBufferSize * 2)
			{
				m_Messages.clear();
				LOGWARNING(L"Queue overflow"sv);
			}

			m_Messages.push(std::move(Message));
			m_MessageEvent.set();
		}

	private:
		const size_t QueueBufferSize = 8192;

		virtual void out(message&& Message) = 0;

		void poll()
		{
			os::debug::set_thread_name(L"Log sink");

			return seh_try_no_ui(
				[&]
				{
					message Message;

					for (;;)
					{
						if (os::handle::wait_any({ m_MessageEvent.native_handle(), m_FinishEvent.native_handle() }) != 0)
							return;

						for (auto Messages = m_Messages.pop_all(); !Messages.empty(); Messages.pop())
						{
							if (m_IsDiscardable && m_FinishEvent.is_signaled())
								return;

							out(std::move(Messages.front()));
						}
					}
				},
				[](DWORD const ExceptionCode)
				{
					LOGERROR(L"SEH Exception {}"sv, ExceptionCode);
				});
		}

		os::synced_queue<message> m_Messages;
		os::event m_MessageEvent { os::event::type::automatic, os::event::state::nonsignaled };
		os::event m_FinishEvent { os::event::type::manual, os::event::state::nonsignaled };

		bool m_IsDiscardable;
		os::thread m_Thread;
	};

	class sink_mode
	{
	public:
		enum class mode
		{
			sync,
			async,
		};

		virtual ~sink_mode() = default;
		virtual mode get_mode() const = 0;
	};

	template<typename sink_mode_type>
	class sink_mode_boilerplate: public sink_mode
	{
		mode get_mode() const override
		{
			return sink_mode_type::mode;
		}
	};

	template<typename sink_type>
	class async final: public sink_type, public sink_mode_boilerplate<async<sink_type>>, private async_impl
	{
	public:
		static constexpr auto mode = sink_mode::mode::async;

		explicit async(bool const IsDiscardable):
			async_impl(IsDiscardable)
		{
		}

		// sink
		void handle(message Message) override
		{
			return in(std::move(Message));
		}

		// async_impl
		void out(message&& Message) override
		{
			return sink_type::handle(std::move(Message));
		}
	};

	template<typename sink_type>
	class sync final: public sink_type, public sink_mode_boilerplate<sync<sink_type>>
	{
	public:
		static constexpr auto mode = sink_mode::mode::sync;

		// sink
		void handle(message Message) override
		{
			SCOPED_ACTION(std::lock_guard)(m_CS);
			sink_type::handle(std::move(Message));
		}

	private:
		os::critical_section m_CS;
	};
}

namespace logging
{
	class engine
	{
	public:
		enum class engine_status
		{
			incomplete,
			in_progress,
			complete,
		};

		NONCOPYABLE(engine);

		engine() = default;

		void configure(string_view const Parameters)
		{
			{
				SCOPED_ACTION(std::lock_guard)(m_CS);
				initialise();
			}

			if (equal_icase(Parameters, L"reconfigure"))
			{
				const auto Status = m_Status.exchange(engine_status::in_progress);
				SCOPE_EXIT{ m_Status = Status; flush_queue(); };

				configure_env();
			}

			for (auto& i: m_Sinks)
			{
				i->configure(Parameters);
			}
		}

		[[nodiscard]]
		bool filter(level const Level)
		{
			switch (m_Status)
			{
			case engine_status::incomplete:
				{
					SCOPED_ACTION(std::lock_guard)(m_CS);
					initialise();
				}
				[[fallthrough]];
			case engine_status::complete:
				return m_Level >= Level;

			case engine_status::in_progress:
				// We don't know yet, so let it pass and be queued
				return true;

			default:
				UNREACHABLE;
			}
		}

		void flush_queue()
		{
			if (m_Status != engine_status::complete || !m_QueuedMessagesCount)
				return;

			for (auto Messages = m_QueuedMessages.pop_all(); !Messages.empty(); Messages.pop())
			{
				const auto& Message = Messages.front();
				if (Message.m_Level <= m_Level)
					submit(Message);
			}

			m_QueuedMessagesCount = 0;
		}

		void log(string_view const Str, level const Level, std::string_view const Function, std::string_view const File, int const Line)
		{
			if (m_Status == engine_status::in_progress)
			{
				message Message(Str, Level, Function, File, Line, Level <= m_TraceLevel? m_TraceDepth : 0);
				m_QueuedMessages.emplace(std::move(Message));
				++m_QueuedMessagesCount;
				return;
			}

			if (!filter(Level))
				return;

			submit({ Str, Level, Function, File, Line, Level <= m_TraceLevel? m_TraceDepth : 0 });
		}

	private:
		void configure_env()
		{
			m_Level = parse_level(get_parameter(L"level"sv), m_Level);

			m_TraceLevel = parse_level(get_parameter(L"trace.level"sv), m_TraceLevel);

			if (size_t Depth; from_string(get_parameter(L"trace.depth"sv), Depth))
				m_TraceDepth = Depth;

			if (m_Level == level::off && m_Sinks.empty())
				return;

			const auto Enumerator = enum_tokens(get_parameter(L"sink"sv), L",;"sv);
			std::unordered_set<string_view> SinkNames;
			std::copy(ALL_CONST_RANGE(Enumerator), std::inserter(SinkNames, SinkNames.end()));

			configure_sinks<
				sink_console,
				sink_pipe,
				sink_debug,
				sink_file,
				sink_null
			>(SinkNames, m_Level != level::off);

			if (m_Sinks.empty())
				m_Level = level::off;
			else
				LOGINFO(L"Logging level: {}"sv, level_to_string(m_Level));
		}

		void initialise()
		{
			if (m_Status != engine_status::incomplete)
				return;

			m_Status = engine_status::in_progress;
			SCOPE_EXIT{ m_Status = engine_status::complete; flush_queue(); };

			// No recursion if it's the helper process
			if (contains(string_view{ GetCommandLine() }, log_argument))
				return;

			LOGINFO(L"{}"sv, build::version_string());
			LOGINFO(L"Windows {}", os::version::os_version());

			configure_env();
		}

		template<typename T>
		void configure_sink(std::unordered_set<string_view> const& SinkNames, bool const AllowAdd)
		{
			const auto Needed = contains(SinkNames, T::name);

			lazy<sink_mode::mode> const NewSinkMode([]
			{
				return get_sink_parameter<T>(L"mode"sv) == L"sync"sv? sink_mode::mode::sync : sink_mode::mode::async;
			});

			const auto same_sink = [](const auto& Sink)
			{
				return Sink->get_name().data() == T::name.data();
			};

			if (const auto SinkIterator = std::find_if(ALL_CONST_RANGE(m_Sinks), same_sink); SinkIterator != m_Sinks.cend())
			{
				if (Needed && dynamic_cast<sink_mode const&>(**SinkIterator).get_mode() == *NewSinkMode)
					return;

				m_Sinks.erase(SinkIterator);

				if (!Needed)
					return;
			}
			else
			{
				if (!Needed)
					return;
			}

			if (!AllowAdd)
				return;

			try
			{
				LOGINFO(L"Sink: {} ({})"sv, T::name, *NewSinkMode == sink_mode::mode::sync? L"sync"sv : L"async"sv);

				*NewSinkMode == sink_mode::mode::sync?
					m_Sinks.emplace_back(std::make_unique<sync<T>>()) :
					m_Sinks.emplace_back(std::make_unique<async<T>>(T::is_discardable));
			}
			catch (const far_exception& e)
			{
				LOGERROR(L"{}"sv, e);
			}
		}

		template<typename... args>
		void configure_sinks(std::unordered_set<string_view> const& SinkNames, bool const AllowAdd)
		{
			(..., configure_sink<args>(SinkNames, AllowAdd));
		}

		void submit(message const& Message)
		{
			for (const auto& i: m_Sinks)
				i->handle(Message);
		}

		os::concurrency::critical_section m_CS;
		std::vector<std::unique_ptr<sink>> m_Sinks;
		os::concurrency::synced_queue<message> m_QueuedMessages;
		std::atomic_size_t m_QueuedMessagesCount;
		std::atomic<level> m_Level{ level::off };
		level m_TraceLevel{ level::error };
		size_t m_TraceDepth{ std::numeric_limits<size_t>::max() };
		std::atomic<engine_status> m_Status{ engine_status::incomplete };
	};

	bool filter(level const Level)
	{
		return log_engine.filter(Level);
	}

	static thread_local size_t RecursionGuard{};

	void log(string_view const Str, level const Level, std::string_view const Function, std::string_view const File, int const Line)
	{
		// Log can potentially log itself, directly or through other parts of the code.
		// Allow one level of recursion for diagnostics
		if (RecursionGuard > 1)
			return;

		++RecursionGuard;
		SCOPE_EXIT{ --RecursionGuard; };

		log_engine.log(Str, Level, Function, File, Line);
	}

	void show()
	{
		log_engine.configure({});
	}

	void configure(string_view const Parameters)
	{
		log_engine.configure(Parameters);
	}

	bool is_log_argument(const wchar_t* const Argument)
	{
		return Argument == log_argument;
	}

	int main(const wchar_t* const PipeName)
	{
		console.SetTitle(concat(L"Far Log Viewer: "sv, PipeName));
		console.SetTextAttributes(colors::ConsoleColorToFarColor(F_LIGHTGRAY | B_BLACK));

		DWORD ConsoleMode = 0;
		console.GetMode(console.GetInputHandle(), ConsoleMode);
		console.SetMode(console.GetInputHandle(), ConsoleMode | ENABLE_EXTENDED_FLAGS | ENABLE_QUICK_EDIT_MODE);

		os::fs::file PipeFile;

		while (!PipeFile.Open(PipeName, GENERIC_READ, 0, {}, OPEN_EXISTING))
		{
			const auto ErrorState = last_error();
			std::wcerr << format(FSTR(L"Can't open pipe {}: {}"sv), PipeName, ErrorState.Win32ErrorStr()) << std::endl;

			if (!ConsoleYesNo(L"Retry"sv, false))
				return EXIT_FAILURE;
		}

		message Message;

		for (;;)
		{
			try
			{
				pipe::read(
					PipeFile.get(),
					Message.m_Level,
					Message.m_Time,
					Message.m_ThreadId,
					Message.m_LevelString,
					Message.m_Data,
					Message.m_Location
				);
			}
			catch (far_exception const& e)
			{
				if (e.Win32Error == ERROR_BROKEN_PIPE)
					return EXIT_SUCCESS;

				std::wcerr << format(FSTR(L"Error reading pipe {}: {}"sv), PipeName, e.format_error()) << std::endl;
				return EXIT_FAILURE;
			}

			sink_console::process(GetStdHandle(STD_OUTPUT_HANDLE), Message);
		}
	}
}

NIFTY_DEFINE(logging::engine, log_engine);
