-- TP2HH  :  Convert "TreePad" file to "HTML Help" project files
-- Author :  S.Zeigerman
-- Started:  29 Dec 2000
-- Ported to Lua: 02-03 Dec 2008

local rex = require "rex_pcre"
local discount = require "discount"
local lxsh = require "lxsh"

local function fprintf(fp_out, fmt, ...)
  fp_out:write(fmt:format(...))
end

local function HTML_convert(s)
  local tb = {["&"]="&amp;", ["<"]="&lt;", [">"]="&gt;", ['"']="&quot;"}
  return (s:gsub('[&<>"]', tb))
end

local function HTML_puts(s, fp_out)
  fp_out:write(HTML_convert(s))
end

local function fputs_indent(s, fp_out, indent)
  for i=1,indent do fp_out:write '\t' end
  fp_out:write(s)
end

local function writeProjectHeader (fp_out, ProjectName, Title, DefaultTopic, codepage, language)
  local WindowName = "MyDefaultWindow"
  fprintf(fp_out, "[OPTIONS]\n")
  fprintf(fp_out, "Compatibility=1.1 or later\n")
  fprintf(fp_out, "Compiled file=%s.chm\n", ProjectName)
  fprintf(fp_out, "Contents file=%s.hhc\n", ProjectName)
  fprintf(fp_out, "Default Window=%s\n",    WindowName)
  fprintf(fp_out, "Default topic=%s\n", DefaultTopic)
  fprintf(fp_out, "Display compile progress=No\n")
  fprintf(fp_out, "Error log file=Logfile.txt\n")
  fprintf(fp_out, "Full-text search=Yes\n")
  fprintf(fp_out, "Language=%s\n", language)
  fprintf(fp_out, "Title=%s\n", Title)
  fprintf(fp_out, "\n[WINDOWS]\n")
  fprintf(fp_out, "%s=,\"%s.hhc\",,,\"%s\",,,,,0x23520,,0x301e,,,,,,,,0\n",
          WindowName, ProjectName, DefaultTopic)
  fprintf(fp_out, "\n[FILES]\n")
end

local function writeTocHeader(fp_out)
  fp_out:write("<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML//EN\">\n",
    "<HTML>\n",
    "<HEAD>\n",
    "<meta name=\"GENERATOR\" content=\"Microsoft&reg; HTML Help Workshop 4.1\">\n",
    "<!-- Sitemap 1.0 -->\n",
    "</HEAD><BODY>\n",
    "<OBJECT type=\"text/site properties\">\n",
    "\t<param name=\"Auto Generated\" value=\"No\">\n",
    "</OBJECT>\n")
end

local function writeTopicHeader(fp_out, Title, fp_template, codepage)
  if not fp_template then
      fp_out:write("<HTML>\n<HEAD>\n<TITLE>\n")
      HTML_puts(Title, fp_out)
      fp_out:write("\n</TITLE>\n</HEAD>\n<BODY>\n")
      -- put the topic title at the article beginning
      fp_out:write("<H2>")
      HTML_puts(Title,  fp_out)
      fp_out:write("</H2>\n<HR>\n")
      return
  end

  -- Using the "template file" as formatting template:
  --   a)  There are two "keywords": <!--Body--> and <!--Title-->
  --       (both are case sensitive !!!)
  --   b)  This keywords serve as placeholders for the document's
  --       body and title respectively.
  --   c)  There must be exactly one <!--Body--> in the template file.
  --   d)  <!--Body--> must begin from the leftmost column. All
  --       following characters in that line will be ignored.
  --   e)  <!--Title--> must not encounter more than once in a line.
  --   f)  No <!--Title--> may be placed after the <!--Body-->
  --
  fp_template:seek("set", 0)
  for line in fp_template:lines() do
      if "<!--Body-->" == line:sub(1,11) then return end
      local cp1, cp2 = line:find("<!--Title-->", 1, true)
      if not cp1 then
          if "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=windows-%s\">" == line then
              fprintf(fp_out, line, codepage)
              fp_out:write("\n")
          else
              fp_out:write(line, "\n")
          end
      else
          fp_out:write(line:sub(1, cp1 - 1))
          HTML_puts(Title, fp_out)
          fp_out:write(line:sub(cp2 + 1), "\n")
      end
  end
end

local function writeTopicFooter(fp_out, fp_template)
  if fp_template then
    fp_out:write(fp_template:read("*all"))
  else
    fp_out:write("</BODY>\n</HTML>\n")
  end
end

local searchPattern = rex.new( [[
  \<a\s.*?\</a\>  |
  &quot;\w+&quot; |
  "\w+"           |
  (\w+(?:\.\w+)*)
]], "ix")

local function postprocess_article (part1, part2, is_markdown)
  if part2 then
    local links, links2 = {}, {}
    for line in part2:gmatch("[^\n]+") do
      local name, url
      if not is_markdown then
        name, url = line:match("^%s*%[([^%]]+)%]%s*:%s*(%S+)")
      end
      if name then
        links[name] = url
      else
        name, url = line:match("^%s*{([^}]+)}%s*:%s*(%S+)")
        if name then links2[name] = url end
      end
    end

    if not is_markdown then
      part1 = part1:gsub("`([^`\n]+)`",
        function(c)
          return links[c] and ('<a href="%s">%s</a>'):format(links[c], c)
        end)
    end

    part1 = rex.gsub(part1, searchPattern,
      function(c)
        if c then
          local url = links2[c:lower()]
          if url then return '<a href="'..url..'">'..c..'</a>' end
        end
      end)
  end
  return is_markdown and part1 or "\n<pre>"..part1.."</pre>\n"
