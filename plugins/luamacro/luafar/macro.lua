-- simple macro processor
-- started: 2012-02-16

local prefix, suffix = "#{", "}"
local pattern = prefix .. "([%a_][%w_]*)" .. suffix

local function get_predefined (t, c)
  if c == "DATE" then
    local t = os.date("*t")
    return ("%d-%02d-%02d"):format(t.year, t.month, t.day)
  elseif c == "TIME" then
    local t = os.date("*t")
    return ("%02d:%02d:%02d"):format(t.hour, t.min, t.sec)
  else
    error(c .. " is undefined")
  end
end

local meta = { __index = get_predefined }

local function preprocess (mapfile, file1, file2)
  assert(file2, "three arguments required")
  local map = setmetatable({}, meta)
  setfenv(assert(loadfile(mapfile)), map)()
  local fp = assert(io.open(file1, "rb"))
  local str = fp:read("*all")
  fp:close()
  str = string.gsub(str, pattern, map)
  fp = assert(io.open(file2, "wb"))
  fp:write(str)
  fp:close()
end

return preprocess
