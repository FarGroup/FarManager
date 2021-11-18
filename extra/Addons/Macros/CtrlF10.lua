if not panel.CheckPanelsExist() then return end -- far.exe -v -e

local F6_ALLOWED, MODAL = 0x00000002, 0x00000800
local function CtrlF10_enabled()
  local State = Area.Viewer
            and band(Viewer.State, MODAL)
             -- CtrlF10 is forbidden in modal viewer
             or band(Editor.State, bor(F6_ALLOWED,MODAL))
             -- CtrlF10 is forbidden in modal editor,
             -- unless it opened with explicit EF_ENABLE_F6 flag
  return State~=MODAL
end

Macro {
  description="Quit from editor/viewer and position to the current file";
  area="Editor Viewer";
  key="CtrlF10";
  flags="NoSendKeysToPlugins";
  condition=CtrlF10_enabled;
  action=function()
    Keys("CtrlF10 Esc F12")
    if Menu.Id==far.Guids.ScreensSwitchId then Keys("1") end
  end;
}
