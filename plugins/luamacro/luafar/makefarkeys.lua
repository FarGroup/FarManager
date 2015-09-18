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

 
local sOutFile = [[
// This is a generated file.
#include <lua.h>
#include <lauxlib.h>

static const char colors[] = "return { %s }";
static const char guids[] = "return { %s }";

// output table is on stack top
void SetFarColors (lua_State *L)
{
  luaL_loadstring(L, colors);
  lua_call(L, 0, 1);
  lua_setfield(L, -2, "Colors");

  luaL_loadstring(L, guids);
  lua_call(L, 0, 1);
  lua_setfield(L, -2, "Guids");
}
]]

local function get_insert (src)
  local tb = {}
  for i,v in ipairs(extract_enums(src)) do
    table.insert(tb, ('%s=%d'):format(v,i-1))
  end
  return table.concat(tb, ",")
end

local function makefarcolors (colors_file, guids_file, out_file)
  local fp = assert(io.open(colors_file))
  local colors = get_insert(fp:read("*all"))
  fp:close()

--{40A699F1-BBDD-4E21-A137-97FFF798B0C8}
--DEFINE_GUID(EditAskSaveExtId,0x40a699f1,0xbbdd,0x4e21,0xa1,0x37,0x97,0xff,0xf7,0x98,0xb0,0xc8);
  fp = assert(io.open(guids_file))
  local collect = {}
  local pat = "^%s*DEFINE_GUID.-([%w_]+).-"..("0[xX](%x+).-"):rep(11)
  for line in fp:lines() do
    local t = { line:match(pat) }
    if t[1] then
      for k=2,#t do t[k] = tonumber(t[k],16) end
      collect[#collect+1] = ("%s='%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X'"):format(unpack(t))
    else
      assert(not line:match("^%s*DEFINE_GUID"))
    end
  end
  assert(#collect >= 83)
  local guids = table.concat(collect, ",")
  fp:close()
  
  fp = io.open(out_file, "w")
  fp:write(sOutFile:format(colors, guids))
  fp:close()
end

local colors_file, guids_file, out_file = ...
assert(colors_file, "Colors file not specified")
assert(guids_file,  "GUIDs file not specified")
assert(out_file,    "Output file not specified")
makefarcolors(colors_file, guids_file, out_file)
