--[[

 Syntax highlighting color scheme based on the Vim color scheme Slate by Ralph Amissah,
 available at http://code.google.com/p/vim/source/browse/runtime/colors/slate.vim.

 Author: Peter Odding <peter@peterodding.com>
 Last Change: July 17, 2011
 URL: http://peterodding.com/code/lua/lxsh/

]]

return {
  comment = { color = 0x666666 },
  constant = { color = 0xffa0a0 },
  default = { color = 0xffffff, background = 0x262626 },
  escape = { color = 0xbdb76b },
  keyword = { color = 0xffff60, bold = true },
  library = { color = 0xfa8072 },
  marker = { color = 0x262626, background = 0xffff00, bold = true },
  number = { color = 0xffa0a0 },
  operator = { color = 0xff0000 },
  preprocessor = { color = 0xff0000, background = 0xffffff },
  prompt = { color = 0x666666 },
  string = { color = 0x87ceeb },
  url = { color = 0x80a0ff, underline = true },
}
