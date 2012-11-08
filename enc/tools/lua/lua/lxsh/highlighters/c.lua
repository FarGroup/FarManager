--[[

 Syntax highlighter for C source code.

 Author: Peter Odding <peter@peterodding.com>
 Last Change: July 10, 2011
 URL: http://peterodding.com/code/lua/lxsh/

]]

local lxsh = require 'lxsh'
local lpeg = require 'lpeg'

return lxsh.highlighters.new {
  lexer = lxsh.lexers.c,
  docs = lxsh.docs.c,
  escape_sequence = lpeg.P'%' * lpeg.R('AZ', 'az', '09')
                  + lpeg.P'\\' * ((#lpeg.R'07' * lpeg.R'07'^-3) + 1),
  has_escapes = function(kind, text)
    return kind == 'constant'
  end,
}

-- vim: ts=2 sw=2 et
