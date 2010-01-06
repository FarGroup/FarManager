#pragma once
#include "newarc.h"

class ArchiveItemArray : public Array<ArchiveItem>
{
public:

	ArchiveItemArray(int delta = DEFAULT_ARRAY_DELTA) : Array<ArchiveItem>(100) { };
};

