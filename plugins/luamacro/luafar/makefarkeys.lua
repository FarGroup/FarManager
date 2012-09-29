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
  local tb = {}
  local i = 0
  for _,v in ipairs(extract_enums(src)) do
    table.insert(tb, ('%s=%d,'):format(v,i))
    i = i + 1
  end
  return table.concat(tb, " ")
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
