#include "common.hpp"

void search_and_replace(wstring& text, const wstring& name, const wstring& value) {
  size_t pos = 0;
  while (true) {
    pos = text.find(name, pos);
    if (pos == wstring::npos) break;
    text.replace(pos, name.size(), value);
    pos += value.size();
  }
}

const wchar_t* prefix = L"<(";
const wchar_t* suffix = L")>";

int wmain(int argc, wchar_t* argv[]) {
  BEGIN_ERROR_HANDLER;
  if (argc < 4)
    FAIL_MSG(L"Usage: preproc ini_file ... in_file out_file");
  unsigned code_page;
  wstring text = load_file(argv[argc - 2], &code_page);
  for (unsigned i = 1; i < argc - 2; i++) {
    Ini::File ini_file;
    ini_file.parse(load_file(argv[i]));
    for (Ini::File::const_iterator section = ini_file.begin(); section != ini_file.end(); section++) {
      for (Ini::Section::const_iterator item = section->second.begin(); item != section->second.end(); item++) {
        search_and_replace(text, prefix + item->first + suffix, item->second);
      }
    }
  }
  save_file(argv[argc - 1], text, code_page);
  return 0;
  END_ERROR_HANDLER;
  return 1;
}
