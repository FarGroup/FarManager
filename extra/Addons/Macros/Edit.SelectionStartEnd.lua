local F = far.Flags
local Selection = {}

Event {
  group="EditorEvent";
  action=function(id,Event)
    if Event==F.EE_CLOSE then
      Selection[id] = nil
    end
  end
}

local function changeBlock(x,id,CurLine,CurPos)
  local sel = editor.GetSelection(id) or Selection[id] or {}
  sel[x.."Line"] = CurLine
  sel[x.."Pos"] = CurPos
  Selection[id] = sel
  if not sel.EndLine or not sel.StartLine then
    return
  end
  local startpos, endpos
  if sel.BlockType==F.BTYPE_COLUMN then --https://bugs.farmanager.com/view.php?id=2759
    startpos = editor.RealToTab(id, sel.StartLine, sel.StartPos)
    endpos = editor.RealToTab(id, sel.EndLine, sel.EndPos)
  else
    startpos = sel.StartPos
    endpos = sel.EndPos
  end
  local width = endpos-startpos+1
  local height = sel.EndLine-sel.StartLine+1
  editor.Select(id, sel.BlockType or F.BTYPE_STREAM, sel.StartLine, startpos, width, height)
end

local function selectStart()
  local info = editor.GetInfo()
  changeBlock("Start", info.EditorID, info.CurLine, info.CurPos)
end

local function selectEnd()
  local info = editor.GetInfo()
  changeBlock("End", info.EditorID, info.CurLine, info.CurPos-1)
end

Macro {
  description="Mark selection start";
  area="Editor";
  key="CtrlShift9";
  action=selectStart;
}

Macro {
  description="Mark selection end";
  area="Editor";
  key="CtrlShift0";
  action=selectEnd;
}

-- Following menu items are useful when recording macros
-- Otherwise do not include them into menu

MenuItem {
  description="[macro] Selection start &(";
  menu="Plugins";
  area="Editor";
  guid="AB0E7F16-C942-4780-AD33-E8493CF66196";
  text=function()
    return far.MacroGetState()~=F.MACROSTATE_NOMACRO and "&( Selection start"
  end;
  action=selectStart;
}

MenuItem {
  description="[macro] Selection end &)";
  menu="Plugins";
  area="Editor";
  guid="6CC1CDA4-A550-48EB-BC65-DD6EE177E2E2";
  text=function()
    return far.MacroGetState()~=F.MACROSTATE_NOMACRO and "&) Selection end"
  end;
  action=selectEnd;
}