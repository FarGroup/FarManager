#ifndef _UNACEFNC_H
#define _UNACEFNC_H

#ifdef __cplusplus
  extern "C" {
#endif /* __cplusplus */

//════════════════════════════════════════════════════════════════════════════════
//════════=====---  Part 3: Functions supported by UNAceV2.Dll ---====════════════
//════════════════════════════════════════════════════════════════════════════════

  //---------------------------------------------------------------------------
  //
  // UnAceV2.Dll supports the following functions:
  //
  //   ACEInitDll
  //   ACEReadArchiveData
  //   ACEList
  //   ACETest
  //   ACEExtract
  //
  // First of all - before any other function is called - the Dll has to be
  // initialized by ACEInitDll(). Using this function the application has
  // to set temporary directory, key file path, comment buffer and
  // callback function pointers.
  // (callback function pointers can be set to NULL for first-try-runs)
  //
  // See description of each function for details about their task, input and
  // output. Return codes of those functions are listed in part 3.1.
  //
  //---------------------------------------------------------------------------
  //
  // Contents:
  //   Part 3.1: UnAceV2.Dll function return codes
  //   Part 3.2: functions and parameter structures
  //
  //---------------------------------------------------------------------------


  //═══════════════════════════════════════════════════════════════════════════
  //===========---  Part 3.1: UnAceV2.Dll function return codes  ---===========
  //═══════════════════════════════════════════════════════════════════════════

      //-----------------------------------------------------------------------
      // These error codes are returned by the UnAceV2.Dll-functions. The meanings
      // of the codes are the same, as they are for the exit codes of ACE.EXE.
      //-----------------------------------------------------------------------

        #define ACE_ERROR_NOERROR       0   // no error; operation succesful
        #define ACE_ERROR_MEM           1   // insufficient memory
        #define ACE_ERROR_FILES         2   // no files specified
        #define ACE_ERROR_FOUND         3   // specified archive not found
        #define ACE_ERROR_FULL          4   // disk full
        #define ACE_ERROR_OPEN          5   // could not open file
        #define ACE_ERROR_READ          6   // read error
        #define ACE_ERROR_WRITE         7   // write error
        #define ACE_ERROR_CLINE         8   // invalid command line
        #define ACE_ERROR_CRC           9   // CRC error
        #define ACE_ERROR_OTHER         10  // other error
        #define ACE_ERROR_EXISTS        11  // file already exists
        #define ACE_ERROR_USER          255 // user break (application
                                            // returned cancel code at
                                            // callback function)


      //-----------------------------------------------------------------------
      // These error codes are returned by the UnAceV2.Dll-functions. They are not
      // used by ACE.EXE yet.
      //-----------------------------------------------------------------------

        #define ACE_ERROR_PARAM       128 // might be used later


  //═══════════════════════════════════════════════════════════════════════════
  //============---  Part 3.2: functions and parameter structures ---==========
  //═══════════════════════════════════════════════════════════════════════════

    //───────────────────────────  ACEInitDll  ────────────────────────────────
    // Initializes ACE dynamic link library. Has to be called before any
    // other function call. May be called more than one time.
    //-------------------------------------------------------------------------
    // Input :
    // Output:
    //-------------------------------------------------------------------------

      //-----------------------------------------------------------------------
      // ACEInitDll() parameter structure.
      //-----------------------------------------------------------------------

        typedef struct sACEInitDllStruc
        {
          tACEGlobalDataStruc
                GlobalData;
          CHAR  Reserved[64];         // has to be filled with zeroes
        } tACEInitDllStruc,
         *pACEInitDllStruc;


      //-----------------------------------------------------------------------
      // ACEInitDll() function declaration.
      //-----------------------------------------------------------------------

        INT __stdcall ACEInitDll(pACEInitDllStruc DllData);


    //───────────────────────  ACEReadArchiveData  ────────────────────────────
    // Tests a file whether it is an archive or not and reads out the archive
    // data.
    //-------------------------------------------------------------------------

      //-----------------------------------------------------------------------
      // ACEReadArchiveData() parameter structure.
      //-----------------------------------------------------------------------

        typedef struct sACEReadArchiveDataStruc
        {
          pACEArchiveDataStruc        // if this pointer is NULL, the
                ArchiveData;          // file passed to ACEReadArchiveData
                                      // is no archive; otherwise it points
                                      // to a tACEArchiveDataStruc structure
                                      // that contains information about the
                                      // archive
          CHAR  Reserved[64];         // has to be filled with zeroes
        } tACEReadArchiveDataStruc,
         *pACEReadArchiveDataStruc;


      //-----------------------------------------------------------------------
      // ACEReadArchiveData() function declaration.
      //-----------------------------------------------------------------------

        INT __stdcall ACEReadArchiveData(LPSTR ArchiveName,
                                         pACEReadArchiveDataStruc ArchiveData);


    //────────────────────────────  ACEList  ──────────────────────────────────
    // Passes the specified files in the archive to StateCallbackProc().
    //-------------------------------------------------------------------------

      //-----------------------------------------------------------------------
      // ACEList() parameter structure.
      //-----------------------------------------------------------------------

        typedef struct sACEListStruc
        {
          tACEFilesStruc              // specifies files to be listed;
                Files;                // see tACEFilesStruc structure
          CHAR  Reserved[64];         // has to be filled with zeroes
        } tACEListStruc,
         *pACEListStruc;


      //-----------------------------------------------------------------------
      // ACEList() function declaration.
      //-----------------------------------------------------------------------

        INT __stdcall ACEList(LPSTR ArchiveName, pACEListStruc List);


    //────────────────────────────  ACETest  ──────────────────────────────────
    // Tests specified files in archive.
    //-------------------------------------------------------------------------

      //-----------------------------------------------------------------------
      // ACETest() parameter structure.
      //-----------------------------------------------------------------------

        typedef struct sACETestStruc
        {
          tACEFilesStruc              // specifies files to test;
                Files;                // see tACEFilesStruc structure
          LPSTR DecryptPassword;      // zero-terminated string,
                                      // case-sensitive (maxlen=56)
          CHAR  Reserved[64];         // has to be filled with zeroes
        } tACETestStruc,
         *pACETestStruc;


      //-----------------------------------------------------------------------
      // ACETest() function declaration.
      //-----------------------------------------------------------------------

        INT __stdcall ACETest(LPSTR ArchiveName, pACETestStruc Test);


    //───────────────────────────  ACEExtract  ────────────────────────────────
    // Extracts specified  files.
    //-------------------------------------------------------------------------

      //-----------------------------------------------------------------------
      // ACEExtract() parameter structure.
      //-----------------------------------------------------------------------

        typedef struct sACEExtractStruc
        {
          tACEFilesStruc                // specifies files to extract;
                  Files;                // see tACEFilesStruc structure
          LPSTR   DestinationDir;       // directory to extract files to
          BOOL    ExcludePath;          // extract files without path
          LPSTR   DecryptPassword;      // password for decryption (if files
                                        // are encrypted);
                                        // zero-terminated string,
                                        // case-sensitive (maxlen=56)
          CHAR    Reserved[64];         // has to be filled with zeroes
        } tACEExtractStruc,
         *pACEExtractStruc;


      //-----------------------------------------------------------------------
      // ACEExtract() function declaration.
      //-----------------------------------------------------------------------

        INT __stdcall ACEExtract(LPSTR ArchiveName,
                                 pACEExtractStruc Extract);


#ifdef __cplusplus
 }
#endif /* __cplusplus */

#endif /* _UNACEFNC_H */
