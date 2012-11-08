--[[

 Syntax highlighter for Lua 5.1 source code.

 Author: Peter Odding <peter@peterodding.com>
 Last Change: July 10, 2011
 URL: http://peterodding.com/code/lua/lxsh/

 TODO Distinguish "function" and corresponding "end" keyword from other keywords (I really like this in Vim).

]]

local lxsh = require 'lxsh'
local lpeg = require 'lpeg'

return lxsh.highlighters.new {
  lexer = lxsh.lexers.lua,
  docs = lxsh.docs.lua,
  escape_sequence = lpeg.P'%' * ('%' + lpeg.R('AZ', 'az', '09'))
                  + lpeg.P'\\' * ((#lpeg.R'09' * lpeg.R'09'^-3) + 1),
  has_escapes = function(kind, text)
    return kind == 'string' and text:find '^[\'"]'
  end,
}

-- vim: ts=2 sw=2 et
