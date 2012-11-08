--[[

 Support for syntax highlighting in LaTeX for the LXSH module.

 Author: Peter Odding <peter@peterodding.com>
 Last Change: July 18, 2011
 URL: http://peterodding.com/code/lua/lxsh/

]]

local formatter = { format = 'latex', extension = '.tex' }

-- Private functions. {{{1

-- http://en.wikibooks.org/wiki/LaTeX/Formatting#Symbols_and_special_characters

local function argescape(s)
  return (s:gsub('[%%_#]', '\\%0'))
end

local function textescape(s)
  return (s:gsub('[%%_#\n%-\\<>|& ^%[\'"{$}]', function(c)
    return c == '\n' and '\\\\\n'
        or c == '[' and '{[}'
        or c == '-' and '$-$' -- two dashes normally become an em-dash
        or c == '\\' and '\\textbackslash{}' -- double backslash is line break
        or c == '<' and '\\textless{}' -- this is normally a combining character?
        or c == '>' and '\\textgreater{}' -- this is normally a combining character?
        or c == '|' and '\\textbar{}'
        or c == '{' and '$\\{$'
        or c == '}' and '$\\}$'
        or c == "'" and c -- "{\\tt\\char'15}"
        or c == '"' and c -- '{\\tt"}'
        or c == ' ' and '\\hspace*{1.75mm}' -- only way I've found to introduce variable leading whitespace
        or c == '^' and '\\^{}'
        or ('\\' .. c)
    end))
end

local function colorcode(n)
  local rgb = string.format('%06x', n)
  local r = tonumber(rgb:sub(1, 2), 16) / 255
  local g = tonumber(rgb:sub(3, 4), 16) / 255
  local b = tonumber(rgb:sub(5, 6), 16) / 255
  return string.format('%.2f,%.2f,%.2f', r, g, b)
end

-- wrap(context, output, options) {{{1

function formatter.wrap(context, output, options)
  table.insert(output, 1, '{\\noindent\\ttfamily')
  table.insert(output, 2, '\\color[rgb]{' .. colorcode(options.colors.default.color) .. '}')
  table.insert(output, 3, '\\pagecolor[rgb]{' .. colorcode(options.colors.default.background) .. '}')
  table.insert(output, '}')
  return table.concat(output)
end

-- token(kind, text, url, options) {{{1

function formatter.token(kind, text, url, options)
  local endline = text:find '\n$'
  text = textescape(text)
  if url then text = ('\\href{%s}{%s}'):format(argescape(url), text) end
  local style = options.colors[kind]
  if style then
    if style.bold then text = ('\\textbf{%s}'):format(text) end
    if style.underline then text = ('\\underline{%s}'):format(text) end
    if style.color then text = ('\\textcolor[rgb]{%s}{%s}'):format(colorcode(style.color), text) end
    if style.background then
      -- FIXME \\ at the end of a colored box eats the line end
      text = ('\\colorbox[rgb]{%s}{%s}'):format(colorcode(style.background), text)
      if endline then text = text .. '\\\\' end
    end
  end
  return text
end

-- demo() {{{1

function formatter.demo(output, options)
  return string.format([[
\documentclass[pdftex]{article}

%% Make more vertical and horizontal space available.
\usepackage[top=1cm, bottom=1cm, left=1cm, right=1cm]{geometry}

%% The default mono space font is kind of ugly IMHO, so we select a more
%% pleasant font. Note that it took me a while to find a mono space font
%% that supported bold text!
\usepackage[T1]{fontenc}
\usepackage[scaled]{beramono}
\renewcommand*\familydefault{\ttdefault}

%% Unicode support.
\usepackage{ucs}

%% Enable literal UTF-8 characters.
\usepackage[utf8x]{inputenc}

%% Enable colored text.
\usepackage{xcolor}

%% Enable hyper links.
\usepackage[pdftex,pdfborder={0 0 0}]{hyperref}

%% Hide page numbers.
\pagestyle{empty}

\begin{document}
%s
\end{document}
]], output)
end

return formatter
