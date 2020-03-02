#include "newarc.h"

LONG_PTR __stdcall ArchiveCallbackThunk(int nMsg, int nParam, LONG_PTR nParam2)
{
	Archive *p = (Archive*)THUNK_MAGIC;
	return p->ArchiveCallback(nMsg, nParam, nParam2);
}

const char * __stdcall GetMsgThunk (int nModuleNumber, int nID)
{
	ArchivePlugin *p = (ArchivePlugin*)THUNK_MAGIC;
	return p->pGetMsg(nModuleNumber, nID);
}