--[[

 Syntax highlighter for shell script code.

 Author: Peter Odding <peter@peterodding.com>
 Last Change: September 30, 2011
 URL: http://peterodding.com/code/lua/lxsh/

]]

local lxsh = require 'lxsh'

return lxsh.highlighters.new {
  lexer = lxsh.lexers.sh,
  aliases = {
    variable = 'constant',
    command = 'keyword',
  },
}

-- vim: ts=2 sw=2 et
