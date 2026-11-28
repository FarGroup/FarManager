-- Started:                 2020-08-15
-- Started for far2m:       2022-01-23
-- Author:                  Shmuel Zeigerman
-- Far3 minimal version:    3.0.3300
-- Plugin:                  any LuaFAR plugin

local F         = far.Flags
local DirSep    = package.config:sub(1,1)
local OsWin     = DirSep == "\\"
local OsLin     = not OsWin
local FarVer    = far.AdvControl(F.ACTL_GETFARMANAGERVERSION, true)
local band, bor = bit64.band, bit64.bor
local Send      = far.SendDlgMessage
local Colors    = far.Colors

local IND_TYPE, IND_X1, IND_Y1, IND_X2, IND_Y2, IND_HISTORY, IND_DATA = 1,2,3,4,5,7,10
local IND_SELECTED = 6
local IND_MASK     = 8
local IND_LIST     = 6
local IND_VBUF     = 6
local IND_FLAGS    = 9

local mod = {} -- this module
local mod_meta = { __index=mod; }

function mod.New(Items)
  assert(type(Items) == "table", "param #1 must be a table")
  return setmetatable({Items=Items}, mod_meta)
end

--- Edit some text (e.g. a DI_EDIT dialog field) in Far editor
-- @param text     : input text
-- @param ext      : extension of temporary file (affects syntax highlighting; optional)
-- @return         : output text (or nil)
function mod.OpenInEditor(text, ext)
  local tempdir
  if OsWin then
    tempdir = win.GetEnv("TEMP")
    if not tempdir then
      far.Message("Environment variable TEMP is not set", "Error", nil, "w")
      return nil
    end
  else
    tempdir = far.InMyTemp() -- luacheck:ignore
  end
  if tempdir:sub(-1) ~= DirSep then tempdir = tempdir..DirSep end

  ext = type(ext)=="string" and ext or ".tmp"
  if ext~="" and ext:sub(1,1)~="." then ext = "."..ext; end

  local fname = ("%sFar-%s%s"):format(tempdir, win.Uuid(win.Uuid()):sub(1,8), ext)
  local fp = io.open(fname, "w")
  if fp then
    fp:write(text or "")
    fp:close()
    text = nil
    local flags = bor(F.EF_DISABLEHISTORY, (FarVer==2) and 0 or F.EF_DISABLESAVEPOS)
    if editor.Editor(fname,nil,nil,nil,nil,nil,flags,nil,nil,65001) == F.EEC_MODIFIED then
      fp = io.open(fname, "r")
      if fp then
        text = fp:read("*all")
        fp:close()
      end
    end
    win.DeleteFile(fname)
    return text
  end
end

-- @param txt     : string
-- @param h_char  : string; optional; defaults to "#"
-- @param h_color : number; optional; defaults to 0xF0 (black on white)
-- @return 1      : userdata: created usercontrol
-- @return 2      : number: usercontrol width
-- @return 3      : number: usercontrol height
local function usercontrol2 (txt, h_char, h_color)
  local COLOR_NORMAL = far.AdvControl("ACTL_GETCOLOR", Colors.COL_DIALOGTEXT)
  local CELL_BLANK = { Char=" "; Attributes=COLOR_NORMAL }
  h_char = h_char or "#"
  h_color = h_color or 0xF0

  local W, H, list = 1, 0, {}
  for line,text in txt:gmatch( "(([^\n]*)\n?)" ) do
    if line ~= "" then
      table.insert(list, text)
      text = text:gsub(h_char, "")
      W = math.max(W, text:len())
      H = H+1
    end
  end

  local buffer = far.CreateUserControl(W, H)
  for y=1,H do
    local line = list[y]
    local len = line:len()
    local ind, attr = 0, COLOR_NORMAL
    for x=1,len do
      local char = line:sub(x,x)
      if char == h_char then
        attr = (attr == COLOR_NORMAL) and h_color or COLOR_NORMAL
      else
        ind = ind + 1
        buffer[(y-1)*W+ind] = {Char=char; Attributes=attr};
      end
    end
    for x=ind+1,W do buffer[(y-1)*W+x] = CELL_BLANK; end
  end
  return buffer, W, H
