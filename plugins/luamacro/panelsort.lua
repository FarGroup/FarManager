local ffi = require "ffi"

-- shellsort [ http://lua-users.org/wiki/LuaSorting ]
-- Written by Rici Lake. The author disclaims all copyright and offers no warranty.
-- For convenience, shellsort returns its first argument.

local incs = { 1391376,
               463792, 198768, 86961, 33936,
               13776, 4592, 1968, 861, 336,
               112, 48, 21, 7, 3, 1 }

local function shellsort(t, n, before)
  for _, h in ipairs(incs) do
    for i = h + 1, n do
      local v = t[i-1]
      for j = i - h, 1, -h do
        local testval = t[j-1]
        if not before(v, testval) then break end
        t[i-1] = testval; i = j
      end
      t[i-1] = v
    end
  end
  return t
end

-- qsort [ extracted from Lua 5.1 distribution (file /test/sort.lua) ]
local function qsort(x,l,u,f)
  if l<u then
    local m=math.random(u-(l-1))+l-1 -- choose a random pivot in range l..u
    x[l],x[m]=x[m],x[l] -- swap pivot to first position
    local t=x[l]        -- pivot value
    m=l
    local i=l+1
    while i<=u do
      -- invariant: x[l+1..m] < t <= x[m+1..i-1]
      if f(x[i],t) then
        m=m+1
        x[m],x[i]=x[i],x[m]  -- swap x[i] and x[m]
      end
      i=i+1
    end
    x[l],x[m]=x[m],x[l]      -- swap pivot to a valid place
    -- x[l+1..m-1] < x[m] <= x[m+1..u]
    qsort(x,l,m-1,f)
    qsort(x,m+1,u,f)
  end
end

local C = ffi.C
local F = far.Flags
local PPIF_SELECTED = F.PPIF_SELECTED
local FILE_ATTRIBUTE_DIRECTORY = 0x00000010
local MCODE_F_SETCUSTOMSORTMODE = 0x80C68
local band = bit.band -- 32 bits, be careful
local tonumber = tonumber

local CustomSortModes = {} -- key=integer, value=function

-- called from user script
local function SetCustomSortMode (whatpanel, nMode, Settings)
  whatpanel = whatpanel==1 and 1 or 0
  assert(type(nMode)=="number" and nMode==math.floor(nMode) and nMode>=100 and nMode<=0x7FFFFFFF)
  assert(type(Settings)=="table")
  assert(type(Settings.Compare)=="function")
  local InvertByDefault = not not Settings.InvertByDefault
  local Indicator = type(Settings.Indicator)=="string" and Settings.Indicator or ""
  CustomSortModes[nMode] = Settings
  return far.MacroCallFar(MCODE_F_SETCUSTOMSORTMODE, whatpanel, nMode, InvertByDefault, Indicator)
end

ffi.cdef[[
  struct FileListItem;
  typedef struct
  {
    const struct FileListItem **Data;
    size_t                      DataSize;
    void                      (*FileListToPluginItem)(const struct FileListItem*, struct PluginPanelItem*);
    int                         SortGroups;
    int                         SelectedFirst;
    int                         DirectoriesFirst;
    int                         SortMode;
    int                         RevertSorting;
    int                         ListPanelMode; // currently not used
    int                         NumericSort;
    int                         CaseSensitiveSort;
    HANDLE                      hSortPlugin;
  } CustomSort;
]]

local function Utf16Buf (str)
  str = win.Utf8ToUtf16(str)
  local buf = ffi.new("wchar_t[?]", #str/2+1)
  ffi.copy(buf, str, #str)
  return buf
end

local DOTS = Utf16Buf("..")

-- called from Far
local function SortPanelItems (params)
--local timeStart = Far.UpTime
--jit.flush()
  params = ffi.cast("CustomSort*", params)
  local tSettings = CustomSortModes[tonumber(params.SortMode)]
  if not tSettings then return end
  local Compare = tSettings.Compare

  local pi1 = ffi.new("struct PluginPanelItem")
  local pi2 = ffi.new("struct PluginPanelItem")

  local SortGroups, SelectedFirst, DirectoriesFirst, RevertSorting

  if tSettings.SortGroups==0 then SortGroups=false
  elseif tSettings.SortGroups==1 then SortGroups=true
  else SortGroups = params.SortGroups ~= 0
  end

  if tSettings.SelectedFirst==0 then SelectedFirst=false
  elseif tSettings.SelectedFirst==1 then SelectedFirst=true
  else SelectedFirst = params.SelectedFirst ~= 0
  end

  if tSettings.DirectoriesFirst==0 then DirectoriesFirst=false
  elseif tSettings.DirectoriesFirst==1 then DirectoriesFirst=true
  else DirectoriesFirst = params.DirectoriesFirst ~= 0
  end

  if tSettings.RevertSorting==0 then RevertSorting=false
  elseif tSettings.RevertSorting==1 then RevertSorting=true
  else RevertSorting = params.RevertSorting ~= 0
  end

  local outParams = {
    SortGroups        = params.SortGroups ~= 0;
    SelectedFirst     = params.SelectedFirst ~= 0;
    DirectoriesFirst  = params.DirectoriesFirst ~= 0;
    RevertSorting     = params.RevertSorting ~= 0;
    NumericSort       = params.NumericSort ~= 0;
    CaseSensitiveSort = params.CaseSensitiveSort ~= 0;
  }

  local function Before(e1, e2)
    params.FileListToPluginItem(e1, pi1)
    params.FileListToPluginItem(e2, pi2)
    ----------------------------------------------------------------------------
    -- TODO: fix in Far
    if C.wcscmp(pi1.FileName,DOTS)==0 then
      if C.wcscmp(pi2.FileName,DOTS)~=0 then return true end

      if pi1.AlternateFileName[0]==0 or C.wcscmp(pi1.AlternateFileName,DOTS)==0 then
        if not (pi2.AlternateFileName[0]==0 or C.wcscmp(pi2.AlternateFileName,DOTS)==0) then return true end
      else
        if pi2.AlternateFileName[0]==0 or C.wcscmp(pi2.AlternateFileName,DOTS)==0 then return false end
      end

    elseif C.wcscmp(pi2.FileName,DOTS)==0 then
      return false
    end
    ----------------------------------------------------------------------------
    -- return pi1.Reserved[0] < pi2.Reserved[0] -- Reserved[0] contains 'Position'
    ----------------------------------------------------------------------------
    if DirectoriesFirst then
      local a1,a2 = band(tonumber(pi1.FileAttributes), FILE_ATTRIBUTE_DIRECTORY),
                    band(tonumber(pi2.FileAttributes), FILE_ATTRIBUTE_DIRECTORY)
      if a1 ~= a2 then return a1 > a2 end
    end
    ----------------------------------------------------------------------------
    if SelectedFirst then
      local s1,s2 = band(tonumber(pi1.Flags),PPIF_SELECTED), band(tonumber(pi2.Flags),PPIF_SELECTED)
      if s1 ~= s2 then return s1 > s2 end
    end
    ----------------------------------------------------------------------------
    if SortGroups then
       -- Reserved[1] contains 'SortGroup'
      if pi1.Reserved[1] ~= pi2.Reserved[1] then return pi1.Reserved[1] < pi2.Reserved[1] end
    end
    ----------------------------------------------------------------------------
    local r = Compare(pi1, pi2, outParams)
    if r ~= 0 then
      if RevertSorting then r = -r end
      return r < 0
    else
      return C._wcsicmp(pi1.FileName,pi2.FileName) < 0
    end
  end

  shellsort(params.Data, tonumber(params.DataSize), Before)
  -- qsort(params.Data, 0, tonumber(params.DataSize)-1, Before)

--far.Message(Far.UpTime - timeStart)
  return true
end

return {
  SortPanelItems=SortPanelItems,
  SetCustomSortMode=SetCustomSortMode,
}
