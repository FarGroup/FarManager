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
#include <stdio.h>

#include "nsCJKDetector.h"
#include "nsMBCSGroupProber.h"
#include "nsUniversalDetector.h"

#if defined(DEBUG_chardet) || defined(DEBUG_jgmyers)
const char *ProberName[] = 
{
  "UTF-8",
  "SJIS",
  "EUC-JP",
  "GB18030",
  "EUC-KR",
  "Big5",
  "EUC-TW",
  "Johab"
};

#endif

nsMBCSGroupProber::nsMBCSGroupProber(PRUint32 aLanguageFilter)
{
  for (PRUint32 i = 0; i < NUM_OF_PROBERS; i++)
  {
    mProbers[i]            = nsnull;
    codePointBuffer[i]     = nsnull;
    codePointBufferSize[i] = 0;
    codePointBufferIdx[i]  = 0;
  }

  mProbers[0] = new nsUTF8Prober();
  if (aLanguageFilter & NS_FILTER_JAPANESE) 
  {
    mProbers[1] = new nsSJISProber(aLanguageFilter == NS_FILTER_JAPANESE);
    mProbers[2] = new nsEUCJPProber(aLanguageFilter == NS_FILTER_JAPANESE);
  }
  if (aLanguageFilter & NS_FILTER_CHINESE_SIMPLIFIED)
    mProbers[3] = new nsGB18030Prober(aLanguageFilter == NS_FILTER_CHINESE_SIMPLIFIED);
  if (aLanguageFilter & NS_FILTER_KOREAN)
  {
    mProbers[4] = new nsEUCKRProber(aLanguageFilter == NS_FILTER_KOREAN);
    mProbers[7] = new nsJohabProber(aLanguageFilter == NS_FILTER_KOREAN);
  }
  if (aLanguageFilter & NS_FILTER_CHINESE_TRADITIONAL) 
  {
    mProbers[5] = new nsBig5Prober(aLanguageFilter == NS_FILTER_CHINESE_TRADITIONAL);
    mProbers[6] = new nsEUCTWProber(aLanguageFilter == NS_FILTER_CHINESE_TRADITIONAL);
  }

  for (PRUint32 i = 0; i < NUM_OF_PROBERS; i++)
  {
    if (mProbers[i]->DecodeToUnicode())
    {
      int j = 0;

      langDetectors[i][j++] = new nsLanguageDetector(&ArabicModel);
      langDetectors[i][j++] = new nsLanguageDetector(&BelarusianModel);
      langDetectors[i][j++] = new nsLanguageDetector(&BulgarianModel);
      langDetectors[i][j++] = new nsLanguageDetector(&CatalanModel);
      langDetectors[i][j++] = new nsLanguageDetector(&CroatianModel);
      langDetectors[i][j++] = new nsLanguageDetector(&CzechModel);
      langDetectors[i][j++] = new nsLanguageDetector(&DanishModel);
      langDetectors[i][j++] = new nsLanguageDetector(&EnglishModel);
      langDetectors[i][j++] = new nsLanguageDetector(&EsperantoModel);
      langDetectors[i][j++] = new nsLanguageDetector(&EstonianModel);
      langDetectors[i][j++] = new nsLanguageDetector(&FinnishModel);
      langDetectors[i][j++] = new nsLanguageDetector(&FrenchModel);
      langDetectors[i][j++] = new nsLanguageDetector(&GermanModel);
      langDetectors[i][j++] = new nsLanguageDetector(&GeorgianModel);
      langDetectors[i][j++] = new nsLanguageDetector(&GreekModel);
      langDetectors[i][j++] = new nsLanguageDetector(&HebrewModel);
      langDetectors[i][j++] = new nsLanguageDetector(&HindiModel);
      langDetectors[i][j++] = new nsLanguageDetector(&HungarianModel);
      langDetectors[i][j++] = new nsLanguageDetector(&IrishModel);
      langDetectors[i][j++] = new nsLanguageDetector(&ItalianModel);
      langDetectors[i][j++] = new nsLanguageDetector(&LatvianModel);
      langDetectors[i][j++] = new nsLanguageDetector(&LithuanianModel);
      langDetectors[i][j++] = new nsLanguageDetector(&MacedonianModel);
      langDetectors[i][j++] = new nsLanguageDetector(&MalteseModel);
      langDetectors[i][j++] = new nsLanguageDetector(&NorwegianModel);
      langDetectors[i][j++] = new nsLanguageDetector(&PolishModel);
      langDetectors[i][j++] = new nsLanguageDetector(&PortugueseModel);
      langDetectors[i][j++] = new nsLanguageDetector(&RomanianModel);
      langDetectors[i][j++] = new nsLanguageDetector(&RussianModel);
      langDetectors[i][j++] = new nsLanguageDetector(&SerbianModel);
      langDetectors[i][j++] = new nsLanguageDetector(&SlovakModel);
      langDetectors[i][j++] = new nsLanguageDetector(&SloveneModel);
      langDetectors[i][j++] = new nsLanguageDetector(&SpanishModel);
      langDetectors[i][j++] = new nsLanguageDetector(&SwedishModel);
      langDetectors[i][j++] = new nsLanguageDetector(&ThaiModel);
      langDetectors[i][j++] = new nsLanguageDetector(&TurkishModel);
      langDetectors[i][j++] = new nsLanguageDetector(&UkrainianModel);
      langDetectors[i][j++] = new nsLanguageDetector(&VietnameseModel);
      langDetectors[i][j++] = new nsCJKDetector();
    }
    else
    {
      for (PRUint32 j = 0; j < NUM_OF_LANGUAGES; j++)
        langDetectors[i][j] = nsnull;
    }
  }
  Reset();
}

