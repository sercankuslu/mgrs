/*********************************************************************
 *
 *	Simple Network Time Protocol (SNTP) Client Version 3
 *  Module for Microchip TCP/IP Stack
 *	 -Locates an NTP Server from public site using DNS
 *	 -Requests UTC time using SNTP and updates SNTPTime structure
 *	  periodically, according to NTP_QUERY_INTERVAL value
 *	- Reference: RFC 1305
 *
 *********************************************************************
 * FileName:        SNTP.c
 * Dependencies:    UDP, ARP, DNS, Tick
 * Processor:       PIC18, PIC24F, PIC24H, dsPIC30F, dsPIC33F, PIC32MX
 * Compiler:        Microchip C32 v1.00 or higher
 *					Microchip C30 v3.01 or higher
 *					Microchip C18 v3.13 or higher
 *					HI-TECH PICC-18 STD 9.50PL3 or higher
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * Copyright © 2002-2007 Microchip Technology Inc.  All rights 
 * reserved.
 *
 * Microchip licenses to you the right to use, modify, copy, and 
 * distribute: 
 * (i)  the Software when embedded on a Microchip microcontroller or 
 *      digital signal controller product (“Device”) which is 
 *      integrated into Licensee’s product; or
 * (ii) ONLY the Software driver source files ENC28J60.c and 
 *      ENC28J60.h ported to a non-Microchip device used in 
 *      conjunction with a Microchip ethernet controller for the 
 *      sole purpose of interfacing with the ethernet controller. 
 *
 * You should refer to the license agreement accompanying this 
 * Software for additional information regarding your rights and 
 * obligations.
 *
 * THE SOFTWARE AND DOCUMENTATION ARE PROVIDED “AS IS” WITHOUT 
 * WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT 
 * LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A 
 * PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL 
 * MICROCHIP BE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR 
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF 
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS 
 * BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE 
 * THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER 
 * SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT 
 * (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE.
 *
 *
 * Author               Date    	Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Darren Wenn			03/08/07	Original
 * Howard Schlunder		06/20/07	Modified for release
 ********************************************************************/
#define __SNTP_C

#include "TCPIP Stack/TCPIP.h"

#if defined(STACK_USE_SNTP_CLIENT)

#define NTP_QUERY_INTERVAL		(10ull*60ull * TICK_SECOND)			// Resynchronize date/time every 10 minutes
#define NTP_FAST_QUERY_INTERVAL	(14ull * TICK_SECOND)				// Wait 8 seconds before retrying if an update fails.  Note that an update attempt can take 6 or more seconds to fail, making the actual requery interval 14 seconds or more.
#define NTP_SERVER_PORT			(123ul)
#define NTP_EPOCH 				(86400ul * (365ul * 70ul + 17ul))	// 0:0:0 1/1/1970
#define NTP_REPLY_TIMEOUT		(6ul*TICK_SECOND)					// Wait 6 seconds for timeout

// These are normally available network time servers
// the actual IP returned from the pool will vary every
// minute so as to spread the load around stratum 1 timeservers
// for best accuracy and network overhead you should locate the 
// pool server closest to your geography, but it will still work
// if you use the global pool.ntp.org address or choose the wrong 
// one or ship your embedded device to another geography.
#define NTP_SERVER	"pool.ntp.org"
//#define NTP_SERVER	"europe.pool.ntp.org"
//#define NTP_SERVER	"asia.pool.ntp.org"
//#define NTP_SERVER	"oceania.pool.ntp.org"
//#define NTP_SERVER	"north-america.pool.ntp.org"
//#define NTP_SERVER	"south-america.pool.ntp.org"
//#define NTP_SERVER	"africa.pool.ntp.org"


