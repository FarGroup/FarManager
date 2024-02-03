-- This script is intended to generate the "flags.c" file

local function enum_blacklist (s)
  return s:find("^FMSG_") or s:find("^[EFMPSV]CTL_") or s:find("^FFCTL_") or s:find("^RECTL_")
end

local function add_defines (src, trg_keys, trg_vals)
  local cast1 = "%(HANDLE%)" -- suppress compiler warnings
  local cast2 = "%(void%*%)" -- suppress compiler warnings
  for k,v in src:gmatch("#define%s+([A-Z][A-Z0-9_]*)[ \t]+([^\n]+)") do
    if k ~= "FARMANAGERVERSION" and k ~= "FAR_INLINE_CONSTANT" then
      table.insert(trg_keys, k)
      if   v:find(cast1) or v:find(cast2) then v = "(INT_PTR)" .. k
      else v = k
      end
      table.insert(trg_vals, v)
    end
  end
end

local function add_enums (src, trg_keys, trg_vals)
  for enum in src:gmatch("%senum%s*[%w_]*%s*(%b{})") do
    for c in enum:gmatch("\n%s*([%w_]+)") do
      if not enum_blacklist(c) then
        table.insert(trg_keys, c)
        table.insert(trg_vals, c)
      end
    end
  end
end

local function add_constants  (src, trg_keys, trg_vals)
  for chunk in src:gmatch("FAR_INLINE_CONSTANT%s+[^;]-;") do
    for k,v in chunk:gmatch("\n%s*([%w_]+)%s*=%s*(%w+)") do
      table.insert(trg_keys, k)
      table.insert(trg_vals, v)
    end
  end
end

local function write_target  (trg_keys, trg_vals)
  print "const char* Keys[] = {"
  for k,v in ipairs(trg_keys) do
    print(string.format('  "%s",', v))
  end
  print("};\n")

  --print "const unsigned __int64 Vals[] = {"
  print "const unsigned __int64 Vals[] = {"
  for k,v in ipairs(trg_vals) do
    print(string.format('  %s,', v))
  end
  print("};\n")
end

-- Windows API constants
local t_winapi = {
  "FOREGROUND_BLUE", "FOREGROUND_GREEN", "FOREGROUND_RED",
  "FOREGROUND_INTENSITY", "BACKGROUND_BLUE", "BACKGROUND_GREEN",
  "BACKGROUND_RED", "BACKGROUND_INTENSITY", "CTRL_C_EVENT", "CTRL_BREAK_EVENT",
  "CTRL_CLOSE_EVENT", "CTRL_LOGOFF_EVENT", "CTRL_SHUTDOWN_EVENT",
  "ENABLE_LINE_INPUT", "ENABLE_ECHO_INPUT", "ENABLE_PROCESSED_INPUT",
  "ENABLE_WINDOW_INPUT", "ENABLE_MOUSE_INPUT", "ENABLE_INSERT_MODE",
  "ENABLE_QUICK_EDIT_MODE", "ENABLE_EXTENDED_FLAGS", "ENABLE_AUTO_POSITION",
  "ENABLE_PROCESSED_OUTPUT", "ENABLE_WRAP_AT_EOL_OUTPUT", "KEY_EVENT",
  "MOUSE_EVENT", "WINDOW_BUFFER_SIZE_EVENT", "MENU_EVENT", "FOCUS_EVENT",
  "CAPSLOCK_ON", "ENHANCED_KEY", "RIGHT_ALT_PRESSED", "LEFT_ALT_PRESSED",
  "RIGHT_CTRL_PRESSED", "LEFT_CTRL_PRESSED", "SHIFT_PRESSED", "NUMLOCK_ON",
  "SCROLLLOCK_ON", "FROM_LEFT_1ST_BUTTON_PRESSED", "RIGHTMOST_BUTTON_PRESSED",
  "FROM_LEFT_2ND_BUTTON_PRESSED", "FROM_LEFT_3RD_BUTTON_PRESSED",
  "FROM_LEFT_4TH_BUTTON_PRESSED", "MOUSE_MOVED", "DOUBLE_CLICK", "MOUSE_WHEELED", "MOUSE_HWHEELED",
  "KEY_WOW64_32KEY", "KEY_WOW64_64KEY",
}


local file_top = [[
// flags.c
// DON'T EDIT: THIS FILE IS AUTO-GENERATED.

#include <lua.h>
#include <plugin.hpp>

extern int bit64_push(lua_State *L, __int64 v);
]]


local file_bottom = [[
// create a table; fill with flags; leave on stack
void push_flags_table (lua_State *L)
{
  int i;
  const int nelem = sizeof(Keys) / sizeof(Keys[0]);
  lua_createtable (L, 0, nelem);
  for (i=0; i<nelem; ++i) {
    bit64_push(L, Vals[i]);
    lua_setfield(L, -2, Keys[i]);
  }
}
]]

local function write_common_flags_file (fname)
  assert (fname, "input file not specified")
  local fp = assert (io.open (fname, "rb"))
  local src = fp:read ("*all")
  fp:close()

  local tb_keys, tb_vals = {}, {}
  add_defines(src, tb_keys, tb_vals)
  add_enums(src, tb_keys, tb_vals)
  add_constants(src, tb_keys, tb_vals)
  for _,v in ipairs(t_winapi) do
    table.insert(tb_keys, v)
    table.insert(tb_vals, v)
  end

  print(file_top)
  write_target(tb_keys, tb_vals)
  print(file_bottom)
end

local fname = assert((...))
write_common_flags_file(fname)
