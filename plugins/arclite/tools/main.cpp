#define TOOLS_TOOL

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

#include <algorithm>
#include <string>
#include <list>
#include <vector>
#include <deque>
#include <map>
#include <set>
#include <sstream>
#include <iomanip>
#include <iostream>

using namespace std::literals;

#include <process.h>
#include <assert.h>

#include "../../common/unicode/plugin.hpp"
#include "../../common/unicode/farcolor.hpp"
#include <shobjidl.h>
#include "error.hpp"
#include "utils.hpp"
#include "sysutils.hpp"
#include "iniparse.hpp"

#include "strutils.cpp"
#include "pathutils.cpp"
#include "sysutils.cpp"
#include "iniparse.cpp"

namespace Far { const ArclitePrivateInfo* get_system_functions() { return nullptr; } }

#define BEGIN_ERROR_HANDLER try {
#define END_ERROR_HANDLER \
  } \
  catch (const Error& e) { \
    if (e.code != NO_ERROR) { \
      std::wstring sys_msg = get_system_message(e.code); \
      if (!sys_msg.empty()) \
        std::wcerr << sys_msg << std::endl; \
    } \
    for (std::list<std::wstring>::const_iterator message = e.messages.begin(); message != e.messages.end(); message++) { \
      std::wcerr << *message << std::endl; \
    } \
    std::wcerr << extract_file_name(widen(e.file)) << L':' << e.line << std::endl; \
  } \
  catch (const std::exception& e) { \
    std::cerr << typeid(e).name() << ": " << e.what() << std::endl; \
  } \
  catch (...) { \
    std::cerr << "unknown error" << std::endl; \
  }

#define CP_UTF16 1200
std::wstring load_file(const std::wstring& file_name, unsigned* code_page = NULL) {
  File file(get_full_path_name(file_name), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN);
  Buffer<char> buffer(file.size());
  unsigned size = file.read(buffer.data(), buffer.size());
  if ((size >= 2) && (buffer.data()[0] == '\xFF') && (buffer.data()[1] == '\xFE')) {
    if (code_page) *code_page = CP_UTF16;
    return std::wstring(reinterpret_cast<wchar_t*>(buffer.data() + 2), (buffer.size() - 2) / 2);
  }
  else if ((size >= 3) && (buffer.data()[0] == '\xEF') && (buffer.data()[1] == '\xBB') && (buffer.data()[2] == '\xBF')) {
    if (code_page) *code_page = CP_UTF8;
    return ansi_to_unicode(std::string(buffer.data() + 3, size - 3), CP_UTF8);
  }
  else {
    if (code_page) *code_page = CP_ACP;
    return ansi_to_unicode(std::string(buffer.data(), size), CP_ACP);
  }
}

void save_file(const std::wstring& file_name, const std::wstring& text, unsigned code_page = CP_ACP) {
  File file(get_full_path_name(file_name), GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS, 0);
  if (code_page == CP_UTF16) {
    const char c_sig[] = { '\xFF', '\xFE' };
    file.write(c_sig, sizeof(c_sig));
    file.write(reinterpret_cast<const char*>(text.data()), text.size() * 2);
  }
  else {
    if (code_page == CP_UTF8) {
      const char c_sig[] = { '\xEF', '\xBB', '\xBF' };
      file.write(c_sig, sizeof(c_sig));
    }
    std::string data = unicode_to_ansi(text, code_page);
    file.write(data.data(), data.size());
  }
}

#include "gendep.cpp"
#include "geninst.cpp"
#include "msgc.cpp"
#include "preproc.cpp"
#include "farver.cpp"

int wmain(int argc, wchar_t* argv[]) {
  BEGIN_ERROR_HANDLER;
  if (argc < 2)
    FAIL_MSG(L"tool gendep | geninst | msgc | preproc | farver");
  std::deque<std::wstring> params;
  for (int i = 2; i < argc; i++) {
    params.push_back(argv[i]);
  }
  std::wstring command(argv[1]);
  if (command == L"gendep")
    gendep(params);
  else if (command == L"geninst")
    geninst(params);
  else if (command == L"msgc")
    msgc(params);
  else if (command == L"preproc")
    preproc(params);
  else if (command == L"farver")
    farver(params);
  return 0;
  END_ERROR_HANDLER;
  return 1;
}
