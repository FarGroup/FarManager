#include "common.hpp"

int wmain(int argc, wchar_t* argv[]) {
  BEGIN_ERROR_HANDLER;
  if (argc != 4)
    FAIL_MSG(L"Usage: convcp in_file out_file codepage");
  unsigned code_page;
  wstring text = load_file(argv[1]);
  save_file(argv[2], text, str_to_int(argv[3]));
  return 0;
  END_ERROR_HANDLER;
  return 1;
}
