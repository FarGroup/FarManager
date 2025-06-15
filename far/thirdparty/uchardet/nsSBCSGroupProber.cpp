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

#include <assert.h>
#include <stdio.h>
#include "prmem.h"

#include "nsSBCharSetProber.h"
#include "nsSBCharSetProber-generated.h"
#include "nsSBCSGroupProber.h"

#include "nsHebrewProber.h"

nsSBCSGroupProber::nsSBCSGroupProber()
{
  nsHebrewProber *hebprober = new nsHebrewProber();
  PRUint32        heb_prober_idx;
  PRUint32        n = 0;

  /* We create more probers than sequence models because of Hebrew handling,
   * making Windows_1255HebrewModel and Ibm862HebrewModel used twice, while
   * Iso_8859_8HebrewModel is currently unused.
   */
  n_sbcs_probers = NUM_OF_SEQUENCE_MODELS + 2;
  mProbers      = new nsCharSetProber*[n_sbcs_probers];
  mIsActive     = new PRBool[n_sbcs_probers];

  mProbers[n++] = new nsSingleByteCharSetProber(&Windows_1251RussianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Koi8_RRussianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_5RussianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Mac_CyrillicRussianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Ibm866RussianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Ibm855RussianModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_7GreekModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Windows_1253GreekModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Cp737GreekModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_5BulgarianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Windows_1251BulgarianModel);

  heb_prober_idx = n;
  mProbers[n++] = hebprober;
  mProbers[n++] = new nsSingleByteCharSetProber(&Windows_1255HebrewModel, PR_FALSE, hebprober); // Logical Hebrew
  mProbers[n++] = new nsSingleByteCharSetProber(&Windows_1255HebrewModel, PR_TRUE, hebprober); // Visual Hebrew
  // Tell the Hebrew prober about the logical and visual probers
  if (mProbers[heb_prober_idx] && mProbers[heb_prober_idx + 1] && mProbers[heb_prober_idx + 2]) // all are not null
  {
    hebprober->SetModelProbers(mProbers[heb_prober_idx + 1], mProbers[heb_prober_idx + 2]);
  }
  else // One or more is null. avoid any Hebrew probing, null them all
  {
    for (PRUint32 i = heb_prober_idx; i <= heb_prober_idx + 2; ++i)
    {
      delete mProbers[i];
      mProbers[i] = 0;
    }
  }
  /* XXX: I should verify a bit more closely the Hebrew case. It doesn't look to
   * me like the additional data handling in nsHebrewProber is really needed
   * ("Final letter analysis for logical-visual decision").
   * For this new support of Hebrew with IBM-862, aka CP862, I just directly use
   * the direct model (in 2 modes, reversed or not, so that it handles both the
   * logical and visual hebrew cases (Wikipedia says: "Hebrew text encoded using
   * code page 862 was usually stored in visual order; nevertheless, a few DOS
   * applications, notably a word processor named EinsteinWriter, stored Hebrew
   * in logical order.")
   */
  mProbers[n++] = new nsSingleByteCharSetProber(&Ibm862HebrewModel, PR_FALSE, NULL); // Logical Hebrew
  mProbers[n++] = new nsSingleByteCharSetProber(&Ibm862HebrewModel, PR_TRUE, NULL); // Visual Hebrew

  mProbers[n++] = new nsSingleByteCharSetProber(&Tis_620ThaiModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_11ThaiModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_1FrenchModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_15FrenchModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Windows_1252FrenchModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_1SpanishModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_15SpanishModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Windows_1252SpanishModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_2HungarianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Windows_1250HungarianModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_1GermanModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Windows_1252GermanModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_3EsperantoModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_3TurkishModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_9TurkishModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_6ArabicModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Windows_1256ArabicModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&VisciiVietnameseModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Windows_1258VietnameseModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_15DanishModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_1DanishModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Windows_1252DanishModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Ibm865DanishModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_13LithuanianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_10LithuanianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_4LithuanianModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_13LatvianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_10LatvianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_4LatvianModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_1PortugueseModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_9PortugueseModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_15PortugueseModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Windows_1252PortugueseModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_3MalteseModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&Windows_1250CzechModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_2CzechModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Mac_CentraleuropeCzechModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Ibm852CzechModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&Windows_1250SlovakModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_2SlovakModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Mac_CentraleuropeSlovakModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Ibm852SlovakModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&Windows_1250PolishModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_2PolishModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_13PolishModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_16PolishModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Mac_CentraleuropePolishModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Ibm852PolishModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_1FinnishModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_4FinnishModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_9FinnishModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_13FinnishModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_15FinnishModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Windows_1252FinnishModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_1ItalianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_3ItalianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_9ItalianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_15ItalianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Windows_1252ItalianModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&Windows_1250CroatianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_2CroatianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_13CroatianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_16CroatianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Mac_CentraleuropeCroatianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Ibm852CroatianModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&Windows_1252EstonianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Windows_1257EstonianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_4EstonianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_13EstonianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_15EstonianModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_1IrishModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_9IrishModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_15IrishModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Windows_1252IrishModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&Windows_1250RomanianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_2RomanianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_16RomanianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Ibm852RomanianModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&Windows_1250SloveneModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_2SloveneModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_16SloveneModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Mac_CentraleuropeSloveneModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Ibm852SloveneModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_1SwedishModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_4SwedishModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_9SwedishModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_15SwedishModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Windows_1252SwedishModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_15NorwegianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_1NorwegianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Windows_1252NorwegianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Ibm865NorwegianModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_1EnglishModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Windows_1252EnglishModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&Windows_1251BelarusianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_5BelarusianModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&Windows_1251UkrainianModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&Windows_1251SerbianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_5SerbianModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&Windows_1251MacedonianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Ibm855MacedonianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_5MacedonianModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&Iso_8859_1CatalanModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Windows_1252CatalanModel);

  mProbers[n++] = new nsSingleByteCharSetProber(&Georgian_AcademyGeorgianModel);
  mProbers[n++] = new nsSingleByteCharSetProber(&Georgian_PsGeorgianModel);

  assert (n_sbcs_probers == n);

  Reset();
}

