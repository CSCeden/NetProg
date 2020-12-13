/*****************************************************************************
* getopt.h - competent and free getopt library.
* $Header: /cvsroot/freegetopt/freegetopt/getopt.h,v 1.2 2003/10/26 03:10:20 vindaci Exp $
*
* Copyright (c)2002-2003 Mark K. Kim
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*
*   * Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in
*     the documentation and/or other materials provided with the
*     distribution.
*
*   * Neither the original author of this software nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
* THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*/
#ifndef GETOPT_H_
#define GETOPT_H_


#ifdef __cplusplus
extern "C" {
#endif


	extern char* optarg;
	extern int optind;
	extern int opterr;
	extern int optopt;

	int getopt(int argc, char** argv, char* optstr);


#ifdef __cplusplus
}
#endif


#endif /* GETOPT_H_ */


/* vim:ts=3
*/


/*****************************************************************************
* getopt.c - competent and free getopt library.
* $Header: /cvsroot/freegetopt/freegetopt/getopt.c,v 1.2 2003/10/26 03:10:20 vindaci Exp $
*
* Copyright (c)2002-2003 Mark K. Kim
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*
*   * Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in
*     the documentation and/or other materials provided with the
*     distribution.
*
*   * Neither the original author of this software nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
* THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <Windows.h>
#include <sstream>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <Windows.h>
#include <iphlpapi.h>
#include <tchar.h>
#include <netcon.h>



static const char* ID = "$Id: getopt.c,v 1.2 2003/10/26 03:10:20 vindaci Exp $";


char* optarg = NULL;
int optind = 0;
int opterr = 1;
int optopt = '?';


static char** prev_argv = NULL;        /* Keep a copy of argv and argc to */
static int prev_argc = 0;              /*    tell if getopt params change */
static int argv_index = 0;             /* Option we're checking */
static int argv_index2 = 0;            /* Option argument we're checking */
static int opt_offset = 0;             /* Index into compounded "-option" */
static int dashdash = 0;               /* True if "--" option reached */
static int nonopt = 0;                 /* How many nonopts we've found */

static void increment_index()
{
	/* Move onto the next option */
	if (argv_index < argv_index2)
	{
		while (prev_argv[++argv_index] && prev_argv[argv_index][0] != '-'
			&& argv_index < argv_index2 + 1);
	}
	else argv_index++;
	opt_offset = 1;
}


/*
* Permutes argv[] so that the argument currently being processed is moved
* to the end.
*/
static int permute_argv_once()
{
	/* Movability check */
	if (argv_index + nonopt >= prev_argc) return 1;
	/* Move the current option to the end, bring the others to front */
	else
	{
		char* tmp = prev_argv[argv_index];

		/* Move the data */
		memmove(&prev_argv[argv_index], &prev_argv[argv_index + 1],
			sizeof(char**) * (prev_argc - argv_index - 1));
		prev_argv[prev_argc - 1] = tmp;

		nonopt++;
		return 0;
	}
}


int getopt(int argc, char** argv, char* optstr)
{
	int c = 0;

	/* If we have new argv, reinitialize */
	if (prev_argv != argv || prev_argc != argc)
	{
		/* Initialize variables */
		prev_argv = argv;
		prev_argc = argc;
		argv_index = 1;
		argv_index2 = 1;
		opt_offset = 1;
		dashdash = 0;
		nonopt = 0;
	}

	/* Jump point in case we want to ignore the current argv_index */
getopt_top:

	/* Misc. initializations */
	optarg = NULL;

	/* Dash-dash check */
	if (argv[argv_index] && !strcmp(argv[argv_index], "--"))
	{
		dashdash = 1;
		increment_index();
	}

	/* If we're at the end of argv, that's it. */
	if (argv[argv_index] == NULL)
	{
		c = -1;
	}
	/* Are we looking at a string? Single dash is also a string */
	else if (dashdash || argv[argv_index][0] != '-' || !strcmp(argv[argv_index], "-"))
	{
		/* If we want a string... */
		if (optstr[0] == '-')
		{
			c = 1;
			optarg = argv[argv_index];
			increment_index();
		}
		/* If we really don't want it (we're in POSIX mode), we're done */
		else if (optstr[0] == '+' || getenv("POSIXLY_CORRECT"))
		{
			c = -1;

			/* Everything else is a non-opt argument */
			nonopt = argc - argv_index;
		}
		/* If we mildly don't want it, then move it back */
		else
		{
			if (!permute_argv_once()) goto getopt_top;
			else c = -1;
		}
	}
	/* Otherwise we're looking at an option */
	else
	{
		char* opt_ptr = NULL;

		/* Grab the option */
		c = argv[argv_index][opt_offset++];

		/* Is the option in the optstr? */
		if (optstr[0] == '-') opt_ptr = strchr(optstr + 1, c);
		else opt_ptr = strchr(optstr, c);
		/* Invalid argument */
		if (!opt_ptr)
		{
			if (opterr)
			{
				fprintf(stderr, "%s: invalid option -- %c\n", argv[0], c);
			}

			optopt = c;
			c = '?';

			/* Move onto the next option */
			increment_index();
		}
		/* Option takes argument */
		else if (opt_ptr[1] == ':')
		{
			/* ie, -oARGUMENT, -xxxoARGUMENT, etc. */
			if (argv[argv_index][opt_offset] != '\0')
			{
				optarg = &argv[argv_index][opt_offset];
				increment_index();
			}
			/* ie, -o ARGUMENT (only if it's a required argument) */
			else if (opt_ptr[2] != ':')
			{
				/* One of those "you're not expected to understand this" moment */
				if (argv_index2 < argv_index) argv_index2 = argv_index;
				while (argv[++argv_index2] && argv[argv_index2][0] == '-');
				optarg = argv[argv_index2];

				/* Don't cross into the non-option argument list */
				if (argv_index2 + nonopt >= prev_argc) optarg = NULL;

				/* Move onto the next option */
				increment_index();
			}
			else
			{
				/* Move onto the next option */
				increment_index();
			}

			/* In case we got no argument for an option with required argument */
			if (optarg == NULL && opt_ptr[2] != ':')
			{
				optopt = c;
				c = '?';

				if (opterr)
				{
					fprintf(stderr, "%s: option requires an argument -- %c\n",
						argv[0], optopt);
				}
			}
		}
		/* Option does not take argument */
		else
		{
			/* Next argv_index */
			if (argv[argv_index][opt_offset] == '\0')
			{
				increment_index();
			}
		}
	}

	/* Calculate optind */
	if (c == -1)
	{
		optind = argc - nonopt;
	}
	else
	{
		optind = argv_index;
	}

	return c;
}

