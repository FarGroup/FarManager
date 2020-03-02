#include <all_far.h>
#pragma hdrstop

#include "Int.h"

//---------------------------------------------------------------------------------
FTPCmdBlock::FTPCmdBlock(FTP *c,int block)
{
	Handle = c;
	hVis   = -1;
	Block(block);
}
FTPCmdBlock::~FTPCmdBlock()
{
	Reset();
}
void FTPCmdBlock::Block(int block)
{
	if(Handle && Handle->hConnect && block != -1)
		hVis = FtpCmdBlock(Handle->hConnect,block);
}
void FTPCmdBlock::Reset(void)
{
	Block(hVis);
	hVis = -1;
}
