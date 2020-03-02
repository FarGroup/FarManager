#ifndef __MARCLNG_HPP__
#define __MARCLNG_HPP__

enum LanguageID
{
  MOk,
  MCancel,
  MReset,

  MWarning,
  MError,

  MExtractTitle,
  MExtractTo,
  MExtrPassword,
  MExtrWithoutPaths,
  MExtrDel,
  MExtrExtract,
  MExtrCancel,

  MExtrVolume,
  MExtrVolumeAsk1,
  MExtrVolumeAsk2,
  MExtrVolumeSelFiles,
  MExtrAllVolumes,

  MGetPasswordTitle,
  MGetPassword,
  MGetPasswordForFile,

  MDeleteTitle,
  MDeleteFile,
  MDeleteFiles,
  MDeleteNumberOfFiles,
  MDeleteDelete,
  MDeleteCancel,

  MCannotPutToFolder,
  MPutToRoot,

  MAddTitle,
  MAddToArc,
  MAddSwitches,
  MAddPassword,
  MAddReenterPassword,
  MAddDelete,
  MAddAdd,
  MAddSelect,
  MAddSave,
  MAddCancel,

  MAddPswNotMatch,

  MSelectArchiver,
  MSelectF4,

  MArcCmdTitle,
  MArcCmdTest,
  MArcCmdComment,
  MArcCmdCommentFiles,
  MArcCmdSFX,
  MArcCmdRecover,
  MArcCmdProtect,
  MArcCmdLock,

  MArcNonZero,
  MArcCommandNotFound,
  MCannotCreateListFile,
  MCannotFindArchivator,

  MArcFormat,

  MArcReadFiles,
  MArcReadTitle,
  MArcReading,

  MBadArchive,
  MUnexpEOF,
  MReadError,

  MSeveralOS,
  MInfoTitle,
  MInfoArchive,
  MInfoArcType,
  MInfoSolid,
  MInfoSFX,
  MInfoVolume,
  MInfoHdrEncrypted,
  MInfoNormal,
  MInfoArcComment,
  MInfoPresent,
  MInfoAbsent,
  MInfoFileComments,
  MInfoPasswords,
  MInfoRecovery,
  MInfoLock,
  MInfoAuthVer,
  MInfoChapters,
  MInfoDict,
  MInfoDictKb,
  MInfoTotalFiles,
  MInfoTotalSize,
  MInfoPackedSize,
  MInfoRatio,

  MAltF6,
  MAltShiftF9,

  MWaitForExternalProgram,

  MCfgLine0,
  MCfgLine1,
  MCfgLine2,

  MGConfigTitle,
  MGConfigHideNone,
  MGConfigHideView,
  MGConfigHideAlways,
  MGConfigProcessShiftF1,
  MGConfigAllowChangeDir,
  MGConfigUseLastHistory,
//  MGConfigDeleteExtFile,
//  MGConfigAddExtArchive,
  MGConfigAutoResetExactArcName,
  MGConfigOldDialogStyle,

  MGConfigDizNames,
  MGConfigReadDiz,
  MGConfigUpdateDiz,

  MArcSettingsExtract,
  MArcSettingsExtractWithoutPath,
  MArcSettingsTest,
  MArcSettingsDelete,
  MArcSettingsComment,
  MArcSettingsCommentFiles,
  MArcSettingsSFX,
  MArcSettingsLock,
  MArcSettingsProtect,
  MArcSettingsRecover,
  MArcSettingsAdd,
  MArcSettingsMove,
  MArcSettingsAddRecurse,
  MArcSettingsMoveRecurse,
  MArcSettingsAllFilesMask,
  MArcSettingsDefaultExt,

  MBackground,
  MExactArcName,

  MComment,
  MInputComment,

  MPriorityOfProcess,
  MIdle_Priority_Class,
  MBelow_Normal_Priority_Class,
  MNormal_Priority_Class,
  MAbove_Normal_Priority_Class,
  MHigh_Priority_Class,
};

#endif // __MARCLNG_HPP__