end

local function calc_x2 (tp, x1, text)
  if tp==F.DI_CHECKBOX or tp==F.DI_RADIOBUTTON then
    return x1 + 3 + text:gsub("&",""):len()
  elseif tp==F.DI_TEXT then
    return x1 - 1 + text:gsub("&",""):len() + 1 -- +1: work around a Far's bug related to ampersands
  else
    return x1
  end
end

function mod:GetDialogState(hDlg)
  local out = {}
  for pos,elem in ipairs(self.Items) do
    if not (elem.noauto or elem.nosave) then
      local tp = type(elem.name)
      if tp=="string" or tp=="number" then
        local item = Send(hDlg, "DM_GETDLGITEM", pos)
        tp = item[IND_TYPE]

        if tp==F.DI_CHECKBOX then
          local val = item[IND_SELECTED]
          out[elem.name] = (val==2) and 2 or (val ~= 0) -- false,true,2

        elseif tp==F.DI_RADIOBUTTON then
          local val = item[IND_SELECTED]
          out[elem.name] = (val ~= 0) -- false,true

        elseif tp==F.DI_EDIT or tp==F.DI_FIXEDIT or tp==F.DI_PSWEDIT or (OsLin and tp==F.DI_MEMOEDIT) then
          out[elem.name] = item[IND_DATA] -- string

        elseif tp==F.DI_COMBOBOX or tp==F.DI_LISTBOX then
          local tt = Send(hDlg, "DM_LISTGETCURPOS", pos)
          out[elem.name] = tt.SelectPos

        end
      end
    end
  end
  return out
end

function mod:SetDialogState(hDlg, Data)
  Send(hDlg, "DM_ENABLEREDRAW", 0)
  for pos,elem in ipairs(self.Items) do
    if not (elem.noauto or elem.noload) then
      if type(elem.name)=="string" or type(elem.name)=="number" then
        local val = Data[elem.name]
        local tp = elem.tp

        if tp=="chbox" then
          val = (val==2 or val==0) and val or (val and 1) or 0
          Send(hDlg, "DM_SETCHECK", pos, val)

        elseif tp=="rbutt" then
          if val then
            Send(hDlg, "DM_SETCHECK", pos, 1)
          end

        elseif tp=="edit" or tp=="fixedit" or tp=="pswedit" or (OsLin and tp=="memo") then
          Send(hDlg, "DM_SETTEXT", pos, val or "")

        elseif tp=="combobox" or tp=="listbox" then
          Send(hDlg, "DM_LISTSETCURPOS", pos, {SelectPos=val or 1})

        end
      end
    end
  end
  Send(hDlg, "DM_ENABLEREDRAW", 1)
end

function mod:ClearControls(hDlg)
  Send(hDlg, "DM_ENABLEREDRAW", 0)
  for pos,elem in ipairs(self.Items) do
    if not (elem.noauto or elem.noload) then
      local tp = elem.tp

      if tp=="chbox" then
        Send(hDlg, "DM_SETCHECK", pos, 0)

      elseif tp=="rbutt" then
        Send(hDlg, "DM_SETCHECK", pos, elem.group and 1 or 0)

      elseif tp=="edit" or tp=="fixedit" or tp=="pswedit" then
        Send(hDlg, "DM_SETTEXT", pos, "")

      elseif tp=="combobox" or tp=="listbox" then
        Send(hDlg, "DM_LISTSETCURPOS", pos, {SelectPos=1})

      end
    end
  end
  Send(hDlg, "DM_ENABLEREDRAW", 1)
end

-- supported dialog item types
local TypeMap = {
    butt           =  F.DI_BUTTON;
    chbox          =  F.DI_CHECKBOX;
    combobox       =  F.DI_COMBOBOX;
    dbox           =  F.DI_DOUBLEBOX;
    edit           =  F.DI_EDIT;
    fixedit        =  F.DI_FIXEDIT;
    listbox        =  F.DI_LISTBOX;
    memo           =  OsLin and F.DI_MEMOEDIT or nil; -- luacheck:ignore
    pswedit        =  F.DI_PSWEDIT;
    rbutt          =  F.DI_RADIOBUTTON;
    sbox           =  F.DI_SINGLEBOX;
    sep            =  "sep";
    sep2           =  "sep2";
    text           =  F.DI_TEXT;
    user           =  F.DI_USERCONTROL;
    user2          =  "usercontrol2";
    vtext          =  F.DI_VTEXT;
}