#include <WinSock2.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <stdbool.h>
#include <string>
//link to librarys
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

static void usage(char* cmd) {
	fprintf(stderr, "Usage: %s -t [IP/slash] or [IP]\n", cmd);
	exit(1);
}//end usage

static void controlc() {
	printf("Enter Ctrl-C to Abort. Please waiting...\n");
	WSACleanup();
	exit(0);
}//end controlc

static u_long lookupAddress(const char* pcHost) {
	struct in_addr address;
	struct hostent* pHE = NULL;
	u_long nRemoteAddr;
	bool was_a_name = false;

	nRemoteAddr = inet_addr(pcHost);

	if (nRemoteAddr == INADDR_NONE) {
		// pcHost isn't a dotted IP, so resolve it through DNS
		pHE = gethostbyname(pcHost);

		if (pHE == 0) {
			return INADDR_NONE;
		}//end if
		nRemoteAddr = *((u_long*)pHE->h_addr_list[0]);
		was_a_name = true;
	}
	if (was_a_name) {
		memcpy(&address, &nRemoteAddr, sizeof(u_long));
		printf("DNS: %s, is %s\n", pcHost, inet_ntoa(address));
	}//end if
	return nRemoteAddr;
}//end lookupAddress

//extern std::ofstream macaddressesFile;
extern int index;
extern std::string Data[256];
static DWORD WINAPI sendAnARP(LPVOID lpArg) {
	u_long DestIp = *(int*)lpArg;

	IPAddr SrcIp = 0;       /* default for src ip */
	ULONG MacAddr[2];       /* for 6-byte hardware addresses */
	ULONG PhysAddrLen = 6;  /* default to length of six bytes */
	BYTE* bPhysAddr;
	DWORD dwRetVal;
	//get reply time
	LARGE_INTEGER response_timer1;
	LARGE_INTEGER response_timer2;
	LARGE_INTEGER cpu_frequency;
	double response_time;


		memset(&MacAddr, 0xff, sizeof(MacAddr));
		PhysAddrLen = 6;

		SetThreadAffinityMask(GetCurrentThread(), 1);
		QueryPerformanceFrequency((LARGE_INTEGER*)&cpu_frequency);

		//sned arp and wait for reply
		QueryPerformanceCounter((LARGE_INTEGER*)&response_timer1);
		dwRetVal = SendARP(DestIp, SrcIp, &MacAddr, &PhysAddrLen);
		QueryPerformanceCounter((LARGE_INTEGER*)&response_timer2);

		response_time = ((double)((response_timer2.QuadPart - response_timer1.QuadPart) * (double)1000.0 / (double)cpu_frequency.QuadPart));

		//output
		bPhysAddr = (BYTE*)&MacAddr;

		if (PhysAddrLen) {
			char message[256];
			std::string filedata[256];
			int i;

			memset(message, 0, sizeof(message));
			//sprintf(message, "Reply that ");

			struct in_addr ip_addr;
			ip_addr.s_addr = DestIp;

			sprintf(message, "%s is ", inet_ntoa(ip_addr));


			for (i = 0; i < (int)PhysAddrLen; i++) {
				if (i == (PhysAddrLen - 1))
				{
					sprintf(message, "%s%.2X", message, (int)bPhysAddr[i]);
				}
				else
				{
					sprintf(message, "%s%.2X:", message, (int)bPhysAddr[i]);
				}
			}//end for
			Data[index] = message;
			index++;
			printf("%s\n", message);

		}//end if

	return 0;
}//end sendAnARP

static u_long getFirstIP(u_long Ip, u_char slash) {
	IPAddr newDestIp = 0;
	int i;
	IPAddr oldDestIp = ntohl(Ip);
	u_long mask = 1 << (32 - slash);
	for (i = 0; i < slash; i++) {
		newDestIp += oldDestIp & mask;
		mask <<= 1;
	}//end for
	//skip network IP
	return ntohl(++newDestIp);
}//end getFirstIP

#define WSA_VERSION MAKEWORD(2, 2) // using winsock 2.2
static bool init_winsock() {
	WSADATA	WSAData = { 0 };
	if (WSAStartup(WSA_VERSION, &WSAData) != 0) {
		// Tell the user that we could not find a usable WinSock DLL.
		if (LOBYTE(WSAData.wVersion) != LOBYTE(WSA_VERSION) ||
			HIBYTE(WSAData.wVersion) != HIBYTE(WSA_VERSION)) {
			fprintf(stderr, "WSAStartup(): Incorrect winsock version\n");
		}//end if
		WSACleanup();
		return false;
	}//end if
	return true;
}//end init_winsock
