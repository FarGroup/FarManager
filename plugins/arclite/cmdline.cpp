#include "utils.hpp"
#include "sysutils.hpp"
#include "farutils.hpp"
#include "common.hpp"
#include "archive.hpp"
#include "options.hpp"
#include "cmdline.hpp"

#define CHECK_FMT(code) { if (!(code)) FAIL(E_BAD_FORMAT); }

std::wstring lc(const std::wstring& str) {
  std::wstring res;
  res.reserve(str.size());
  for (unsigned i = 0; i < str.size(); i++) {
    res += tolower(str[i]);
  }
  return res;
}

CommandArgs parse_command(const std::wstring& cmd_text) {
  CommandArgs cmd_args;
  cmd_args.cmd = cmdOpen;
  bool is_token = false;
  bool is_quote = false;
  std::wstring token;
  for (unsigned i = 0; i < cmd_text.size(); i++) {
    if (is_token) {
      if (cmd_text[i] == L'"') {
        is_quote = !is_quote;
        token += cmd_text[i];
      }
      else if (!is_quote && cmd_text[i] == L' ') {
        is_token = false;
        cmd_args.args.push_back(token);
        token.clear();
      }
      else
        token += cmd_text[i];
    }
    else if (cmd_text[i] != L' ') {
      is_token = true;
      if (cmd_text[i] == L'"')
        is_quote = true;
      token += cmd_text[i];
    }
  }
  if (!token.empty())
    cmd_args.args.push_back(token);
  if (!cmd_args.args.empty()) {
    std::wstring cmd = lc(cmd_args.args.front());
    if (cmd == L"c")
      cmd_args.cmd = cmdCreate;
    else if (cmd == L"u")
      cmd_args.cmd = cmdUpdate;
    else if (cmd == L"x")
      cmd_args.cmd = cmdExtract;
    else if (cmd == L"e")
      cmd_args.cmd = cmdExtractItems;
    else if (cmd == L"d")
      cmd_args.cmd = cmdDeleteItems;
    else if (cmd == L"t")
      cmd_args.cmd = cmdTest;
    if (cmd_args.cmd != cmdOpen)
      cmd_args.args.erase(cmd_args.args.begin());
  }
  return cmd_args;
}

CommandArgs parse_plugin_call(const OpenMacroInfo *omi) {
  CommandArgs cmd_args;
  cmd_args.cmd = cmdOpen;
  for (size_t i = 0; i < omi->Count; ++i) {
    const auto& v = omi->Values[i];
    CHECK_FMT(v.Type == FMVT_STRING);
    std::wstring s = v.String;
    if (i == 0) {
      if (s == L"c" || s == L"C") { cmd_args.cmd = cmdCreate; continue; }
      if (s == L"u" || s == L"U") { cmd_args.cmd = cmdUpdate; continue; }
      if (s == L"x" || s == L"X") { cmd_args.cmd = cmdExtract; continue; }
      if (s == L"e" || s == L"E") { cmd_args.cmd = cmdExtractItems; continue; }
      if (s == L"d" || s == L"D") { cmd_args.cmd = cmdDeleteItems; continue; }
      if (s == L"t" || s == L"T") { cmd_args.cmd = cmdTest; continue; }
    }
    cmd_args.args.push_back(s);
  }
  return cmd_args;
}

struct Param {
  std::wstring name;
  std::wstring value;
};

bool is_param(const std::wstring& param_str) {
  return !param_str.empty() && (param_str[0] == L'-' || param_str[0] == L'/');
}

Param parse_param(const std::wstring& param_str) {
  Param param;
  size_t p = param_str.find(L':');
  if (p == std::wstring::npos) {
    param.name = lc(param_str.substr(1));
  }
  else {
    param.name = lc(param_str.substr(1, p - 1));
    param.value = unquote(param_str.substr(p + 1));
  }
  return param;
}

bool parse_bool_value(const std::wstring& value) {
  if (value.empty())
    return true;
  std::wstring lcvalue = lc(value);
  if (lcvalue == L"y")
    return true;
  else if (lcvalue == L"n")
    return false;
  else
    CHECK_FMT(false);
}

TriState parse_tri_state_value(const std::wstring& value) {
  if (value.empty())
    return triTrue;
  std::wstring lcvalue = lc(value);
  if (lcvalue == L"y")
    return triTrue;
  else if (lcvalue == L"n")
    return triFalse;
  else if (lcvalue == L"a")
    return triUndef;
  else
    CHECK_FMT(false);
}

std::list<std::wstring> parse_listfile(const std::wstring& str) {
  std::list<std::wstring> files;
  unsigned pos = 0;
  for (unsigned i = 0; i < str.size(); i++) {
    if (str[i] == L'\r' || str[i] == L'\n') {
      if (pos != i)
        files.push_back(str.substr(pos, i - pos));
      pos = i + 1;
    }
  }
  if (pos != str.size())
    files.push_back(str.substr(pos, str.size() - pos));
  return files;
}


// arc:[-d] [-t:<arc_type>] [-p:<password>] <archive>

