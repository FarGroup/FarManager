typedef int (*DETECTARCHIVE)(const unsigned char *buffer, int size);

int IsTarHeader  (const unsigned char *Data, int DataSize);
int IsRarHeader  (const unsigned char *Data, int DataSize);
int IsZipHeader  (const unsigned char *Data, int DataSize);
int IsArjHeader  (const unsigned char *Data, int DataSize);
int IsCabHeader  (const unsigned char *Data, int DataSize);
int IsLzhHeader  (const unsigned char *Data, int DataSize);
int Is7zHeader   (const unsigned char *Data, int DataSize);
int IsNSISHeader (const unsigned char *Data, int DataSize);
int IsIsoHeader  (const unsigned char *Data, int DataSize);
int IsUdfHeader  (const unsigned char *Data, int DataSize);
int IsPEHeader   (const unsigned char *Data, int DataSize);
int IsMachoHeader(const unsigned char *Data, int DataSize);
int IsHfsHeader  (const unsigned char *Data, int DataSize);
int IsLzmaHeader (const unsigned char *Data, int DataSize);
