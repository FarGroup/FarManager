#pragma once

struct DLNGHeader {
	char ID[5];
	unsigned char Version;
	char PrgID[2];
	unsigned char PrgVER;
};	

struct DLNGHeaderV4 {
	char ID[5];
	unsigned char Version;
	char PrgID[2];
	unsigned char PrgVER;
	int IndexOffset;
	int IndexNum;
	int IndexCRC;
	int DataOffset;
	int DataSize;
	int DataCSize;
	int DataCRC;

	//  0 = None (Default & Faster)
	//  1 = ZLib
	unsigned char Compression; 

	//  0 = Not available
	// 40 = DLNGC v4.0
	unsigned char Manifacturer;

	int ExtendedHeaderSize;
	int ExtendedHeaderCRC;
	int IconSize;
};	

struct DLNGHeaderV3 {
	char ID[5];
    unsigned char Version;
    char PrgID[2];
    unsigned char PrgVER;
    int IndexOffSet;
    int IndexNum;
    unsigned short FileSize;
    int FileCRC;

    //  0 = Not available
    // 30 = DLNGC v3.0
    unsigned char Manufacturer;

    int ExtendedHeaderSize;
    int ExtendedHeaderCRC;
    int IconSize;
};

struct DLNGExtendedHeader { 
/*
     Name: WideString;
     Author: WideString;
     URL: WideString;
     Email: WideString;
     FontName: WideString;
*/
}; //упоротый заголовок

struct DLNGIndexEntryV3 {
	char ID[6];
	unsigned short Length;
};

struct DLNGIndexEntryV4 {
	char ID[6];
	int Offset;
	unsigned short Length;
};
