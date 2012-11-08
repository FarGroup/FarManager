-- Started: 2005-Sep-09
-- This module offers functions for writing and reading TreeSpice files.
-- Currently, it "exports" the following functions:
--    * NewWriter() -- create a new treespice object
--       * self:WriteHeader() -- method: write the header
--       * self:WriteNode()   -- method: write a node
--       * self.write()       -- may be reassigned to the user-defined function
--    * ReadHeader() -- read the file header
--    * Nodes()      -- iterate nodes in a "generic for" loop

local string_gsub, string_sub, string_match =
      string.gsub, string.sub, string.match
local io_open, io_write = io.open, io.write
local table_insert, table_concat = table.insert, table.concat

-- datatypes: user to tsi conversion
local dt_tsi = {
    text="Text", rtf="RTF", html="HTML",
    vtext="V_Text", vrtf="V_RTF", vhtml="V_HTML"
}

-- datatypes: tsi to user conversion
local dt_user = {
    Text="text", RTF="rtf", HTML="html",
    V_Text="vtext", V_RTF="vrtf", V_HTML="vhtml"
}

-- write node start;
-- parameters `datatype' and `id' are optional;
local function  wr_node_start (self, node)
    local s = "<node>" ..
              (node.id and ("\nid="..node.id) or "") ..
              "\nlv=" .. node.level ..
              "\ndt=" .. dt_tsi[node.datatype or "text"] ..
              "\nnm=" .. string_gsub(node.name, "\n", " ") ..
              (node.ctime and ("\nctime="..node.ctime) or "") ..
              (node.mtime and ("\nmtime="..node.mtime) or "") ..
              "\n<article>\n"
    self.write(s)
end

-- write node end
local function wr_node_end(self)
    self.write("</article>\n</node>\n")
end

-- prefix lines in article
local function prefix_lines(text)
    if text == "" then return ""; end
    text = string_gsub(text.."\n", "([^\n]*\n)", "#_%1")
    return text
end

local function WriteHeader(self)
    self.write("<header>\ntag=SMZ1\nver=2.7\n</header>\n")
end

-- Write a node.
--   Parameter `node.datatype' must be one of the following strings:
--       "text", "rtf", "html", "vtext", "vrtf", "vhtml".
--   Parameters `nodeid', `article', `prefix' and `suffix' are optional.
local function WriteNode(self, node)
    wr_node_start(self, node)
    if node.prefix  then self.write(prefix_lines(node.prefix))  end
    if node.article then self.write(prefix_lines(node.article)) end
    if node.suffix  then self.write(prefix_lines(node.suffix))  end
    wr_node_end(self)
end

-- Create a new treespice object.
--   Parameter `write_func' is optional.
local function NewWriter (write_func)
    local t = {}
    t.write        = write_func or io_write
    t.WriteHeader  = WriteHeader
    t.WriteNode    = WriteNode
    return t
end

-- Read the file header
local function ReadHeader(filename)
    local f = io_open(filename)
    local hdr
    if f then
        local chunk = f:read(1024)
        if chunk then
            hdr = string_match(chunk, "^%s*(<header>.-</header>)")
        end
        f:close()
    end
    return hdr
end

--  Iterate nodes in a "generic for" loop.
local function Nodes (filename)
    -- Open the file
    local h = io_open(filename) -- upvalue
    if not h then error("Couldn't open file."); end

    local function abort()
        h:close()
        error("File with incorrect format or corrupted.")
    end

    -- Skip the file header
    local line -- doesn't need to be upvalue;
    repeat line = h:read("*l") until not line or line == "</header>"
    if not line then abort(); end

    local tNode = {} -- make it upvalue, for implementing cleanbreak feature;
    local NodeNumber = 0

    return function()
        if tNode.cleanbreak then h:close(); return; end -- normal cleanbreak

        local line = h:read("*l")
        if not line then h:close(); return; end -- normal end of file
        if line ~= "<node>" then abort(); end

        tNode = { article = "" }
        while true do
            line = h:read("*l")
            if not line then abort(); end
            if line == "</node>" then -- normal end of node
              NodeNumber = NodeNumber + 1
              return tNode, NodeNumber
            end

            local var,val = string_match(line, "^%s*([^=%s]+)%s*%=(.*)")
            if     var == "lv" then tNode.level = val
            elseif var == "nm" then tNode.name = val
            elseif var == "id" then tNode.id = val
            elseif var == "dt" then tNode.datatype = dt_user[val]
            elseif var == "ctime" then tNode.ctime = val
            elseif var == "mtime" then tNode.mtime = val
            elseif line == "<article>" then
                local tArticle = {}
                while true do
                    line = h:read("*l")
                    if not line then abort(); end
                    if line == "</article>" then break; end -- normal end of article
                    if string_sub(line,1,2) == "#_" then
                        table_insert(tArticle, string_sub(line,3)) -- delete "#_"
                    else abort()
                    end
                end
                tNode.article = table_concat(tArticle, "\n")
                if tNode.article ~= "" then
                    tNode.article = tNode.article .. "\n"
                end
            else
                abort()
            end
        end
    end
end

return {
    NewWriter = NewWriter,
    ReadHeader = ReadHeader,
    Nodes = Nodes,
}
