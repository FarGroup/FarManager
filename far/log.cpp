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

// Common:
#include "common/from_string.hpp"
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

		static std::unordered_map<string_view, level> LevelMap
		{
			{ L"fatal"sv,     level::fatal     },
			{ L"error"sv,     level::error     },
			{ L"warning"sv,   level::warning   },
			{ L"notice"sv,    level::notice    },
			{ L"info"sv,      level::info      },
			{ L"debug"sv,     level::debug     },
			{ L"trace"sv,     level::trace     },
			{ L"all"sv,       level::all       },
		};

		const auto ItemIterator = LevelMap.find(Str);
		return ItemIterator == LevelMap.cend()? Default : ItemIterator->second;
	}

	static string_view level_to_string(logging::level const Level)
	{
		using level = logging::level;

		switch (Level)
		{
		case level::off:             return L"off";
		case level::fatal:           return L"fatal"sv;
		case level::error:           return L"error"sv;
		case level::warning:         return L"warning"sv;
		case level::notice:          return L"notice"sv;
		case level::info:            return L"info"sv;
		case level::debug:           return L"debug"sv;
		case level::trace:           return L"trace"sv;
		case level::all:             return L"all"sv;
		default:                     return L"???"sv;
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

	static std::pair<string, string> get_time()
	{
		const auto Time = os::chrono::nt_clock::to_filetime(os::chrono::nt_clock::now());
		SYSTEMTIME SystemTime;
		if (!FileTimeToSystemTime(&Time, &SystemTime))
			return {};

		return
		{
			format(
				FSTR(L"{0:04}/{1:02}/{2:02}"),
				SystemTime.wYear,
				SystemTime.wMonth,
				SystemTime.wDay
			),
			format(
				FSTR(L"{0:02}:{1:02}:{2:02}.{3:03}"),
				SystemTime.wHour,
				SystemTime.wMinute,
				SystemTime.wSecond,
				SystemTime.wMilliseconds
			)
		};
	}

	static string get_location(string_view const Function, string_view const Location)
	{
		return format(FSTR(L"{0}, {1}"), Function, Location);
	}

	static string get_thread_id()
	{
		return str(GetCurrentThreadId());
	}

	struct message
	{
		message(string_view const Str, logging::level const Level, string_view const Function, string_view const Location):
			m_ThreadId(get_thread_id()),
			m_LevelString(level_to_string(Level)),
			m_Data(Str),
			m_Location(get_location(Function, Location)),
			m_Level(Level)
		{
			std::tie(m_Date, m_Time) = get_time();
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
	};

	template<bool Value>
	struct discardable
	{
		static constexpr auto is_discardable = Value;
	};

	class sink_debug: public discardable<false>, public sink
	{
	public:
		using sink::sink;

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

		static constexpr string_view name = L"debug"sv;
	};

	class sink_console: public discardable<true>, public sink
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
					SetConsoleTextAttribute(m_Buffer, m_SavedAttributes);
				}

			private:
				WORD m_SavedAttributes;
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

				const time_check TimeCheck;

				for (;;)
				{
					if (TimeCheck && CheckForEscSilent())
					{
						console.SetActiveScreenBuffer(console.GetOutputHandle());
						return;
					}
				}
			}
		}

		static constexpr string_view name = L"console"sv;

	private:
		os::handle m_Buffer;
	};

	class sink_file: public discardable<false>, public sink
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

				LOGERROR(L"{0}", e);
			}
		}

		static constexpr string_view name = L"file"sv;

	private:
		static string make_filename()
		{
			return path::join(get_sink_parameter<sink_file>(L"path"sv), MkStrFTime(L"Far.%Y.%m0.%d_%H.%M.%S.txt"sv));
		}

		static os::fs::file open_file()
		{
			os::fs::file File(make_filename(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS);
			if (!File)
				throw MAKE_FAR_EXCEPTION(L"Can't create a log file"sv);

			LOGINFO(L"Logging to {0}", File.GetName());

			File.SetPointer(0, {}, FILE_END);
			return File;
		}

		os::fs::file m_File;
		os::fs::filebuf m_StreamBuffer;
		std::ostream m_Stream;
		encoding::writer m_Writer;
		string_view m_Eol{ eol::win.str() };
	};

	class sink_pipe: public discardable<true>, public sink
	{
	public:
		explicit sink_pipe()
		{
			(void)os::fs::GetModuleFileName(nullptr, nullptr, m_ThisModule);
		}

		void connect()
		{
			if (!PeekNamedPipe(m_Pipe.native_handle(), {}, 0, {}, {}, {}))
				disconnect();

			if (m_Connected)
				return;

			STARTUPINFO si{ sizeof(si) };
			PROCESS_INFORMATION pi{};

			if (!CreateProcess(m_ThisModule.c_str(), UNSAFE_CSTR(format(FSTR(L"\"{0}\" {1} {2}"), m_ThisModule, log_argument, m_PipeName)), {}, {}, false, CREATE_NEW_CONSOLE, {}, {}, &si, &pi))
			{
				LOGERROR(L"{0}", error_state::fetch());
				return;
			}

			os::handle(pi.hThread);
			os::handle(pi.hProcess);

			while (!ConnectNamedPipe(m_Pipe.native_handle(), {}) && GetLastError() != ERROR_PIPE_CONNECTED)
			{
				LOGWARNING(L"ConnectNamedPipe({0}): {1}", m_PipeName, error_state::fetch());
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

				LOGERROR(L"{0}", e);
			}
		}

		static constexpr string_view name = L"pipe"sv;

	private:
		string m_PipeName{ format(FSTR(L"\\\\.\\pipe\\far_{0}.log"), GetCurrentProcessId()) };
		os::handle m_Pipe{ CreateNamedPipe(m_PipeName.c_str(), PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 1, 0, 0, 0, {}) };
		string m_ThisModule;
		bool m_Connected{};
	};

	class async_impl
	{
	protected:
		template<typename... args>
		async_impl(os::event& FinishEvent, bool const IsDiscardable):
			m_FinishEvent(FinishEvent),
			m_IsDiscardable(IsDiscardable)
		{
			m_Thread = os::thread(os::thread::mode::join, &async_impl::poll, this);
		}

		virtual ~async_impl() = default;

		void in(message Message)
		{
			m_Messages.push(std::move(Message));
			m_MessageEvent.set();
		}

	private:
		virtual void out(message&& Message) = 0;

		void poll()
		{
			return seh_try_no_ui(
				[&]
				{
					message Message;

					for (;;)
					{
						if (os::handle::wait_any({ m_MessageEvent.native_handle(), m_FinishEvent.native_handle() }) != 0)
							return;

						while (m_Messages.try_pop(Message))
						{
							if (m_IsDiscardable && m_FinishEvent.is_signaled())
								return;

							out(std::move(Message));
						}
					}
				},
				[](DWORD const ExceptionCode)
				{
					LOGERROR(L"SEH Exception {0}", ExceptionCode);
				});
		}

		os::synced_queue<message> m_Messages;
		os::event m_MessageEvent { os::event::type::automatic, os::event::state::nonsignaled };
		const os::event& m_FinishEvent;
		bool m_IsDiscardable;
		os::thread m_Thread;
	};

	template<typename sink_type>
	class async final: public sink_type, private async_impl
	{
	public:
		async(os::event& FinishEvent, bool const IsDiscardable):
			async_impl(FinishEvent, IsDiscardable)
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

		~engine()
		{
			m_Finish.set();
		}

		void configure(string_view const Parameters)
		{
			{
				SCOPED_ACTION(std::lock_guard)(m_CS);
				initialise();
			}

			for (auto& i: m_Sinks)
			{
				i->configure(Parameters);
			}
		}

		[[nodiscard]]
		bool filter(level const Level)
		{
			SCOPED_ACTION(std::lock_guard)(m_CS);

			if (m_Status == engine_status::in_progress)
			{
				// We don't know yet, so let it pass and be queued
				return true;
			}

			initialise();

			return m_Level >= Level;
		}

		void log(string_view const Str, level const Level, string_view const Function, string_view const Location)
		{
			SCOPED_ACTION(std::lock_guard)(m_CS);

			// Log can potentially log itself, directly or through other parts of the code.
			// Allow one level of recursion for diagnostics
			if (m_RecursionGuard > 1)
				return;

			++m_RecursionGuard;
			SCOPE_EXIT{ --m_RecursionGuard; };

			if (m_Status == engine_status::in_progress)
			{
				message Message(Str, Level, Function, Location);
				m_QueuedMessages.emplace(std::move(Message));
				return;
			}

			while (!m_QueuedMessages.empty())
			{
				auto Message = std::move(m_QueuedMessages.front());
				m_QueuedMessages.pop();

				if (m_Level >= Message.m_Level)
					submit(Message);
			}

			if (!filter(Level))
				return;

			submit({ Str, Level, Function, Location });
		}

	private:
		void initialise()
		{
			if (m_Status == engine_status::complete)
				return;

			m_Status = engine_status::in_progress;

			// No recursion if it's the helper process
			if (contains(string_view{ GetCommandLine() }, log_argument))
				return;

			m_Level = parse_level(lower(get_parameter(L"level"sv)), m_Level);

			if (m_Level == level::off)
				return;

			LOGINFO(L"{0}", build::version_string());

			m_TraceLevel = parse_level(lower(get_parameter(L"trace.level"sv)), m_TraceLevel);

			if (size_t Depth; from_string(get_parameter(L"trace.depth"sv), Depth))
				m_TraceDepth = Depth;

			const auto Enumerator = enum_tokens(get_parameter(L"sink"sv), L",;"sv);
			std::unordered_set<string_view> SinkNames;
			std::copy(ALL_CONST_RANGE(Enumerator), std::inserter(SinkNames, SinkNames.end()));

			add_sinks
			<
				sink_console,
				sink_pipe,
				sink_debug,
				sink_file
			>(SinkNames);

			if (m_Sinks.empty())
				m_Level = level::off;

			LOGINFO(L"Logging level: {0}", level_to_string(m_Level));

			m_Status = engine_status::complete;
		}

		template<typename T>
		void add_sink(std::unordered_set<string_view> const& Handlers)
		{
			if (!contains(Handlers, T::name))
				return;

			try
			{
				LOGINFO(L"Sink: {0}", T::name);

				int SyncValue;
				const auto Sync = from_string(get_sink_parameter<T>(L"sync"sv), SyncValue) && SyncValue == 1;

				m_Sinks.emplace_back(
					Sync?
						std::make_unique<T>() :
						std::make_unique<async<T>>(m_Finish, T::is_discardable)
				);
			}
			catch (const far_exception& e)
			{
				LOGERROR(L"{0}", e);
			}
		}

		template<typename... args>
		void add_sinks(std::unordered_set<string_view> const& Handlers)
		{
			(..., add_sink<args>(Handlers));
		}

		void submit(message Message)
		{
			if (Message.m_Level <= m_TraceLevel)
			{
				Message.m_Data += L"\nLog stack:\n"sv;

				const auto FramesToSkip = 3; // log -> engine.log -> submit
				tracer::get_symbols({}, os::debug::current_stack(FramesToSkip, m_TraceDepth), [&](string_view const TraceLine)
				{
					append(Message.m_Data, TraceLine, L'\n');
				});
			}

			for (const auto& i: m_Sinks)
				i->handle(Message);
		}

		size_t m_RecursionGuard{};
		os::event m_Finish{ os::event::type::manual, os::event::state::nonsignaled };
		os::concurrency::critical_section m_CS;
		std::list<std::unique_ptr<sink>> m_Sinks;
		std::queue<message> m_QueuedMessages;
		level m_Level{ level::off };
		level m_TraceLevel{ level::error };
		size_t m_TraceDepth{ std::numeric_limits<size_t>::max() };
		engine_status m_Status{ engine_status::incomplete };
	};

	string detail::wide(std::string_view const Str)
	{
		return encoding::ansi::get_chars(Str);
	}

	bool filter(level const Level)
	{
		return log_engine.filter(Level);
	}

	void log(string_view const Str, level const Level, string_view const Function, string_view const Location)
	{
		log_engine.log(Str, Level, Function, Location);
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

		os::fs::file PipeFile;

		while (!PipeFile.Open(PipeName, GENERIC_READ, 0, {}, OPEN_EXISTING))
		{
			const auto ErrorState = error_state::fetch();
			std::wcerr << format(FSTR(L"Can't open pipe {0}: {1}"), PipeName, ErrorState.Win32ErrorStr()) << std::endl;

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

				std::wcerr << format(FSTR(L"Error reading pipe {0}: {1}"), PipeName, e.format_error()) << std::endl;
				return EXIT_FAILURE;
			}

			sink_console::process(GetStdHandle(STD_OUTPUT_HANDLE), Message);
		}
	}
}

NIFTY_DEFINE(logging::engine, log_engine);
