/*********************************************************************
 *
 *                  TCP Module Defs for Microchip TCP/IP Stack
 *
 *********************************************************************
 * FileName:        TCP.h
 * Dependencies:    StackTsk.h
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
 * Nilesh Rajbharti     5/8/01  	Original        (Rev 1.0)
 * Howard Schlunder		11/30/06	See "TCPIP Stack Version.txt" file
 ********************************************************************/
#ifndef __TCP_H
#define __TCP_H


typedef BYTE TCP_SOCKET;

#define INVALID_SOCKET      (0xFE)
#define UNKNOWN_SOCKET      (0xFF)

#define LOOPBACK_IP					(DWORD)0x7f000001	//127.0.0.1

// TCP States as defined by RFC 793
typedef enum _TCP_STATE
{
    TCP_LOOPBACK = 0,		// Special state for loopback sockets
    TCP_LOOPBACK_CLOSED,	// Special state for loopback sockets

	TCP_GET_DNS_MODULE,		// Special state for TCP client mode sockets
	TCP_DNS_RESOLVE,		// Special state for TCP client mode sockets
	TCP_GATEWAY_SEND_ARP,	// Special state for TCP client mode sockets
	TCP_GATEWAY_GET_ARP,	// Special state for TCP client mode sockets

    TCP_LISTEN,				// Normal TCP states specified in RFC
    TCP_SYN_SENT,
    TCP_SYN_RECEIVED,
    TCP_ESTABLISHED,
    TCP_FIN_WAIT_1,
    TCP_FIN_WAIT_2,
    TCP_CLOSING,
//	TCP_TIME_WAIT,
	TCP_CLOSE_WAIT,
    TCP_LAST_ACK,
    TCP_CLOSED
} TCP_STATE;

// TCP Control Block (TCB) Information
// Stubs are stored in local PIC RAM
// Current size is 29 bytes (PIC18, 30 bytes (PIC24/dsPIC), or 48 (PIC32)
typedef struct _TCB_STUB
{
	PTR_BASE bufferTxStart;		// TCB is located sizeof(TCB) bytes before this address
	PTR_BASE bufferRxStart;
	PTR_BASE bufferEnd;
	PTR_BASE txHead;
	PTR_BASE txTail;
	PTR_BASE rxHead;
	PTR_BASE rxTail;
    DWORD eventTime;		// Packet retransmissions, state changes
	WORD eventTime2;		// Window updates, automatic transmission
	union
	{
		WORD delayedACKTime;	// Delayed Acknowledgement timer
		WORD closeWaitTime;		// TCP_CLOSE_WAIT timeout timer
	} OverlappedTimers;
    TCP_STATE smState;
    struct
    {
        unsigned char bServer : 1;
		unsigned char bTimerEnabled	: 1;
		unsigned char bTimer2Enabled : 1;
		unsigned char bDelayedACKTimerEnabled : 1;
		unsigned char bOneSegmentReceived : 1;
		unsigned char bHalfFullFlush : 1;
		unsigned char bTXASAP : 1;
		unsigned char bTXFIN : 1;
		unsigned char bSocketReset : 1;
		unsigned char filler : 7;
    } Flags;
	WORD_VAL remoteHash;	// Hash consists of remoteIP, remotePort, localPort for connected sockets.  It is a localPort number only for listening server sockets.
	BYTE vMemoryMedium;
} TCB_STUB;

// The rest of the TCB is stored in Ethernet buffer RAM
// Current size is 37 (PIC18), 38 (PIC24/dsPIC), or 40 bytes (PIC32)
typedef struct _TCB
{
	DWORD		retryInterval;
	DWORD		MySEQ;
	DWORD		RemoteSEQ;
	PTR_BASE	txUnackedTail;
    WORD_VAL	remotePort;
    WORD_VAL	localPort;
	WORD		remoteWindow;
	WORD		wFutureDataSize;
	union
	{
		NODE_INFO	niRemoteMACIP;		// 6 bytes for MAC and IP address
		DWORD		dwRemoteHost;		// RAM or ROM pointer to a hostname string (ex: "www.microchip.com")
	} remote;
	SHORT		sHoleSize;
    struct
    {
        unsigned char bFINSent : 1;
		unsigned char bSYNSent : 1;
		unsigned char bRemoteHostIsROM : 1;
		unsigned char filler : 5;
    } flags;
	BYTE		retryCount;
	BYTE		vSocketPurpose;
} TCB;

typedef struct _SOCKET_INFO
{
	NODE_INFO remote;
	WORD_VAL remotePort;
} SOCKET_INFO;