nsMBCSGroupProber::~nsMBCSGroupProber()
{
  for (PRUint32 i = 0; i < NUM_OF_PROBERS; i++)
  {
    delete mProbers[i];

    if (codePointBufferSize[i] != 0)
      delete [] codePointBuffer[i];

    for (PRUint32 j = 0; j < NUM_OF_LANGUAGES; j++)
      if (langDetectors[i][j])
        delete langDetectors[i][j];
  }
}

#define CANDIDATE_THRESHOLD 0.3f

int nsMBCSGroupProber::GetCandidates()
{
  int num_candidates = 0;

  CheckCandidates();

  for (PRUint32 i = 0; i < NUM_OF_PROBERS; i++)
    for (PRUint32 j = 0; j < NUM_OF_LANGUAGES; j++)
      if (candidates[i][j])
        num_candidates++;

  return num_candidates;
}

const char* nsMBCSGroupProber::GetCharSetName(int candidate)
{
  int num_candidates = GetCandidates();
  int candidate_it   = 0;

  if (num_candidates == 0)
    return NULL;
  else if (candidate >= num_candidates)
    /* Just show the first candidate. */
    candidate = 0;

  for (PRUint32 i = 0; i < NUM_OF_PROBERS; i++)
    for (PRUint32 j = 0; j < NUM_OF_LANGUAGES; j++)
      if (candidates[i][j])
      {
        if (candidate == candidate_it)
        {
          /* We assume that probers included in the nsMBCSGroupProber
           * return only one candidate themselves.
           * */
          return mProbers[i]->GetCharSetName(0);
        }
        candidate_it++;
      }

  /* Should not happen. */
  return NULL;
}

const char* nsMBCSGroupProber::GetLanguage(int candidate)
{
  const char* lang   = NULL;
  int num_candidates = GetCandidates();
  int candidate_it   = 0;

  if (num_candidates == 0)
    return NULL;
  else if (candidate >= num_candidates)
    /* Just show the first candidate. */
    candidate = 0;

  for (PRUint32 i = 0; i < NUM_OF_PROBERS; i++)
    for (PRUint32 j = 0; j < NUM_OF_LANGUAGES; j++)
      if (candidates[i][j])
      {
        if (candidate == candidate_it)
        {
          /* We assume that probers included in the nsMBCSGroupProber
           * return only one candidate themselves.
           * */
          lang = mProbers[i]->GetLanguage(0);

          if (! lang)
          {
            /* The prober does not come with its own language. */
            if (langDetectors[i][j])
              lang = langDetectors[i][j]->GetLanguage();
          }

          return lang;
        }
        candidate_it++;
      }

  return lang;
}

