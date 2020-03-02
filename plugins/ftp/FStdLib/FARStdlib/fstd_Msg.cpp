#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

/** @ingroup GetMsg
    @fn LPCSTR FP_GetMsg( int MsgId )
    @brief Retrive text message by it number.

    @param MsgId Index of message in LNG file.

    @return Text message from language file if parameter is message index or
            static empty string on error.
*/

/** @ingroup GetMsg
    @fn LPCSTR FP_GetMsg( LPCSTR Msg )
    @brief Retrive text message by it number.

    @param Msg Index of message in LNG file. Index can be converted to LPCSTR using FMSG parameter.
               You can set this parameter to real local string pointer.

    @return Text message from language file if parameter is message index or
            parameter if it real text string.
*/

/** @def FISMSG( v )
    @brief Checks if string real string or emulated LNG value.

    Returns TRUE for real strings.
*/

/** @def FGETID( v )
    @brief Returns FAR LNG number from emulated strings.

    ! Use it only after FISMSG returns FALSE.
*/

/** @def FMSG( v )
    @brief Create LPCSTR data from parameter.

    Parameter may be a real string or index of messssage in language file.
*/

static LPCSTR WINAPI _FP_GetMsgINT(int MsgId)
{
	CHK_INITED
	MsgId = Abs(MsgId);

	if(MsgId < FAR_MAX_LANGID)
		return FP_Info->GetMsg(FP_Info->ModuleNumber,MsgId);
	else
		return NULL;
}

static LPCSTR WINAPI _FP_GetMsgSTR(LPCSTR String)
{
	LPCSTR res;
	CHK_INITED

	if(!String) return "";

	if(!FISMSG(String))
		res = FP_Info->GetMsg(FP_Info->ModuleNumber,FGETID(String));
	else
		res = String;

	return res;
}

/** @var FP_GetMsgINT
    @brief Callback function for get language message specified by WORD identifer.
*/
FP_GetMsgINT_t FP_GetMsgINT = _FP_GetMsgINT;

/** @var FP_GetMsgSTR
    @ingroup GetMsg
    @brief Callback function for get language message specified by LPCSTR identifer.
*/
FP_GetMsgSTR_t FP_GetMsgSTR = _FP_GetMsgSTR;
