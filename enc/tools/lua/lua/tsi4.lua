-- Started: 2022-11-01
-- New TreeSpice file format:
--   - nodes and articles separated
--   - have its own IDs
--   - nodes reference articles

local datatypes = {Text=1; RTF=1; HTML=1; V_Text=1; V_RTF=1; V_HTML=1;}

local pattern_head = ("^%s*<TreeSpice version 4%.0>")                                  :gsub("%s+", "%%s+")
local pattern_node = ("^%s*node lv=(%d+) id=(%d+) art=(%d+) nm=(.*)")                  :gsub("%s+", "%%s+")
local pattern_art =  ("^%s*article id=(%d+) dt=(%S+) ctime=(%d+) mtime=(%d+) nm=(.*)") :gsub("%s+", "%%s+")

local function _ReadFile (fp)
  for line in fp:lines() do
    if line ~= "" then
      if string.match(line, pattern_head) then break
      else return nil, "header not found"
      end
    end
  end

  local tNodes, tArticles = {},{}
  local state = "nodes"
  local article

  for line in fp:lines() do
    if line ~= "" then
      if state == "nodes" then
        local lv, id, art, nm = string.match(line, pattern_node)
        if lv then
          local t = {}
          t.level, t.id, t.art, t.name = tonumber(lv), tonumber(id), tonumber(art), nm
          table.insert(tNodes, t)
        else
          if string.match(line, pattern_art) then
            state = "articles"
          else
            return nil, "article expected"
          end
        end
      end

      if state == "articles" then
        local id, dt, ctime, mtime, nm = string.match(line, pattern_art)
        if id then
          if article then
            article.content = table.concat(article.content, "\n")
            table.insert(tArticles, article)
          end
          local t = { content={}; }
          t.id, t.ctime, t.mtime, t.name = tonumber(id), tonumber(ctime), tonumber(mtime), nm
          t.datatype = datatypes[dt] and dt or "Text"
          article = t
        else
          local txt = string.match(line, "^#_(.*)")
          if txt then
            table.insert(article.content, txt)
          else
            return nil, "unexpected line: " .. line
          end
        end
      end
    end
  end
  if article then
    article.content = table.concat(article.content, "\n")
    table.insert(tArticles, article)
  end
  return tNodes, tArticles
end

local function _WriteFile (fp, nodes, articles)
  fp:write("<TreeSpice version 4.0>\n\n")

  for _,v in ipairs(nodes) do
    local s = ("node lv=%d id=%d art=%d nm=%s\n"):format(v.lv, v.id, v.art, v.nm)
    fp:write(s)
  end
  fp:write("\n")

  table.sort(articles, function(a,b) return a.id < b.id end)
  for _,v in ipairs(articles) do
    local s = ("article id=%d dt=%s ctime=%.0f mtime=%.0f nm=%s\n"):format(v.id, v.dt, v.ctime, v.mtime, v.nm)
    fp:write(s)
    for line,eol in v.content:gmatch("([^\n]*)(\n?)") do
      if line=="" and eol=="" then break end
      fp:write("#_", line, "\n")
    end
    if v.content:sub(-1) == "\n" then
      fp:write("#_\n")
    end
    fp:write("\n")
  end
end

local function ReadFile(fname)
  local fp, msg = io.open(fname)
  if not fp then return fp, msg end
  local nodes, articles = _ReadFile(fp)
  fp:close()
  return nodes, articles
end

local function WriteFile(fname, nodes, articles)
  local fp, msg = io.open(fname, "w")
  if not fp then return fp, msg end
  _WriteFile(fp, nodes, articles)
  fp:close()
  return true
end

return {
  ReadFile = ReadFile;
  WriteFile = WriteFile;
}