typedef struct _NTP_PACKET
{
	struct
	{
		BYTE mode			: 3;
		BYTE versionNumber 	: 3;
		BYTE leapIndicator	: 2;
	} flags;

	BYTE stratum;
	CHAR poll;
	CHAR precision;
	DWORD root_delay;
	DWORD root_dispersion;
	DWORD ref_identifier;
	DWORD ref_ts_secs;
	DWORD ref_ts_fraq;
	DWORD orig_ts_secs;
	DWORD orig_ts_fraq;
	DWORD recv_ts_secs;
	DWORD recv_ts_fraq;
	DWORD tx_ts_secs;
	DWORD tx_ts_fraq;
} NTP_PACKET;

static DWORD dwSNTPSeconds = 0;
static DWORD dwLastUpdateTick = 0;


/*********************************************************************
 * Function:        void SNTPClient(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          Sets the local date and time to match remote NTP 
 *					server value
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            Require one available UDP socket resource.  The 
 *					UDP socket is freed while SNTP module is idle.
 ********************************************************************/
void SNTPClient(void)
{
	NTP_PACKET			pkt;
	WORD		 		w;
	static NODE_INFO	Server;
	static DWORD		dwTimer;
	static UDP_SOCKET	MySocket;
	static enum
	{
		SM_HOME = 0,
		SM_NAME_RESOLVE,
		SM_ARP_START_RESOLVE,
		SM_ARP_RESOLVE,
		SM_ARP_START_RESOLVE2,
		SM_ARP_RESOLVE2,
		SM_ARP_START_RESOLVE3,
		SM_ARP_RESOLVE3,
		SM_ARP_RESOLVE_FAIL,
		SM_UDP_SEND,
		SM_UDP_RECV,
		SM_SHORT_WAIT,
		SM_WAIT
	} SNTPState = SM_HOME;

	switch(SNTPState)
	{
		case SM_HOME:
			// Obtain ownership of the DNS resolution module
			if(!DNSBeginUsage())
				break;

			// Obtain the IP address associated with the server name
			DNSResolveROM((ROM BYTE*)NTP_SERVER, DNS_TYPE_A);
			dwTimer = TickGet();
			SNTPState = SM_NAME_RESOLVE;
			break;

		case SM_NAME_RESOLVE:
			// Wait for DNS resolution to complete
			if(!DNSIsResolved(&Server.IPAddr)) 
			{
				if((TickGet() - dwTimer) > (5 * TICK_SECOND)) 
				{
					DNSEndUsage();
					dwTimer = TickGetDiv64K();
					SNTPState = SM_SHORT_WAIT;
				}
				break;
			}
			
			// Obtain DNS resolution result
			if(!DNSEndUsage())
			{
				// No valid IP address was returned from the DNS 
				// server.  Quit and fail for a while if host is not valid.
				dwTimer = TickGetDiv64K();
				SNTPState = SM_SHORT_WAIT;
				break;
			}
			SNTPState = SM_ARP_START_RESOLVE;
			// No need to break

		case SM_ARP_START_RESOLVE:
		case SM_ARP_START_RESOLVE2:
		case SM_ARP_START_RESOLVE3:
			// Obtain the MAC address associated with the server's IP address 
			ARPResolve(&Server.IPAddr);
			dwTimer = TickGet();
			SNTPState++;
			break;

		case SM_ARP_RESOLVE:
		case SM_ARP_RESOLVE2:
		case SM_ARP_RESOLVE3:
			// Wait for the MAC address to finish being obtained
			if(!ARPIsResolved(&Server.IPAddr, &Server.MACAddr))
			{
				// Time out if too much time is spent in this state
				if(TickGet() - dwTimer > 1*TICK_SECOND)
				{
					// Retransmit ARP request by going to next SM_ARP_START_RESOLVE state or fail by going to SM_ARP_RESOLVE_FAIL state.
					SNTPState++;
				}
				break;
			}
			SNTPState = SM_UDP_SEND;
			break;

		case SM_ARP_RESOLVE_FAIL:
			// ARP failed after 3 tries, abort and wait for next time query
			dwTimer = TickGetDiv64K();
			SNTPState = SM_SHORT_WAIT;
			break;

		case SM_UDP_SEND:
			// Open up the sending UDP socket
			MySocket = UDPOpen(0, &Server, NTP_SERVER_PORT);
			if(MySocket == INVALID_UDP_SOCKET)
				break;

			// Make certain the socket can be written to
			if(!UDPIsPutReady(MySocket))
			{
				UDPClose(MySocket);
				break;
			}

			// Transmit a time request packet
			memset(&pkt, 0, sizeof(pkt));
			pkt.flags.versionNumber = 3;	// NTP Version 3
			pkt.flags.mode = 3;				// NTP Client
			pkt.orig_ts_secs = swapl(NTP_EPOCH);
			UDPPutArray((BYTE*) &pkt, sizeof(pkt));	
			UDPFlush();	
			
			dwTimer = TickGet();
			SNTPState = SM_UDP_RECV;		
			break;

		case SM_UDP_RECV:
			// Look for a response time packet
			if(!UDPIsGetReady(MySocket)) 
			{
				if((TickGet()) - dwTimer > NTP_REPLY_TIMEOUT)
				{
					// Abort the request and wait until the next timeout period
					UDPClose(MySocket);
					dwTimer = TickGetDiv64K();
					SNTPState = SM_SHORT_WAIT;
					break;
				}
				break;
			}
			
			// Get the response time packet
			w = UDPGetArray((BYTE*) &pkt, sizeof(pkt));
			UDPClose(MySocket);
			dwTimer = TickGetDiv64K();
			SNTPState = SM_WAIT;

			// Validate packet size
			if(w != sizeof(pkt)) 
			{
				break;	
			}
			
			// Set out local time to match the returned time
			dwLastUpdateTick = TickGet();
			dwSNTPSeconds = swapl(pkt.tx_ts_secs) - NTP_EPOCH;
			// Do rounding.  If the partial seconds is > 0.5 then add 1 to the seconds count.
			if(((BYTE*)&pkt.tx_ts_fraq)[0] & 0x80)
				dwSNTPSeconds++;

			break;

		case SM_SHORT_WAIT:
			// Attempt to requery the NTP server after a specified NTP_FAST_QUERY_INTERVAL time (ex: 8 seconds) has elapsed.
			if(TickGetDiv64K() - dwTimer > (NTP_FAST_QUERY_INTERVAL/65536ull))
				SNTPState = SM_HOME;	
			break;

		case SM_WAIT:
			// Requery the NTP server after a specified NTP_QUERY_INTERVAL time (ex: 10 minutes) has elapsed.
			if(TickGetDiv64K() - dwTimer > (NTP_QUERY_INTERVAL/65536ull))
				SNTPState = SM_HOME;	

			break;
	}
}


