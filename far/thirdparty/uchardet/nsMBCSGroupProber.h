/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *			Proofpoint, Inc.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef nsMBCSGroupProber_h__
#define nsMBCSGroupProber_h__

#include "nsSJISProber.h"
#include "nsUTF8Prober.h"
#include "nsEUCJPProber.h"
#include "nsGB2312Prober.h"
#include "nsEUCKRProber.h"
#include "nsJohabProber.h"
#include "nsBig5Prober.h"
#include "nsEUCTWProber.h"

#include "nsLanguageDetector-generated.h"

#define NUM_OF_PROBERS    8
/* All the generated language model + the CJK detector.  */
#define NUM_OF_LANGUAGES  (NUM_OF_LANGUAGE_MODELS + 1)

class nsMBCSGroupProber: public nsCharSetProber {
public:
  nsMBCSGroupProber(PRUint32 aLanguageFilter);
  virtual ~nsMBCSGroupProber();
  nsProbingState HandleData(const char* aBuf, PRUint32 aLen,
                            int** codePointBuffer,
                            int*  codePointBufferIdx);
  int         GetCandidates();
  const char* GetCharSetName(int candidate);
  const char* GetLanguage(int candidate);
  nsProbingState GetState(void) {return mState;}
  void      Reset(void);
  float     GetConfidence(int candidate);
  void      SetOpion() {}

#ifdef DEBUG_chardet
  void  DumpStatus();
#endif
#ifdef DEBUG_jgmyers
  void GetDetectorState(nsUniversalDetector::DetectorState (&states)[nsUniversalDetector::NumDetectors], PRUint32 &offset);
#endif

protected:
  nsProbingState mState;
  nsCharSetProber* mProbers[NUM_OF_PROBERS];
  PRBool          mIsActive[NUM_OF_PROBERS];
  PRUint32 mActiveNum;
  PRUint32 mKeepNext;

  PRBool   candidates[NUM_OF_PROBERS][NUM_OF_LANGUAGES];

  int *codePointBuffer[NUM_OF_PROBERS];
  int  codePointBufferSize[NUM_OF_PROBERS];
  int  codePointBufferIdx[NUM_OF_PROBERS];

  nsLanguageDetector *langDetectors[NUM_OF_PROBERS][NUM_OF_LANGUAGES];

private:
  void CheckCandidates();
};

#endif /* nsMBCSGroupProber_h__ */

