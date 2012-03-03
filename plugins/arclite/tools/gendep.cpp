const wchar_t* c_ext_list[] = {
  L"c", L"h", L"cpp", L"hpp", L"rc",
};

bool is_valid_ext(const wchar_t* file_name) {
  const wchar_t* ext = wcsrchr(file_name, L'.');
  if (ext) ext++;
  else ext = L"";
  for (unsigned i = 0; i < ARRAYSIZE(c_ext_list); i++) {
    if (_wcsicmp(ext, c_ext_list[i]) == 0)
      return true;
  }
  return false;
}

class Parser {
private:
  const wstring& text;
  size_t pos;
  bool is_one_of(wchar_t ch, const wchar_t* char_set) {
    const wchar_t* c = char_set;
    while (*c && *c != ch) c++;
    return *c == ch;
  }
  bool end() const {
    return pos == text.size();
  }
public:
  Parser(const wstring& text, size_t pos): text(text), pos(pos) {
  }
  void ws() {
    while (!end() && is_one_of(text[pos], L" \t"))
      pos++;
  }
  bool ch(wchar_t c) {
    if (end() || text[pos] != c) return false;
    pos++;
    return true;
  }
  bool str(const wchar_t* str) {
    while (!end() && *str && *str == text[pos]) {
      pos++;
      str++;
    }
    return *str == 0;
  }
  wstring extract(const wchar_t* end_chars) {
    size_t start = pos;
    while (!end() && !is_one_of(text[pos], end_chars))
      pos++;
    return wstring(text.data() + start, pos - start);
  }
};

bool is_include_directive(const wstring& text, size_t pos, wstring& file_name) {
  Parser parser(text, pos);
  parser.ws();
  if (!parser.ch(L'#')) return false;
  parser.ws();
  if (!parser.str(L"include")) return false;
  parser.ws();
  if (!parser.ch(L'"') && !parser.ch(L'<')) return false;
  file_name = parser.extract(L"\">\n");
  if (!parser.ch(L'"') && !parser.ch(L'>')) return false;
  return true;
}

bool file_exists(const wstring& file_path) {
  return GetFileAttributesW(long_path(get_full_path_name(file_path)).c_str()) != INVALID_FILE_ATTRIBUTES;
}

void fix_slashes(wstring& path) {
  for (size_t i = 0; i < path.size(); i++) {
    if (path[i] == L'/') path[i] = L'\\';
  }
}

list<wstring> get_include_file_list(const wstring& file_path, const list<wstring>& include_dirs) {
  list<wstring> file_list;
  wstring text = load_file(file_path);
  wstring inc_file;
  size_t pos = 0;
  while (true) {
    if (is_include_directive(text, pos, inc_file)) {
      fix_slashes(inc_file);
      wstring inc_path = add_trailing_slash(extract_file_path(file_path)) + inc_file;
      bool found = file_exists(inc_path);
      for (list<wstring>::const_iterator inc_dir = include_dirs.begin(); !found && inc_dir != include_dirs.end(); inc_dir++) {
        inc_path = add_trailing_slash(*inc_dir) + inc_file;
        found = file_exists(inc_path);
      }
      if (found) file_list.push_back(inc_path);
    }
    pos = text.find(L'\n', pos);
    if (pos == wstring::npos) break;
    else pos++;
  }
  return file_list;
}

void process_file(wstring& output, set<wstring>& file_set, const wstring& file_path, const list<wstring>& include_dirs) {
  if (file_set.count(file_path)) return;
  file_set.insert(file_path);
  list<wstring> include_files = get_include_file_list(file_path, include_dirs);
  if (!include_files.empty()) {
    output.append(file_path).append(1, L':');
    for (list<wstring>::const_iterator inc_file = include_files.begin(); inc_file != include_files.end(); inc_file++) {
      output.append(1, L' ').append(*inc_file);
    }
    output.append(1, L'\n');
  }
  for (list<wstring>::const_iterator inc_file = include_files.begin(); inc_file != include_files.end(); inc_file++) {
    process_file(output, file_set, *inc_file, include_dirs);
  }
}

#define CHECK_CMD(code) if (!(code)) FAIL_MSG(L"Usage: gendep [-I<include> | <source_dir> ...]")
void parse_cmd_line(const deque<wstring>& params, list<wstring>& source_dirs, list<wstring>& include_dirs) {
  source_dirs.assign(1, wstring());
  for (auto param = params.cbegin(); param != params.cend(); ++param) {
    if (substr_match(*param, 0, L"-I")) {
      wstring inc_dir = param->substr(2);
      CHECK_CMD(!inc_dir.empty());
      fix_slashes(inc_dir);
      include_dirs.push_back(inc_dir);
    }
    else {
      wstring src_dir = *param;
      fix_slashes(src_dir);
      source_dirs.push_back(src_dir);
    }
  }
}
#undef CHECK_CMD

void gendep(const deque<wstring>& params) {
  list<wstring> source_dirs, include_dirs;
  parse_cmd_line(params, source_dirs, include_dirs);
  wstring output;
  set<wstring> file_set;
  for (list<wstring>::const_iterator src_dir = source_dirs.begin(); src_dir != source_dirs.end(); src_dir++) {
    DirList dir_list(get_full_path_name(src_dir->empty() ? L"." : *src_dir));
    while (dir_list.next()) {
      if (!dir_list.data().is_dir() && is_valid_ext(dir_list.data().cFileName)) {
        process_file(output, file_set, add_trailing_slash(*src_dir) + dir_list.data().cFileName, include_dirs);
      }
    }
  }
  cout << unicode_to_ansi(output, CP_ACP);
}
