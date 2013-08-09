local ffi = require "ffi"
require "far2.ffi.farapi"
local C = ffi.C
local sorts = require "sorts"

local F=far.Flags
local SM_UNSORTED,SM_NAME,SM_EXT,SM_FULLNAME = F.SM_UNSORTED,F.SM_NAME,F.SM_EXT,F.SM_FULLNAME
local PPIF_SELECTED=F.PPIF_SELECTED
local PPIF_PROCESSDESCR=F.PPIF_PROCESSDESCR
local FILE_ATTRIBUTE_DIRECTORY=0x00000010
local MCODE_F_SETCUSTOMSORTMODE=0x80C68
local band=bit.band -- 32 bits, be careful
local tonumber=tonumber

local CustomSortModes = {} -- key=integer, value=function

-- called from user script
local function SetCustomSortMode (whatpanel, nMode, fCompare, bInvertByDefault, sIndicator)
  assert(whatpanel==0 or whatpanel==1)
  assert(type(nMode)=="number" and nMode==math.floor(nMode) and nMode>=100 and nMode<=0x7FFFFFFF)
  assert(type(fCompare)=="function")
  assert(type(sIndicator)=="string")
  CustomSortModes[nMode] = fCompare
  return far.MacroCallFar(MCODE_F_SETCUSTOMSORTMODE, whatpanel, nMode, bInvertByDefault, sIndicator)
end

ffi.cdef[[
  struct FileListItem;
  typedef struct
  {
    const struct FileListItem **Data;
    size_t                      DataSize;
    void                      (*FileListToPluginItem)(const struct FileListItem*, struct PluginPanelItem*);
    int                         ListSortGroups;
    int                         ListSelectedFirst;
    int                         ListDirectoriesFirst;
    int                         ListSortMode;
    int                         RevertSorting;
    int                         ListPanelMode; // currently not used
    int                         ListNumericSort;
    int                         ListCaseSensitiveSort;
    HANDLE                      hSortPlugin;
  } CustomSort;

  size_t wcslen(const wchar_t*);
  int wcscmp(const wchar_t*,const wchar_t*);
  int _wcsicmp(const wchar_t*,const wchar_t*);
  int MessageBoxW(void*,const wchar_t*,const wchar_t*,int);
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
  params = ffi.cast("CustomSort*", params)
  local fCompare = CustomSortModes[tonumber(params.ListSortMode)]
  if not fCompare then return end

  local pi1 = ffi.new("struct PluginPanelItem")
  local pi2 = ffi.new("struct PluginPanelItem")

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
		if params.ListSortMode == SM_UNSORTED then
			if params.ListSelectedFirst ~= 0 then
        local s1,s2 = band(tonumber(pi1.Flags),PPIF_SELECTED), band(tonumber(pi2.Flags),PPIF_SELECTED)
        if s1 ~= s2 then return s1 > s2 end --> TODO: fix in Far
      end
      if params.RevertSorting ~= 0 then pi1,pi2 = pi2,pi1 end
			return pi1.Reserved[0] < pi2.Reserved[0] -- Reserved[0] contains 'Position'
		end
    ----------------------------------------------------------------------------
		if params.ListDirectoriesFirst then
			local a1,a2 = band(tonumber(pi1.FileAttributes), FILE_ATTRIBUTE_DIRECTORY),
                    band(tonumber(pi2.FileAttributes), FILE_ATTRIBUTE_DIRECTORY)
      if a1 ~= a2 then return a1 > a2 end
		end
    ----------------------------------------------------------------------------
    if params.ListSelectedFirst ~= 0 then
      local s1,s2 = band(tonumber(pi1.Flags),PPIF_SELECTED), band(tonumber(pi2.Flags),PPIF_SELECTED)
      if s1 ~= s2 then return s1 > s2 end
    end
    ----------------------------------------------------------------------------
    if params.ListSortGroups and
      (params.ListSortMode==SM_NAME or params.ListSortMode==SM_EXT or params.ListSortMode==SM_FULLNAME)
    then
       -- Reserved[1] contains 'SortGroup'
      if pi1.Reserved[1] ~= pi2.Reserved[1] then return pi1.Reserved[1] < pi2.Reserved[1] end
    end
    ----------------------------------------------------------------------------
    if params.RevertSorting ~= 0 then pi1,pi2 = pi2,pi1 end

    --return C.wcslen(pi1.FileName) < C.wcslen(pi2.FileName)
    --return C._wcsicmp(pi1.FileName, pi2.FileName) < 0
    return fCompare(pi1,pi2)
  end

  sorts.shellsort(params.Data, Before, params.DataSize)
  --sorts.qsort(params.Data, 0, params.DataSize-1, Before)
  --sorts.selectionsort(params.Data, params.DataSize, Before)
  return true
end

return {
  SortPanelItems=SortPanelItems,
  SetCustomSortMode=SetCustomSortMode,
}