void nsMBCSGroupProber::Reset(void)
{
  mActiveNum = 0;
  for (PRUint32 i = 0; i < NUM_OF_PROBERS; i++)
  {
    if (mProbers[i])
    {
      mProbers[i]->Reset();
      mIsActive[i] = PR_TRUE;
      ++mActiveNum;

      if (codePointBufferSize[i] == 0 && mProbers[i]->DecodeToUnicode())
      {
        codePointBufferSize[i] = 1024;
        codePointBuffer[i] = new int[codePointBufferSize[i]];
      }
      codePointBufferIdx[i] = 0;
    }
    else
      mIsActive[i] = PR_FALSE;

    for (PRUint32 j = 0; j < NUM_OF_LANGUAGES; j++)
    {
      if (langDetectors[i][j])
        langDetectors[i][j]->Reset();

      candidates[i][j] = false;
    }
  }
  mState = eDetecting;
  mKeepNext = 0;
}

nsProbingState nsMBCSGroupProber::HandleData(const char* aBuf, PRUint32 aLen,
                                             int** cpBuffer,
                                             int*  cpBufferIdx)
{
  nsProbingState st;
  PRUint32 start = 0;
  PRUint32 keepNext = mKeepNext;

  //do filtering to reduce load to probers
  for (PRUint32 pos = 0; pos < aLen; ++pos)
  {
    if (aBuf[pos] & 0x80)
    {
      if (!keepNext)
        start = pos;
      keepNext = 2;
    }
    else if (keepNext)
    {
      if (--keepNext == 0)
      {
        for (PRUint32 i = 0; i < NUM_OF_PROBERS; i++)
        {
          int sequenceLength;

          if (!mIsActive[i])
            continue;

          sequenceLength = pos + 1 - start;

          if (codePointBuffer[i] && codePointBufferIdx[i] + sequenceLength > codePointBufferSize[i])
          {
            for (PRUint32 j = 0; j < NUM_OF_LANGUAGES; j++)
              langDetectors[i][j]->HandleData(codePointBuffer[i], codePointBufferIdx[i]);
            codePointBufferIdx[i] = 0;
          }

          if (codePointBuffer[i])
            {
              while (sequenceLength > 0)
                {
                  int subLength = (sequenceLength > codePointBufferSize[i]) ? codePointBufferSize[i] : sequenceLength;

                  st = mProbers[i]->HandleData(aBuf + start, subLength,
                                               &(codePointBuffer[i]), &(codePointBufferIdx[i]));

                  if (codePointBufferIdx[i] > 0)
                  {
                    for (PRUint32 j = 0; j < NUM_OF_LANGUAGES; j++)
                      langDetectors[i][j]->HandleData(codePointBuffer[i], codePointBufferIdx[i]);
                    codePointBufferIdx[i] = 0;
                  }

                  sequenceLength -= subLength;
                }
            }
          else
            {
              st = mProbers[i]->HandleData(aBuf + start, sequenceLength, NULL, NULL);
            }

          if (st == eFoundIt)
          {
            float cf = mProbers[i]->GetConfidence(0);

            for (PRUint32 j = 0; j < NUM_OF_LANGUAGES; j++)
            {
              float langConf = langDetectors[i][j] ? langDetectors[i][j]->GetConfidence() : 1.0;

              if (cf * langConf > CANDIDATE_THRESHOLD)
              {
                /* There is at least one (charset, lang) couple for
                 * which the confidence is high enough.
                 */
                mState = eFoundIt;
                return mState;
              }
            }
          }
        }
      }
    }
    else
    {
      for (PRUint32 i = 0; i < NUM_OF_PROBERS; i++)
      {
        if (codePointBuffer[i])
        {
          if (codePointBufferIdx[i] == codePointBufferSize[i] - 1)
          {
            for (PRUint32 j = 0; j < NUM_OF_LANGUAGES; j++)
              langDetectors[i][j]->HandleData(codePointBuffer[i], codePointBufferIdx[i]);
            codePointBufferIdx[i] = 0;
          }

          codePointBuffer[i][(codePointBufferIdx[i])++] = aBuf[pos];
        }
      }
    }
  }

  if (keepNext) {
    for (PRUint32 i = 0; i < NUM_OF_PROBERS; i++)
    {
      if (!mIsActive[i])
        continue;

      if (codePointBuffer[i])
        st = mProbers[i]->HandleData(aBuf + start, aLen - start,
                                     &(codePointBuffer[i]), &(codePointBufferIdx[i]));
      else
        st = mProbers[i]->HandleData(aBuf + start, aLen - start, NULL, NULL);

      if (codePointBufferIdx[i] > 0 && codePointBuffer[i])
      {
        for (PRUint32 j = 0; j < NUM_OF_LANGUAGES; j++)
          langDetectors[i][j]->HandleData(codePointBuffer[i], codePointBufferIdx[i]);
        codePointBufferIdx[i] = 0;
      }

      if (st == eFoundIt)
      {
        float cf = mProbers[i]->GetConfidence(0);

        for (PRUint32 j = 0; j < NUM_OF_LANGUAGES; j++)
        {
          float langConf = langDetectors[i][j] ? langDetectors[i][j]->GetConfidence() : 1.0;

          if (cf * langConf > CANDIDATE_THRESHOLD)
          {
            /* There is at least one (charset, lang) couple for
             * which the confidence is high enough.
             */
            mState = eFoundIt;
            return mState;
          }
        }
      }
    }
  }
  mKeepNext = keepNext;

  return mState;
}