void TCPInit(void);
SOCKET_INFO* TCPGetRemoteInfo(TCP_SOCKET hTCP);
#define TCPListen(port)			TCPOpen(0, TCP_OPEN_SERVER, port, TCP_PURPOSE_DEFAULT)
#define TCPConnect(remote,port)	TCPOpen((DWORD)remote, TCP_OPEN_NODE_INFO, port, TCP_PURPOSE_DEFAULT)
BOOL TCPWasReset(TCP_SOCKET hTCP);
BOOL TCPIsConnected(TCP_SOCKET hTCP);
void TCPDisconnect(TCP_SOCKET hTCP);
WORD TCPIsPutReady(TCP_SOCKET hTCP);
BOOL TCPPut(TCP_SOCKET hTCP, BYTE byte);
WORD TCPPutArray(TCP_SOCKET hTCP, BYTE *Data, WORD Len);
BYTE* TCPPutString(TCP_SOCKET hTCP, BYTE *Data);
WORD TCPIsGetReady(TCP_SOCKET hTCP);
WORD TCPGetRxFIFOFree(TCP_SOCKET hTCP);
BOOL TCPGet(TCP_SOCKET hTCP, BYTE *byte);
WORD TCPGetArray(TCP_SOCKET hTCP, BYTE *buffer, WORD count);
WORD TCPFindEx(TCP_SOCKET hTCP, BYTE cFind, WORD wStart, WORD wSearchLen, BOOL bTextCompare);
#define TCPFind(a,b,c,d)					TCPFindEx(a,b,c,0,d)
WORD TCPFindArrayEx(TCP_SOCKET hTCP, BYTE *cFindArray, WORD wLen, WORD wStart, WORD wSearchLen, BOOL bTextCompare);
#define TCPFindArray(a,b,c,d,e)				TCPFindArrayEx(a,b,c,d,0,e)
void TCPDiscard(TCP_SOCKET hTCP);
BOOL TCPProcess(NODE_INFO *remote, IP_ADDR *localIP, WORD len);
void TCPTick(void);
void TCPFlush(TCP_SOCKET hTCP);


#define TCP_OPEN_SERVER		0
#define TCP_OPEN_RAM_HOST	1
#define TCP_OPEN_ROM_HOST	2
#define TCP_OPEN_IP_ADDRESS	3
#define TCP_OPEN_NODE_INFO	4
TCP_SOCKET TCPOpen(DWORD dwRemoteHost, BYTE vRemoteHostType, WORD wPort, BYTE vSocketPurpose);


// ROM function variants for PIC18
#if defined(__18CXX)
	WORD TCPFindROMArrayEx(TCP_SOCKET hTCP, ROM BYTE *cFindArray, WORD wLen, WORD wStart, WORD wSearchLen, BOOL bTextCompare);
	#define TCPFindROMArray(a,b,c,d,e)		TCPFindROMArrayEx(a,b,c,d,0,e)
	WORD TCPPutROMArray(TCP_SOCKET hTCP, ROM BYTE *Data, WORD Len);
	ROM BYTE* TCPPutROMString(TCP_SOCKET hTCP, ROM BYTE *Data);
#else
	#define TCPFindROMArray(a,b,c,d,e) 		TCPFindArray(a,(BYTE*)b,c,d,e)
	#define TCPFindROMArrayEx(a,b,c,d,e,f) 	TCPFindArrayEx(a,(BYTE*)b,c,d,e,f)
	#define TCPPutROMArray(a,b,c)			TCPPutArray(a,(BYTE*)b,c)
	#define TCPPutROMString(a,b)			TCPPutString(a,(BYTE*)b)
#endif

WORD TCPGetTxFIFOFull(TCP_SOCKET hTCP);
#define TCPGetRxFIFOFull(a)					TCPIsGetReady(a)
#define TCPGetTxFIFOFree(a) 				TCPIsPutReady(a)

#define TCP_ADJUST_GIVE_REST_TO_RX	0x01u
#define TCP_ADJUST_GIVE_REST_TO_TX	0x02u
#define TCP_ADJUST_PRESERVE_RX		0x04u
#define TCP_ADJUST_PRESERVE_TX		0x08u
BOOL TCPAdjustFIFOSize(TCP_SOCKET hTCP, WORD wMinRXSize, WORD wMinTXSize, BYTE vFlags);


TCP_SOCKET TCPOpenLoopback(WORD destPort);
BOOL TCPCloseLoopback(TCP_SOCKET hTCP);
BOOL TCPIsLoopback(TCP_SOCKET hTCP);
WORD TCPInject(TCP_SOCKET hTCP, BYTE *buffer, WORD len);
WORD TCPSteal(TCP_SOCKET hTCP, BYTE *buffer, WORD len);

#endif
