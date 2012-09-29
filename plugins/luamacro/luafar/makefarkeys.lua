-- started: 2008-12-15 by Shmuel Zeigerman

local function extract_enums (src)
  local collector = {}
  for enum in src:gmatch("%senum%s*[%w_]*%s*(%b{})") do
    for line in enum:gmatch("[^\n]+") do
      if line:match("^%s*#") then table.insert(collector, line)
      else
        local var = line:match("^%s*([%a_][%w_]*)")
        if var then table.insert(collector, var) end
      end
    end
  end
  return collector
end

local sTmpFile = [[
#include <windows.h>
#include <stdio.h>
#include <farcolor.hpp>

int main() {
]]

local sOutFile = [[
// This is a generated file.
#include <lua.h>
#include <lauxlib.h>

static const char colors[] = "return { $colors }";

// output table is on stack top
void SetFarColors (lua_State *L)
{
  luaL_loadstring(L, colors);
  lua_call(L, 0, 1);
  lua_setfield(L, -2, "Colors");
}
]]

local function get_insert (in_dir, src)
  local tb = { sTmpFile, }
  for _,v in ipairs(extract_enums(src)) do
    if v:match("^%s*#") then table.insert(tb, v)
    else table.insert(tb, ('  printf("%s=%%u,", (unsigned int)%s);'):format(v,v))
    end
  end
  table.insert(tb, "  return 0;\n}\n")

  local fp = assert(io.open("tmp.c", "w"))
  fp:write(table.concat(tb, "\n"))
  fp:close()

  -- Note 1: "-m32" makes possible to cross-compile LuaFAR for 64-bit target on 32-bit OS.
  -- Note 2: "-DWINVER=0x601" corresponds to Windows 7.
  -- assert(0 == os.execute("gcc -o tmp.exe -m32 -DWINVER=0x601 -I"..in_dir.." tmp.c"))

  assert((0 == os.execute("cl.exe /nologo /I"..in_dir.." tmp.c > nul 2>&1")) or (0 == os.execute("gcc -o tmp.exe -m32 -DWINVER=0x601 -I"..in_dir.." tmp.c > nul 2>&1")))

  fp = assert(io.popen("tmp.exe"))
  local str = fp:read("*all")
  fp:close()
  os.remove "tmp.exe"
  os.remove "tmp.c"
  os.remove "tmp.obj"
  return str
end

local function makefarcolors (in_dir, out_file)
  local fp = assert(io.open(in_dir.."\\farcolor.hpp"))
  local src = fp:read("*all")
  fp:close()
  local out = sOutFile:gsub("$colors", get_insert(in_dir, src))

  fp = io.open(out_file, "w")
  fp:write(out)
  fp:close()
end

local in_dir, out_file = ...
assert(in_dir, "input directory not specified")
assert(out_file, "output file not specified")
makefarcolors(in_dir, out_file)
