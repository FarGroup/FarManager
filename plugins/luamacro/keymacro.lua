-- coding: utf-8

--------------------------------------------------------------------------------
local Shared = ...
local Msg, MacroInit, MacroStep, ErrMsg, pack, loadmacro =
  Shared.Msg, Shared.MacroInit, Shared.MacroStep, Shared.ErrMsg, Shared.pack, Shared.loadmacro

local F = far.Flags
local MPRT_NORMALFINISH, MPRT_ERRORFINISH, MPRT_HASNOMACRO =
  F.MPRT_NORMALFINISH, F.MPRT_ERRORFINISH, F.MPRT_HASNOMACRO

local type, setmetatable = type, setmetatable
local bit = bit or bit64
local band, bor, bxor, lshift = bit.band, bit.bor, bit.bxor, bit.lshift
local math_floor = math.floor
local far_FarMacroCallToLua, far_MacroCallFar, far_Text = far.FarMacroCallToLua, far.MacroCallFar, far.Text
far.FarMacroCallToLua = nil

local MACROSTATE_NOMACRO          = F.MACROSTATE_NOMACRO
local MACROSTATE_EXECUTING        = F.MACROSTATE_EXECUTING
local MACROSTATE_EXECUTING_COMMON = F.MACROSTATE_EXECUTING_COMMON

local MFLAGS_ENABLEOUTPUT        = 0x00000001 -- не подавлять обновление экрана во время выполнения макроса
local MFLAGS_NOSENDKEYSTOPLUGINS = 0x00000002 -- не передавать плагинам клавиши во время записи/воспроизведения макроса
local MFLAGS_POSTFROMPLUGIN      = 0x10000000 -- последовательность пришла от АПИ
--------------------------------------------------------------------------------

local MCODE_F_KEYMACRO = 0x80C68
local Import = {
  RestoreMacroChar        = function()  far_MacroCallFar(MCODE_F_KEYMACRO, 1) end,
  ScrBufLock              = function()  far_MacroCallFar(MCODE_F_KEYMACRO, 2) end,
  ScrBufUnlock            = function()  far_MacroCallFar(MCODE_F_KEYMACRO, 3) end,
  ScrBufResetLockCount    = function()  far_MacroCallFar(MCODE_F_KEYMACRO, 4) end,
  GetUseInternalClipboard = function()  return far_MacroCallFar(MCODE_F_KEYMACRO, 5) end,
  SetUseInternalClipboard = function(v) far_MacroCallFar(MCODE_F_KEYMACRO, 6, v) end,
  KeyNameToKey            = function(v) return far_MacroCallFar(MCODE_F_KEYMACRO, 7, v) end,
}
--------------------------------------------------------------------------------

local NewMacroRecord do
  local MacroRecord = {
    m_lang="lua",     -- Язык макропоследовательности
    m_flags=0,        -- Флаги макропоследовательности
    m_key=-1,         -- Назначенная клавиша
    m_code="",        -- оригинальный "текст" макроса
    m_macroId=0,      -- Идентификатор загруженного макроса в плагине LuaMacro; 0 для макроса, запускаемого посредством MSSC_POST.
    m_macrovalue=nil, -- Значение, хранимое исполняющимся макросом
    m_handle=nil      -- Хэндл исполняющегося макроса
  }
  local meta = { __index=MacroRecord }

  function MacroRecord:Flags() return self.m_flags end
  function MacroRecord:GetHandle() return self.m_handle end
  function MacroRecord:SetHandle(handle) self.m_handle=handle end
  function MacroRecord:GetValue() return self.m_macrovalue end
  function MacroRecord:SetValue(val) self.m_macrovalue=val end

  NewMacroRecord = function (MacroId, Lang, Code, Flags, Key)
    local mr = {
      m_macroId=MacroId, m_lang=Lang, m_flags=Flags, m_key=Key, m_code=Code,
      m_macrovalue=nil,
      m_handle=nil
    }
    return setmetatable(mr, meta)
  end
end
--------------------------------------------------------------------------------

-- Специализированная очередь, оптимизированная для очередей макросов.
-- Оптимизация по скорости и памяти, имеет смысл при добавлении в цикле сотен тысяч элементов.
-- Данная оптимизация накладывает следующие ограничения (соблюдаемые в нашем случае):
--   Элементы должны быть таблицами.
--   Очередь добавляет в элементы поле _next.
--   Нельзя добавлять элемент в очередь, если он уже имеется в этой очереди.
local NewQueue do
  local queue = {}
  local meta = { __index=queue }
  function queue:add(v)
    if self.last ~= nil then
      self.last._next = v
      self.last = v
    else
      self.first, self.last = v, v
    end
  end
  function queue:addqueue(q)
    if q.last ~= nil then
      if self.last ~= nil then
        self.last._next = q.first
        self.last = q.last
      else
        self.first, self.last = q.first, q.last
      end
    end
  end
  function queue:remove()
    if self.first ~= nil then
      self.first = self.first._next
      if self.first==nil then self.last=nil end
    end
  end
  function queue:empty() return self.first==nil end
  NewQueue = function() return setmetatable({}, meta) end
