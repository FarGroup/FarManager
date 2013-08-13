local list = {
  "UtExecuteMacroTitle",
  "UtExecuteMacroBottom",

  -- macro browser help window
  "MBHelpLine1",
  "MBHelpLine2",
  "MBHelpLine3",
  "MBHelpLine4",
  "MBHelpLine5",
  "MBHelpLine6",
  "MBHelpLine7",
  "MBHelpLine8",
  "MBHelpLine9",

  -- macro browser separators
  "MBSepMacros",
  "MBSepActiveMacros",
  "MBSepEvents",

  -- macro browser messages
  "MBNoFileNameAvail",
  "MBFileNotFound",

  -- custom panel sort
  "PSDefaultMenuItemText",
  "PSMenuTitle",
}

local GetMsg = far.GetMsg
for i=1,#list do list[list[i]] = i-1 end
return setmetatable({}, { __index=function(t,s) return GetMsg(list[s]) end })
