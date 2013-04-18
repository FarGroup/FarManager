local guid="0E92FC81-4888-4297-A85D-31C79E0E0CEE"
local function EditCaseExist () return Plugin.Exist(guid) end
local function EditCase (Command) return Plugin.Call(guid,Command) end

Macro {
  area="Editor"; key="CtrlShiftDown"; description="Change the case text  - Lower";
  condition=EditCaseExist;
  action=function() EditCase("Lower") end;
}

Macro {
  area="Editor"; key="CtrlShiftUp"; description="Change the case text  - Upper";
  condition=EditCaseExist;
  action=function() EditCase("Upper") end;
}

--[[
Macro {
  area="Editor"; key="???"; description="Change the case text - Title";
  condition=EditCaseExist;
  action=function() EditCase("Title") end;
}

Macro {
  area="Editor"; key="???"; description="Change the case text  - Toggle";
  condition=EditCaseExist;
  action=function() EditCase("Toggle") end;
}

Macro {
  area="Editor"; key="???"; description="Change the case text  - Cyclic";
  condition=EditCaseExist;
  action=function() EditCase("Cyclic") end;
}
--]]
