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

void preproc(const deque<wstring>& params) {
  if (params.size() < 3)
    FAIL_MSG(L"Usage: preproc ini_file ... in_file out_file");
  unsigned code_page;
  wstring text = load_file(params[params.size() - 2], &code_page);
  for (unsigned i = 0; i < params.size() - 2; ++i) {
    Ini::File ini_file;
    ini_file.parse(load_file(params[i]));
    for (Ini::File::const_iterator section = ini_file.begin(); section != ini_file.end(); section++) {
      for (Ini::Section::const_iterator item = section->second.begin(); item != section->second.end(); item++) {
        search_and_replace(text, prefix + item->first + suffix, item->second);
      }
    }
  }
  save_file(params[params.size() - 1], text, code_page);
}