/*********************************************************************
 * Function:        DWORD SNTPGetUTCSeconds(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          DWORD indicating the number of seconds since 
 *					00:00:00 on January 1st, 1970 UTC
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            Use this time value for absolute time stamping.
 *
 *					Do not use this API for time difference 
 *					measurements. Since this value depends on what 
 *					the remote NTP server periodically provides us 
 *					with, one call to SNTPGetUTCSeconds() could 
 *					return a higher number than a second call to 
 *					SNTPGetUTCSeconds() made at a future time.  Use 
 *					the TickGet*() API for reliable time difference 
 *					calculations.
 ********************************************************************/
DWORD SNTPGetUTCSeconds(void)
{
	DWORD dwTickDelta;
	DWORD dwTick;

	// Update the dwSNTPSeconds variable with the number of seconds 
	// that has elapsed
	dwTick = TickGet();
	dwTickDelta = TickGet() - dwLastUpdateTick;
	while(dwTickDelta > TICK_SECOND)
	{
		dwSNTPSeconds++;
		dwTickDelta -= TICK_SECOND;
	}
	
	// Save the tick and residual fractional seconds for the next call
	dwLastUpdateTick = dwTick - dwTickDelta;

	return dwSNTPSeconds;
}

#endif  //if defined(STACK_USE_SNTP_CLIENT)