-- supported dialog item flags
local FlagsMap = {
    boxcolor               = F.DIF_BOXCOLOR;
    btnnoclose             = F.DIF_BTNNOCLOSE;
    centergroup            = F.DIF_CENTERGROUP;
    centertext             = F.DIF_CENTERTEXT;
    default                = F.DIF_DEFAULTBUTTON;
    disable                = F.DIF_DISABLE;
    dropdown               = F.DIF_DROPDOWNLIST;
    editexpand             = F.DIF_EDITEXPAND;
    editor                 = F.DIF_EDITOR;
    editpath               = F.DIF_EDITPATH;
    focus                  = F.DIF_FOCUS;
    group                  = F.DIF_GROUP;
    hidden                 = F.DIF_HIDDEN;
    lefttext               = F.DIF_LEFTTEXT;
    listautohighlight      = F.DIF_LISTAUTOHIGHLIGHT;
    listnoampersand        = F.DIF_LISTNOAMPERSAND;
    listnobox              = F.DIF_LISTNOBOX;
    listnoclose            = F.DIF_LISTNOCLOSE;
    listwrapmode           = F.DIF_LISTWRAPMODE;
    manualaddhistory       = F.DIF_MANUALADDHISTORY;
    moveselect             = F.DIF_MOVESELECT;
    noautocomplete         = F.DIF_NOAUTOCOMPLETE;
    nobrackets             = F.DIF_NOBRACKETS;
    nofocus                = F.DIF_NOFOCUS;
    readonly               = F.DIF_READONLY;
    selectonentry          = F.DIF_SELECTONENTRY;
    setshield              = F.DIF_SETSHIELD;
    showampersand          = F.DIF_SHOWAMPERSAND;
    tristate               = F.DIF_3STATE;                -- !!!
    uselasthistory         = F.DIF_USELASTHISTORY;
}
if (FarVer == 2) then
    FlagsMap.colormask             = F.DIF_COLORMASK              -- luacheck:ignore
    FlagsMap.setcolor              = F.DIF_SETCOLOR               -- luacheck:ignore
else
    FlagsMap.editpathexec          = F.DIF_EDITPATHEXEC           -- luacheck:ignore
    FlagsMap.listtrackmouse        = F.DIF_LISTTRACKMOUSE         -- luacheck:ignore
    FlagsMap.listtrackmouseinfocus = F.DIF_LISTTRACKMOUSEINFOCUS  -- luacheck:ignore
    FlagsMap.righttext             = F.DIF_RIGHTTEXT              -- luacheck:ignore
    FlagsMap.wordwrap              = F.DIF_WORDWRAP               -- luacheck:ignore
end

---- Replacement for far.Dialog() with much cleaner syntax of dialog description.
-- @param inData table : contains an array part ("items") and a dictionary part ("properties")

--    Supported properties for entire dialog (all are optional):
--        guid          : string   : a text-form guid
--        width         : number   : dialog width
--        x1            : number   : x1 coordinate (0-based)
--        y1            : number   : y1 coordinate (0-based)
--        help          : str|func : string: help topic, function: called on F1 press
--        flags         : number   : dialog flags
--        proc          : function : dialog procedure
--        data          : any      : Param2 on DN_INITFIALOG; get/set via DM_GETDLGDATA/DM_SETDLGDATA

--    Supported properties for a dialog item (all are optional except tp):
--        tp            : string   : type; mandatory
--        text          : string   : text
--        name          : string   : used as a key in the output table
--        val           : number/boolean : value for DI_CHECKBOX, DI_RADIOBUTTON initialization
--        flags         : number   : flag or flags combination
--        hist          : string   : history name for DI_EDIT, DI_FIXEDIT
--        mask          : string   : mask value for DI_FIXEDIT, DI_TEXT, DI_VTEXT
--        x1            : number   : left position
--        x2            : number   : right position
--        y1            : number   : top position
--        y2            : number   : bottom position
--        width         : number   : width
--        height        : number   : height
--        ystep         : number   : vertical offset relative to the previous item; may be <= 0; default=1
--        list          : table    : mandatory for DI_COMBOBOX, DI_LISTBOX
--        buffer        : userdata : buffer for DI_USERCONTROL