void nsMBCSGroupProber::CheckCandidates()
{
  for (int i = 0; i < NUM_OF_PROBERS; i++)
  {
    if (! mIsActive[i])
    {
      for (int j = 0; j < NUM_OF_LANGUAGES; j++)
        candidates[i][j] = false;
    }
    else
    {
      float cf = mProbers[i]->GetConfidence(0);

      if (mProbers[i]->DecodeToUnicode())
      {
        for (int j = 0; j < NUM_OF_LANGUAGES; j++)
        {
          float langConf;

          /* Process any remaining language data first. */
          if (codePointBufferIdx[i] > 0 && codePointBuffer[i])
            langDetectors[i][j]->HandleData(codePointBuffer[i], codePointBufferIdx[i]);

          /* Now check the confidence in this (charset, lang) couple. */
          langConf = langDetectors[i][j]->GetConfidence();
          candidates[i][j] = (cf * langConf > CANDIDATE_THRESHOLD);
        }
        codePointBufferIdx[i] = 0;
      }
      else
      {
        for (int j = 0; j < NUM_OF_LANGUAGES; j++)
          candidates[i][j] = (cf > CANDIDATE_THRESHOLD);
      }
    }
  }
}

float nsMBCSGroupProber::GetConfidence(int candidate)
{
  int num_candidates = GetCandidates();
  int candidate_it   = 0;

  PRUint32 i;

  if (num_candidates == 0)
    return 0.0;
  else if (candidate >= num_candidates)
    /* Just show the first candidate. */
    candidate = 0;

  switch (mState)
  {
  case eNotMe:
    return (float)0.01;
  case eFoundIt:
  default:
    for (i = 0; i < NUM_OF_PROBERS; i++)
    {
      for (PRUint32 j = 0; j < NUM_OF_LANGUAGES; j++)
        if (candidates[i][j])
        {
          if (candidate == candidate_it)
          {
            float cf       = mProbers[i]->GetConfidence(0);
            float langConf = 1.0;

            if (langDetectors[i][j])
              langConf = langDetectors[i][j]->GetConfidence();

            return cf * langConf;
          }
          candidate_it++;
        }
    }
  }

  /* Should not happen. */
  return 0.0;
}

#ifdef DEBUG_chardet
void nsMBCSGroupProber::DumpStatus()
{
  PRUint32 i;
  float cf;
  
  GetConfidence(0);
  for (i = 0; i < NUM_OF_PROBERS; i++)
  {
    if (!mIsActive[i])
      printf("  MBCS inactive: [%s] (confidence is too low).\r\n", ProberName[i]);
    else
    {
      cf = mProbers[i]->GetConfidence(0);
      printf("  MBCS %1.3f: [%s]\r\n", cf, ProberName[i]);
    }
  }
}
#endif

#ifdef DEBUG_jgmyers
void nsMBCSGroupProber::GetDetectorState(nsUniversalDetector::DetectorState (&states)[nsUniversalDetector::NumDetectors], PRUint32 &offset)
{
  for (PRUint32 i = 0; i < NUM_OF_PROBERS; ++i) {
    states[offset].name = ProberName[i];
    states[offset].isActive = mIsActive[i];
    states[offset].confidence = mIsActive[i] ? mProbers[i]->GetConfidence(0) : 0.0;
    ++offset;
  }
}
#endif /* DEBUG_jgmyers */
