#pragma once

#define FACILITY_INTERNAL 0xFFF
#define E_MESSAGE MAKE_HRESULT(SEVERITY_ERROR, FACILITY_INTERNAL, 0)

struct Error {
  HRESULT code;
  list<wstring> messages;
  const char* file;
  int line;
  Error(): code(NO_ERROR), file(__FILE__), line(__LINE__) {
  }
  Error(HRESULT code, const char* file, int line): code(code), file(file), line(line) {
  }
  Error(HRESULT code, const wstring& message, const char* file, int line): code(code), messages(1, message), file(file), line(line) {
  }
  Error(const wstring& message, const char* file, int line): code(E_MESSAGE), messages(1, message), file(file), line(line) {
  }
  Error(const wstring& message1, const wstring& message2, const char* file, int line): code(E_MESSAGE), messages(1, message1), file(file), line(line) {
    messages.push_back(message2);
  }
  Error(const std::exception& e): code(E_MESSAGE), file(__FILE__), line(__LINE__) {
    string message(string(typeid(e).name()) + ": " + e.what());
    messages.push_back(wstring(message.begin(), message.end()));
  }
  operator bool() const {
    return code != NO_ERROR;
  }
};

#define FAIL(code) throw Error(code, __FILE__, __LINE__)
#define FAIL_MSG(message) throw Error(message, __FILE__, __LINE__)

#define CHECK_SYS(code) { if (!(code)) FAIL(HRESULT_FROM_WIN32(GetLastError())); }
#define CHECK_ADVSYS(code) { DWORD __ret = (code); if (__ret != ERROR_SUCCESS) FAIL(HRESULT_FROM_WIN32(__ret)); }
#define CHECK_COM(code) { HRESULT __ret = (code); if (FAILED(__ret)) FAIL(__ret); }
#define CHECK(code) { if (!(code)) FAIL_MSG(L#code); }

#define IGNORE_ERRORS(code) { try { code; } catch (...) { } }
