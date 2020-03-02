#ifndef _STRUCS_H
#define _STRUCS_H

#ifdef __cplusplus
  extern "C" {
#endif /* __cplusplus */

//═════════════════════════════════════════════════════════════════════════════
//════════════════=====---  Part 1: different structures ---====═══════════════
//═════════════════════════════════════════════════════════════════════════════

  //---------------------------------------------------------------------------
  //
  // Herein different structures used at callback functions and
  // AceV2.Dll functions are declared.
  //
  //---------------------------------------------------------------------------
  //
  // Contents:
  //   Part 1.1: structures used in callback structures
  //   Part 1.2: structures used in function structures
  //
  //---------------------------------------------------------------------------


  //═══════════════════════════════════════════════════════════════════════════
  //========---  Part 1.1: structures used in callback structures  ---=========
  //═══════════════════════════════════════════════════════════════════════════

    //───────────────────────  comment buffer structure  ──────────────────────
    // Used in tACEGlobalDataStruc. Application has to specify where the
    // comment is or should be stored.
    //-------------------------------------------------------------------------

        typedef struct sACECommentStruc
        {
          LPSTR Buf;                  // area where the comments are stored
                                      // by the application or by the Dll
                                      // comments must be zero-terminated
          ULONG BufSize;              // size of this area
          ULONG State;                // state of comment - see ACE_COMMENT
                                      // constants below
        } tACECommentStruc,
         *pACECommentStruc;


      //-----------------------------------------------------------------------
      // Defines for the tACECommentStruc.State field, which tells about
      // the result of the last comment extraction.
      //-----------------------------------------------------------------------

        #define ACE_COMMENT_OK          0    // comment extraction went fine

        #define ACE_COMMENT_SMALLBUF    1    // comment buffer too small to
                                             // store the whole comment in

        #define ACE_COMMENT_NONE        255  // No comment present


    //───────────────────────  global data structure  ─────────────────────────
    // This structure contains information for the Dll being interesting for
    // nearly all functions. The Dll has to be initialized with this
    // structure passed to tACEInitDll(). This structure is also passed
    // by the callback functions.
    //-------------------------------------------------------------------------

              //---------------------------------------------------------------
              // Forward declarations of callback function parameters
              // used in this structure.
              //---------------------------------------------------------------

                typedef union sACEInfoCallbackProcStruc
                                       *pACEInfoCallbackProcStruc;

                typedef union sACEErrorCallbackProcStruc
                                       *pACEErrorCallbackProcStruc;

                typedef union sACERequestCallbackProcStruc
                                       *pACERequestCallbackProcStruc;

                typedef union sACEStateCallbackProcStruc
                                       *pACEStateCallbackProcStruc;


        typedef struct sACEGlobalDataStruc
        {
        //------------------------ fields for ACE and UnACE -------------------

          void  *Obj;           // ---- reserved for application! ----
                                // thought to be used as a pointer to
                                // an object; when a callback-function is
                                // called, the object pointer can be used to
                                // handle the callback in a specific way;
                                // the pointer has to be initialized by
                                // ACEInitDll()

          ULONG MaxArchiveTestBytes;

                                // how many bytes of a file should be
                                // looked upon at archive header search?

          ULONG MaxFileBufSize;

                                // maximum buffer size for buffered
                                // I/O operations

          tACECommentStruc      // used to exchange comment data
                Comment;        // between application and Dll
                                // using callback functions

          LPSTR DecryptPassword;

                                // the DecryptPassword specified at
                                // ACEInitDll() is overwritten by the
                                // DecryptPassword field of tACEAddStruc and
                                // other function-specific structures;
                                // but this field can be used to change the
                                // password at callback function calls

          BOOL UseVBStructures; //  passes structures to callback functions
          			//  much better suited for Visual Basic

          CHAR  Reserved1[60];  // has to be filled with zeros

        //--------------------------- fields for ACE only ---------------------

          LPSTR EncryptPassword;

                                // things stated at the description of the
                                // DecryptPassword field apply here as well


          LPSTR TempDir;        // directory to save temporary archive

        //-------- registration (ACE DLLs only, but not used at ACL) ----------

          LPSTR KeyPath;        // necessary for reading and writing key file

          LPSTR UserAV;         // Dll returns the AV string (if registered)
                                // in this field

          LPSTR IsGeneralKey;   // DLL returns the key, if it is a general key

          HWND  OwnerWindow;    // specifies the applications window to be
                                // parent of the registration reminder dialog

        //--------------------------- fields for ACE only ---------------------

          ULONG CompressionLevel;
                                // contains the currently used compression
                                // level - may be changed during compression
                                // operation

          CHAR  Reserved2[56];  // has to be filled with zeros


        //----------------------  callback functions (ACE and UnACE) ----------

          INT (__stdcall *InfoCallbackProc)
                          (pACEInfoCallbackProcStruc Info);

                                //  Informs application about details like the
                                //  creation of a temporary archive.
                                //  See ACE_CALLBACK_INFO definitions for
                                //  more information.

          INT (__stdcall *ErrorCallbackProc)
                           (pACEErrorCallbackProcStruc Error);

                                //  Called to let the application know about
                                //  errors. The Code field of Error will
                                //  contain an ACE_CALLBACK_ERROR value.

          INT (__stdcall *RequestCallbackProc)
                           (pACERequestCallbackProcStruc Request);

                                //  Called to let the application know about
                                //  requests for data. Code will contain an
                                //  ACE_CALLBACK_REQUEST value.

          INT (__stdcall *StateCallbackProc)
                           (pACEStateCallbackProcStruc State);

                                //  Called to let the application know about
                                //  the progress of operations.

        //-------------------------- different new fields ---------------------
          CHAR  Reserved3[64];  // has to be filled with zeros
        } tACEGlobalDataStruc,
         *pACEGlobalDataStruc;


    //───────────────────────  archive data structure  ────────────────────────
    // Informs the callback functions about the current archive, its volume
    // number, the archive-flags (see ACE_FLAG constants), the creation host
    // system (see ACE_HOST constants) and the AV if present in archive.
    // Also used at ACEReadArchiveData().
    //-------------------------------------------------------------------------

        typedef struct sACEArchiveDataStruc
        {
          LPSTR ArchiveName;
          ULONG VolumeNumber,
                Flags,          // see ACE_ARCFLAG defines below
                HostCreated,    // see ACE_HOST defines below
                TimeCreated,    // in MS-DOS format
                VersionCreated,
                VersionExtract; // version needed to extract files
          LPSTR AV;             // not used in ACL
          CHAR  Reserved[64];   // filled with zeros
        } tACEArchiveDataStruc,
         *pACEArchiveDataStruc;


      //-----------------------------------------------------------------------
      // Flag constants for tACEArchiveDataStruc.Flags field.
      //-----------------------------------------------------------------------

        #define ACE_ARCFLAG_MAINCOMMENT         2
        #define ACE_ARCFLAG_SFX                 512
        #define ACE_ARCFLAG_LIMITSFXJR          1024  // is an SFX archive
                                                      // that supports 256k
                                                      // dictionary only
        #define ACE_ARCFLAG_MULTIVOLUME         2048
        #define ACE_ARCFLAG_AV                  4096  // not used in ACL
        #define ACE_ARCFLAG_RECOVERYREC         8192
        #define ACE_ARCFLAG_LOCK                16384
        #define ACE_ARCFLAG_SOLID               32768


      //-----------------------------------------------------------------------
      // Host system used to create an archive. Used at
      // tACEArchiveDataStruc.HostCreated field.
      //-----------------------------------------------------------------------

        #define ACE_HOST_MSDOS                  0     // archive created by
                                                      // MSDOS ACE archiver
        #define ACE_HOST_OS2                    1     // created by OS2 ACE
        #define ACE_HOST_WIN32                  2     // created by Win32 ACE


    //────────────────────────  file data structure  ──────────────────────────
    // Contains information about an archived file.
    //-------------------------------------------------------------------------

        typedef struct sACEFileDataStruc
        {
          LPSTR SourceFileName;      // relative file name
          LPSTR DestinationFileName; // absolute file name;
                                     // valid for add and extract only!
          ULONG Flags,               // see ACE_FILEFLAG defines below
                CRC32,
                Method,              // 0=stored, 1=LZ77, 2=V20Compression
                Dictionary;          // DictionarySize = 2^Dictionary
          DWORDLONG
                CompressedSize,
                Size;
          ULONG Time,
                Attributes;
          CHAR  Reserved[64];        // filled with zeros
        } tACEFileDataStruc,
         *pACEFileDataStruc;


      //-----------------------------------------------------------------------
      // Flag constants for the tACEFileData.Flags field.
      //-----------------------------------------------------------------------

        #define ACE_FILEFLAG_FILECOMMENT        2     // file has comment
        #define ACE_FILEFLAG_SPLITBEFORE        4096  // continued from
                                                      // previous volume
        #define ACE_FILEFLAG_SPLITAFTER         8192  // continued on
                                                      // next volume
        #define ACE_FILEFLAG_PASSWORD           16384 // is encrypted
        #define ACE_FILEFLAG_SOLID              32768 // uses data of previous
                                                      // files (solid mode)


    //────────────────────────  copy info structure  ──────────────────────────
    // Is passed to ACEInfoCallbackProc with ACE_CALLBACK_INFO_COPY as code.
    // Informs application about the progress of copying either an archive to
    // a temporary archive, or a temporary archive back to a normal archive.
    //-------------------------------------------------------------------------

        typedef struct sACECopyInfoStruc
        {
          LPSTR SourceFileName,      // source file
                DestinationFileName; // the file copying the source to
          DWORDLONG
                CopiedBytes,         // bytes already copied
                FileSize;            // source file size
          CHAR  Reserved[64];        // filled with zeros
        } tACECopyInfoStruc,
         *pACECopyInfoStruc;


    //────────────────────  operation progress structure  ─────────────────────
    // Used to state the progress of the current operation.
    //-------------------------------------------------------------------------

        typedef struct sACEProgressDataStruc
        {
          LPSTR     Addr;               // address of uncompressed data block
          ULONG     Size;               // size of uncompressed data block
          DWORDLONG TotalProcessedSize; // counted by Dll:
                                        // total uncompressed bytes processed
          DWORDLONG TotalCompressedSize;// total compressed bytes processed
          DWORDLONG TotalSize;          // total uncompressed bytes to process
                                        // (sum of all original file sizes)
          DWORDLONG FileProcessedSize;  // uncompr. bytes of file processed
          DWORDLONG FileCompressedSize; // compr. bytes of file processed
          DWORDLONG FileSize;           // uncompressed file size
        } tACEProgressDataStruc,
         *pACEProgressDataStruc;


  //═══════════════════════════════════════════════════════════════════════════
  //========---  Part 1.2: structures used in function structures  ---=========
  //═══════════════════════════════════════════════════════════════════════════

    //─────────────────────────  file list structure  ─────────────────────────
    // This structure is used in the function specific structures.
    // The application has to use this structure to indicate which files
    // have to be processed by the DLL.
    //-------------------------------------------------------------------------

        typedef struct sACEFilesStruc
        {
          LPSTR SourceDir;            // maybe a real or an archive directory
          LPSTR FileList;             // pointer to list of files to process;
                                      // zero-terminated; files have to be
                                      // separated by carriage-return (0xd);
                                      // FileList may/will be modified by the
                                      // Dll; if nothing is specified, "*"
                                      // will be used
                                      // (attention at ACEDelete!!)
          LPSTR ExcludeList;          // list of files to exclude from process
          BOOL  FullMatch;            // specifications must fully match
                                      // (no files with the same name in
				      //  subdirs are processed)
          BOOL  RecurseSubDirs;       // recurse subdirectories
           			      // (valid for add operations only)
          CHAR  Reserved[60] ;         // has to be filled with zeros
                                      // for future: possibly in-/excluding
                                      // file attributes and date/time range
        } tACEFilesStruc,
         *pACEFilesStruc;


    //─────────────────────  V2.0 compression structure ───────────────────────
    // Specifies whether to use v2.0 compression or not. If you use v2.0
    // compression you can also specify which v2.0 compression techniques
    // you want to use. (ACE only)
    //-------------------------------------------------------------------------

        typedef struct sACEV20CompressionStruc
        {
          BOOL  DoUse,                // if DoUse=1 and all other fields are
                DoUseDelta,           // zero, then all v2.0 compression
                DoUseExe,             // techniques will be used
                DoUsePic,
                DoUseSound;
          CHAR  Reserved[64];         // has to be filled with zeros
        } tACEV20CompressionStruc,
         *pACEV20CompressionStruc;


    //───────────────────  compression parameter structure  ───────────────────
    // Used in tACEAddStruc and tACEDeleteStruc. (ACE only)
    //-------------------------------------------------------------------------

        typedef struct sACECompressParamsStruc
        {
          ULONG Level,                // see ACE_LEVEL constants below
                Dictionary;           // 15(32k)..22(4Mb)

          tACEV20CompressionStruc     // indicates if (or which) v2.0
                V20Compression;       // compression techniques shall be used

          BOOL  TestAfter;            // make a test for CRC check errors
                                      // after compression
          CHAR  Reserved[64];         // has to be filled with zeros
        } tACECompressParamsStruc,
         *pACECompressParamsStruc;


      //-----------------------------------------------------------------------
      // Tells the Dll which compression level to use. (ACE only)
      //-----------------------------------------------------------------------

        #define ACE_LEVEL_STORE     0 // save file only; do not compress
        #define ACE_LEVEL_FASTEST   1 // compress very fast
        #define ACE_LEVEL_FAST      2 // compress fast
        #define ACE_LEVEL_NORMAL    3 // good compromise between speed and
                                      // compression rate
        #define ACE_LEVEL_GOOD      4 // achieves good compression
        #define ACE_LEVEL_BEST      5 // best compression; bit slow


#ifdef __cplusplus
 }
#endif /* __cplusplus */

#endif /* _STRUCS_H */
