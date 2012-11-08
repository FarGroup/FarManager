--[[

 Support for syntax highlighting in HTML for the LXSH module.

 Author: Peter Odding <peter@peterodding.com>
 Last Change: October 4, 2011
 URL: http://peterodding.com/code/lua/lxsh/

 TODO Tests for the formatters

]]

local formatter = { format = 'html', extension = '.html' }

-- escape(text, options) {{{1

local html_entities = {
  ['<'] = '&lt;',
  ['>'] = '&gt;',
  ['&'] = '&amp;',
}

local function escape(text, options)
  -- Escape special characters.
  text = text:gsub('[<>&]', html_entities)
  if options.encodews then
    text = text:gsub(' ', '&nbsp;')
  end
  return text
end

-- style(kind, options) {{{1

local function css_rules(attrs, readable)
  local rules = {}
  if type(attrs) == 'table' then
    if attrs.color then rules[#rules + 1] = string.format('color:#%06X', attrs.color) end
    if attrs.background then rules[#rules + 1] = string.format('background:#%06X', attrs.background) end
    if attrs.bold then rules[#rules + 1] = 'font-weight:bold' end
    if attrs.underline then rules[#rules + 1] = 'text-decoration:underline' end
  end
  return table.concat(rules, readable and '; ' or ';')
end

-- wrap(context, output, options) {{{1

function formatter.wrap(context, output, options)
  local wrapper = options.wrapper or 'pre'
  local elem = '<' .. wrapper
  if not options.external then
    elem = elem .. ' style="' .. css_rules(options.colors.default) .. '"'
  end
  table.insert(output, 1, elem .. ' class="sourcecode ' .. context.lexer.language .. '">')
  table.insert(output, '</' .. wrapper .. '>')
  local html = table.concat(output)
  if options.encodews then
    html = html:gsub('\r?\n', '<br>')
  end
  return html
end

-- token(kind, text, url, options) {{{1

function formatter.token(kind, text, url, options)
  text = escape(text, options)
  if kind == 'whitespace' then
    -- Don't wrap whitespace tokens to save space.
    return text
  end
  -- Obfuscate e-mail addresses.
  if url and url:find '^mailto:' then
    url = url:gsub('.', function(c)
      return ('&#%d;'):format(c:byte())
    end)
  end
  -- Decide whether to use inline or external styles (if any).
  local attr = options.external and 'class' or 'style'
  local value = options.external and kind or css_rules(options.colors[kind])
  local style = (value or '') ~= '' and (' ' .. attr .. '="' .. value .. '"') or ''
  if url then
    return ('<a href="%s"%s>%s</a>'):format(url, style, text)
  else
    if style ~= '' then
      return ('<span%s>%s</span>'):format(style, text)
    else
      return text
    end
  end
end

-- demo() {{{1

function formatter.demo(output, options)
  return table.concat({
    [[
<html>
<head>
<style type="text/css">
html, body {
  margin: 0;
  padding: 0;
  height: 100%;
}
pre {
  margin: 0;
  padding: 1em;
  height: auto !important; /* real browsers */
  height: 100%; /* IE6: treaded as min-height*/
  min-height: 100%; /* real browsers */
}
</style>
]], formatter.preamble(options.colors, true),
'\n</head>\n<body>\n', output, '\n</body>\n</html>\n'})
end

-- preamble(default, includeswitcher) {{{1

-- Generate the HTML to include the LXSH style sheets (CSS files) and
-- optionally the JavaScript for the style sheet switcher.

function formatter.preamble(default, includeswitcher)
  local template = '<link rel="%s" type="text/css" href="http://peterodding.com/code/lua/lxsh/styles/%s.css" title="%s">'
  local output = {}
  for _, style in ipairs { 'earendel', 'slate', 'wiki' } do
    local rel = lxsh.colors[style] == default and 'stylesheet' or 'alternate stylesheet'
    output[#output + 1] = template:format(rel, style, style:gsub('^%w', string.upper))
  end
  if includeswitcher then
    output[#output + 1] = '<script type="text/javascript" src="http://peterodding.com/code/lua/lxsh/styleswitcher.js"></script>'
  end
  return table.concat(output, '\n')
end

-- stylesheet(name) {{{1

-- Generate a CSS style sheet from an LXSH color scheme.

function formatter.stylesheet(name)
  local keys = {}
  for k in pairs(lxsh.colors[name]) do
    keys[#keys + 1] = k
  end
  table.sort(keys)
  local output = {}
  for _, key in ipairs(keys) do
    local rules = css_rules(lxsh.colors[name][key], true)
    if key == 'default' then
      output[#output + 1] = ('.sourcecode { font-family: Monaco, Consolas, monospace; %s; }'):format(rules)
    elseif key == 'url' then
      output[#output + 1] = ('.sourcecode a:link, .sourcecode a:visited { %s; }'):format(rules)
    elseif key == 'library' then
      rules = (rules .. ';'):gsub(';', ' !important;')
      output[#output + 1] = ('.sourcecode .%s { %s }'):format(key, rules)
    else
      output[#output + 1] = ('.sourcecode .%s { %s; }'):format(key, rules)
    end
  end
  return table.concat(output, '\n')
end

return formatter
