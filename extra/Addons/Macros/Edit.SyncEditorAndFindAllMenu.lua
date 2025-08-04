--------------------------------------------------------------------------------
-- Editor: Synchronizes the Editor with Find All menu.
-- When Find All menu is active, highlights in the Editor
-- the pattern pointed by the current menu item.
--
-- This macro is based on https://forum.farmanager.com/viewtopic.php?t=11977
--
-- Customization points:
--
-- The color used to highlight the pattern.
local HighlightColor = 0x5f
-- If true, after menu is opened, selects the menu item corresponding
-- to the first search pattern found below the cursor position.
local PositionMenuOnStart = true
-- If true, highlights the search patterns in the text while the user moves around the menu items.
-- If false, highlights the search pattern on Ctrl+Enter.
local TrackMenu = true
-- If true, leaves the last highlighted pattern after the menu was closed.
local KeepHighlightOnExit = false
--------------------------------------------------------------------------------

local FindAllListSyncId = "8564FC78-88C0-4044-9EB2-2FD345B52936"

local F = far.Flags
local EditorFindAllListId = win.Uuid(far.Guids.EditorFindAllListId)

local EditorInfo = nil
local LastSeenItemData = nil
local LastAddedColorCoordinates = nil

-- Returns smallest I in [1, N] with GetData(I) >= target, or N if all keys are < target.
local function LowerBound(N, Target, GetData, Less)
  Less = Less or function(u, v) return u < v end
  if N <= 0 then return 0 end

  local Lo, Hi = 1, N + 1 -- half-open [Lo, Hi)
  while Lo < Hi do
    local Mid = math.floor((Lo + Hi) / 2)
    if Less(GetData(Mid), Target) then
      Lo = Mid + 1
    else
      Hi = Mid
    end
  end

  if Lo == N + 1 then
    return 1 -- all keys < Target -- wrap around
  else
    return Lo
  end
end

local function CoordinatesFromMenuItem(I)
  return Menu.GetItemExtendedData(I)
end

local function CoordinatesLess(A, B)
  if A.Line ~= B.Line then return A.Line < B.Line end
  if A.Position ~= B.Position then return A.Position < B.Position end
  return A.Length < B.Length
end

local function AddColor()
  if LastSeenItemData then
    editor.AddColor(EditorInfo.EditorID,
                    LastSeenItemData.Line,
                    LastSeenItemData.Position,
                    LastSeenItemData.Position + LastSeenItemData.Length - 1,
                    F.ECF_NONE,
                    HighlightColor,
                    nil, -- Priority
                    FindAllListSyncId)
  LastAddedColorCoordinates = { Line = LastSeenItemData.Line, Position = LastSeenItemData.Position }
  end
end

local function DelColor()
  if LastAddedColorCoordinates then
    editor.DelColor(EditorInfo.EditorID, LastAddedColorCoordinates.Line, LastAddedColorCoordinates.Position, FindAllListSyncId)
    LastAddedColorCoordinates = nil
  end
end

local function PositionEditor()
  -- Consider ensuring that the position is visible by moving the menu if necessary
  if EditorInfo and LastSeenItemData then
    editor.SetPosition(EditorInfo.EditorID,
                       LastSeenItemData.Line,
                       LastSeenItemData.Position + LastSeenItemData.Length - 1,
                       nil, -- CurTabPos
                       LastSeenItemData.Line - 2,
                       1) -- LeftPos
    editor.SetPosition(EditorInfo.EditorID,
                       LastSeenItemData.Line,
                       LastSeenItemData.Position,
                       nil, -- CurTabPos
                       LastSeenItemData.Line - 2,
                       nil) -- LeftPos
  end
end

local function HighlighText()
  DelColor()
  AddColor()
  PositionEditor()
  editor.Redraw(EditorInfo.EditorID);
end

local function SetupMenuAndEditor(FarDialogEvent)
  local SelectPos = 1
  if PositionMenuOnStart then
    SelectPos = LowerBound(Object.ItemCount,
                           { Line = EditorInfo.CurLine, Position = EditorInfo.CurPos, Length = 0 },
                           CoordinatesFromMenuItem,
                           CoordinatesLess)
    FarDialogEvent.hDlg:send(F.DM_LISTSETCURPOS, 1, { SelectPos = SelectPos })
  end
  LastSeenItemData = Menu.GetItemExtendedData(SelectPos)
  if TrackMenu then HighlighText() end
end

local function OnInitDialog(FarDialogEvent)
  EditorInfo = editor.GetInfo(nil)
  mf.postmacro(function() SetupMenuAndEditor(FarDialogEvent) end)
end

local function OnListChange()
  LastSeenItemData = Menu.GetItemExtendedData()
  if TrackMenu then HighlighText() end
end

local function OnCtrlEnter()
  if not TrackMenu then HighlighText() end
end

local function OnCloseDialog(FarDialogEvent)
  DelColor()
  if FarDialogEvent.Param1 > 0 then
    if KeepHighlightOnExit then
      LastSeenItemData = Menu.GetItemExtendedData()
      AddColor()
    end
  else
    editor.SetPosition(EditorInfo.EditorID, EditorInfo)
    editor.Redraw(EditorInfo.EditorID);
  end
end

Event {
  description="Sychronize editor with the Find All menu";
  group="DialogEvent";
  condition = function(Event, FarDialogEvent)
    if Event ~= F.DE_DLGPROCEND then return false end
    local DialogInfo = FarDialogEvent.hDlg:send(F.DM_GETDIALOGINFO)
    return DialogInfo and DialogInfo.Id == EditorFindAllListId
  end;
  action = function(Event, FarDialogEvent)
    if     FarDialogEvent.Msg == F.DN_INITDIALOG                        then OnInitDialog(FarDialogEvent)
    elseif FarDialogEvent.Msg == F.DN_LISTCHANGE                        then OnListChange()
    elseif FarDialogEvent.Msg == F.DN_CONTROLINPUT
       and FarDialogEvent.Param2.EventType == F.KEY_EVENT
       and far.InputRecordToName(FarDialogEvent.Param2) == "CtrlEnter"  then OnCtrlEnter()
    elseif FarDialogEvent.Msg == F.DN_CLOSE                             then OnCloseDialog(FarDialogEvent)
    end
  end;
}
