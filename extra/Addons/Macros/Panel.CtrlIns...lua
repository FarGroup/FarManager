-- if Ctrl+Ins pressed on '..' and command line is empty and no selected items
-- then copy full path of the current folder to clipboard

local function not_empty(str)
  return str~="" and str
end

Macro {
  description="Copy current path when pressed on '..'";
  area="Shell";
  key="CtrlIns";
  condition=function()
    return APanel.Visible and APanel.Folder and APanel.Current==".."
       and CmdLine.Empty and not APanel.Selected
  end;
  action=function()
    far.CopyToClipboard(not_empty(APanel.UNCPath) or not_empty(APanel.Format) or APanel.Path0)
  end
}
