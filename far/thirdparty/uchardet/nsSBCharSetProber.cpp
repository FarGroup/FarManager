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
 * The Original Code is Mozilla Universal charset detector code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *          Shy Shalom <shooshX@gmail.com>
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
#include <stdio.h>
#include "nsSBCharSetProber.h"

nsProbingState nsSingleByteCharSetProber::HandleData(const char* aBuf, PRUint32 aLen,
                                                     int** codePointBuffer,
                                                     int*  codePointBufferIdx)
{
  unsigned char order;

  for (PRUint32 i = 0; i < aLen; i++)
  {
    order = mModel->charToOrderMap[(unsigned char)aBuf[i]];

    mTotalChar++;
    if (order == ILL)
    {
      /* When encountering an illegal codepoint, no need
       * to continue analyzing data. */
      mState = eNotMe;
      break;
    }
    else if (order == CTR)
    {
      mCtrlChar++;
    }
    else if (order < mModel->freqCharCount)
    {
      mFreqChar++;

      if (mLastOrder < mModel->freqCharCount)
      {
        mTotalSeqs++;
        if (!mReversed)
          ++(mSeqCounters[mModel->precedenceMatrix[mLastOrder*mModel->freqCharCount+order]]);
        else // reverse the order of the letters in the lookup
          ++(mSeqCounters[mModel->precedenceMatrix[order*mModel->freqCharCount+mLastOrder]]);
      }
      else if (mLastOrder < SYMBOL_CAT_ORDER)
      {
        mSeqCounters[NEGATIVE_CAT]++;
        mTotalSeqs++;
      }
    }
    else if (order < SYMBOL_CAT_ORDER)
    {
      mOutChar++;

      if (mLastOrder < SYMBOL_CAT_ORDER)
      {
        mTotalSeqs++;
        mSeqCounters[NEGATIVE_CAT]++;
      }
    }
    mLastOrder = order;
  }

  if (mState == eDetecting)
    if (mTotalSeqs > SB_ENOUGH_REL_THRESHOLD)
    {
      float cf = GetConfidence(0);
      if (cf > POSITIVE_SHORTCUT_THRESHOLD)
        mState = eFoundIt;
      else if (cf < NEGATIVE_SHORTCUT_THRESHOLD)
        mState = eNotMe;
    }

  return mState;
}

void nsSingleByteCharSetProber::Reset(void)
{
  mState = eDetecting;
  mLastOrder = 255;
  for (PRUint32 i = 0; i < NUMBER_OF_SEQ_CAT; i++)
    mSeqCounters[i] = 0;
  mTotalSeqs = 0;
  mTotalChar = 0;
  mCtrlChar  = 0;
  mFreqChar  = 0;
  mOutChar   = 0;
}

//#define NEGATIVE_APPROACH 1

float nsSingleByteCharSetProber::GetConfidence(int candidate)
{
#ifdef NEGATIVE_APPROACH
  if (mTotalSeqs > 0)
    if (mTotalSeqs > mSeqCounters[NEGATIVE_CAT]*10 )
      return ((float)(mTotalSeqs - mSeqCounters[NEGATIVE_CAT]*10))/mTotalSeqs * mFreqChar / mTotalChar;
  return (float)0.01;
#else  //POSITIVE_APPROACH
  float r;

  if (mTotalSeqs > 0) {
    float positiveSeqs = mSeqCounters[POSITIVE_CAT];
    float probableSeqs = mSeqCounters[PROBABLE_CAT];
    float negativeSeqs = mSeqCounters[NEGATIVE_CAT];

    r = (positiveSeqs + probableSeqs / 4 - negativeSeqs * 4) / mTotalSeqs / mModel->mTypicalPositiveRatio;
    r = r * (mTotalChar - mOutChar - mCtrlChar) / mTotalChar;
    r = r * mFreqChar / mTotalChar;

    return r;
  }
  return (float)0.01;
#endif
}

const char* nsSingleByteCharSetProber::GetCharSetName(int candidate)
{
  if (!mNameProber)
    return mModel->charsetName;
  return mNameProber->GetCharSetName(0);
}

const char* nsSingleByteCharSetProber::GetLanguage(int candidate)
{
  if (!mNameProber)
    return mModel->langName;
  return mNameProber->GetLanguage(0);
}

#ifdef DEBUG_chardet
void nsSingleByteCharSetProber::DumpStatus()
{
  printf("  SBCS: %1.3f [%s]\r\n", GetConfidence(0), GetCharSetName(0));
}
#endif
