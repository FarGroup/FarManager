#pragma once
#include "newarc.h"

class ArchiveItemArray : public Array<ArchiveItem>
{
public:

	ArchiveItemArray(int delta = DEFAULT_ARRAY_DELTA) : Array<ArchiveItem>(100) { };
};

class ArchivePItemArray : public Array<ArchiveItem*>
{
public:

	ArchivePItemArray(int delta = DEFAULT_ARRAY_DELTA) : Array<ArchiveItem*>(100) { };
};

