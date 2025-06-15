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

#include "nsBig5Prober.h"

void nsBig5Prober::Reset(void)
{
  mState           = eDetecting;
  isFirstByte      = PR_TRUE;
  /* Actually symbol and numbers. */
  symbolCount      = 0;
  asciiLetterCount = 0;
  graphCharCount   = 0;
  freqCharCount    = 0;
  reservedCount    = 0;
  rareCharCount    = 0;
}

nsProbingState nsBig5Prober::HandleData(const char *aBuf,
                                        PRUint32    aLen,
                                        int        **codePointBuffer,
                                        int         *codePointBufferIdx)
{
  if (mState == eNotMe)
    return mState;

  for (PRUint32 i = 0; i < aLen; i++)
  {
    unsigned char c = (unsigned char) aBuf[i];

    if (isFirstByte)
    {
      /* Wikipedia says: "Big5 does not specify a single-byte component;
       * however, ASCII (or an extension) is used in practice."
       */
      if ((c >= 0x20 && c <= 0x40) ||
          (c >= 0x5C && c <= 0x60) ||
          (c >= 0x7C && c <= 0x7E))
      {
        /* Symbols and numbers. */
        symbolCount++;
      }
      else if ((c >= 0x41 && c <= 0x5A) ||
               (c >= 0x61 && c <= 0x7A))
      {
        asciiLetterCount++;
      }
      else if (c >= 0x81 && c <= 0xfe)
      {
        /* Actual Big5 character in 2 bytes. */
        isFirstByte = PR_FALSE;
        firstByte   = c;
      }
      else
      {
        /* Invalid. */
        mState = eNotMe;
        return mState;
      }
    }
    else
    {
      if (/* Reserved for user-defined characters */
          (firstByte == 0x81 && c >= 0x40) ||
          (firstByte > 0x81 && firstByte < 0xA0) ||
          (firstByte == 0xA0 && c < 0xFE)  ||
          /* Reserved, not for user-defined characters */
          (firstByte == 0xA3 &&
           (c >= 0xC0 || c <= 0xFE)) ||
          /* Reserved, not for user-defined characters */
          (firstByte == 0xC6 && c >= 0xA1) ||
          (firstByte > 0xC6 && firstByte < 0xC8) ||
          (firstByte == 0xC8 && c < 0xFE)  ||
          /* Reserved, not for user-defined characters */
          (firstByte == 0xF9 && c >= 0xD6) ||
          (firstByte > 0xF9 && firstByte < 0xFE) ||
          (firstByte == 0xFE && c < 0xFE))
      {
        reservedCount++;
      }
      else if ((firstByte == 0xA1 && c >= 0x40) ||
               (firstByte > 0xA1 && firstByte < 0xA3) ||
               (firstByte == 0xA3 && c < 0xBF))
      {
        graphCharCount++;
      }
      else if ((firstByte == 0xA4 && c >= 0x40) ||
               (firstByte > 0xA4 && firstByte < 0xC6) ||
               (firstByte == 0xC6 && c < 0x7E))
      {
        freqCharCount++;
      }
      else if ((firstByte == 0xC9 && c >= 0x40) ||
               (firstByte > 0xC9 && firstByte < 0xF9) ||
               (firstByte == 0xF9 && c < 0xD5))
      {
        rareCharCount++;
      }
      else
      {
        /* Invalid. */
        mState = eNotMe;
        return mState;
      }
      isFirstByte = PR_TRUE;
    }
  }

  return mState;
}

float nsBig5Prober::GetConfidence(int candidate)
{
  float    confidence;
  PRUint32 letterCount = asciiLetterCount + freqCharCount + rareCharCount;
  PRUint32 charCount   = letterCount + symbolCount + graphCharCount + reservedCount;

  confidence = (freqCharCount + 0.5 * rareCharCount - asciiLetterCount - 2.0 * reservedCount) / (float) charCount;

  return confidence;
}
