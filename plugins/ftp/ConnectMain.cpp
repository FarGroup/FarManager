#include <all_far.h>
#pragma hdrstop

#include "Int.h"

void Connection::lostpeer()
{
	if(connected)
	{
		AbortAllRequest(FALSE);
		cout = 0;
		connected = 0;
	}

	pswitch(1);

	if(connected)
	{
		AbortAllRequest(FALSE);
		cout = 0;
		connected = 0;
	}

	proxflag = 0;
	pswitch(0);
}

/*
 * Command parser.
 */
int Connection::ProcessCommand(LPCSTR LineToProcess)
{
	PROC(("ProcessCommand","%s",LineToProcess))
	struct cmd *c;
	BOOL rc = FALSE;

	do
	{
		ResetOutput();
		line = LineToProcess;
		makeargv();

		if(margc == 0)
		{
			Log(("!margc"));
			SetLastError(ERROR_INVALID_PARAMETER);
			break;
		}

		c = getcmd(margv[0]);

		if(c == (cmd*)-1 || c == 0)
		{
			Log(("!cmd"));
			SetLastError(ERROR_INVALID_PARAMETER);
			break;
		}

		if(margc < c->c_args+1)
		{
			Log(("No enough parameters"));
			SetLastError(ERROR_INVALID_PARAMETER);
			break;
		}

		if(c->c_conn && !connected)
		{
			Log(("!connected: conn %d iscon: %d",c->c_conn,connected));
			SetLastError(ERROR_INTERNET_CONNECTION_ABORTED);
			break;
		}

		brk_flag  = FALSE;
		code      = 0;
		ErrorCode = 0;
		SysError  = FALSE;
		ExecCmdTab(c, margc, margv);
		rc = GetExitCode();
	}
	while(0);

	brk_flag = FALSE;
	Log(("rc=%d",rc));
	return rc;
}


struct cmd *Connection::getcmd(char *name)
{
	const char *p, *q;
	struct cmd *c, *found;
	int nmatches, longest;

	longest = 0;
	nmatches = 0;
	found = 0;

	for(c = cmdtabdata; (p = c->c_name)!=NULL; c++)
	{
		for(q = name; *q == *p++; q++)
			if(*q == 0)             /* exact match? */
				return (c);

		if(!*q)                         /* the name was a prefix */
		{
			if(q - name > longest)
			{
				longest = (int)(q - name);
				nmatches = 1;
				found = c;
			}
			else if(q - name == longest)
				nmatches++;
		}
	}

	if(nmatches > 1)
		return (struct cmd *)-1;

	return found;
}

/*
 * Slice a string up into argc/argv.
 */


void Connection::makeargv()
{
	const char **argp;
	margc = 0;
	argp = (const char **)margv;
	stringbase = line.c_str();  /* scan from first of buffer */
	argbuf.Alloc(line.Length()+100);
	argbase = argbuf.c_str();           /* store from first of buffer */
	slrflag = 0;

	while((*argp++ = slurpstring())!=0)
		margc++;
}

/*
 * Parse string into argbuf;
 * implemented with FSM to
 * handle quoting and strings
 */
const char *Connection::slurpstring()
{
	int got_one = 0;
	char *sb = stringbase;
	char *ap = argbase;
	char *tmp = argbase;            /* will return this if token found */

	if(*sb == '!' || *sb == '$')    /* recognize ! as a token for shell */
	{
		switch(slrflag)         /* and $ as token for macro invoke */
		{
			case 0:
				slrflag++;
				stringbase++;
				return ((*sb == '!') ? "!" : "$");
				/* NOTREACHED */
			case 1:
				slrflag++;
				altarg = stringbase;
				break;
			default:
				break;
		}
	}

S0:

	switch(*sb)
	{
		case '\0':
			goto OUT1;
		case ' ':
		case '\t':
			sb++;
			goto S0;
		default:

			switch(slrflag)
			{
				case 0:
					slrflag++;
					break;
				case 1:
					slrflag++;
					altarg = sb;
					break;
				default:
					break;
			}

			goto S1;
	}

S1:

	switch(*sb)
	{
		case ' ':
		case '\t':
		case '\0':
			goto OUT1;      /* end of token */
		case '\\':
			sb++;
			goto S2;  /* slurp next character */
		case '\x1':
			sb++;
			goto S3;  /* slurp quoted string */
		default:
			*ap++ = *sb++;  /* add character to token */
			got_one = 1;
			goto S1;
	}

S2:

	switch(*sb)
	{
		case '\0':
			goto OUT1;
		default:
			*ap++ = *sb++;
			got_one = 1;
			goto S1;
	}

S3:

	switch(*sb)
	{
		case '\0':
			goto OUT1;
		case '\x1':
			sb++;
			goto S1;
		default:
			*ap++ = *sb++;
			got_one = 1;
			goto S3;
	}

OUT1:

	if(got_one)
		*ap++ = '\0';

	argbase = ap;                   /* update storage pointer */
	stringbase = sb;                /* update scan pointer */

	if(got_one)
	{
		return(tmp);
	}

	switch(slrflag)
	{
		case 0:
			slrflag++;
			break;
		case 1:
			slrflag++;
			altarg = (char *) 0;
			break;
		default:
			break;
	}

	return((char *)0);
}