OpenCommand parse_open_command(const CommandArgs& ca) {
  OpenCommand command;
  const std::vector<std::wstring>& args = ca.args;
  CHECK_FMT(!args.empty());
  bool arc_type_spec = false;
  for (unsigned i = 0; i + 1 < args.size(); i++) {
    CHECK_FMT(is_param(args[i]));
    Param param = parse_param(args[i]);
    if (param.name == L"d")
      command.options.detect = true;
    else if (param.name == L"t") {
      arc_type_spec = true;
      command.options.arc_types = ArcAPI::formats().find_by_name(param.value);
      CHECK_FMT(!command.options.arc_types.empty());
    }
    else if (param.name == L"p") {
      CHECK_FMT(!param.value.empty());
      command.options.password = param.value;
    }
    else
      CHECK_FMT(false);
  }
  if (!arc_type_spec) {
    command.options.arc_types = ArcAPI::formats().get_arc_types();
  }
  CHECK_FMT(!is_param(args.back()));
  command.options.arc_path = unquote(expand_env_vars(args.back()));
  return command;
}


// arc:c [-pr:name] [-t:<arc_type>] [-l:<level>] [-m:<method>] [-s[:(y|n)]] [-p:<password>] [-eh[:(y|n)]] [-sfx[:<module>]] [-v:<volume_size>]
//   [-mf[:(y|n)]] [-ie[:(y|n)]] [-adv:<advanced>] <archive> (<file1> <file2> ... | @<filelist>)
// arc:u [-l:<level>] [-m:<method>] [-s[:(y|n)]] [-p:<password>] [-eh[:(y|n)]]
//   [-mf[:(y|n)]] [-ie[:(y|n)]] [-o[:(o|s)]] [-adv:<advanced>] <archive> (<file1> <file2> ... | @<filelist>)
//   <level> = 0|1|3|5|7|9
//   <method> = lzma|lzma2|ppmd

const unsigned c_levels[] = { 0, 1, 3, 5, 7, 9 };
const wchar_t* c_methods[] = { L"lzma", L"lzma2", L"ppmd" };

UpdateCommand parse_update_command(const CommandArgs& ca) {
  bool create = ca.cmd == cmdCreate;
  if (!create) {
    for (unsigned i = 0; i < ca.args.size(); i++) {
      if (!is_param(ca.args[i])) {
        create = !File::exists(Far::get_absolute_path(unquote(ca.args[i])));
        break;
      }
    }
  }
  UpdateCommand command;
  const std::vector<std::wstring>& args = ca.args;
  command.new_arc = create;
  command.level_defined = false;
  command.method_defined = false;
  command.solid_defined = false;
  command.encrypt_defined = false;
  bool arc_type_spec = false;
  for (unsigned i = 0; i < args.size() && is_param(args[i]); i++) {
    Param param = parse_param(args[i]);
    if (param.name == L"pr") {
      CHECK_FMT(create);
      unsigned prof_idx = g_profiles.find_by_name(param.value);
      CHECK_FMT(prof_idx < g_profiles.size());
      static_cast<ProfileOptions&>(command.options) = g_profiles[prof_idx].options;
      command.level_defined = true;
      command.method_defined = true;
      command.solid_defined = true;
      command.encrypt_defined = true;
      arc_type_spec = true;
      break;
    }
  }
  unsigned i = 0;
  for (; i < args.size() && is_param(args[i]); i++) {
    Param param = parse_param(args[i]);
    if (param.name == L"pr") {
    }
    else if (param.name == L"t") {
      CHECK_FMT(create);
      arc_type_spec = true;
      ArcTypes arc_types = ArcAPI::formats().find_by_name(param.value);
      CHECK_FMT(!arc_types.empty());
      command.options.arc_type = arc_types.front();
    }
    else if (param.name == L"l") {
      command.level_defined = true;
      command.options.level = str_to_int(param.value);
      CHECK_FMT(std::find(c_levels, c_levels + ARRAYSIZE(c_levels), command.options.level) != c_levels + ARRAYSIZE(c_levels));
    }
    else if (param.name == L"m") {
      command.method_defined = true;
      command.options.method = param.value;
      auto method = lc(command.options.method);
      const auto& codecs = ArcAPI::codecs();
      bool is_external = std::any_of(codecs.cbegin(), codecs.cend(), [method](const CDllCodecInfo& c) { return method == lc(c.Name); });
      CHECK_FMT(is_external || find(c_methods, c_methods + ARRAYSIZE(c_methods), method) != c_methods + ARRAYSIZE(c_methods));
    }
    else if (param.name == L"s") {
      command.solid_defined = true;
      command.options.solid = parse_bool_value(param.value);
    }
    else if (param.name == L"p") {
      CHECK_FMT(!param.value.empty());
      command.encrypt_defined = true;
      command.options.password = param.value;
      command.options.encrypt = true;
    }
    else if (param.name == L"eh")
      command.options.encrypt_header = parse_tri_state_value(param.value);
    else if (param.name == L"sfx") {
      CHECK_FMT(create);
      command.options.create_sfx = true;
      if (param.value.empty())
        command.options.sfx_options.name = L"7z.sfx";
      else
        command.options.sfx_options.name = param.value;
    }
    else if (param.name == L"v") {
      CHECK_FMT(create);
      CHECK_FMT(!param.value.empty());
      command.options.enable_volumes = true;
      command.options.volume_size = param.value;
    }
    else if (param.name == L"mf")
      command.options.move_files = parse_bool_value(param.value);
    else if (param.name == L"ie")
      command.options.ignore_errors = parse_bool_value(param.value);
    else if (param.name == L"o") {
      CHECK_FMT(!create);
      std::wstring lcvalue = lc(param.value);
      if (lcvalue.empty())
        command.options.overwrite = oaOverwrite;
      else if (lcvalue == L"o")
        command.options.overwrite = oaOverwrite;
      else if (lcvalue == L"s")
        command.options.overwrite = oaSkip;
      else
        CHECK_FMT(false);
    }
    else if (param.name == L"adv") {
      CHECK_FMT(!param.value.empty());
      command.options.advanced = param.value;
    }
    else
      CHECK_FMT(false);
  }
  CHECK_FMT(i + 1 < args.size());
  CHECK_FMT(!is_param(args[i]));
  command.options.arc_path = unquote(args[i]);
  i++;
  if (create && !arc_type_spec) {
    ArcTypes arc_types = ArcAPI::formats().find_by_ext(extract_file_ext(command.options.arc_path));
    if (!arc_types.empty())
      command.options.arc_type = arc_types.front();
  }
  for (; i < args.size(); i++) {
    CHECK_FMT(!is_param(args[i]));
    if (args[i][0] == L'@')
      command.listfiles.push_back(unquote(args[i].substr(1)));
    else
      command.files.push_back(unquote(args[i]));
  }

  if (command.options.level == 0)
    command.options.method = c_method_copy;

  return command;
}


