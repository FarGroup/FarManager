local LOCKED = 0x200
Macro {
  description="Use internal editor as viewer";
  area="Shell Search";
  key="F3";
  flags="NoSendKeysToPlugins";
  condition=function()
    return APanel.Visible
       and not APanel.Empty
       and not APanel.Folder
       and (not APanel.Plugin or band(APanel.OPIFlags,far.Flags.OPIF_REALNAMES)~=0)
  end;
  action=function()
    Keys("F4")
    if Dlg.Id==far.Guids.EditorOpenRSHId then
      Keys("Enter")
    end
    if Area.Editor and band(Editor.State,LOCKED)==0 then
      Keys("CtrlL")
    end
  end;
}