-- @return1 out  table : contains final values of dialog items indexed by 'name' field of 'inData' items
-- @return2 pos number : return value of API far.Dialog()
----------------------------------------------------------------------------------------------------
function mod:Compile()
  local inData = self.Items
  local inFlags = inData.flags or 0
  assert(type(inFlags)=="number", "'Data.flags' must be a number")
  local HMARGIN = (0 ~= band(inFlags,F.FDLG_SMALLDIALOG)) and 0
    or (#inData==1 and inData[1].tp=="listbox") and 1 or 3           -- horisontal margin
  local VMARGIN = (0 == band(inFlags,F.FDLG_SMALLDIALOG)) and 1 or 0 -- vertical margin
  local W = inData.width or 76
  local Y, H = VMARGIN-1, 0
  local outData = {}
  local cgroup = { y=nil; width=0; } -- centergroup
  local x2_defer = {}
  local EMPTY = {}

  for i,v in ipairs(inData) do
    assert(type(v)=="table", "dialog element #"..i.." is not a table")
    local tp = v.tp and TypeMap[v.tp]
    if not tp then error("Unsupported dialog item type: "..tostring(v.tp)); end

    local flags = v.flags or 0
    assert(type(flags)=="number", "type of 'flags' is not a number")
    for k,w in pairs(v) do
      local f = w and FlagsMap[k]
      if f then flags = bor(flags,f); end
    end

    local text = v.text or (type(v.val)=="string" and v.val) or ""

    local prev = (i > 1) and outData[i-1] or EMPTY
    local is_cgroup = (tp==F.DI_BUTTON) and band(flags,F.DIF_CENTERGROUP)~=0
    local x1 = tonumber(v.x1)                  or
               v.x1=="" and prev[IND_X1]       or
               HMARGIN+2
    local x2 = tonumber(v.x2)                  or
               v.x2=="" and prev[IND_X2]       or
               v.width  and x1+v.width-1       or
               x2_defer
    local y1 = tonumber(v.y1)                  or
               v.ystep  and Y + v.ystep        or
               v.y1=="" and prev[IND_Y1]       or
               cgroup.y and is_cgroup and Y    or
               Y + 1
    local y2 = tonumber(v.y2)                  or
               v.y2=="" and prev[IND_Y2]       or
               v.height and y1+v.height-1      or
               y1
    if is_cgroup then
      local textlen = text:gsub("&", ""):len()
      local left = (y1==cgroup.y) and cgroup.width+1 or 2
      cgroup.width = left + textlen + (band(flags,F.DIF_NOBRACKETS)~=0 and 0 or 4)
      cgroup.y = y1
    else
      cgroup.width, cgroup.y = 0, nil
    end

    local function MkItem (inp)
      inp = inp or {}
      local t = { tp, x1, y1, x2, y2, 0, 0, 0, 0, text }
      t [IND_FLAGS] = inp.flags or flags
      if inp.tp                  then t [IND_TYPE    ] = inp.tp;   end
      if inp.x1                  then t [IND_X1      ] = inp.x1;   end
      if inp.x2                  then t [IND_X2      ] = inp.x2;   end
      if inp.y1                  then t [IND_Y1      ] = inp.y1;   end
      if inp.y2                  then t [IND_Y2      ] = inp.y2;   end
      if inp.mask                then t [IND_MASK    ] = inp.mask; end
      if inp.list                then t [IND_LIST    ] = inp.list; end
      if inp.hist                then t [IND_HISTORY ] = inp.hist; end
      if inp.val                 then t [IND_SELECTED] = inp.val;  end
      if inp.vbuf                then t [IND_VBUF    ] = inp.vbuf; end
      if inp.text                then t [IND_DATA    ] = inp.text; end
      return t
    end

    if tp == F.DI_DOUBLEBOX or tp == F.DI_SINGLEBOX then
      if i == 1 then outData[i] = MkItem { x1=HMARGIN; y2=0; }
      else           outData[i] = MkItem()
      end

    elseif tp == F.DI_TEXT then
      outData[i] = MkItem()

    elseif OsLin and tp == F.DI_MEMOEDIT then -- luacheck:ignore
      outData[i] = MkItem()

    elseif tp == F.DI_VTEXT then
      if v.mask then flags = bor(flags, F.DIF_SEPARATORUSER); end -- set the flag automatically
      outData[i] = MkItem { flags=flags; mask=v.mask; }

    elseif tp=="sep" or tp=="sep2" then
      x1, x2 = v.x1 or -1, v.x2 or -1
      flags = bor(flags, tp=="sep2" and F.DIF_SEPARATOR2 or F.DIF_SEPARATOR)
      if v.mask then flags = bor(flags, F.DIF_SEPARATORUSER); end -- set the flag automatically
      outData[i] = MkItem { tp=F.DI_TEXT; y2=y1, mask=v.mask; }

    elseif tp == F.DI_EDIT then
      if v.hist then flags = bor(flags, F.DIF_HISTORY); end -- set the flag automatically
      outData[i] = MkItem { y2=0; flags=flags; hist=v.hist; text=v.val or v.text; }

    elseif tp == F.DI_FIXEDIT then
      if v.hist then flags = bor(flags, F.DIF_HISTORY);  end -- set the flag automatically
      if v.mask then flags = bor(flags, F.DIF_MASKEDIT); end -- set the flag automatically
      outData[i] = MkItem { y2=0; flags=flags; hist=v.hist; mask=v.mask; text=v.val or v.text; }

    elseif tp == F.DI_PSWEDIT then
      outData[i] = MkItem { y2=0; }

    elseif tp == F.DI_CHECKBOX then
      local val = (v.val==2 and 2) or (v.val and v.val~=0 and 1) or 0
      outData[i] = MkItem { x2=0; y2=y1; val=val; }

    elseif tp == F.DI_RADIOBUTTON then
      local val = v.val and v.val~=0 and 1 or 0
      outData[i] = MkItem { x2=0; y2=y1; val=val; }

    elseif tp == F.DI_BUTTON then
      outData[i] = MkItem { x2=0; y2=y1; }

    elseif tp == F.DI_COMBOBOX then
      assert(type(v.list)=="table", "\"list\" field must be a table")
      local dropdown = 0 ~= band(flags,F.DIF_DROPDOWNLIST)
      local index = dropdown and v.val and v.val>=1 and v.val<=#v.list and v.val
      v.list.SelectIndex = index or v.list.SelectIndex
      text = dropdown and "" or text
      outData[i] = MkItem { y2=y1; list=v.list; }

    elseif tp == F.DI_LISTBOX then
      assert(type(v.list)=="table", "\"list\" field must be a table")
      outData[i] = MkItem { list=v.list; }

    elseif tp == F.DI_USERCONTROL then
      outData[i] = MkItem { vbuf=v.buffer; }

    elseif tp == "usercontrol2" then
      assert(far.CreateUserControl, "Far 3.0.3590 or newer required to support usercontrol")
      assert(type(v.text)=="string" and v.text~="",    "invalid 'text' attribute in usercontrol2")
      assert(not v.hchar  or type(v.hchar)=="string",  "invalid 'hchar' attribute in usercontrol2")
      assert(not v.hcolor or type(v.hcolor)=="number", "invalid 'hcolor' attribute in usercontrol2")
      local buffer, wd, ht = usercontrol2(v.text, v.hchar, v.hcolor)
      x2 = x1 + wd - 1
      y2 = y1 + ht - 1
      outData[i] = MkItem { tp=F.DI_USERCONTROL; x2=x2; y2=y2; vbuf=buffer; }

    end

    if x2 == x2_defer then x2 = calc_x2(tp,x1,text); end
    W = math.max(W, x2+HMARGIN+3, cgroup.width+2*HMARGIN)
    Y = math.max(y1, y2)
    H = math.max(H, Y)

    if type(v.colors) == "table" then
      outData[i].colors = {}
      for j,w in ipairs(v.colors) do
        outData[i].colors[j] = type(w)=="string" and far.AdvControl(F.ACTL_GETCOLOR, Colors[w]) or w
      end
    end

  end

  -- second pass (with W already having its final value)
  for i,item in ipairs(outData) do
    if i == 1 then
      if item[IND_TYPE]==F.DI_DOUBLEBOX or item[IND_TYPE]==F.DI_SINGLEBOX then
        item[IND_X2] = W - HMARGIN - 1
        if inData[1].height then
          item[IND_Y2] = item[IND_Y1] + inData[1].height - 1
        else
          item[IND_Y2] = H + 1
        end
        H = item[IND_Y2] + 1 + VMARGIN
      else
        H = H + 1 + VMARGIN
      end
    end
    if item[IND_X2] == x2_defer then
      item[IND_X2] = W-HMARGIN-3
    end
  end

  local guid  = inData.guid and win.Uuid(inData.guid) or ("\0"):rep(16)
  local x1    = inData.x1 or -1
  local y1    = inData.y1 or -1
  local x2    = x1==-1 and W or x1+W-1
  local y2    = y1==-1 and H or y1+H-1
  local help  = type(inData.help)=="string" and inData.help or nil
  local items = outData
  local flags = inData.flags or 0
  local proc  = inData.proc or function() end
  local data  = inData.data

  return {
    guid, x1, y1, x2, y2, help, items, flags, function() end, data,
    guid  = guid;
    x1    = x1;
    y1    = y1;
    x2    = x2;
    y2    = y2;
    help  = help;
    items = items;
    flags = flags;
    proc  = proc;
    data  = data;
  }
end

function mod:Run()
  local oData = self:Compile()
  local items = oData.items
  local inData = self.Items
  local UserProc = oData.proc

  local function DlgProc(hDlg, Msg, Par1, Par2)
    if Msg == F.DN_CLOSE then
      Msg = (inData[Par1] and not inData[Par1].cancel) and Msg or "EVENT_CANCEL"
      return UserProc(hDlg, Msg, Par1, self:GetDialogState(hDlg))

    elseif FarVer == 2 and Msg==F.DN_KEY or
           FarVer == 3 and Msg==F.DN_CONTROLINPUT and Par2.EventType==F.KEY_EVENT and Par2.KeyDown then
      local keyname = FarVer==2 and far.KeyToName(Par2) or FarVer==3 and far.InputRecordToName(Par2)
      if UserProc(hDlg, "EVENT_KEY", Par1, keyname) then
        return true
      elseif keyname == "F1" then
        if type(inData.help) == "function" then
          inData.help()
        end
      elseif keyname == "F4" then
        if items[Par1][IND_TYPE] == F.DI_EDIT and not inData[Par1].skipF4 then
          local txt = Send(hDlg, "DM_GETTEXT", Par1)
          txt = mod.OpenInEditor(txt, inData[Par1].ext)
          if txt then Send(hDlg, "DM_SETTEXT", Par1, txt); end
        end
      end

    elseif (FarVer == 2) and (Msg == F.DN_MOUSECLICK or Msg == F.DN_MOUSEEVENT) or
           (FarVer == 3) and (Msg == F.DN_CONTROLINPUT and Par2.EventType == F.MOUSE_EVENT) then
      if Par1 <= 0 then Par1 = nil; end
      if UserProc(hDlg, "EVENT_MOUSE", Par1, Par2) then return true end

    elseif Msg == F.DN_CTLCOLORDLGITEM then
      return UserProc(hDlg, Msg, Par1, Par2) or items[Par1].colors

    else
      return UserProc(hDlg, Msg, Par1, Par2)

    end

  end
  ----------------------------------------------------------------------------------------------
  oData[9] = DlgProc
  local hDlg = far.DialogInit(unpack(oData))
  if hDlg then
    if F.FDLG_NONMODAL and 0 ~= band(oData.flags, F.FDLG_NONMODAL) then
      return hDlg -- non-modal dialogs were introduced in build 3.0.5047
    end
  else
    far.Message("Error occured in far.DialogInit()", "module 'simpledialog'", nil, "w")
    return nil
  end
  ----------------------------------------------------------------------------------------------
  local ret = far.DialogRun(hDlg)
  if ret < 1 or inData[ret].cancel then
    far.DialogFree(hDlg)
    return nil
  end
  local out = self:GetDialogState(hDlg)
  far.DialogFree(hDlg)
  return out, ret
end

function mod:Indexes()
  local Pos, Elem = {}, {}
  for i,v in ipairs(self.Items) do
    if type(v) ~= "table" then
      error("element #"..i.." is not a table")
    end
    if v.name then Pos[v.name], Elem[v.name] = i,v; end
  end
  return Pos, Elem
end

function mod:LoadData(Data)
  assert(type(Data)=="table", "arg #1 is not a table")
  for _,v in ipairs(self.Items) do
    if v.name and not (v.noauto or v.noload) and Data[v.name]~=nil then
      v.val = Data[v.name]
    end
  end
end

function mod:SaveData(Out, Data)
  assert(type(Out)=="table", "arg #1 is not a table")
  assert(type(Data)=="table", "arg #2 is not a table")
  for k,v in pairs(Out) do Data[k]=v end
end

--[[---------------------------------------------------------------------------
Purpose:
  Get automatically highlighting (hot keys) for strings in a menu or dialog.
-------------------------------------------------------------------------------
Parameters:
  @Arr:
    An array of strings. The strings containing & are excluded from the
    processing but their highlighted characters will be considered reserved
    and not available for highlighting in other strings.
  @Patt (optional):
    Lua pattern for determining what characters can be highlighted.
    The default is '%w', but '%S', '%a', etc. can also be used.
-------------------------------------------------------------------------------
Returns:
  @Out:
    A table containing highlighted strings (if any) placed at the same indexes
    as those strings are in the input array.
    Out.n is set to the number of strings highlighted by the function.
    Out.n will be <= the number of unhighlighted strings in Arr.
-------------------------------------------------------------------------------
--]]

local function hilite (Arr, Patt)
  Patt = Patt or "%w"
  local charstate, indexes, wei, out = {}, {}, {}, {n=0}

  -- Initialize 'charstate' and 'indexes'
  local patt2 = "%&(" .. Patt .. ")"
  for i, str in ipairs(Arr) do
    local _, n = str:lower():gsub(patt2, function(c) charstate[c]="reserved" end, 1)
    if n == 0 then table.insert(indexes,i) end
  end

  -- Initialize 'wei' as a number of times a char is found in words to be highlighted.
  for _, v in ipairs(indexes) do
    local used = {}
    for c in Arr[v]:lower():gmatch(Patt) do
      if not (charstate[c] or used[c]) then
        wei[c] = (wei[c] or 0) + 1
        used[c] = true
      end
    end
  end

  -- Get "weight" of a word.
  local function get_weight (str)
    local w, used = 0, {}
    for c in str:lower():gmatch(Patt) do
      if not (charstate[c] or used[c]) then
        w = w + 1/wei[c]
        used[c] = true
      end
    end
    return w
  end

  -- Sort
  table.sort(indexes, function(i1, i2)
    return get_weight(Arr[i1]) < get_weight(Arr[i2]) end)

  -- Assign
  for _, v in ipairs(indexes) do
    local found
    local s = Arr[v]:gsub(Patt,
      function(c)
        local c_lower = c:lower()
        if not found and not charstate[c_lower] then
          found = true
          charstate[c_lower] = "assigned"
          return "&"..c
        end
      end)
    if found then
      out[v], out.n = s, out.n+1
    end
  end
  return out
end

--- Automatically assign hot keys in dialog items.
-- @param Items : an array of dialog items (tables);
--                an item may have a boolean field 'nohilite' that means no automatic highlighting;
function mod:AssignHotKeys()
  local Items = self.Items
  local types = { butt=1; chbox=1; rbutt=1; text=1; vtext=1; }
  local arr, idx = {}, {}
  for i,v in ipairs(Items) do
    if types[v.tp] and not v.nohilite then
      local n = #arr+1
      arr[n], idx[n] = v.text, i
    end
  end
  local out = hilite(arr)
  for k,v in pairs(out) do
    if type(k)=="number" then Items[idx[k]].text=v end
  end
end

return mod