// arc:x [-ie[:(y|n)]] [-o[:(o|s|r|a)]] [-mf[:(y|n)]] [-p:<password>] [-sd[:(a|y|n)]] [-da[:(y|n)]] <archive1> <archive2> ... <path>
//
// arc:e [-ie[:(y|n)]] [-o[:(o|s|r|a)]] [-mf[:(y|n)]] [-p:<password>] [-out:<outdir>] <archive> <extract_item> ...
//
// arc:d [-ie[:(y|n)]] [-p:<password>] <archive> <delete_item> ...
//
static void parse_extract_params(const CommandArgs& ca, ExtractOptions& o, std::vector<std::wstring>& items) {
  bool options_enabled = true;
  for (const auto& a : ca.args) {
    if (options_enabled && a == L"--")
      options_enabled = false;
    else if (options_enabled && is_param(a)) {
      auto param = parse_param(a);
      if (param.name == L"ie")
        o.ignore_errors = parse_bool_value(param.value);
      else if (ca.cmd != cmdDeleteItems && param.name == L"o") {
        auto lcvalue = lc(param.value);
        if (lcvalue.empty() || lcvalue == L"o")
          o.overwrite = oaOverwrite;
        else if (lcvalue == L"s")
          o.overwrite = oaSkip;
        else if (lcvalue == L"r")
          o.overwrite = oaRename;
        else if (lcvalue == L"a")
          o.overwrite = oaAppend;
        else
          CHECK_FMT(false);
      }
      else if (ca.cmd != cmdDeleteItems && param.name == L"mf")
        o.move_files = parse_tri_state_value(param.value);
      else if (param.name == L"p") {
        CHECK_FMT(!param.value.empty());
        o.password = param.value;
      }
      else if (ca.cmd == cmdExtract && param.name == L"sd")
        o.separate_dir = parse_tri_state_value(param.value);
      else if (ca.cmd == cmdExtract && param.name == L"da")
        o.delete_archive = parse_bool_value(param.value);
      else if (ca.cmd == cmdExtractItems && param.name == L"out" && !param.value.empty())
        o.dst_dir = unquote(param.value);
      else
        CHECK_FMT(false);
    }
    else {
      items.push_back(unquote(a));
    }
  }
}
//
ExtractCommand parse_extract_command(const CommandArgs& ca) {
  ExtractCommand command;
  parse_extract_params(ca, command.options, command.arc_list);
  CHECK_FMT(command.arc_list.size() >= 2);
  command.options.dst_dir = command.arc_list.back();
  command.arc_list.pop_back();
  return command;
}
//
ExtractItemsCommand parse_extractitems_command(const CommandArgs& ca) {
  ExtractItemsCommand command;
  parse_extract_params(ca, command.options, command.items);
  CHECK_FMT(command.items.size() >= 2);
  command.archname = command.items.front();
  command.items.erase(command.items.begin());
  return command;
}

// arc:t <archive1> <archive2> ...
//
TestCommand parse_test_command(const CommandArgs& ca) {
  TestCommand command;
  const std::vector<std::wstring>& args = ca.args;
  for (unsigned i = 0; i < args.size(); i++) {
    CHECK_FMT(!is_param(args.back()));
    command.arc_list.push_back(unquote(args[i]));
  }
  return command;
}