end
--------------------------------------------------------------------------------

local NewMacroState do
  local MacroState = {
    IntKey=0, -- "описание реально нажатой клавиши"
    Executing=0,
    MacroQueue=nil,
    HistoryDisable=0,
    UseInternalClipboard=false
  }
  local meta = { __index=MacroState }
  function MacroState:GetCurMacro() return self.MacroQueue.first end
  function MacroState:RemoveCurMacro() self.MacroQueue:remove() end
  NewMacroState = function() return setmetatable({ MacroQueue=NewQueue() }, meta) end
end
--------------------------------------------------------------------------------

local NewStack do
  local stack = {}
  local meta = { __index=stack }
  function stack:top() return self[#self] end
  function stack:pop() local v=self[#self]; self[#self]=nil; return v; end
  function stack:push(v) self[#self+1]=v end
  function stack:empty() return self[1]==nil end
  NewStack = function() return setmetatable({}, meta) end
end
--------------------------------------------------------------------------------

local KeyMacro = {}
local CurState = NewMacroState()
local StateStack = NewStack()
local LastMessage = {}
local MacroPluginIsRunning = 0
--------------------------------------------------------------------------------

local function GetCurMacro() return CurState:GetCurMacro() end
local function GetTopMacro() return StateStack[1] and StateStack:top():GetCurMacro() end
local function RemoveCurMacro() CurState:RemoveCurMacro() end

local function IsExecuting()
  local m = GetCurMacro()
  if m and m:GetHandle() then
    return band(m:Flags(),MFLAGS_NOSENDKEYSTOPLUGINS)~=0 and MACROSTATE_EXECUTING or MACROSTATE_EXECUTING_COMMON
  else
    return StateStack[1] and MACROSTATE_EXECUTING_COMMON or MACROSTATE_NOMACRO
  end
end

local function GetHistoryDisableMask()
  return CurState.HistoryDisable
end

local function SetHistoryDisableMask (Mask)
  local OldHistoryDisable = CurState.HistoryDisable
  CurState.HistoryDisable = Mask
  return OldHistoryDisable
end

local function IsHistoryDisable (TypeHistory)
  return GetCurMacro() and band(CurState.HistoryDisable, lshift(1,TypeHistory))~=0 and 1 or 0
end

local function IsDisableOutput()
  local m = GetCurMacro()
  return m and band(m:Flags(),MFLAGS_ENABLEOUTPUT)==0 and 1 or 0
end

local function PushState (withClip)
  if withClip then
    CurState.UseInternalClipboard = Import.GetUseInternalClipboard()
  end
  StateStack:push(CurState)
  CurState = NewMacroState()
end

local function PopState (withClip)
  if StateStack[1] then
    StateStack:top().MacroQueue:addqueue(CurState.MacroQueue)
    CurState = StateStack:pop()
    if withClip then
      Import.SetUseInternalClipboard(CurState.UseInternalClipboard)
    end
  end
end

-- инициализация всех переменных
function KeyMacro.InitInternalVars (InitedRAM)
  if InitedRAM then
    CurState.MacroQueue = NewQueue()
    CurState.Executing = MACROSTATE_NOMACRO
  end
  CurState.HistoryDisable = 0
end

function KeyMacro.mmode (Action, nValue)     -- N=MMode(Action[,Value])
  local TopMacro = GetTopMacro()
  if not TopMacro then return false end

  if Action==1 then -- DisableOutput
    local Result = band(TopMacro:Flags(),MFLAGS_ENABLEOUTPUT)==1 and 0 or 1
    if (nValue==0 or nValue==1 or nValue==2) and nValue~=Result then
      TopMacro.m_flags = bxor(TopMacro.m_flags, MFLAGS_ENABLEOUTPUT)
      far_Text()
    end
    return Result
  elseif Action==2 then -- Get MacroRecord Flags
    return bor(lshift(TopMacro:Flags(),8), 0xFF)
  else
    return 0
  end
end

local function CheckCurMacro()
  local macro = GetCurMacro()
  if macro then
    local handle = macro:GetHandle()
    if handle == nil then
      --MacroPluginIsRunning = MacroPluginIsRunning + 1
      PushState(false)
      handle = MacroInit(macro.m_macroId, macro.m_lang, macro.m_code)
      PopState(false)
      --MacroPluginIsRunning = MacroPluginIsRunning - 1
      if handle then macro:SetHandle(handle) end
    end
    if handle then
      return macro
    end
    RemoveCurMacro()
    Import.RestoreMacroChar()
  end
end

local function CallStep()
  while true do
    local macro = CheckCurMacro()
    if not macro then return end

    Import.ScrBufResetLockCount()

    PushState(false)
    local r1,r2
    local value, handle = macro:GetValue(), macro:GetHandle()
    if type(value) == "userdata" then
      r1,r2 = MacroStep(handle, far_FarMacroCallToLua(value))
    elseif value ~= nil then
      r1,r2 = MacroStep(handle, value)
    else
      r1,r2 = MacroStep(handle)
    end
    PopState(false)
    macro:SetValue(nil)

    if not (r1==MPRT_NORMALFINISH or r1==MPRT_ERRORFINISH) then
      if band(macro:Flags(),MFLAGS_ENABLEOUTPUT)==0 then Import.ScrBufLock() end
      return r1, r2
    end

    if band(macro:Flags(),MFLAGS_ENABLEOUTPUT)==0 then Import.ScrBufUnlock() end

    RemoveCurMacro()

    if not GetCurMacro() then
      Import.RestoreMacroChar()
    end
  end
end

local function GetInputFromMacro()
  if MacroPluginIsRunning==0 and not GetCurMacro() then
    if StateStack[1] then
      PopState(true)
      return false
    else
      return MPRT_HASNOMACRO
    end
  end
  MacroPluginIsRunning = MacroPluginIsRunning + 1
  local r1,r2 = CallStep()
  MacroPluginIsRunning = MacroPluginIsRunning - 1
  return r1,r2
end

function KeyMacro.PostNewMacro (macroId, code, flags, key, postFromPlugin)
  flags = flags or 0
  flags = postFromPlugin and bor(flags,MFLAGS_POSTFROMPLUGIN) or flags
  local AKey = 0
  if key then
    local dKey = Import.KeyNameToKey(key)
    if dKey ~= -1 then
      AKey = dKey
    end
  end
  CurState.MacroQueue:add(NewMacroRecord(macroId,"lua",code,flags,AKey))
  return true
end

local function TryToPostMacro (Mode, TextKey, IntKey)
  local m = Shared.utils.GetMacro(Mode, TextKey, true, false)
  if m then
    KeyMacro.PostNewMacro(m.id, m.code, m.flags, TextKey, false)
    SetHistoryDisableMask(0)
    CurState.IntKey = IntKey
    return true
  end
end

function KeyMacro.DisableHistory (State)
  State = type(State)=="number" and math_floor(State)
  local t = StateStack:top()
  local oldHistoryDisable = t and t.HistoryDisable or 0
  if t and State then t.HistoryDisable = State end
  return oldHistoryDisable
end

function KeyMacro.Dispatch (opcode, ...)
  local p1 = (...)
  if     opcode==1 then PushState(p1)
  elseif opcode==2 then PopState(p1)
  elseif opcode==3 then return IsExecuting()
  elseif opcode==4 then return IsDisableOutput()
  elseif opcode==5 then return p1 and SetHistoryDisableMask(p1) or GetHistoryDisableMask()
  elseif opcode==6 then return IsHistoryDisable(p1)
  elseif opcode==7 or opcode==8 then
    local mr
    if opcode==7 then mr=GetCurMacro() else mr=GetTopMacro() end
    if mr then
      LastMessage = pack(mr.m_flags, mr.m_key)
      return MPRT_NORMALFINISH, LastMessage
    end
  elseif opcode==9  then return GetCurMacro() and 0 or 1
  elseif opcode==10 then return #StateStack
  elseif opcode==11 then return CurState.IntKey
  elseif opcode==12 then
    local Lang,Code,Flags,AKey = ...
    local f1,f2 = loadmacro(Lang,Code)
    if f1 then
      CurState.MacroQueue:add(NewMacroRecord({ f1,f2,HasFunction=true },Lang,Code,Flags,AKey))
      return true
    else
      ErrMsg(f2, Msg.MMacroPErrorTitle)
    end
  elseif opcode==13 then local t=StateStack:top() return t and t.IntKey or 0
  elseif opcode==14 then
    local m = GetCurMacro()
    if m then m:SetValue(p1) end
  elseif opcode==15 then return GetInputFromMacro()
  elseif opcode==16 then return TryToPostMacro(...)
  end
end

return KeyMacro
