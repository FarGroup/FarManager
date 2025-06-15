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
 *          BYVoid <byvoid.kcp@gmail.com>
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
#include "uchardet.h"
#include <string.h>
#include <stdlib.h>
#include <map>
#include <string>
#include <vector>
#include "nscore.h"
#include "nsUniversalDetector.h"

typedef struct _UChardetCandidate
{
    char  *encoding;
    char  *language;
    float  confidence;
} UChardetCandidate;

class HandleUniversalDetector : public nsUniversalDetector
{
protected:
    std::vector<UChardetCandidate> candidates;
    std::vector<UChardetCandidate> weighed_candidates;
    std::map<std::string, float> weights;
    float default_weight;

public:
    HandleUniversalDetector()
    : nsUniversalDetector(NS_FILTER_ALL), default_weight(1.0)
    {
    }

    virtual ~HandleUniversalDetector()
    {
        Reset();
    }

    virtual void Report(const char *encoding,
                        const char *language,
                        float       confidence)
    {
        std::vector<UChardetCandidate>::iterator it;
        UChardetCandidate                        candidate;

        for (it = candidates.begin(); it != candidates.end(); it++)
        {
            if (strcmp(it->encoding, encoding) == 0 &&
                it->language && language && strcmp(it->language, language) == 0)
            {
                /* Already reported. Bail out or update the confidence
                 * when needed.
                 */
                if (confidence > it->confidence)
                {
                    candidates.erase(it);
                    break;
                }
                else
                {
                    return;
                }
            }
        }

        candidate = UChardetCandidate();
        candidate.encoding   = strdup(encoding);
        candidate.language   = language ? strdup(language) : NULL;
        candidate.confidence = confidence;

        for (it = candidates.begin(); it != candidates.end(); it++)
        {
            if (it->confidence < confidence)
                break;
        }
        candidates.insert(it, candidate);

        if (weights.size() > 0)
            WeighCandidates();
    }

    virtual void Reset()
    {
        std::vector<UChardetCandidate>::iterator it;

        nsUniversalDetector::Reset();
        for (it = candidates.begin(); it != candidates.end(); it++)
        {
            free(it->encoding);
            if (it->language)
                free(it->language);
        }
        candidates.clear();
    }

    size_t GetCandidates() const
    {
        return candidates.size();
    }

    const char* GetCharset(size_t i)
    {
        if (weights.size() > 0)
            return (weighed_candidates.size() > i) ? weighed_candidates[i].encoding : "";
        return (candidates.size() > i) ? candidates[i].encoding : "";
    }

    float GetConfidence(size_t i)
    {
        if (weights.size() > 0)
            return (weighed_candidates.size() > i) ? weighed_candidates[i].confidence : 0.0;
        return (candidates.size() > i) ? candidates[i].confidence : 0.0;
    }

    const char* GetLanguage(size_t i)
    {
        if (weights.size() > 0)
            return (weighed_candidates.size() > i) ? weighed_candidates[i].language : NULL;
        return (candidates.size() > i) ? candidates[i].language : NULL;
    }

    void WeighLanguage(const char *language,
                       float       weight)
    {
        weights[language] = weight;
        WeighCandidates();
    }

    void WeighDefault(float weight)
    {
        default_weight = weight;
        WeighCandidates();
    }

private:

    void WeighCandidates()
    {
        std::vector<UChardetCandidate>::iterator it;
        std::vector<UChardetCandidate>::iterator it2;
        UChardetCandidate                        candidate;

        weighed_candidates.clear();
        for (it = candidates.begin(); it != candidates.end(); it++)
        {
            std::map<std::string, float>::iterator weight_it;
            float                                  confidence;

            confidence = it->confidence * default_weight;
            if (it->language)
            {
                weight_it = weights.find(it->language);
                if (weight_it != weights.end())
                    confidence = weight_it->second * it->confidence;
            }

            candidate = UChardetCandidate();
            candidate.encoding   = it->encoding;
            candidate.language   = it->language;
            candidate.confidence = confidence;

            for (it2 = weighed_candidates.begin(); it2 != weighed_candidates.end(); it2++)
            {
                if (it2->confidence < confidence)
                    break;
            }
            weighed_candidates.insert(it2, candidate);
        }
    }
};

uchardet_t uchardet_new(void)
{
    return reinterpret_cast<uchardet_t> (new HandleUniversalDetector());
}

void uchardet_delete(uchardet_t ud)
{
    delete reinterpret_cast<HandleUniversalDetector*>(ud);
}

int uchardet_handle_data(uchardet_t ud, const char * data, size_t len)
{
    nsresult ret = NS_OK;

    if (len > 0)
        ret = reinterpret_cast<HandleUniversalDetector*>(ud)->HandleData(data, (PRUint32)len);

    return (ret != NS_OK);
}

void uchardet_data_end(uchardet_t ud)
{
    reinterpret_cast<HandleUniversalDetector*>(ud)->DataEnd();
}

void uchardet_reset(uchardet_t ud)
{
    reinterpret_cast<HandleUniversalDetector*>(ud)->Reset();
}

const char* uchardet_get_charset(uchardet_t ud)
{
    return reinterpret_cast<HandleUniversalDetector*>(ud)->GetCharset(0);
}

size_t uchardet_get_n_candidates (uchardet_t ud)
{
    return reinterpret_cast<HandleUniversalDetector*>(ud)->GetCandidates();
}

float uchardet_get_confidence (uchardet_t ud,
                               size_t     candidate)
{
    return reinterpret_cast<HandleUniversalDetector*>(ud)->GetConfidence(candidate);
}

const char * uchardet_get_encoding (uchardet_t ud,
                                    size_t     candidate)
{
    return reinterpret_cast<HandleUniversalDetector*>(ud)->GetCharset(candidate);
}

const char * uchardet_get_language (uchardet_t ud,
                                    size_t     candidate)
{
    return reinterpret_cast<HandleUniversalDetector*>(ud)->GetLanguage(candidate);
}

void uchardet_weigh_language (uchardet_t  ud,
                              const char *language,
                              float       weight)
{
    reinterpret_cast<HandleUniversalDetector*>(ud)->WeighLanguage(language, weight);
}

void uchardet_set_default_weight (uchardet_t  ud,
                                  float       weight)
{
    reinterpret_cast<HandleUniversalDetector*>(ud)->WeighDefault(weight);
}
