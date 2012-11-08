--[[

 Infrastructure to make it easier to define lexers using LPeg
 and perform syntax highlighting based on the defined lexers.

 Author: Peter Odding <peter@peterodding.com>
 Last Change: December 3, 2011
 URL: http://peterodding.com/code/lua/lxsh/

]]

local function autoload(path, constructor)
  return setmetatable({}, {
    __index = function(self, key)
      -- The init.lua file for a group of submodules is always loaded first.
      if constructor and not rawget(self, 'new') then
        require(path .. '.init')
        local value = rawget(self, key)
        if value then return value end
      end
      -- Load the requested submodule.
      local value = require(path .. '.' .. key)
      self[key] = value
      return value
    end,
  })
end

local lxsh = {
  _VERSION = '0.8.7',
  lexers = autoload('lxsh.lexers', true),
  highlighters = autoload('lxsh.highlighters', true),
  formatters = autoload 'lxsh.formatters',
  colors = autoload 'lxsh.colors',
  docs = autoload 'lxsh.docs',
}

function lxsh.sync(token, lnum, cnum)
  local lastidx
  lnum, cnum = lnum or 1, cnum or 1
  if token:find '\n' then
    for i in token:gmatch '()\n' do
      lnum = lnum + 1
      lastidx = i
    end
    cnum = #token - lastidx + 1
  else
    cnum = cnum + #token
  end
  return lnum, cnum
end

-- Register LXSH in the global scope if it doesn't clash with an existing
-- global variable and bypass strict.lua because "we know what we're doing"
-- (in other words, "lua -llxsh" is very convenient).
if not rawget(_G, 'lxsh') then
  rawset(_G, 'lxsh', lxsh)
end

return lxsh

-- vim: ts=2 sw=2 et
