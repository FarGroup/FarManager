;vc_crt_fix.asm

;Workaround for Visual C++ CRT incompatibility with old Windows versions

;Copyright © 2010 Far Group
;All rights reserved.
;
;Redistribution and use in source and binary forms, with or without
;modification, are permitted provided that the following conditions
;are met:
;1. Redistributions of source code must retain the above copyright
;   notice, this list of conditions and the following disclaimer.
;2. Redistributions in binary form must reproduce the above copyright
;   notice, this list of conditions and the following disclaimer in the
;   documentation and/or other materials provided with the distribution.
;3. The name of the authors may not be used to endorse or promote products
;   derived from this software without specific prior written permission.
;
;THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
;IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
;OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
;IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
;INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
;NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
;DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
;THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
;(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
;THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

ifndef X64
.model flat
endif

.const

HOOK MACRO name, size, args:VARARG
	WRAPPER EQU @CatStr(Wrapper_, name)
ifdef X64
	DARGS   EQU
	IMP     EQU @CatStr(__imp_, name)
else
	DARGS   EQU args
	IMP     EQU @CatStr(__imp__, name, @, size)
endif

	WRAPPER proto stdcall DARGS
	IMP dq WRAPPER
	public IMP
ENDM

ifndef X64
HOOK EncodePointer                          ,  4, :dword
HOOK DecodePointer                          ,  4, :dword
HOOK GetModuleHandleExW                     , 12, :dword, :dword, :dword
HOOK InitializeSListHead                    ,  4, :dword
HOOK InterlockedFlushSList                  ,  4, :dword
HOOK InterlockedPopEntrySList               ,  4, :dword
HOOK InterlockedPushEntrySList              ,  8, :dword, :dword
HOOK InterlockedPushListSListEx             , 16, :dword, :dword, :dword, :dword
HOOK RtlFirstEntrySList                     ,  4, :dword
HOOK QueryDepthSList                        ,  4, :dword
HOOK GetNumaHighestNodeNumber               ,  4, :dword
HOOK GetLogicalProcessorInformation         ,  8, :dword, :dword
HOOK SetThreadStackGuarantee                ,  4, :dword
endif
HOOK InitializeCriticalSectionEx            , 12, :dword, :dword, :dword
HOOK CompareStringEx                        , 36, :dword, :dword, :dword, :dword, :dword, :dword, :dword, :dword, :dword
HOOK LCMapStringEx                          , 36, :dword, :dword, :dword, :dword, :dword, :dword, :dword, :dword, :dword
HOOK AcquireSRWLockExclusive                ,  4, :dword
HOOK ReleaseSRWLockExclusive                ,  4, :dword
HOOK SleepConditionVariableSRW              , 16, :dword, :dword, :dword, :dword
HOOK WakeAllConditionVariable               ,  4, :dword

end