end

-- For long subjects, this function is _much_ faster than subj:match("(.-)delim(.*)")
local function split1 (subj, delim)
  local from,to = subj:find(delim)
  if from then return subj:sub(1,from-1), subj:sub(to+1)
  else return subj, nil
  end
end

-- By default, this program assumes that each article of the input Treepad
-- file is in plain text format.
-- But if a user wants to keep some articles already in HTML format, he/she
-- must insert a line "<!--HTML-->\n" (case sensitive, without quotes), at the
-- very beginning of each such article, to let this program know that these
-- articles should go to the output "as they are" (with no added header, no
-- added footer and no conversion).
local function process_article (article)
  local line, start = article:match "^([^\n]*)\n()"
  if line == "<!--HTML-->" then
    return article, true
  end
  local is_markdown = (line == "<markdown>")
  local part1, part2 = split1(article, "@@@")
  if is_markdown then
    part1 = part1:sub(start)
    part1 = part1:gsub("```(.-)```",
      function(c)
        return lxsh.highlighters.lua(c,
          { formatter=lxsh.formatters.html,
            external=false,
            colors=lxsh.colors.earendel })
      end
    )
    part1 = discount(part1)
  else
    part1 = HTML_convert(part1 or article)
    part1 = rex.gsub(part1, [[ \*\*(.*?)\*\* | \*(.*?)\* | `(\*+)` ]],
      function(c1,c2,c3)
        return c1 and "<strong>" ..c1.. "</strong>" or -- make bold
               c2 and "<em>"     ..c2.. "</em>" or     -- make italic
               c3                                      -- literal asterisks
      end, nil, "x")
  end
  return postprocess_article(part1, part2, is_markdown), false
end

-- generate: files, project, TOC and FileIndex
local function generateFPT (NodeIterator, ProjectName, fp_template, codepage, language)
  -- create a FileIndex, a project file and a TOC file
  local fIndex = io.open(ProjectName..".htm", "wt")
  fIndex:write("<HTML><HEAD><TITLE>\nFile Locator\n</TITLE></HEAD>\n<BODY>\n")

  local fProj = io.open(ProjectName..".hhp", "wt")
  local fToc  = io.open(ProjectName..".hhc", "wt")
  writeTocHeader(fToc)

  local ProjectHeaderReady
  local nodeLevel = -1
  local ProjectFiles = {}
  for node, nodenumber in NodeIterator do
    local filename = ("%d.html"):format(node.id)
    local fCurrent = assert(io.open(filename, "wt"))
    local article, ready = process_article(node.article)
    if ready then
      fCurrent:write(article)
    else
      writeTopicHeader(fCurrent, node.name, fp_template, codepage)
      fCurrent:write(article)
      writeTopicFooter(fCurrent, fp_template)
    end
    fCurrent:close()

    if not ProjectHeaderReady then
      writeProjectHeader(fProj, ProjectName, node.name, filename, codepage, language)
      ProjectHeaderReady = true
    end

    -- include file name into the project file;
    -- do not write directly - put in a table for sorting (work around a conjectural compiler bug)
    table.insert(ProjectFiles, filename)

    -- get node level
    -- put node info into the TOC and the FileIndex
    local newLevel = tonumber(node.level)
    for i = nodeLevel, newLevel+1, -1 do -- end of subtree ?
      fputs_indent("</UL>\n", fToc, i)
      fputs_indent("</UL>\n", fIndex, i)
    end
    if newLevel > nodeLevel then  -- the first child
      fputs_indent("<UL>\n", fToc, newLevel)
      fputs_indent("<UL>\n", fIndex, newLevel)
    end

    fputs_indent("<LI><A HREF=\"", fIndex, newLevel)
    fIndex:write(filename)
    fIndex:write("\">")
    HTML_puts(node.name, fIndex)
    fIndex:write("</A>\n")

    fputs_indent("<LI> <OBJECT type=\"text/sitemap\">\n", fToc, newLevel)
    fputs_indent("<param name=\"Name\" value=\"", fToc, newLevel+1)
    local p = node.name:gsub('"', '&quot;')
    fToc:write(p)
    fToc:write("\">\n")
    fputs_indent("<param name=\"Local\" value=\"", fToc, newLevel+1)
    fToc:write(filename)
    fToc:write("\">\n")
    fputs_indent("</OBJECT>\n", fToc, newLevel+1)

    nodeLevel = newLevel
  end

  table.sort(ProjectFiles)
  for _,nm in ipairs(ProjectFiles) do fProj:write(nm, "\n") end
  fProj:close()

  for i = nodeLevel, 0, -1 do
    fputs_indent("</UL>\n", fToc, i)
    fputs_indent("</UL>\n", fIndex, i)
  end
  fToc:write("</BODY></HTML>\n")
  fIndex:write("</BODY></HTML>\n")
  fToc:close()
  fIndex:close()
end

local function syntax()
  io.stderr:write[[
TP2HH: Treepad to HTML Help Conversion Utility
  Syntax: TP2HH <input file> <library> <language> [<template file>] [<codepage>]
]]
end

do
  local datafile, lib, language, tem, codepage = ...
  if not language then
    syntax(); return
  end
  assert(lib=="hjt" or lib=="tsi", "library not supported")
  lib = require(lib)
  local fp_template = tem and assert(io.open(tem))
  local project_name = datafile:match("[^/\\]+$"):gsub("%.[^.]+$", "")
  generateFPT(lib.Nodes(datafile), project_name, fp_template, codepage, language)
  if fp_template then fp_template:close() end
end

