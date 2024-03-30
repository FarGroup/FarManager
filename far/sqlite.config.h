#ifndef SQLITE_CONFIG_H_C8BF39A5_DAD7_4B50_BAE1_04F174FA5302
#define SQLITE_CONFIG_H_C8BF39A5_DAD7_4B50_BAE1_04F174FA5302
#pragma once

/*
sqlite.config.h
*/
/*
Copyright © 2024 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//----------------------------------------------------------------------------

#define SQLITE_DEFAULT_MEMSTATUS 0
#define SQLITE_DEFAULT_WAL_SYNCHRONOUS 1

//#define SQLITE_OMIT_AUTHORIZATION 1
//#define SQLITE_OMIT_AUTOINIT 1
//#define SQLITE_OMIT_COMPILEOPTION_DIAGS 1
//#define SQLITE_OMIT_DECLTYPE 1
//#define SQLITE_OMIT_DEPRECATED 1
//#define SQLITE_OMIT_EXPLAIN 1
//#define SQLITE_OMIT_PROGRESS_CALLBACK 1

#define SQLITE_WIN32_NO_ANSI 1

#ifdef _DEBUG
#define SQLITE_DEBUG 1
#define SQLITE_ENABLE_API_ARMOR 1
#endif

#endif // SQLITE_CONFIG_H_C8BF39A5_DAD7_4B50_BAE1_04F174FA5302
