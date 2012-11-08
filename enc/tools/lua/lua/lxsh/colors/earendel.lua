--[[

 Syntax highlighting color scheme based on the Vim color scheme Earendel by
 Georg Dahn, available at http://www.vim.org/scripts/script.php?script_id=2188.

 Author: Peter Odding <peter@peterodding.com>
 Last Change: July 18, 2011
 URL: http://peterodding.com/code/lua/lxsh/

 FIXME Make sure Georg Dahn doesn't mind my copying his color scheme!

]]

return {
  comment = { color = 0x558817 },
  constant = { color = 0xa8660d },
  default = { color = 0x000000, background = 0xffffff },
  escape = { color = 0x844631 },
  keyword = { color = 0x2239a8, bold = true },
  library = { color = 0x0e7c6b },
  marker = { color = 0x512b1e, background = 0xfedc56, bold = true },
  number = { color = 0xa8660d },
  operator = { color = 0x2239a8, bold = true },
  preprocessor = { color = 0xa33243 },
  prompt = { color = 0x558817 },
  url = { color = 0x272fc2, underline = true },
}
