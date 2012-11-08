--[[

 Syntax highlighter for BibTeX source code.

 Author: Peter Odding <peter@peterodding.com>
 Last Change: September 29, 2011
 URL: http://peterodding.com/code/lua/lxsh/

]]

local lxsh = require 'lxsh'

return lxsh.highlighters.new {
  lexer = lxsh.lexers.bib,
  docs = lxsh.docs.bib,
  aliases = {
    delimiter = 'operator',
  },
}

-- vim: ts=2 sw=2 et
