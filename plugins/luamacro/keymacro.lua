-- coding: utf-8

local Shared = ...
local pack, MacroInit = Shared.pack, Shared.MacroInit

local F = far.Flags
local bit = bit or bit64
local band,bor,bxor,lshift = bit.band,bit.bor,bit.bxor,bit.lshift
local LastMessage = {}

local MACROMODE_NOMACRO          =0  -- не в режиме макро
local MACROMODE_EXECUTING        =1  -- исполнение: без передачи плагину пимп
local MACROMODE_EXECUTING_COMMON =2  -- исполнение: с передачей плагину пимп

local MFLAGS_ENABLEOUTPUT        = 0x00000001 -- не подавлять обновление экрана во время выполнения макроса
local MFLAGS_NOSENDKEYSTOPLUGINS = 0x00000002 -- не передавать плагинам клавиши во время записи/воспроизведения макроса

local MacroRecord = {
  m_lang="lua",     -- Язык макропоследовательности
  m_flags=0,        -- Флаги макропоследовательности
  m_key=-1,         -- Назначенная клавиша
  m_code="",        -- оригинальный "текст" макроса
  m_macroId=0,      -- Идентификатор загруженного макроса в плагине LuaMacro; 0 для макроса, запускаемого посредством MSSC_POST.
  m_macrovalue=nil, -- Значение, хранимое исполняющимся макросом
  m_handle=0        -- Хэндл исполняющегося макроса
}
local meta_MacroRecord = { __index=MacroRecord }

local function NewMacroRecord(Lang, Flags, MacroId, Key, Code)
  local mr = {
    m_lang=Lang,
    m_flags=Flags,
    m_key=Key,
    m_code=Code,
    m_macroId=MacroId,
    m_macrovalue=nil,
    m_handle=0
  }
  return setmetatable(mr, meta_MacroRecord)
end

function MacroRecord:Flags() return self.m_flags end
function MacroRecord:GetHandle() return self.m_handle end
function MacroRecord:SetHandle(handle) self.m_handle=handle end
function MacroRecord:GetValue() return self.m_macrovalue end
function MacroRecord:SetValue(val) self.m_macrovalue=val end
--------------------------------------------------------------------------------

local MacroState = {
  IntKey=0, -- "описание реально нажатой клавиши"
  Executing=0,
  m_MacroQueue=nil,
  HistoryDisable=0,
  UseInternalClipboard=false
}
local meta_MacroState = { __index=MacroState }
local function NewMacroState()
  return setmetatable({ m_MacroQueue={} }, meta_MacroState)
end
function MacroState:GetCurMacro() return self.m_MacroQueue[1] end
function MacroState:RemoveCurMacro() table.remove(self.m_MacroQueue, 1) end
--------------------------------------------------------------------------------

