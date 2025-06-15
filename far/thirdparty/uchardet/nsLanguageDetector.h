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
 * The Original Code is Mozilla Universal charset detector code. This
 * file was later added by Jehan in 2021 to add language detection.
 *
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2001 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *          Jehan <zemarmot.net> (2021)
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
#ifndef nsLanguageDetector_h__
#define nsLanguageDetector_h__

#include "nscore.h"

#define LANG_SB_ENOUGH_REL_THRESHOLD      2048
#define LANG_POSITIVE_SHORTCUT_THRESHOLD  (float)0.95
#define LANG_NEGATIVE_SHORTCUT_THRESHOLD  (float)0.05

#define LANG_NUMBER_OF_SEQ_CAT 4
#define LANG_POSITIVE_CAT      3
#define LANG_PROBABLE_CAT      2
#define LANG_NEUTRAL_CAT       1
#define LANG_NEGATIVE_CAT      0

typedef struct
{
  const char*          langName;

  /* Table mapping codepoints to character orders. */
  const PRUint32*      charOrderTable;
  int                  charOrderTableSize;

  /* freqCharCount x freqCharCount table of 2-char sequence's frequencies. */
  const PRUint8* const precedenceMatrix;
  /* The count of frequent characters.
   * precedenceMatrix size is the square of this count.
   * charOrderTable can be bigger as it can contain equivalent
   * characters. Yet it maps to this range of orders.
   */
  int                  freqCharCount;
  /* Most languages have 3 or 4 characters which are used more than 40% of the
   * times. We count how many they are and what ratio they are used.
   */
  int                  veryFreqCharCount;
  float                veryFreqRatio;
  /* Most languages will have a whole range of characters which in cumulated
   * total are barely used a few percents of the times. We count how many they
   * are and what ratio they are used.
   */
  int                  lowFreqOrder;
  float                lowFreqRatio;
} LanguageModel;

typedef enum {
  STATE_DETECTING,
  STATE_FOUND,
  STATE_UNLIKELY
} nsDetectState;

class nsLanguageDetector {
public:
  nsLanguageDetector(const LanguageModel *model) : mModel(model) { Reset(); }
  virtual ~nsLanguageDetector() {}

  /* Unlike nsSingleByteCharSetProber, it is charset-unaware and only
   * looks at unicode codepoints.
   */
  virtual const char*   GetLanguage();
  virtual nsDetectState HandleData(const int* codepoints, PRUint32 cpLen);
  virtual void          Reset(void);
  virtual float         GetConfidence(void);

protected:
  const LanguageModel* const mModel;
  nsDetectState              mState;
  /* Order of last character */
  int                        mLastOrder;

  PRUint32 mTotalSeqs;
  PRUint32 mSeqCounters[LANG_NUMBER_OF_SEQ_CAT];

  PRUint32 mTotalChar;
  /*PRUint32 mCtrlChar;*/
  /*PRUint32 mEmoticons;*/
  /*PRUint32 mVariousBetween;*/
  /* Characters that fall in our sampling range */
  PRUint32 mFreqChar;
  /* Most common characters from our sampling range */
  PRUint32 mVeryFreqChar;
  /* Most rare characters from our sampling range */
  PRUint32 mLowFreqChar;
  PRUint32 mOutChar;

private:

  int GetOrderFromCodePoint(int codePoint);
};

#endif /* nsLanguageDetector_h__ */
