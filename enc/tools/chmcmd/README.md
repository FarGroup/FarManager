Slightly modified version of chmcmd from freepascal https://github.com/graemeg/freepascal

Original version always generates CHM files marked as English US (lcid 0x0409, codepage 1252) - this version generates CHM files marked as Russian (lcid 0x0419, codepage 1251).

MS hhc.exe always generates CHM files with current user locale (which we can't control when building in CI environment) which causes the russian Encyclopaedia index tab to look garbled if the build machine locale is not Russian.

FPC diff below:

    diff --git a/packages/chm/src/chmfiftimain.pas b/packages/chm/src/chmfiftimain.pas
    index c535a6f729..c7c544b922 100644
    --- a/packages/chm/src/chmfiftimain.pas
    +++ b/packages/chm/src/chmfiftimain.pas
    @@ -311,7 +311,7 @@ begin
       FHeaderRec.TotalWords := FWordList.TotalDIfferentWords;
       FHeaderRec.TotalWordsLengthPart1 := FWordList.TotalWordLength;
       FHeaderRec.TotalWordsLength := FWordList.TotalDifferentWordLength;
    -  FHeaderRec.WindowsCodePage := 1252;
    +  FHeaderRec.WindowsCodePage := 1251;

       FStream.Position := 0;

    @@ -349,7 +349,7 @@ begin
       FStream.WriteDWord(NtoLE(FHeaderRec.HTMLFilesCount-1));
       for i := 0 to 23 do FStream.WriteByte(0);
       FStream.WriteDWord(NtoLE(FHeaderRec.WindowsCodePage));
    -  FStream.WriteDWord(NtoLE(DWord(1033))); // LCID
    +  FStream.WriteDWord(NtoLE(DWord(1037))); // LCID
       for i := 0 to 893 do FStream.WriteByte(0);
     end;

    diff --git a/packages/chm/src/chmwriter.pas b/packages/chm/src/chmwriter.pas
    index 7950c5ca58..96a193606b 100644
    --- a/packages/chm/src/chmwriter.pas
    +++ b/packages/chm/src/chmwriter.pas
    @@ -267,7 +267,7 @@ begin
         HeaderLength := NToLE(DWord(SizeOf(TITSFHeader) + (SizeOf(TGuid)*2)+ (SizeOf(TITSFHeaderEntry)*2) + SizeOf(TITSFHeaderSuffix)));
         Unknown_1 := NToLE(DWord(1));
         TimeStamp:= NToBE(MilliSecondOfTheDay(Now)); //bigendian
    -    LanguageID := NToLE(DWord($0409)); // English / English_US
    +    LanguageID := NToLE(DWord($0419)); // English / English_US
       end;
     end;

    @@ -312,7 +312,7 @@ begin

         Unknown2 := NToLE(Longint(-1));
         //DirectoryChunkCount: LongWord;
    -    LanguageID := NToLE(DWord($0409));
    +    LanguageID := NToLE(DWord($0419));
         GUID := ITSPHeaderGUID;
         LengthAgain := NToLE(DWord($54));
         Unknown3 := NToLE(Longint(-1));
    @@ -984,7 +984,7 @@ begin
       FSection0.WriteWord(NToLE(Word(4)));
       FSection0.WriteWord(NToLE(Word(36))); // size

    -  FSection0.WriteDWord(NToLE(DWord($0409)));
    +  FSection0.WriteDWord(NToLE(DWord($0419)));
       FSection0.WriteDWord(0);
       FSection0.WriteDWord(NToLE(DWord(Ord(FFullTextSearch and FFullTextSearchAvailable))));

    @@ -1290,8 +1290,8 @@ begin

       ObjStream.WriteDWord(NtoLE($04000000));
       ObjStream.WriteDWord(NtoLE(11));  // bit flags
    -  ObjStream.WriteDWord(NtoLE(DWord(1252)));
    -  ObjStream.WriteDWord(NtoLE(DWord(1033)));
    +  ObjStream.WriteDWord(NtoLE(DWord(1251)));
    +  ObjStream.WriteDWord(NtoLE(DWord(1049)));
       ObjStream.WriteDWord(NtoLE($00000000));
       ObjStream.WriteDWord(NtoLE($00000000));
       ObjStream.WriteDWord(NtoLE($00145555));
    @@ -1346,8 +1346,8 @@ begin

       ObjStream.WriteDWord(NtoLE($04000000));
       ObjStream.WriteDWord(NtoLE(DWord(1)));
    -  ObjStream.WriteDWord(NtoLE(DWord(1252)));
    -  ObjStream.WriteDWord(NtoLE(DWord(1033)));
    +  ObjStream.WriteDWord(NtoLE(DWord(1251)));
    +  ObjStream.WriteDWord(NtoLE(DWord(1049)));
       ObjStream.WriteDWord(NtoLE(DWord(0)));

       // second entry
    @@ -1364,8 +1364,8 @@ begin
       ObjStream.WriteByte($66);

       ObjStream.WriteDWord(NtoLE(DWord(666))); // not kidding
    -  ObjStream.WriteDWord(NtoLE(DWord(1252)));
    -  ObjStream.WriteDWord(NtoLE(DWord(1033)));
    +  ObjStream.WriteDWord(NtoLE(DWord(1251)));
    +  ObjStream.WriteDWord(NtoLE(DWord(1049)));
       ObjStream.WriteDWord(NtoLE(DWord(10031)));
       ObjStream.WriteDWord(NtoLE(DWord(0)));

    @@ -2348,8 +2348,8 @@ begin
       hdr.nrblock        :=NToLE(blocknr);      // Number of blocks
       hdr.treedepth      :=NToLE(word(TreeDepth));    // The depth of the tree of blocks (1 if no index blocks, 2 one level of index blocks, ...)
       hdr.nrkeywords     :=NToLE(Totalentries); // number of keywords in the file.
    -  hdr.codepage       :=NToLE(dword(1252));         // Windows code page identifier (usually 1252 - Windows 3.1 US (ANSI))
    -  hdr.lcid           :=NToLE(0);            //  ???? LCID from the HHP file.
    +  hdr.codepage       :=NToLE(dword(1251));         // Windows code page identifier (usually 1252 - Windows 3.1 US (ANSI))
    +  hdr.lcid           :=NToLE(dword(1049));            //  ???? LCID from the HHP file.
       if not chw then
         hdr.ischm        :=NToLE(dword(1))             // 0 if this a BTREE and is part of a CHW file, 1 if it is a BTree and is part of a CHI or CHM file
       else
    @@ -2470,4 +2470,3 @@ begin
     end;

     end.
    -
