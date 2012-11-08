--[[

 Syntax highlighting color scheme based on the
 color scheme used on http://lua-users.org/wiki/.

 Author: Peter Odding <peter@peterodding.com>
 Last Change: July 17, 2011
 URL: http://peterodding.com/code/lua/lxsh/

]]

return {
  comment = { color = 0x00A000 },
  constant = { color = 0x009090 },
  default = { color = 0x000000, background = 0xffffff },
  keyword = { color = 0x000080, bold = true },
  library = { color = 0x900090 },
  marker = { background = 0xffff00 },
  prompt = { color = 0x00A000 },
  url = { color = 0x0000FF, underline = true }
}
