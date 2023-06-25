#ifndef REPARSE_TAGS_HPP_B420D40D_647E_4CD7_86BF_84FA2F707D45
#define REPARSE_TAGS_HPP_B420D40D_647E_4CD7_86BF_84FA2F707D45
#pragma once

/*
reparse_tags.hpp

*/
/*
Copyright © 2021 Far Group
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

// Internal:

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

// https://codemachine.com/downloads/win10.2004/ntifs.h
enum
{
	IO_REPARSE_TAG_IFSTEST_CONGRUENT                         = 0x00000009,
	IO_REPARSE_TAG_MOONWALK_HSM                              = 0x0000000A,
	IO_REPARSE_TAG_TSINGHUA_UNIVERSITY_RESEARCH              = 0x0000000B,
	IO_REPARSE_TAG_ARKIVIO                                   = 0x0000000C,
	IO_REPARSE_TAG_SOLUTIONSOFT                              = 0x2000000D,
	IO_REPARSE_TAG_COMMVAULT                                 = 0x0000000E,
	IO_REPARSE_TAG_OVERTONE                                  = 0x0000000F,
	IO_REPARSE_TAG_SYMANTEC_HSM2                             = 0x00000010,
	IO_REPARSE_TAG_ENIGMA_HSM                                = 0x00000011,
	IO_REPARSE_TAG_SYMANTEC_HSM                              = 0x00000012,
	IO_REPARSE_TAG_INTERCOPE_HSM                             = 0x00000013,
	IO_REPARSE_TAG_KOM_NETWORKS_HSM                          = 0x00000014,
	IO_REPARSE_TAG_MEMORY_TECH_HSM                           = 0x00000015,
	IO_REPARSE_TAG_BRIDGEHEAD_HSM                            = 0x00000016,
	IO_REPARSE_TAG_OSR_SAMPLE                                = 0x20000017,
	IO_REPARSE_TAG_GLOBAL360_HSM                             = 0x00000018,
	IO_REPARSE_TAG_ALTIRIS_HSM                               = 0x00000019,
	IO_REPARSE_TAG_HERMES_HSM                                = 0x0000001A,
	IO_REPARSE_TAG_POINTSOFT_HSM                             = 0x0000001B,
	IO_REPARSE_TAG_GRAU_DATASTORAGE_HSM                      = 0x0000001C,
	IO_REPARSE_TAG_COMMVAULT_HSM                             = 0x0000001D,
	IO_REPARSE_TAG_DATASTOR_SIS                              = 0x0000001E,
	IO_REPARSE_TAG_EDSI_HSM                                  = 0x0000001F,
	IO_REPARSE_TAG_HP_HSM                                    = 0x00000020,
	IO_REPARSE_TAG_SER_HSM                                   = 0x00000021,
	IO_REPARSE_TAG_DOUBLE_TAKE_HSM                           = 0x00000022,
	IO_REPARSE_TAG_WISDATA_HSM                               = 0x00000023,
	IO_REPARSE_TAG_MIMOSA_HSM                                = 0x00000024,
	IO_REPARSE_TAG_HSAG_HSM                                  = 0x00000025,
	IO_REPARSE_TAG_ADA_HSM                                   = 0x00000026,
	IO_REPARSE_TAG_AUTN_HSM                                  = 0x00000027,
	IO_REPARSE_TAG_NEXSAN_HSM                                = 0x00000028,
	IO_REPARSE_TAG_DOUBLE_TAKE_SIS                           = 0x00000029,
	IO_REPARSE_TAG_SONY_HSM                                  = 0x0000002A,
	IO_REPARSE_TAG_ELTAN_HSM                                 = 0x0000002B,
	IO_REPARSE_TAG_UTIXO_HSM                                 = 0x0000002C,
	IO_REPARSE_TAG_QUEST_HSM                                 = 0x0000002D,
	IO_REPARSE_TAG_DATAGLOBAL_HSM                            = 0x0000002E,
	IO_REPARSE_TAG_QI_TECH_HSM                               = 0x2000002F,
	IO_REPARSE_TAG_DATAFIRST_HSM                             = 0x00000030,
	IO_REPARSE_TAG_C2CSYSTEMS_HSM                            = 0x00000031,
	IO_REPARSE_TAG_WATERFORD                                 = 0x00000032,
	IO_REPARSE_TAG_RIVERBED_HSM                              = 0x00000033,
	IO_REPARSE_TAG_CARINGO_HSM                               = 0x00000034,
	IO_REPARSE_TAG_MAXISCALE_HSM                             = 0x20000035,
	IO_REPARSE_TAG_CITRIX_PM                                 = 0x00000036,
	IO_REPARSE_TAG_OPENAFS_DFS                               = 0x00000037,
	IO_REPARSE_TAG_ZLTI_HSM                                  = 0x00000038,
	IO_REPARSE_TAG_EMC_HSM                                   = 0x00000039,
	IO_REPARSE_TAG_VMWARE_PM                                 = 0x0000003A,
	IO_REPARSE_TAG_ARCO_BACKUP                               = 0x0000003B,
	IO_REPARSE_TAG_CARROLL_HSM                               = 0x0000003C,
	IO_REPARSE_TAG_COMTRADE_HSM                              = 0x0000003D,
	IO_REPARSE_TAG_EASEVAULT_HSM                             = 0x0000003E,
	IO_REPARSE_TAG_HDS_HSM                                   = 0x0000003F,
	IO_REPARSE_TAG_MAGINATICS_RDR                            = 0x00000040,
	IO_REPARSE_TAG_GOOGLE_HSM                                = 0x00000041,
	IO_REPARSE_TAG_QUADDRA_HSM                               = 0x00000042,
	IO_REPARSE_TAG_HP_BACKUP                                 = 0x00000043,
	IO_REPARSE_TAG_DROPBOX_HSM                               = 0x00000044,
	IO_REPARSE_TAG_ADOBE_HSM                                 = 0x00000045,
	IO_REPARSE_TAG_HP_DATA_PROTECT                           = 0x00000046,
	IO_REPARSE_TAG_ACTIVISION_HSM                            = 0x00000047,
	IO_REPARSE_TAG_HDS_HCP_HSM                               = 0x00000048,
	IO_REPARSE_TAG_AURISTOR_FS                               = 0x00000049,
	IO_REPARSE_TAG_ITSTATION                                 = 0x0000004A,
	IO_REPARSE_TAG_SPHARSOFT                                 = 0x0000004B,
	IO_REPARSE_TAG_ALERTBOOT                                 = 0x2000004C,
	IO_REPARSE_TAG_MTALOS                                    = 0x0000004D,
	IO_REPARSE_TAG_CTERA_HSM                                 = 0x0000004E,
	IO_REPARSE_TAG_NIPPON_HSM                                = 0x0000004F,
	IO_REPARSE_TAG_REDSTOR_HSM                               = 0x00000050,
	IO_REPARSE_TAG_NEUSHIELD                                 = 0x00000051,
	IO_REPARSE_TAG_DOR_HSM                                   = 0x00000052,
	IO_REPARSE_TAG_SHX_BACKUP                                = 0x00000053,
	IO_REPARSE_TAG_NVIDIA_UNIONFS                            = 0x20000054,
	IO_REPARSE_TAG_HUBSTOR_HSM                               = 0x00000055,
	IO_REPARSE_TAG_IMANAGE_HSM                               = 0x20000056,
	IO_REPARSE_TAG_EASEFILTER_HSM                            = 0x00000057,
	IO_REPARSE_TAG_ACRONIS_HSM_0                             = 0x00000060,
	IO_REPARSE_TAG_ACRONIS_HSM_1                             = 0x00000061,
	IO_REPARSE_TAG_ACRONIS_HSM_2                             = 0x00000062,
	IO_REPARSE_TAG_ACRONIS_HSM_3                             = 0x00000063,
	IO_REPARSE_TAG_ACRONIS_HSM_4                             = 0x00000064,
	IO_REPARSE_TAG_ACRONIS_HSM_5                             = 0x00000065,
	IO_REPARSE_TAG_ACRONIS_HSM_6                             = 0x00000066,
	IO_REPARSE_TAG_ACRONIS_HSM_7                             = 0x00000067,
	IO_REPARSE_TAG_ACRONIS_HSM_8                             = 0x00000068,
	IO_REPARSE_TAG_ACRONIS_HSM_9                             = 0x00000069,
	IO_REPARSE_TAG_ACRONIS_HSM_A                             = 0x0000006A,
	IO_REPARSE_TAG_ACRONIS_HSM_B                             = 0x0000006B,
	IO_REPARSE_TAG_ACRONIS_HSM_C                             = 0x0000006C,
	IO_REPARSE_TAG_ACRONIS_HSM_D                             = 0x0000006D,
	IO_REPARSE_TAG_ACRONIS_HSM_E                             = 0x0000006E,
	IO_REPARSE_TAG_ACRONIS_HSM_F                             = 0x0000006F,
};

#endif // REPARSE_TAGS_HPP_B420D40D_647E_4CD7_86BF_84FA2F707D45
