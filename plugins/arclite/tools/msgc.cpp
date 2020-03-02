struct MsgPair {
  std::wstring id;
  std::wstring phrase;
  bool operator==(const MsgPair& lp) const {
    return id == lp.id;
  }
  bool operator<(const MsgPair& lp) const {
    return id < lp.id;
  }
};

struct MsgFile {
  std::deque<MsgPair> msg_list;
  std::wstring lang_tag;
  unsigned code_page;
};

#define CHECK_PARSE(code) if (!(code)) FAIL_MSG(L"Parse error at " + file_path + L":" + int_to_str(line_cnt + 1))
MsgFile load_msg_file(const std::wstring& file_path) {
  MsgFile msg_file;
  std::wstring text = load_file(file_path, &msg_file.code_page);
  MsgPair mp;
  unsigned line_cnt = 0;
  size_t pos_start = 0;
  while (pos_start < text.size()) {
    size_t pos_end = text.find(L'\n', pos_start);
    if (pos_end == std::wstring::npos) pos_end = text.size();
    if (text[pos_start] == L'#') {
      // skip comment
    }
    else if (strip(text.substr(pos_start, pos_end - pos_start)).size() == 0) {
      // skip empty line
    }
    else {
      size_t pos_sep = text.find(L'=', pos_start);
      CHECK_PARSE((pos_sep != std::wstring::npos) && (pos_sep <= pos_end));
      mp.id = strip(text.substr(pos_start, pos_sep - pos_start));
      if (line_cnt == 0) {
        // first line must be language tag
        CHECK_PARSE(mp.id == L".Language");
        msg_file.lang_tag = strip(text.substr(pos_start, pos_end - pos_start));
      }
      else {
        // convert id
        for (unsigned i = 0; i < mp.id.size(); i++) {
          if (((mp.id[i] >= L'A') && (mp.id[i] <= L'Z')) || ((mp.id[i] >= L'0') && (mp.id[i] <= L'9')) || (mp.id[i] == L'_')) {
          }
          else if ((mp.id[i] >= L'a') && (mp.id[i] <= L'z')) {
            mp.id[i] = mp.id[i] - L'a' + L'A';
          }
          else if (mp.id[i] == L'.') {
            mp.id[i] = L'_';
          }
          else {
            CHECK_PARSE(false);
          }
        }
        mp.id.insert(0, L"MSG_");
        mp.phrase = L'"' + strip(text.substr(pos_sep + 1, pos_end - pos_sep - 1)) + L'"';
        msg_file.msg_list.push_back(mp);
      }
    }
    pos_start = pos_end + 1;
    line_cnt++;
  }

  sort(msg_file.msg_list.begin(), msg_file.msg_list.end());
  return msg_file;
}
#undef CHECK_PARSE

struct FileNamePair {
  std::wstring in;
  std::wstring out;
};

#define CHECK_CMD(code) if (!(code)) FAIL_MSG(L"Usage: msgc -in msg_file [msg_file2 ...] -out header_file lng_file [lng_file2 ...]")
void parse_cmd_line(const std::deque<std::wstring>& params, std::deque<FileNamePair>& files, std::wstring& header_file) {
  CHECK_CMD(params.size());
  unsigned idx = 0;
  CHECK_CMD(params[idx] == L"-in");
  idx++;
  files.clear();
  FileNamePair fnp;
  while ((idx < params.size()) && (params[idx] != L"-out")) {
    fnp.in = params[idx];
    files.push_back(fnp);
    idx++;
  }
  CHECK_CMD(files.size());
  CHECK_CMD(idx != params.size());
  idx++;
  CHECK_CMD(idx != params.size());
  header_file = params[idx];
  idx++;
  for (unsigned i = 0; i < files.size(); i++) {
    CHECK_CMD(idx != params.size());
    files[i].out = params[idx];
    idx++;
  }
}
#undef CHECK_CMD

void msgc(const std::deque<std::wstring>& params) {
  std::deque<FileNamePair> files;
  std::wstring header_file;
  parse_cmd_line(params, files, header_file);
  // load message files
  std::deque<MsgFile> msgs;
  for (unsigned i = 0; i < files.size(); i++) {
    msgs.push_back(load_msg_file(files[i].in));
    if (i) {
      if (msgs[i].msg_list != msgs[i - 1].msg_list) FAIL_MSG(L"Message files '" + files[i].in + L"' and '" + files[i - 1].in + L"' do not match");
    }
  }
  // create header file
  std::wstring header_data;
  for (unsigned i = 0; i < msgs[0].msg_list.size(); i++) {
    header_data.append(L"#define " + msgs[0].msg_list[i].id + L" " + int_to_str(i) + L"\n");
  }
  save_file(header_file, header_data, CP_ACP);
  // create Far language files
  std::wstring lng_data;
  for (unsigned i = 0; i < msgs.size(); i++) {
    lng_data = msgs[i].lang_tag + L'\n';
    for (unsigned j = 0; j < msgs[i].msg_list.size(); j++) {
      lng_data += msgs[i].msg_list[j].phrase + L'\n';
    }
    save_file(files[i].out, lng_data, msgs[i].code_page);
  }
}
