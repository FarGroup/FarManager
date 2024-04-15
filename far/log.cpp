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
#include "imports.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "palette.hpp"
#include "pathmix.hpp"
#include "pipe.hpp"
#include "strmix.hpp"
#include "tracer.hpp"
#include "res.hpp"

// Platform:
#include "platform.hpp"
#include "platform.concurrency.hpp"
#include "platform.debug.hpp"
#include "platform.env.hpp"
#include "platform.fs.hpp"
#include "platform.version.hpp"

// Common:
#include "common/from_string.hpp"
#include "common/scope_exit.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

namespace
{
	const auto log_argument = L"/service:log_viewer"sv;

	auto get(string_view const Name)
	{
		return os::env::get(Name);
	}

	auto parameter(string_view const Name)
	{
		return L"far.log."sv + Name;
	}

	auto get_parameter(string_view const Name)
	{
		return get(parameter(Name));
	}

	auto sink_parameter(string_view const SinkName, string_view const Name)
	{
		return parameter(far::format(L"sink.{}.{}"sv, SinkName, Name));
	}

	auto get_sink_parameter(string_view const SinkName, string_view const Name)
	{
		return get(sink_parameter(SinkName, Name));
	}

	auto parse_level(string_view const Str, logging::level const Default)
	{
		if (Str.empty())
			return Default;

		using level = logging::level;

		static std::unordered_map<string_view, level, string_comparer_icase, string_comparer_icase> LevelMap
		{
#define STRLEVEL(x) { WIDE_SV_LITERAL(x), level::x }
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

	auto level_to_string(logging::level const Level)
	{
		using level = logging::level;

		switch (Level)
		{
#define LEVELTOSTR(x) case level::x: return WIDE_SV_LITERAL(x)
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

	WORD level_to_color(logging::level const Level)
	{
		static constexpr WORD Colors[]
		{
			F_BLACK,
			F_WHITE | B_LIGHTRED,
			F_LIGHTRED,
			F_YELLOW,
			F_LIGHTCYAN,
			F_LIGHTGRAY,
			F_CYAN,
			F_DARKGRAY,
		};

		return Colors[std::to_underlying(Level) / logging::detail::step];
	}

	auto get_thread_id()
	{
		return str(GetCurrentThreadId());
	}

	struct message
	{
		message(string&& Str, logging::level const Level, source_location const& Location, size_t const TraceDepth, bool const IsDebugMessage):
			m_ThreadId(get_thread_id()),
			m_LevelString(level_to_string(Level)),
			m_Data(std::move(Str)),
			m_Location(source_location_to_string(Location)),
			m_Level(Level),
			m_IsDebugMessage(IsDebugMessage)
		{
			std::tie(m_Date, m_Time) = format_datetime(os::chrono::now_utc());

			if (TraceDepth)
			{
				m_Data += L"\nLog stack:\n"sv;

				const auto FramesToSkip = IsDebugMessage? 2 : 3; // [log] -> engine::log -> this ctor
				tracer.current_stacktrace({}, [&](string_view const TraceLine)
				{
					append(m_Data, TraceLine, L'\n');
				},
				FramesToSkip, TraceDepth);
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
		bool m_IsDebugMessage;
	};

	class sink
	{
	public:
		NONCOPYABLE(sink);

		sink() = default;
		virtual ~sink() = default;
		virtual void configure(string_view Parameters) = 0;
		virtual void handle(message const& Message) = 0;
	};

	template<typename sink_type>
	class sink_boilerplate: public sink, public sink_type
	{
	public:
		void configure(string_view const Parameters) override
		{
			this->configure_impl(Parameters);
		}
	};

	template<bool Value>
	struct discardable
	{
		static constexpr auto is_discardable = Value;
	};

	struct no_config
	{
		static void configure_impl(string_view)
		{
		}
	};

	class sink_null: public no_config, public discardable<true>
	{
	public:
		static void handle_impl(message const&)
		{
		}

		static constexpr auto name = L"null"sv;
	};

	class sink_debug: public no_config, public discardable<false>
	{
	public:
		void handle_impl(message const& Message) const
		{
			if (Message.m_IsDebugMessage)
				return;

			os::debug::print(far::format(
				L"[{}][{}][{}] {} [{}]\n"sv,
				Message.m_Time,
				Message.m_ThreadId,
				Message.m_LevelString,
				Message.m_Data,
				Message.m_Location
			));
		}

		static constexpr auto name = L"debug"sv;
	};

	class sink_console: public discardable<true>
	{
	public:
		sink_console()
		{
			initialize_ui();
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
					if (!get_console_screen_buffer_info(Buffer, &csbi))
						throw far_exception(L"get_console_screen_buffer_info"sv);

					if (!SetConsoleTextAttribute(Buffer, Color))
						throw far_exception(L"SetConsoleTextAttributes"sv);

					m_SavedAttributes = csbi.wAttributes;
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
				if (!WriteConsole(Buffer, Str.data(), static_cast<DWORD>(Str.size()), &Written, {}))
					throw far_exception(L"WriteConsole"sv);
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

		void handle_impl(message const& Message)
		{
			if (!m_Buffer)
				return;

			try
			{
				process(m_Buffer.native_handle(), Message);
			}
			catch (far_exception const& e)
			{
				m_Buffer.close();

				LOGERROR(L"{}"sv, e);
			}
		}

		void configure_impl(string_view const Parameters)
		{
			if (Parameters.empty())
			{
				while (!console.SetActiveScreenBuffer(m_Buffer.native_handle()))
				{
					if (GetLastError() != ERROR_INVALID_HANDLE)
						return;

					LOGINFO(L"Reinitializing");
					initialize_ui();
				}

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
		void initialize_ui()
		{
			m_Buffer.reset(CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, {}, CONSOLE_TEXTMODE_BUFFER, {}));
			if (!m_Buffer)
				return;

			SHORT Width = console.GetLargestWindowSize(m_Buffer.native_handle()).x;
			if (!Width)
				Width = 80;

			SetConsoleScreenBufferSize(m_Buffer.native_handle(), { Width, 9999 });
		}

		os::handle m_Buffer;
	};

	class sink_file: public no_config, public discardable<false>
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

		void handle_impl(message const& Message)
		{
			if (!m_File)
				return;

			try
			{
				m_Writer.write(
					L"["sv,
					Message.m_Date,
					L" "sv,
					Message.m_Time,
					L"]["sv,
					Message.m_ThreadId,
					L"]["sv,
					Message.m_LevelString,
					L"] "sv,
					Message.m_Data,
					L" "sv,
					L"["sv,
					Message.m_Location,
					L"]"sv,
					m_Eol
				);

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
			return path::join
			(
				get_sink_parameter(name, L"path"sv),
				unique_name() + L".txt"sv
			);
		}

		static os::fs::file open_file()
		{
			os::fs::file File(make_filename(), GENERIC_WRITE, os::fs::file_share_read, nullptr, OPEN_ALWAYS);
			if (!File)
				throw far_exception(L"Can't create a log file"sv);

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

	class sink_pipe: public no_config, public discardable<true>
	{
	public:
		void handle_impl(message const& Message)
		{
			try
			{
				connect();

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
		void connect()
		{
			if (m_Pipe)
			{
				if (PeekNamedPipe(m_Pipe.native_handle(), {}, 0, {}, {}, {}))
					return;

				LOGWARNING(L"PeekNamedPipe({}): {}"sv, m_PipeName, os::last_error());
				disconnect();
			}

			m_Pipe.reset(CreateNamedPipe(m_PipeName.c_str(), PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 1, 0, 0, 0, {}));
			if (!m_Pipe)
				throw far_exception(far::format(L"CreateNamedPipe({})"sv, m_PipeName));

			STARTUPINFO si{ sizeof(si) };
			PROCESS_INFORMATION pi{};

			if (!CreateProcess(m_ThisModule.c_str(), UNSAFE_CSTR(far::format(L"\"{}\" {} {}"sv, m_ThisModule, log_argument, m_PipeName)), {}, {}, false, CREATE_NEW_CONSOLE, {}, {}, &si, &pi))
				throw far_exception(L"CreateProcess()"sv);

			os::handle(pi.hThread);
			os::handle(pi.hProcess);

			if (!ConnectNamedPipe(m_Pipe.native_handle(), {}))
			{
				const auto Error = os::last_error();
				if (Error.Win32Error != ERROR_PIPE_CONNECTED)
					throw far_exception(error_state_ex{ Error, far::format(L"ConnectNamedPipe({})"sv, m_PipeName) });
			}
		}

		void disconnect()
		{
			if (!m_Pipe)
				return;

			if (!DisconnectNamedPipe(m_Pipe.native_handle()))
				LOGWARNING(L"DisconnectNamedPipe({}): {}"sv, m_PipeName, os::last_error());

			m_Pipe = {};
		}

		string m_PipeName{ far::format(L"\\\\.\\pipe\\far_{}.log"sv, GetCurrentProcessId()) };
		os::handle m_Pipe;
		string m_ThisModule{ os::fs::get_current_process_file_name() };
	};

	template<typename impl>
	class synchronized_impl: public impl
	{
	protected:
		explicit synchronized_impl(auto&&... Args):
			impl(FWD(Args)...)
		{}

		void handle_impl_synchronized(message const& Message)
		{
			SCOPED_ACTION(std::scoped_lock)(m_CS);
			impl::handle_impl(Message);
		}

	private:
		os::critical_section m_CS;
	};

	class async_impl
	{
	protected:
		explicit async_impl(std::function<void(message&&)> Out, bool const IsDiscardable, string_view const Name):
			m_Out(std::move(Out)),
			m_IsDiscardable(IsDiscardable),
			m_Thread(&async_impl::poll, this, Name)
		{
		}

		virtual ~async_impl()
		{
			m_FinishEvent.set();
		}

		void handle_impl(message const& Message)
		{
			// The receiver takes all available messages in one go so this shouldn't happen.
			// However, should it fail to do so, this will prevent us from eating all the RAM:
			if (m_Messages.size() > QueueBufferSize * 2)
			{
				m_Messages.clear();
				LOGERROR(L"Queue overflow"sv);
			}

			m_Messages.emplace(Message);
			m_MessageEvent.set();
		}

	private:
		const size_t QueueBufferSize = 8192;

		void poll(string_view const Name)
		{
			os::debug::set_thread_name(far::format(L"Log sink ({})"sv, Name));

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

							m_Out(std::move(Messages.front()));
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
		std::function<void(message&&)> m_Out;
		bool m_IsDiscardable;
		os::thread m_Thread;
	};

	enum class sink_mode
	{
		sync,
		async,
	};

	template<typename sink_type>
	class async final: public sink_boilerplate<sink_type>, private synchronized_impl<async_impl>
	{
	public:
		static constexpr auto mode = sink_mode::async;

		explicit async(bool const IsDiscardable):
			synchronized_impl([&](message const& Message){ sink_boilerplate<sink_type>::handle_impl(Message); }, IsDiscardable, sink_type::name)
		{
		}

		// sink
		void handle(message const& Message) override
		{
			this->handle_impl_synchronized(Message);
		}
	};

	template<typename sink_type>
	class sync final: public synchronized_impl<sink_boilerplate<sink_type>>
	{
	public:
		static constexpr auto mode = sink_mode::sync;

		// sink
		void handle(message const& Message) override
		{
			this->handle_impl_synchronized(Message);
		}
	};

	auto parse_sink_mode(string_view const Mode)
	{
		return Mode == L"sync"sv? sink_mode::sync : sink_mode::async;
	}

	std::unique_ptr<sink> create_sink(string_view SinkName, sink_mode Mode);

	class sink_composite: public discardable<false>
	{
	public:
		sink_composite()
		{
			unordered_string_set SeenSinks;

			for (const auto& i: enum_tokens(get_sink_parameter(name, L"sinks"sv), L",;"sv))
			{
				// No funny stuff
				if (equal_icase(i, name) || SeenSinks.contains(i))
					continue;

				const auto SinkMode = parse_sink_mode(get_sink_parameter(i, L"mode"sv));

				try
				{
					if (auto Sink = create_sink(i, SinkMode))
					{
						m_Sinks.emplace_back(std::move(Sink));
						SeenSinks.emplace(i);
					}
					else
						LOGWARNING(L"Unknown sink {}"sv, i);
				}
				catch (far_exception const& e)
				{
					LOGERROR(L"{}"sv, e);
				}
			}
		}

		void handle_impl(message const& Message) const
		{
			for (const auto& i: m_Sinks)
				i->handle(Message);
		}

		void configure_impl(string_view const Parameters) const
		{
			for (const auto& i: m_Sinks)
				i->configure(Parameters);
		}

		static constexpr auto name = L"composite"sv;

	private:
		std::vector<std::unique_ptr<sink>> m_Sinks;
	};

	void log_sink(string_view const SinkName, sink_mode const Mode)
	{
		LOGINFO(L"Sink: {} ({})"sv, SinkName, Mode == sink_mode::sync? L"sync"sv : L"async"sv);
	}

	template<typename T>
	std::unique_ptr<sink> create_sink(sink_mode const Mode)
	{
		log_sink(T::name, Mode);

		if (Mode == sink_mode::sync)
			return std::make_unique<sync<T>>();
		else
			return std::make_unique<async<T>>(T::is_discardable);
	}

	template<typename... args>
	std::unique_ptr<sink> create_sink(string_view const SinkName, sink_mode const Mode)
	{
		if (std::unique_ptr<sink> Result; (... || (SinkName == args::name && ((Result = create_sink<args>(Mode))))))
			return Result;

		return nullptr;
	}

	std::unique_ptr<sink> create_sink(string_view const SinkName, sink_mode const Mode)
	{
		return create_sink<
			sink_composite,
			sink_console,
			sink_pipe,
			sink_debug,
			sink_file,
			sink_null>
		(SinkName, Mode);
	}
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
			LOGINFO(L"Logger exit"sv);

			if (m_VectoredHandler && imports.RemoveVectoredExceptionHandler)
				imports.RemoveVectoredExceptionHandler(m_VectoredHandler);

			s_Destroyed = true;
		}

		void configure(string_view const Parameters)
		{
			initialise();

			if (equal_icase(Parameters, L"reconfigure"))
			{
				const auto Status = m_Status.exchange(engine_status::in_progress);
				SCOPE_EXIT{ m_Status = Status; flush_queue(); };

				configure_env();
			}

			if (m_Sink)
				m_Sink->configure(Parameters);
		}

		[[nodiscard]]
		bool filter(level const Level)
		{
			if (s_Destroyed)
				return false;

			if (s_Suppressed)
				return false;

			switch (m_Status)
			{
			case engine_status::incomplete:
				initialise();
				[[fallthrough]];
			case engine_status::complete:
				return m_Level >= Level;

			case engine_status::in_progress:
				// We don't know yet, so let it pass and be queued
				return true;

			default:
				std::unreachable();
			}
		}

		void flush_queue()
		{
			if (m_Status != engine_status::complete)
				return;

			for (auto Messages = m_QueuedMessages.pop_all(); !Messages.empty(); Messages.pop())
			{
				const auto& Message = Messages.front();
				if (Message.m_Level <= m_Level)
					submit(Message);
			}
		}

		void log(string&& Str, level const Level, bool const IsDebugMessage, source_location const& Location)
		{
			thread_local size_t RecursionGuard{};
			// Log can potentially log itself, directly or through other parts of the code.
			// Allow one level of recursion for diagnostics
			if (RecursionGuard > 1)
				return;

			++RecursionGuard;
			SCOPE_EXIT{ --RecursionGuard; };

			const auto TraceDepth = Level <= m_TraceLevel? m_TraceDepth : 0;

			SCOPED_ACTION(os::last_error_guard);

			if (m_Status == engine_status::in_progress)
			{
				if (TraceDepth)
					m_WithSymbols.emplace(L""sv);

				m_QueuedMessages.push({ std::move(Str), Level, Location, TraceDepth, IsDebugMessage });
				return;
			}

			if (!filter(Level))
				return;

			if (TraceDepth)
				m_WithSymbols.emplace(L""sv);

			submit({ std::move(Str), Level, Location, TraceDepth, IsDebugMessage });
		}

		static void suppress()
		{
			++s_Suppressed;
		}

		static void restore()
		{
			assert(s_Suppressed);

			--s_Suppressed;
		}

	private:
		void configure_env()
		{
			m_Level = parse_level(get_parameter(L"level"sv), m_Level);

			m_TraceLevel = parse_level(get_parameter(L"trace.level"sv), m_TraceLevel);

			if (size_t Depth; from_string(get_parameter(L"trace.depth"sv), Depth))
				m_TraceDepth = Depth;

			auto SinkName = get_parameter(L"sink"sv);
			if (SinkName.find_first_of(L",;") != SinkName.npos)
			{
				// far.log.sink=foo;bar
				// is the same as
				// far.log.sink=composite
				// far.log.sink.composite.sinks=foo;bar
				os::env::set(parameter(L"sink"sv), sink_composite::name);
				os::env::set(sink_parameter(sink_composite::name, L"sinks"sv), SinkName);
				SinkName = sink_composite::name;
			}

			const auto SinkMode = parse_sink_mode(get_sink_parameter(SinkName, L"mode"sv));

			// The old one must be closed before creating a new one
			m_Sink.reset();

			m_Sink = create_sink(SinkName, SinkMode);

			if (!m_Sink)
				m_Level = level::off;
			else
				LOGINFO(L"Logging level: {}"sv, level_to_string(m_Level));
		}

		void initialise()
		{
			SCOPED_ACTION(std::scoped_lock)(m_CS);

			if (m_Status != engine_status::incomplete)
				return;

			m_Status = engine_status::in_progress;
			SCOPE_EXIT{ m_Status = engine_status::complete; flush_queue(); };

			m_VectoredHandler = imports.AddVectoredExceptionHandler?
				imports.AddVectoredExceptionHandler(false, debug_print) :
				nullptr;

			SCOPED_ACTION(os::last_error_guard);

			// No recursion if it's the helper process
			if (contains(string_view{ GetCommandLine() }, log_argument))
				return;

			LOGINFO(L"{}"sv, build::version_string());
			LOGINFO(L"{}"sv, os::version::os_version());

			configure_env();
		}


		void submit(message const& Message) const
		{
			m_Sink->handle(Message);
		}

		static void debug_print(void const* Ptr, size_t const Size, bool const IsUnicode) noexcept
		{
			const auto Level = level::debug;
			if (!logging::filter(Level))
				return;

			try
			{
				log_engine.log(
					far::format(
						L"{}"sv,
						IsUnicode?
							string_view{ static_cast<wchar_t const*>(Ptr), Size } :
							encoding::ansi::get_chars(std::string_view{ static_cast<char const*>(Ptr), Size })
					),
					Level,
					true,
					source_location::current()
				);
			}
			catch (...)
			{
			}
		}

		static void debug_print_seh(void const* Ptr, size_t const Size, bool const IsUnicode) noexcept
		{
			return seh_try_no_ui(
			[&]
			{
				return debug_print(Ptr, Size, IsUnicode);
			},
			[](DWORD)
			{
			});
		}

		static LONG NTAPI debug_print(EXCEPTION_POINTERS* const Pointers)
		{
			const auto& Record = *Pointers->ExceptionRecord;

			switch (const auto Code = static_cast<NTSTATUS>(Record.ExceptionCode))
			{
			case DBG_PRINTEXCEPTION_WIDE_C:
			case DBG_PRINTEXCEPTION_C:
				{
					if (Record.NumberParameters < 2 || !Record.ExceptionInformation[0] || !Record.ExceptionInformation[1])
						break;

					const auto IsUnicode = Code == DBG_PRINTEXCEPTION_WIDE_C;

					if (thread_local bool IgnoreNextAnsiMessage; IgnoreNextAnsiMessage)
					{
						IgnoreNextAnsiMessage = false;
						break;
					}
					else
						IgnoreNextAnsiMessage = IsUnicode;

					const auto Ptr = view_as<void const*>(Record.ExceptionInformation[1]);
					const auto Size = static_cast<size_t>(Record.ExceptionInformation[0] - 1);

					debug_print_seh(Ptr, Size, IsUnicode);
				}
				break;
			}

			return EXCEPTION_CONTINUE_SEARCH;
		}

		imports_nifty_objects::initialiser m_ImportsReference;
		tracer_nifty_objects::initialiser m_TracerReference;
		os::concurrency::critical_section m_CS;
		os::concurrency::synced_queue<message> m_QueuedMessages;
		std::atomic<level> m_Level{ level::off };
		level m_TraceLevel{ level::fatal };
		size_t m_TraceDepth{ std::numeric_limits<size_t>::max() };
		std::atomic<engine_status> m_Status{ engine_status::incomplete };
		std::unique_ptr<sink> m_Sink;
		void* m_VectoredHandler{};
		std::optional<tracer_detail::tracer::with_symbols> m_WithSymbols;
		static inline bool s_Destroyed;
		static inline std::atomic_size_t s_Suppressed;
	};

	bool filter(level const Level)
	{
		return log_engine.filter(Level);
	}

	void log(string&& Str, level const Level, source_location const& Location)
	{
		log_engine.log(std::move(Str), Level, false, Location);
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

	int main(string_view const PipeName)
	{
		consoleicons::instance().set_icon(FAR_ICON_LOG);

		console.SetTitle(L"Far Log Viewer: "sv + PipeName);
		console.SetTextAttributes(colors::NtColorToFarColor(F_LIGHTGRAY | B_BLACK));

		DWORD ConsoleMode = 0;
		console.GetMode(console.GetInputHandle(), ConsoleMode);
		console.SetMode(console.GetInputHandle(), ConsoleMode | ENABLE_EXTENDED_FLAGS | ENABLE_QUICK_EDIT_MODE);

		os::fs::file PipeFile;

		while (!PipeFile.Open(PipeName, GENERIC_READ, 0, {}, OPEN_EXISTING))
		{
			const auto ErrorState = os::last_error();

			if (!ConsoleYesNo(L"Retry"sv, false, [&]{ std::wcerr << far::format(L"Can't open pipe {}: {}"sv, PipeName, ErrorState) << std::endl; }))
				return EXIT_FAILURE;
		}

		for (message Message; ;)
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
				const auto pause = []
				{
					if (!os::is_interactive_user_session())
						return;

					// Q: Why not just system("pause") or cin.get() or whatever?
					// A: We want to keep the log window open, but we don't want to do it ourselves,
					//    because otherwise this log process stays alive, lingers in the background
					//    and prevents you from recompiling the binary during development, which is annoying.
					//    With this approach we basically transfer the console to cmd, so it's someone else's problem.

					STARTUPINFO si{ sizeof(si) };
					PROCESS_INFORMATION pi{};
					if (CreateProcess({}, UNSAFE_CSTR(L"cmd.exe /c pause"s), {}, {}, false, 0, {}, {}, &si, &pi))
					{
						os::handle(pi.hThread);
						os::handle(pi.hProcess);
					}
				};

				if (e.Win32Error == ERROR_BROKEN_PIPE)
				{
					// If the last logged message was a warning or worse, the user probably wants to see it
					if (Message.m_Level < level::info)
						pause();

					return EXIT_SUCCESS;
				}

				std::wcerr << far::format(L"Error reading pipe {}: {}"sv, PipeName, e) << std::endl;
				pause();
				return EXIT_FAILURE;
			}

			try
			{
				sink_console::process(GetStdHandle(STD_OUTPUT_HANDLE), Message);
			}
			catch (far_exception const& e)
			{
				std::wcerr << far::format(L"sink_console::process(): {}"sv, e) << std::endl;
			}
		}
	}

	suppressor::suppressor()
	{
		engine::suppress();
	}

	suppressor::~suppressor()
	{
		engine::restore();
	}
}

NIFTY_DEFINE(logging::engine, log_engine);


#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("log.level")
{
	using logging::level;

	const auto UnmappedLevel = level{ std::to_underlying(level::off) + 1 };

	static const struct
	{
		string_view StrLevel;
		level Level;
		WORD Color;
	}
	Tests[]
	{
		{ {},           UnmappedLevel,  F_BLACK,              },
		{ L"lalala"sv,  UnmappedLevel,  F_BLACK,              },

		{ L"off"sv,     level::off,     F_BLACK,              },
		{ L"fatal"sv,   level::fatal,   F_WHITE | B_LIGHTRED, },
		{ L"error"sv,   level::error,   F_LIGHTRED,           },
		{ L"warning"sv, level::warning, F_YELLOW,             },
		{ L"notice"sv,  level::notice,  F_LIGHTCYAN,          },
		{ L"info"sv,    level::info,    F_LIGHTGRAY,          },
		{ L"debug"sv,   level::debug,   F_CYAN,               },
		{ L"trace"sv,   level::trace,   F_DARKGRAY,           },
		{ L"all"sv,     level::all,     F_DARKGRAY,           },
	};

	{
		const auto ExpectedLevels = (std::to_underlying(level::all) + 1ull) / logging::detail::step + 1;
		REQUIRE(std::size(Tests) >= ExpectedLevels);
	}

	for (const auto& i: Tests)
	{
		REQUIRE(parse_level(i.StrLevel, UnmappedLevel) == i.Level);
		REQUIRE(level_to_string(i.Level) == (i.Level == UnmappedLevel? L"???"sv : i.StrLevel));
		REQUIRE(level_to_color(i.Level) == i.Color);
	}
}

TEST_CASE("log.misc")
{
	REQUIRE(parameter(L"banana"sv) == L"far.log.banana"sv);
	REQUIRE(sink_parameter(L"ham"sv, L"eggs"sv) == L"far.log.sink.ham.eggs"sv);
	REQUIRE(parse_sink_mode(L"sync"sv) == sink_mode::sync);
	REQUIRE(parse_sink_mode(L"async"sv) == sink_mode::async);
	REQUIRE(parse_sink_mode(L"whatever"sv) == sink_mode::async);
	REQUIRE(logging::is_log_argument(log_argument.data()));
}

#endif