nsSBCSGroupProber::~nsSBCSGroupProber()
{
  for (PRUint32 i = 0; i < n_sbcs_probers; i++)
  {
    delete mProbers[i];
  }
  delete [] mProbers;
  delete [] mIsActive;
}


const char* nsSBCSGroupProber::GetCharSetName(int candidate)
{
  //if we have no answer yet
  if (mBestGuess == -1)
  {
    GetConfidence(0);
    //no charset seems positive
    if (mBestGuess == -1)
      //we will use default.
      mBestGuess = 0;
  }
  return mProbers[mBestGuess]->GetCharSetName(0);
}

const char* nsSBCSGroupProber::GetLanguage(int candidate)
{
  if (mBestGuess == -1)
  {
    GetConfidence(0);
    if (mBestGuess == -1)
      mBestGuess = 0;
  }
  return mProbers[mBestGuess]->GetLanguage(0);
}

void  nsSBCSGroupProber::Reset(void)
{
  mActiveNum = 0;
  for (PRUint32 i = 0; i < n_sbcs_probers; i++)
  {
    if (mProbers[i]) // not null
    {
      mProbers[i]->Reset();
      mIsActive[i] = PR_TRUE;
      ++mActiveNum;
    }
    else
      mIsActive[i] = PR_FALSE;
  }
  mBestGuess = -1;
  mState = eDetecting;
}


nsProbingState nsSBCSGroupProber::HandleData(const char* aBuf, PRUint32 aLen,
                                             int** codePointBuffer,
                                             int*  codePointBufferIdx)
{
  nsProbingState st;
  PRUint32 i;
  char *newBuf1 = 0;
  PRUint32 newLen1 = 0;

  //apply filter to original buffer, and we got new buffer back
  //depend on what script it is, we will feed them the new buffer 
  //we got after applying proper filter
  //this is done without any consideration to KeepEnglishLetters
  //of each prober since as of now, there are no probers here which
  //recognize languages with English characters.
  if (!FilterWithoutEnglishLetters(aBuf, aLen, &newBuf1, newLen1))
    goto done;
  
  if (newLen1 == 0)
    goto done; // Nothing to see here, move on.

  for (i = 0; i < n_sbcs_probers; i++)
  {
     if (!mIsActive[i])
       continue;
     st = mProbers[i]->HandleData(newBuf1, newLen1, codePointBuffer, codePointBufferIdx);
     if (st == eFoundIt)
     {
       mBestGuess = i;
       mState = eFoundIt;
       break;
     }
     else if (st == eNotMe)
     {
       mIsActive[i] = PR_FALSE;
       mActiveNum--;
       if (mActiveNum <= 0)
       {
         mState = eNotMe;
         break;
       }
     }
  }

done:
  PR_FREEIF(newBuf1);

  return mState;
}

float nsSBCSGroupProber::GetConfidence(int candidate)
{
  PRUint32 i;
  float bestConf = 0.0, cf;

  switch (mState)
  {
  case eFoundIt:
    return (float)0.99; //sure yes
  case eNotMe:
    return (float)0.01;  //sure no
  default:
    for (i = 0; i < n_sbcs_probers; i++)
    {
      if (!mIsActive[i])
        continue;
      cf = mProbers[i]->GetConfidence(0);
      if (bestConf < cf)
      {
        bestConf = cf;
        mBestGuess = i;
      }
    }
  }
  return bestConf;
}

#ifdef DEBUG_chardet
void nsSBCSGroupProber::DumpStatus()
{
  PRUint32 i;
  float cf;
  
  cf = GetConfidence(0);
  printf(" SBCS Group Prober --------begin status \r\n");
  for (i = 0; i < n_sbcs_probers; i++)
  {
    if (!mIsActive[i])
      printf("  inactive: [%s] (i.e. confidence is too low).\r\n", mProbers[i]->GetCharSetName(0));
    else
      mProbers[i]->DumpStatus();
  }
  printf(" SBCS Group found best match [%s] confidence %f.\r\n",  
         mProbers[mBestGuess]->GetCharSetName(0), cf);
}
#endif
