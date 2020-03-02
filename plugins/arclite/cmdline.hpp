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
  std::vector<std::wstring> args;
};

CommandArgs parse_command(const std::wstring& cmd_text);
CommandArgs parse_plugin_call(const OpenMacroInfo *omi);

std::list<std::wstring> parse_listfile(const std::wstring& str);

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
  std::vector<std::wstring> files;
  std::vector<std::wstring> listfiles;
};

UpdateCommand parse_update_command(const CommandArgs& args);

struct ExtractCommand {
  ExtractOptions options;
  std::vector<std::wstring> arc_list;
};

ExtractCommand parse_extract_command(const CommandArgs& args);

struct ExtractItemsCommand {
  ExtractOptions options;
  std::wstring       archname;
  std::vector<std::wstring>  items;
};

ExtractItemsCommand parse_extractitems_command(const CommandArgs& args);

struct TestCommand {
  std::vector<std::wstring> arc_list;
};

TestCommand parse_test_command(const CommandArgs& args);
