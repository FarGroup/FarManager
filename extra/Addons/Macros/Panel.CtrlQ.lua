-- This macro is intended to open QView panel with maximum available width
-- narrowing opposit filepanel (ideally to the width of it's single column)

-- IMPORTANT!!:
-- panel mode should be set to have it's first column width as static value
-- e.g. width can be '15', but not '0' and not '40%'

-- by default the macro uses panel mode 6 (Descriptions) so you should
-- open Options/File panel modes/Descriptions and fix the value manually

-- alternatively you can setup separate mode, and change constant
-- below to it's number

local QViewMode  = 6  -- Default: 6 (Descriptions)
local CommonMode = 2  -- Default: 2 (Medium)

local MinWidth = 15   -- better if == panel's first column width

local function setQViewMode(P)
  if panel.GetPanelInfo(nil,P).ViewMode==CommonMode then
    panel.SetViewMode(nil,P,QViewMode)
  end
end
local function setCommonMode(P)
  if panel.GetPanelInfo(nil,P).ViewMode==QViewMode then
    panel.SetViewMode(nil,P,CommonMode)
  end
end

local QVIEW,APANEL,PPANEL = 2,1,0
Macro {
  description="Toggle 'maximized' quick view panel";
  area="Shell Search";
  key="CtrlQ";
  condition=function() --http://bugs.farmanager.com/view.php?id=2692
    return not APanel.Folder or PPanel.Type==QVIEW
  end;
  action=function()
    Keys("CtrlQ")
    if PPanel.FilePanel then
      setCommonMode(APANEL)
      Keys("CtrlClear")
    else
      setQViewMode(APANEL)
      local key = PPanel.Left and "CtrlRight" or "CtrlLeft"
      for i=1,APanel.Width-MinWidth-2 do Keys(key) end
    end
  end;
}

Macro {
  description="Toggle 'maximized' quick view panel";
  area="QView";
  key="CtrlQ";
  action=function()
    Keys("CtrlQ")
    if PPanel.FilePanel then
      setCommonMode(PPANEL)
      Keys("CtrlClear")
    end
  end;
}
