#ifndef _ACEDLL_H
#define _ACEDLL_H

#ifdef __cplusplus
  extern "C" {
#endif /* __cplusplus */

//═════════════════════════════════════════════════════════════════════════════
//═===--                                                               --===═══
//══=====----    ACE-Archiver: dynamic link library for Win32    ----======════
//═══════====____                                            ____====══════════
//═════════════════════════════════════════════════════════════════════════════
//
//      The use of this Dll within another program than the ACE-Archiver
//      itself has to be agreet upon contract. Any use of this Dll without
//      such a contract or any violation of such a contract is seen as an
//      attack against the copyright on the archiver ACE. This copyright is
//      holded by ACE Compression Software, the create of the ACE archiver.
//─────────────────────────────────────────────────────────────────────────────


//═════════════════════════════════════════════════════════════════════════════
//═════════════=====---  Part 1: different structures ---====══════════════════
//═════════════════════════════════════════════════════════════════════════════

#include "includes/strucs.h"

//═════════════════════════════════════════════════════════════════════════════
//══════════════════=====---  Part 2: callbacks ---====════════════════════════
//═════════════════════════════════════════════════════════════════════════════

#include "includes/callback.h"

//═════════════════════════════════════════════════════════════════════════════
//════════=====---  Part 3: Functions supported by UnAceV2.Dll ---====═════════
//═════════════════════════════════════════════════════════════════════════════

#include "includes/unacefnc.h"

//═════════════════════════════════════════════════════════════════════════════
//═══════=====---  Part 4: Functions supported by the AceV2.Dll ---====════════
//═════════════════════════════════════════════════════════════════════════════

#include "includes/acefnc.h"



#ifdef __cplusplus
 }
#endif /* __cplusplus */

#endif /* _ACEDLL_H */
