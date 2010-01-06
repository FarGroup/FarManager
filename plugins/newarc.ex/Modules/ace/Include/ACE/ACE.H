#ifndef _ACEDLL_H
#define _ACEDLL_H

#ifdef __cplusplus
  extern "C" {
#endif /* __cplusplus */

//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
//อ===--                                                               --===อออ
//ออ=====----    ACE-Archiver: dynamic link library for Win32    ----======ออออ
//อออออออ====____                                            ____====ออออออออออ
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
//
//      The use of this Dll within another program than the ACE-Archiver
//      itself has to be agreet upon contract. Any use of this Dll without
//      such a contract or any violation of such a contract is seen as an
//      attack against the copyright on the archiver ACE. This copyright is
//      holded by ACE Compression Software, the create of the ACE archiver.
//ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ


//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
//อออออออออออออ=====---  Part 1: different structures ---====ออออออออออออออออออ
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

#include "includes/strucs.h"

//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
//ออออออออออออออออออ=====---  Part 2: callbacks ---====ออออออออออออออออออออออออ
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

#include "includes/callback.h"

//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
//ออออออออ=====---  Part 3: Functions supported by UnAceV2.Dll ---====อออออออออ
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

#include "includes/unacefnc.h"

//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
//อออออออ=====---  Part 4: Functions supported by the AceV2.Dll ---====ออออออออ
//อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ

#include "includes/acefnc.h"



#ifdef __cplusplus
 }
#endif /* __cplusplus */

#endif /* _ACEDLL_H */
