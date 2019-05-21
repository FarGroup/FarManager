#pragma once

#define E_BAD_FORMAT MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x201)

enum CommandType {
  cmdOpen,
  cmdCreate,
  cmdUpdate,
  cmdExtract,
  cmdTest,
  cmdExtractItems,
  cmdDeleteItems
};

struct CommandArgs {
  CommandType cmd;
  vector<wstring> args;
};

CommandArgs parse_command(const wstring& cmd_text);
CommandArgs parse_plugin_call(const OpenMacroInfo *omi);

list<wstring> parse_listfile(const wstring& str);

struct OpenCommand {
  OpenOptions options;
};

OpenCommand parse_open_command(const CommandArgs& args);

struct UpdateCommand {
  bool new_arc;
  bool level_defined;
  bool method_defined;
  bool solid_defined;
  bool encrypt_defined;
  UpdateOptions options;
  vector<wstring> files;
  vector<wstring> listfiles;
};

UpdateCommand parse_update_command(const CommandArgs& args);

struct ExtractCommand {
  ExtractOptions options;
  vector<wstring> arc_list;
};

ExtractCommand parse_extract_command(const CommandArgs& args);

struct ExtractItemsCommand {
  ExtractOptions options;
  wstring       archname;
  vector<wstring>  items;
};

ExtractItemsCommand parse_extractitems_command(const CommandArgs& args);

struct TestCommand {
  vector<wstring> arc_list;
};

TestCommand parse_test_command(const CommandArgs& args);
