#pragma once

#include <typeinfo>

#define E_MESSAGE MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x200)

struct Error {
  HRESULT code;
  std::list<std::wstring> objects;
  std::list<std::wstring> messages;
  std::list<std::wstring> warnings;
  const char* file;
  int line;

  Error() : code(NO_ERROR), file(__FILE__), line(__LINE__)
  {}
  Error(HRESULT code, const char* file, int line)
  : code(code), file(file), line(line)
  {}
  Error(HRESULT code, const std::wstring& message, const char* file, int line)
  : code(code), messages{message}, file(file), line(line)
  {}
  Error(const std::wstring& message, const char* file, int line)
  : code(E_MESSAGE), messages{message}, file(file), line(line)
  {}
  Error(const std::wstring& message1, const std::wstring& message2, const char* file, int line)
  : code(E_MESSAGE), messages{message1, message2}, file(file), line(line)
  {}
  Error(const std::exception& e)
  : code(E_MESSAGE), file(__FILE__), line(__LINE__)
  {
    std::string message(std::string(typeid(e).name()) + ": " + e.what());
    messages.emplace_back(message.begin(), message.end());
  }

  void SetResults(std::list<std::wstring>&& errs, std::list<std::wstring>&& wrns) {
    messages = errs; warnings = wrns;
    code = messages.empty() ? (warnings.empty() ? NO_ERROR : S_FALSE) : E_MESSAGE;
  }
  void Append(const Error& error) {
    if (&error != this) {
      for (const auto& msg : error.messages) {
        messages.emplace_back(msg);
        code = error.code; file = error.file; line = error.line;
      }
    }
  }

  operator bool() const {
    return code != NO_ERROR;
  }
};

#define FAIL(code) throw Error(code, __FILE__, __LINE__)
#define FAIL_MSG(message) throw Error(message, __FILE__, __LINE__)

#define CHECK_SYS(code) do { if (!(code)) FAIL(HRESULT_FROM_WIN32(GetLastError())); } while(false)
#define CHECK_ADVSYS(code) do { DWORD __ret = (code); if (__ret != ERROR_SUCCESS) FAIL(HRESULT_FROM_WIN32(__ret)); } while(false)
#define CHECK_COM(code) do { HRESULT __ret = (code); if (FAILED(__ret)) FAIL(__ret); } while(false)
#define CHECK(code) do { if (!(code)) FAIL_MSG(L###code); } while(false)

#define IGNORE_ERRORS(code) do { try { code; } catch (...) { } } while(false)

#define CONCAT1(a, b) a##b
#define CONCAT(a, b) CONCAT1(a, b)
#define MAKE_UNIQUE(name) CONCAT(name, __COUNTER__)
#define GUARD(code) std::shared_ptr<void> MAKE_UNIQUE(guard)(nullptr, [&](void*) { code; })
