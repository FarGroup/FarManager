/*
nsUniversalDetectorEx.cpp

UCD wrapper

*/
/*
Copyright © 2011 Far Group
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

#include "headers.hpp"
#pragma hdrstop

#include "nsUniversalDetectorEx.hpp"

#include "UCD/prmem.c"
#include "UCD/CharDistribution.cpp"
#include "UCD/JpCntx.cpp"
#include "UCD/LangBulgarianModel.cpp"
#include "UCD/LangCyrillicModel.cpp"
#include "UCD/LangGreekModel.cpp"
#include "UCD/LangHebrewModel.cpp"
#include "UCD/LangHungarianModel.cpp"
#include "UCD/LangThaiModel.cpp"
#include "UCD/nsBig5Prober.cpp"
#include "UCD/nsCharSetProber.cpp"
#include "UCD/nsEscCharsetProber.cpp"
#include "UCD/nsEscSM.cpp"
#include "UCD/nsEUCJPProber.cpp"
#include "UCD/nsEUCKRProber.cpp"
#include "UCD/nsEUCTWProber.cpp"
#include "UCD/nsGB2312Prober.cpp"
#include "UCD/nsHebrewProber.cpp"
#include "UCD/nsLatin1Prober.cpp"
#include "UCD/nsMBCSGroupProber.cpp"
#include "UCD/nsMBCSSM.cpp"
#include "UCD/nsSBCharSetProber.cpp"
#include "UCD/nsSBCSGroupProber.cpp"
#include "UCD/nsSJISProber.cpp"
#include "UCD/nsUniversalDetector.cpp"
#include "UCD/nsUTF8Prober.cpp"