local stack = {}
local meta_stack = { __index=stack }
local function NewStack() return setmetatable({}, meta_stack) end
function stack:top() return self[#self] end
function stack:pop() local v=self[#self]; self[#self]=nil; return v; end
function stack:push(v) self[#self+1]=v end
function stack:empty() return self[1]==nil end
--------------------------------------------------------------------------------

local KeyMacro = {
  m_StateStack = NewStack(),
  m_CurState = NewMacroState(),
}

function KeyMacro:GetCurMacro() return self.m_CurState:GetCurMacro() end
function KeyMacro:GetTopMacro() return self.m_StateStack[1] and self.m_StateStack:top():GetCurMacro() end
function KeyMacro:RemoveCurMacro() self.m_CurState:RemoveCurMacro() end

function KeyMacro:GetCurRecord()
  return self.m_Recording ~= MACROMODE_NOMACRO and self.m_Recording or self:IsExecuting()
end

function KeyMacro:IsExecuting()
  local m = self:GetCurMacro()
  if m and m:GetHandle()~=0 then
    return band(m:Flags(),MFLAGS_NOSENDKEYSTOPLUGINS)~=0 and MACROMODE_EXECUTING or MACROMODE_EXECUTING_COMMON
  else
    return self.m_StateStack[1] and MACROMODE_EXECUTING_COMMON or MACROMODE_NOMACRO
  end
end

function KeyMacro:GetHistoryDisableMask()
  return self.m_CurState.HistoryDisable
end

function KeyMacro:SetHistoryDisableMask (Mask)
  local OldHistoryDisable = self.m_CurState.HistoryDisable
  self.m_CurState.HistoryDisable = Mask
  return OldHistoryDisable
end

function KeyMacro:IsHistoryDisable (TypeHistory)
  return self.m_CurState.m_MacroQueue[1] and
    band(self.m_CurState.HistoryDisable, lshift(1,TypeHistory))~=0 and 1 or 0
end

function KeyMacro:IsDisableOutput()
  local m = self:GetCurMacro()
  return m and band(m:Flags(),MFLAGS_ENABLEOUTPUT)==0 and 1 or 0
end

function KeyMacro:PushState (withClip)
  if withClip then
    --self.m_CurState.UseInternalClipboard = Clipboard:GetUseInternalClipboardState()
    self.m_CurState.UseInternalClipboard = mf.clip(5,-1)==2 --TODO
  end
  self.m_StateStack:push(self.m_CurState)
  self.m_CurState = NewMacroState()
end

function KeyMacro:PopState (withClip)
  if self.m_StateStack[1] then
    local q = self.m_StateStack:top().m_MacroQueue
    for i,v in ipairs(self.m_CurState.m_MacroQueue) do
      q[#q+1] = v
    end
    self.m_CurState = self.m_StateStack:pop()
    if withClip then
      --Clipboard:SetUseInternalClipboardState(self.m_CurState.UseInternalClipboard)
      mf.clip(5, self.m_CurState.UseInternalClipboard and 2 or 1) --TODO
    end
  end
end

-- инициализация всех переменных
function KeyMacro:InitInternalVars (InitedRAM)
  if InitedRAM then
    self.m_CurState.m_MacroQueue = {}
    self.m_CurState.Executing = MACROMODE_NOMACRO
  end
  self.m_CurState.HistoryDisable = 0
  self.m_Recording = MACROMODE_NOMACRO
end

function KeyMacro:SetHandle (handle)
  local macro = self:GetCurMacro()
  if macro then macro:SetHandle(handle) end
end

function KeyMacro:mmode (Action, nValue)     -- N=MMode(Action[,Value])
  local TopMacro = self:GetTopMacro()
  if not TopMacro then return false end

  if Action==1 then -- DisableOutput
    local Result = band(TopMacro:Flags(),MFLAGS_ENABLEOUTPUT)==1 and 0 or 1
    nValue = type(nValue)=="number" and math.floor(nValue)
    if nValue and nValue>=0 and nValue<=2 and nValue~=Result then
      TopMacro.m_flags = bxor(TopMacro.m_flags, MFLAGS_ENABLEOUTPUT)
    end
    return Result

  elseif Action==2 then -- Get MacroRecord Flags
    return bor(lshift(TopMacro:Flags(),8), 0xFF)
  end

  return 0
end

function KeyMacro:CheckCurMacro()
  local mr = self:GetCurMacro()
  if mr then
    local handle = mr:GetHandle()
    if handle == 0 then
      handle = MacroInit(mr.m_macroId, mr.m_lang, mr.m_code)
      if handle then mr:SetHandle(handle) end
    end
    if handle and handle ~= 0 then
      LastMessage = pack(mr.m_flags, mr.m_key, mr.m_macrovalue, mr.m_handle)
      return F.MPRT_NORMALFINISH, LastMessage
    else
      self:RemoveCurMacro()
      return -1 -- RestoreMacroChar()
    end
  end
end

function KeyMacro:Dispatch (opcode,p1,p2,p3,p4,p5)
  if     opcode==1  then self:PushState(p1)
  elseif opcode==2  then self:PopState(p1)
  elseif opcode==3  then self:InitInternalVars(p1)
  elseif opcode==4  then return self:IsExecuting()
  elseif opcode==5  then return self:IsDisableOutput()
  elseif opcode==6  then return self:SetHistoryDisableMask(p1)
  elseif opcode==7  then return self:GetHistoryDisableMask()
  elseif opcode==8  then return self:IsHistoryDisable(p1)
  elseif opcode==9 or opcode==10 then
    local mr
    if opcode==9 then mr=self:GetCurMacro() else mr=self:GetTopMacro() end
    if mr then
      LastMessage = pack(mr.m_flags, mr.m_key, mr.m_macrovalue, mr.m_handle)
      return F.MPRT_NORMALFINISH, LastMessage
    end
  elseif opcode==11 then self:RemoveCurMacro()
  elseif opcode==12 then return #self.m_CurState.m_MacroQueue
  elseif opcode==13 then self.m_CurState.IntKey = p1
  elseif opcode==14 then return #self.m_StateStack
  elseif opcode==15 then return self.m_CurState.IntKey
  elseif opcode==16 then
    table.insert(self.m_CurState.m_MacroQueue, NewMacroRecord(p1,p2,p3,p4,p5))
    return true
  elseif opcode==17 then local t=self.m_StateStack:top() return t and t.IntKey or 0
  elseif opcode==18 then
    local t = self.m_StateStack:top()
    local oldHistoryDisable = t and t.HistoryDisable or 0
    if t and p1 then t.HistoryDisable = p1 end
    return oldHistoryDisable
  elseif opcode==19 then self:SetHandle(p1)
  elseif opcode==20 then
    local m=self:GetCurMacro()
    if m then m:SetValue(p1) end
  elseif opcode==21 then return self:CheckCurMacro()
  end
end

return KeyMacro
