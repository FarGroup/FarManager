--[[

 Support for syntax highlighting in RTF for the LXSH module.

 Author: Peter Odding <peter@peterodding.com>
 Last Change: July 18, 2011
 URL: http://peterodding.com/code/lua/lxsh/

]]

local formatter = { format = 'rtf', extension = '.rtf' }

-- Private functions. {{{1

local colortables = {}

local function getcolortable(colors)
  local colortbl = colortables[colors]
  if not colortbl then
    colortbl = {}
    local function addcolor(c)
      if c and not colortbl[c] then
        colortbl[#colortbl + 1] = c
        colortbl[c] = #colortbl
      end
    end
    for _, styles in pairs(colors) do
      addcolor(styles.color)
      addcolor(styles.background)
    end
    colortables[colors] = colortbl
  end
  return colortbl
end

-- token(kind, text, url, options) {{{1

local rtfescapes = {
  ['\\'] = '\\\\',
  ['{'] = '\\{',
  ['}'] = '\\}',
  ['\t'] = '\\tab',
  ['\n'] = '\\par\n',
}

function formatter.token(kind, text, url, options)
  text = text:gsub('[\\{}\t\n]', rtfescapes)
  local style = options.colors[kind]
  if style then
    local colortbl = getcolortable(options.colors)
    local cmds = {}
    if style.color then cmds[#cmds + 1] = '\\cf' .. colortbl[style.color] end
    if style.background then cmds[#cmds + 1] = '\\chcbpat' .. colortbl[style.background] end
    if style.bold then cmds[#cmds + 1] = '\\b' end
    if style.underline then cmds[#cmds + 1] = '\\ul' end
    text = '{' .. table.concat(cmds) .. ' ' .. text .. '}'
  end
  if url then
    text = '{\\field{\\*\\fldinst HYPERLINK "' .. url .. '"}{\\fldrslt ' .. text .. '}}'
  end
  return text
end

-- wrap(context, output, options) {{{1

function formatter.wrap(context, output, options)
  table.insert(output, 1, '{\\rtf1\\ansi\\deflang1033{\\fonttbl{\\f0 Consolas;}}\\deff0')
  table.insert(output, 2, '{\\colortbl;')
  local colortbl = getcolortable(options.colors)
  local i = 3
  for idx = 1, #colortbl do
    local template = '\\red%i\\green%i\\blue%i;'
    local rgb = string.format('%06x', colortbl[idx])
    local r = tonumber(rgb:sub(1, 2), 16)
    local g = tonumber(rgb:sub(3, 4), 16)
    local b = tonumber(rgb:sub(5, 6), 16)
    table.insert(output, i, template:format(r, g, b))
    i = i + 1
  end
  table.insert(output, i, '}')
  table.insert(output, i + 1, '{\\*\\pgdsctbl{\\pgdsc0{\\cbpat' .. colortbl[options.colors.default.background] .. '}\\pgdscnxt0 Standard;}}')
  table.insert(output, '}')
  return table.concat(output)
end

-- demo(output, options) {{{1

function formatter.demo(output, options)
  return output
end

-- }}}1

return formatter
