local list = {
  "UtExecuteMacroTitle",

  -- macro browser help window
  "MBShowHelp",
  "MBOpenEditor",
  "MBOpenModalEditor",
  "MBSortByArea",
  "MBSortByKey",
  "MBSortByDescription",
  "MBExecuteMacro",
  "MBHideInactiveMacros",

  -- macro browser separators
  "MBSepMacros",
  "MBSepActiveMacros",
  "MBSepEvents",

  -- macro browser messages
  "MBNoFileNameAvail",
}

local GetMsg = far.GetMsg
for i=1,#list do list[list[i]] = i-1 end
return setmetatable({}, { __index=function(t,s) return GetMsg(list[s]) end })
