local HighlightColor = 0x5f
local FindAllListSyncId = "8564FC78-88C0-4044-9EB2-2FD345B52936"

local F = far.Flags
local EditorFindAllListId = win.Uuid(far.Guids.EditorFindAllListId)

local EditorInfo = nil
local HighlightedText = nil

local function DelColor()
  if HighlightedText then
    editor.DelColor(EditorInfo.EditorID, HighlightedText.Line, HighlightedText.Pos, FindAllListSyncId)
    HighlightedText = nil
  end
end

local function PositionEditor()
  editor.SetPosition(EditorInfo.EditorID,
                     HighlightedText.Line,
                     HighlightedText.Position,
                     nil, -- CurTabPos
                     HighlightedText.Line - 1,
                     0) -- LeftPos
end

local function HighlighText()
  DelColor()
  HighlightedText = Menu.GetItemExtendedData()
  if HighlightedText then
    editor.AddColor(EditorInfo.EditorID,
                    HighlightedText.Line,
                    HighlightedText.Position,
                    HighlightedText.Position + HighlightedText.Length - 1,
                    F.ECF_NONE,
                    HighlightColor,
                    nil, -- Priority
                    FindAllListSyncId)
    PositionEditor()
  end
  editor.Redraw(EditorInfo.EditorID);
end

local function OnInitDialog()
  EditorInfo = editor.GetInfo(nil)
  mf.postmacro(HighlighText)
end

local function OnListChange()
  HighlighText()
end

local function OnCloseDialog(FarDialogEvent)
  DelColor()
  if FarDialogEvent.Param1 <= 0 then
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
    if     FarDialogEvent.Msg == F.DN_INITDIALOG then OnInitDialog()
    elseif FarDialogEvent.Msg == F.DN_LISTCHANGE then OnListChange()
    elseif FarDialogEvent.Msg == F.DN_CLOSE      then OnCloseDialog(FarDialogEvent)
    end
  end;
}
