local guid="B66C436D-E657-419C-86D3-6834B9ACF3D2"
local function BracketsExist () return Plugin.Exist(guid) end
local function Brackets (Command) return Plugin.Call(guid,Command) end

Macro {
  area="Editor"; key="Ctrl["; description="Find matching bracket - Forward";
  condition=BracketsExist;
  action=function() Brackets("SearchFwd") end;
}

Macro {
  area="Editor"; key="Ctrl]"; description="Find matching bracket - Backward";
  condition=BracketsExist;
  action=function() Brackets("SearchBack") end;
}

Macro {
  area="Editor"; key="CtrlShift["; description="Find & Select matching bracket - Forward";
  condition=BracketsExist;
  action=function() Brackets("SelectFwd") end;
}

Macro {
  area="Editor"; key="CtrlShift]"; description="Find & Select matching bracket - Backward";
  condition=BracketsExist;
  action=function() Brackets("SelectBack") end;
}

--[[
Macro {
  area="Editor"; key="CtrlShiftM"; description="Brackets matching - Config";
  condition=BracketsExist;
  action=function() Brackets("Config") end;
}
--]]
