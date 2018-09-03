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


.486
.model flat

EncodePointerWrapper proto stdcall :dword
DecodePointerWrapper proto stdcall :dword
GetModuleHandleExWWrapper proto stdcall :dword, :dword, :dword
InitializeSListHeadWrapper proto stdcall :dword
InterlockedFlushSListWrapper proto stdcall :dword
InterlockedPopEntrySListWrapper proto stdcall :dword
InterlockedPushEntrySListWrapper proto stdcall :dword, :dword
InterlockedPushListSListExWrapper proto stdcall :dword, :dword, :dword, :dword
RtlFirstEntrySListWrapper proto stdcall :dword
QueryDepthSListWrapper proto stdcall :dword
GetNumaHighestNodeNumberWrapper proto stdcall :dword
GetLogicalProcessorInformationWrapper proto stdcall :dword, :dword

.const
align 4
__imp__EncodePointer@4 dd EncodePointerWrapper
__imp__DecodePointer@4 dd DecodePointerWrapper
__imp__GetModuleHandleExW@12 dd GetModuleHandleExWWrapper
__imp__InitializeSListHead@4 dd InitializeSListHeadWrapper
__imp__InterlockedFlushSList@4 dd InterlockedFlushSListWrapper
__imp__InterlockedPopEntrySList@4 dd InterlockedPopEntrySListWrapper
__imp__InterlockedPushEntrySList@8 dd InterlockedPushEntrySListWrapper
__imp__InterlockedPushListSListEx@16 dd InterlockedPushListSListExWrapper
__imp__RtlFirstEntrySList@4 dd RtlFirstEntrySListWrapper
__imp__QueryDepthSList@4 dd QueryDepthSListWrapper
__imp__GetNumaHighestNodeNumber@4 dd GetNumaHighestNodeNumberWrapper
__imp__GetLogicalProcessorInformation@8 dd GetLogicalProcessorInformationWrapper

public \
__imp__EncodePointer@4,
__imp__DecodePointer@4,
__imp__GetModuleHandleExW@12,
__imp__InitializeSListHead@4,
__imp__InterlockedFlushSList@4,
__imp__InterlockedPopEntrySList@4,
__imp__InterlockedPushEntrySList@8,
__imp__InterlockedPushListSListEx@16,
__imp__RtlFirstEntrySList@4,
__imp__QueryDepthSList@4,
__imp__GetNumaHighestNodeNumber@4,
__imp__GetLogicalProcessorInformation@8
end
