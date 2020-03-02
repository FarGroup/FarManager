#ifndef _ACEFNC_H
#define _ACEFNC_H

#ifdef __cplusplus
  extern "C" {
#endif /* __cplusplus */

//════════════════════════════════════════════════════════════════════════════════
//═══════=====---  Part 4: Functions supported by the AceV2.Dll ---====═══════════
//════════════════════════════════════════════════════════════════════════════════

  //---------------------------------------------------------------------------
  //
  // AceV2.Dll function support goes beyond UnAceV2.Dll by these functions:
  //
  //   ACEDelete
  //   ACEAdd
  //   ACEReadSFXData
  //   ACERepair
  //   ACESetComments
  //   ACEEncryptFiles
  //   ACEAddSFX
  //   ACELock
  //   ACEAddRecoveryRecord
  //   ACEAddAV                (not supported in ACL)
  //   ACERegister             (not supported in ACL)
  //   ACEShowRegisterDialog   (not supported in ACL)
  //
  // Look up on UNACE.H for information about Dll initialization and function
  // return codes!
  //
  //---------------------------------------------------------------------------
  //
  // Contents:
  //   Part 4.1: TimeSet defines
  //   Part 4.2: SFX module name defines
  //   Part 4.3: functions and parameter structures
  //
  //---------------------------------------------------------------------------


  //═══════════════════════════════════════════════════════════════════════════
  //=====================---  Part 4.1: TimeSet defines  ---===================
  //═══════════════════════════════════════════════════════════════════════════
  // Set the TimeSet fields to one of these values.
  //---------------------------------------------------------------------------
        #define ACE_SETTIME_MODIFY      0  // set archive time to current
                                           // system time
        #define ACE_SETTIME_KEEP        1  // keep archive time
        #define ACE_SETTIME_LASTFILE    2  // set archive time to time of
                                           // freshest file in archive


  //═══════════════════════════════════════════════════════════════════════════
  //=================---  Part 4.2: SFX module name defines  ---===============
  //═══════════════════════════════════════════════════════════════════════════

    //──────────────────────────  SFX module defines  ─────────────────────────
    // Specify a SFX module name to add an SFX using ACEAddSFX() or ACEAdd().
    // (SFX-file name/ACE_SFXMODULE_NONE/ACE_SFXMODULE_DEFAULT)
    // (see description of these constants)
    //-------------------------------------------------------------------------

        #define ACE_SFX_NONE            "none"     // do not add an SFX
        #define ACE_SFX_DEFAULT         "default"  // add defaul SFX (DOS-SFX
                                                   // with LFN support)


  //═══════════════════════════════════════════════════════════════════════════
  //============---  Part 4.3: functions and parameter structures ---==========
  //═══════════════════════════════════════════════════════════════════════════

    //───────────────────────────  ACEDelete  ─────────────────────────────────
    // Remove files from archive.
    //-------------------------------------------------------------------------

      //-----------------------------------------------------------------------
      // ACEDelete() parameter structure.
      //-----------------------------------------------------------------------

        typedef struct sACEDeleteStruc
        {
          tACEFilesStruc              // specifies files to delete;
                Files;                // see tACEFilesStruc structure
          LPSTR DecryptPassword;      // password for decryption (if files
                                      // are encrypted);
                                      // zero-terminated string,
                                      // case-sensitive (maxlen=56)
          tACECompressParamsStruc     // compression parameters
                CompressParams;       // needed at solid archives only
                                      // (for repacking of files)
          ULONG TimeSet;              // see ACE_SETTIME constants
          CHAR  Reserved[64];         // has to be filled with zeroes
        } tACEDeleteStruc,
         *pACEDeleteStruc;


      //-----------------------------------------------------------------------
      // ACEDelete() function declaration.
      //-----------------------------------------------------------------------

        INT __stdcall ACEDelete(LPSTR ArchiveName, pACEDeleteStruc Delete);


    //────────────────────────────  ACEAdd  ───────────────────────────────────
    // Add files to an archive. Create new archive if it does not exist.
    // If MainCmt is non-zero, then a comment is taken from CmtBuf at
    // creation to become the main comment. This function can work in
    // different modes: normal add, move, update, fresh
    // (See sACEAddStruc.AddMode)
    //-------------------------------------------------------------------------

      //-----------------------------------------------------------------------
      // Tells ACEAdd() how to perform the add operation.
      //-----------------------------------------------------------------------

        #define ACE_ADD_MODE_ADD        0  // normal add operation
        #define ACE_ADD_MODE_MOVE       1  // move files to archive
                                           // (delete after add)
        #define ACE_ADD_MODE_FRESH      2  // add modified files only
        #define ACE_ADD_MODE_UPDATE     3  // add modified and new files only


      //-----------------------------------------------------------------------
      // Volume size defines.
      //-----------------------------------------------------------------------

        #define ACE_VOLUME_NONE         -1 // do not create a multi volume
                                           // archive
        #define ACE_VOLUME_AUTO         -2 // determine volume size by space
                                           // on destination disk


      //-----------------------------------------------------------------------
      // Defines how the paths of archived files are stored in the archive.
      //---------------------------------------------------------------------------

        #define ACE_SAVEPATH_RECURSED   0  // do not save the part of the path
                                           // specified in the file list
        #define ACE_SAVEPATH_NONE       1  // do not save any path (exclude)
        #define ACE_SAVEPATH_FULL       2  // save full path without drive
        #define ACE_SAVEPATH_DRIVE      3  // save full path with drive letter


      //-----------------------------------------------------------------------
      // ACEAdd() parameter structure.
      //-----------------------------------------------------------------------

        typedef struct sACEAddStruc
        {
          tACEFilesStruc              // specifies files to add;
                Files;                // see tACEFilesStruc structure
          LPSTR DestinationDir;       // archive directory to add files to
          ULONG Mode;                 // see ACE_ADD_MODE defines
          ULONG SavePath;             // see ACE_SAVEPATH defines
          LPSTR EncryptPassword;      // encryption and decryption passwords,
          LPSTR DecryptPassword;      // case-sensitive (maxlen=56)

          tACECompressParamsStruc     // compression parameters
                CompressParams;

          BOOL  MainComment,          // add main comment at archive creation
                                      // (use the comment buffer you
                                      // initialized at ACEInitDll())
                Solid,                // create solid archives
                Lock,                 // lock archive after creation
                RecoveryRecord,       // add recovery record after creation
                AV;                   // add AV to archive (if registered)
                                      // not used in ACL
          LONGLONG
                VolumeSize;           // ACE_VOLUME_NONE, ACE_VOLUME_AUTO
                                      // or any other value being treated
                                      // as absolute volume size in bytes
          LPSTR SFXName;              // set SFXName to ACE_SFX_NONE if
                                       // you do not want to add a SFX

          ULONG TimeSet;              // see ACE_SETTIME constants
          BOOL  SaveNTFSSecurity;     // save NTFS security data
          BOOL  SaveNTFSStreams;      // save NTFS streams
          CHAR  Reserved[56];         // has to be filled with zeroes
        } tACEAddStruc,
         *pACEAddStruc;


      //-----------------------------------------------------------------------
      // ACEAdd() function declaration.
      //-----------------------------------------------------------------------

        INT __stdcall ACEAdd(LPSTR ArchiveName, pACEAddStruc Add);


    //─────────────────────────  ACEReadSFXData  ──────────────────────────────
    // Retrieves information from a SFX module
    // (stored in *.SFX files or default SFX).
    //-------------------------------------------------------------------------

      //-----------------------------------------------------------------------
      // Constants to be used for the LanguageIndex field of
      // tACEReadSFXDataStruc.
      //-----------------------------------------------------------------------

        #define ACE_READSFXDATA_ENGLISH         0
        #define ACE_READSFXDATA_GERMAN          1
        #define ACE_READSFXDATA_DUTCH           2
        #define ACE_READSFXDATA_SPANISH         3
        #define ACE_READSFXDATA_RUSSIAN         4
        #define ACE_READSFXDATA_POLISH          5
        #define ACE_READSFXDATA_PORTUGUESE      6
        #define ACE_READSFXDATA_FINNISH         7
        #define ACE_READSFXDATA_ROMANIAN        8
        #define ACE_READSFXDATA_CZECH		9


      //-----------------------------------------------------------------------
      // ACEReadSFXData() parameter structure.
      //-----------------------------------------------------------------------

        typedef struct sACEReadSFXDataStruc
        {
          ULONG   LanguageIndex;        // specifies which language to read
                                        // out of the SFX Header (titles,
                                        // description)
          LPSTR   ShortTitle,           // returns short title of SFX module
                                        // (maxlen=9)
                  MediumTitle,          // returns medium title of SFX module
                                        // (maxlen=23)
                  LongTitle,            // returns long title of SFX module
                                        // (maxlen=45)
                  Description;          // returns description of SFX module
                                        // (maxlen=513)
          ULONG   Start;                // returns start of module within the
                                        // file
          ULONG   Size;                 // returns size of the module
          CHAR    Reserved[64];         // has to be filled with zeroes
        } tACEReadSFXDataStruc,
         *pACEReadSFXDataStruc;


      //-----------------------------------------------------------------------
      // ACEReadSFXData() function declaration.
      //-----------------------------------------------------------------------

        INT __stdcall ACEReadSFXData(LPSTR SFXName,
                                     pACEReadSFXDataStruc SFXData);


    //────────────────────────────  ACERepair  ────────────────────────────────
    // Recover files of a (broken) archive. If there is a recovery record -
    // it is very likely that the archive can be fully repaired.
    //-------------------------------------------------------------------------

      //-----------------------------------------------------------------------
      // ACERepair() parameter structure.
      //-----------------------------------------------------------------------

        typedef struct sACERepairStruc
        {
          LPSTR RepairedArchiveName;  // file name of repaired archive
          CHAR  Reserved[64];         // has to be filled with zeroes
        } tACERepairStruc,
         *pACERepairStruc;


      //-----------------------------------------------------------------------
      // ACERepair() function declaration.
      //-----------------------------------------------------------------------

        INT __stdcall ACERepair(LPSTR ArchiveName, pACERepairStruc Repair);


    //─────────────────────────  ACESetComments  ──────────────────────────────
    // Sets the comment(s) of an archive. If the FileList field has been set
    // to NULL or no file has been specified, the main comment is going to be
    // modified - otherwhise the comments of the specified files.
    // The comments may be changed at StateCallbackProc.
    //-------------------------------------------------------------------------

      //-----------------------------------------------------------------------
      // ACESetComments() parameter structure.
      //-----------------------------------------------------------------------

        typedef struct sACESetCommentsStruc
        {
          tACEFilesStruc              // files to set comments of;
                Files;                // see tACEFilesStruc structure;
                                      // if Files.FileList is NULL, then the
                                      // main comment will be modified
          ULONG TimeSet;              // see ACE_SETTIME constants
          CHAR  Reserved[64];         // has to be filled with zeroes
        } tACESetCommentsStruc,
         *pACESetCommentsStruc;


      //-----------------------------------------------------------------------
      // ACESetComments() function declaration.
      //-----------------------------------------------------------------------

        INT __stdcall ACESetComments(LPSTR ArchiveName,
                                     pACESetCommentsStruc SetComments);


    //─────────────────────────  ACEEncryptFiles  ─────────────────────────────
    // Encrypt specified files of archive using the given password.
    //-------------------------------------------------------------------------

      //-----------------------------------------------------------------------
      // ACEEncryptFiles() parameter structure.
      //-----------------------------------------------------------------------

        typedef struct sACEEncryptFilesStruc
        {
          tACEFilesStruc              // files to encrypt/decrypt
                Files;                // see tACEFilesStruc structure;
          LPSTR EncryptPassword;      // if empty, the files will be
                                      // decrypted (if they were encrypted)
                                      // (maxlen=56)
          LPSTR DecryptPassword;      // already encrypted files will have
                                      // to be decrypted before being
                                      // encrypted with the new password
          ULONG TimeSet;              // see ACE_SETTIME constants
          CHAR  Reserved[64];         // has to be filled with zeroes
        } tACEEncryptFilesStruc,
         *pACEEncryptFilesStruc;


      //-----------------------------------------------------------------------
      // ACEEncryptFiles() function declaration.
      //-----------------------------------------------------------------------

        INT __stdcall ACEEncryptFiles(LPSTR ArchiveName,
                                      pACEEncryptFilesStruc EncryptFiles);


    //───────────────────────────  ACEAddSFX  ─────────────────────────────────
    // Add SFX module to archive.
    //-------------------------------------------------------------------------

      //-----------------------------------------------------------------------
      // ACEAddSFX() parameter structure.
      //-----------------------------------------------------------------------

        typedef struct sACEAddSFXStruc
        {
          LPSTR SFXName;              // set SFXName to ACE_SFX_DEFAULT if
                                      // you want to add the default SFX
                                      // module
          LPSTR SFXArchiveName;       // returns the new name of the archive
                                      // (should be *.exe)
          ULONG TimeSet;              // see ACE_SETTIME constants
          CHAR  Reserved[64];         // has to be filled with zeroes
        } tACEAddSFXStruc,
         *pACEAddSFXStruc;


      //-----------------------------------------------------------------------
      // ACEAddSFX() function declaration.
      //-----------------------------------------------------------------------

        INT __stdcall ACEAddSFX(LPSTR ArchiveName,
                                pACEAddSFXStruc AddSFXModule);


    //────────────────────────────  ACELock  ──────────────────────────────────
    // Lock archiv, so that it can not be modified any more.
    //-------------------------------------------------------------------------

      //-----------------------------------------------------------------------
      // ACELock() parameter structure.
      //-----------------------------------------------------------------------

        typedef struct sACELockStruc
        {
          ULONG TimeSet;              // see ACE_SETTIME constants
          CHAR  Reserved[64];         // has to be filled with zeroes
        } tACELockStruc,
         *pACELockStruc;


      //-----------------------------------------------------------------------
      // ACELock() function declaration.
      //-----------------------------------------------------------------------

        INT __stdcall ACELock(LPSTR ArchiveName, pACELockStruc Lock);


    //──────────────────────  ACEAddRecoveryRecord  ───────────────────────────
    // Adds recovery record to archive.
    //-------------------------------------------------------------------------

      //-----------------------------------------------------------------------
      // ACEAddRecoveryRecord() parameter structure.
      //-----------------------------------------------------------------------

        typedef struct sACEAddRecoveryRecordStruc
        {
          ULONG TimeSet;              // see ACE_SETTIME constants
          CHAR  Reserved[64];         // has to be filled with zeroes
        } tACEAddRecoveryRecordStruc,
         *pACEAddRecoveryRecordStruc;


      //-----------------------------------------------------------------------
      // ACEAddRecoveryRecord() function declaration.
      //-----------------------------------------------------------------------

        INT __stdcall ACEAddRecoveryRecord(LPSTR ArchiveName,
                        pACEAddRecoveryRecordStruc AddRecoveryRecord);


    //────────────────────────────  ACEAddAV  ─────────────────────────────────
    // Add AV to archive. (not supported in ACL)
    //-------------------------------------------------------------------------

      //-----------------------------------------------------------------------
      // ACEAddAV() parameter structure.
      //-----------------------------------------------------------------------

        typedef struct sACEAddAVStruc
        {
          ULONG TimeSet;              // see ACE_SETTIME constants
          CHAR  Reserved[64];         // has to be filled with zeroes
        } tACEAddAVStruc,
         *pACEAddAVStruc;


      //-----------------------------------------------------------------------
      // ACEAddAV() function declaration.
      //-----------------------------------------------------------------------

        INT __stdcall ACEAddAV(LPSTR ArchiveName, pACEAddAVStruc AddAV);


    //──────────────────────────  ACERegister  ────────────────────────────────
    // Registers AceV2.Dll. Program has to tell the 27digit-registration-code
    // first. If the key is not a general but a user-specific one, the name
    // of the user and his AV will be retrieved via RequestCallbackProc
    // (with ACE_CALLBACK_REQUEST_REGISTER). The key will be saved in the
    // keyfile specified in ACEGlobalDataStruc. (not supported in ACL)
    //-------------------------------------------------------------------------

      //-----------------------------------------------------------------------
      // ACERegister() parameter structure.
      //-----------------------------------------------------------------------

        typedef struct sACERegisterStruc
        {
          CHAR  RegCode[33];          // registration code in the format
                                      // "ACEaaaabbbbccccdddd"
          CHAR  Reserved[64];         // has to be filled with zeroes
        } tACERegisterStruc,
         *pACERegisterStruc;


      //-----------------------------------------------------------------------
      // ACERegister() function declaration.
      //-----------------------------------------------------------------------

        INT __stdcall ACERegister(pACERegisterStruc Register);


    //──────────────────────────  ACEShowRegisterDialog  ──────────────────────
    // Registers AceV2.Dll. Program has to tell the 19digit-registration-code
    // first. If the key is not a general but a user-specific one, the name
    // of the user and his AV will be retrieved via RequestCallbackProc
    // (with ACE_CALLBACK_REQUEST_REGISTER). The key will be saved in the
    // keyfile specified in ACEGlobalDataStruc. (not supported in ACL)
    //-------------------------------------------------------------------------

      //-----------------------------------------------------------------------
      // ACERegister() function declaration.
      //-----------------------------------------------------------------------

        INT __stdcall ACEShowRegisterDialog(void);


#ifdef __cplusplus
 }
#endif /* __cplusplus */

#endif /* _ACEFNC_H */
