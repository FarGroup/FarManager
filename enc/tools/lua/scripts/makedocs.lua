local dir, name = treespice.GetFileName() : match("^(.-\\)([^\\]+)$")

local suffix =
  "NAME=".. name:gsub("%.[^.]+$", "") ..
  " ARTICLE_ID=" .. treespice.GetTreeView():GetSelectedNode():GetId()

os.execute ("cd " .. dir .. " && start make all display " .. suffix)

