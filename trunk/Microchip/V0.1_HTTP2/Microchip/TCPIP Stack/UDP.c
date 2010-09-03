/*********************************************************************
 *
 *	User Datagram Protocol (UDP) Communications Layer
 *  Module for Microchip TCP/IP Stack
 *	 -Provides unreliable, minimum latency transport of application 
 *    datagram (packet) oriented data
 *	 -Reference: RFC 768
 *
 *********************************************************************
 * FileName:        UDP.c
 * Dependencies:    IP, Ethernet (ENC28J60.c or ETH97J60.c)
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
 * Author               Date    Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Nilesh Rajbharti     3/19/01  Original        (Rev 1.0)
 * Nilesh Rajbharti     2/26/03  Fixed UDPGet and UDPProcess bugs
 *                               as discovered and fixed by John Owen
 *                               of Powerwave.
 *                               1. UDPGet would return FALSE on last good byte
 *                               2. UDPProcess was incorrectly calculating length.
 * Nilesh Rajbharti     5/19/03  Added bFirstRead flag similar to TCP
 *                               to detect very first UDPGet and
 *                               reset MAC Rx pointer to begining of
 *                               UDP data area.  This would ensure that
 *                               if UDP socket has pending Rx data and
 *                               another module resets MAC Rx pointer,
 *                               next UDP socket Get would get correct
 *                               data.
 * Robert Sloan (RSS)    5/29/03 Improved FindMatchingSocket()
 * Nilesh Rajbharti     12/2/03  Added UDPChecksum logic in UDPProcess()
 * Nilesh Rajbharti     12/5/03  Modified UDPProcess() and FindMatchingSocket()
 *                               to include localIP as new parameter.
 *                               This corrects pseudo header checksum
 *                               logic in UDPProcess().
 *                               It also corrects broadcast packet
 *                               matching correct in FindMatchingSocket().
 * Howard Schlunder		1/16/06	 Fixed an imporbable RX checksum bug 
 *								 when using a Microchip Ethernet controller)
 * Howard Schlunder		6/02/06	 Fixed a bug where all RXed UDP packets 
 *								 without a checksum (0x0000) were thrown
 *								 away.  No checksum is legal in UDP.
 * Howard Schlunder		8/10/06	 Fixed a bug where UDP sockets would 
 *								 unintentionally keep the remote MAC 
 *								 address cached, even after calling 
 *								 UDPInit(), UDPClose(), or reseting 
 *								 the part without clearing all the 
 *								 PIC memory.
 ********************************************************************/
#define __UDP_C

#include "TCPIP Stack/TCPIP.h"

#if defined(STACK_USE_UDP)

#define LOCAL_UDP_PORT_START_NUMBER (4096u)
#define LOCAL_UDP_PORT_END_NUMBER   (8192u)


UDP_SOCKET_INFO		UDPSocketInfo[MAX_UDP_SOCKETS];
UDP_SOCKET			activeUDPSocket;
WORD UDPTxCount;
WORD UDPRxCount;
static UDP_SOCKET	LastPutSocket = INVALID_UDP_SOCKET;
static WORD wPutOffset;
static struct
{
	unsigned char bFirstRead : 1;
	unsigned char bWasDiscarded : 1;
} Flags;
static UDP_SOCKET SocketWithRxData = INVALID_UDP_SOCKET;


static UDP_SOCKET FindMatchingSocket(UDP_HEADER *h, NODE_INFO *remoteNode,
                                    IP_ADDR *localIP);

/*********************************************************************
 * Function:        void UDPInit(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Initializes internal variables.
 *
 * Note:
 ********************************************************************/
void UDPInit(void)
{
    UDP_SOCKET s;

    for ( s = 0; s < MAX_UDP_SOCKETS; s++ )
    {
		UDPClose(s);
    }
	Flags.bWasDiscarded = 1;
}

//////////////////////////////////////////////////////////////////////
/*!
\brief		Creates a UDP socket handle for transmiting or receiving 
			UDP packets.
\pre		UDPInit() must have been previously called.
\param[in]	localPort	UDP port number to listen on.\n
						If 0, stack will dynamically assign a unique 
						port number to use.
\param[in]	*remoteNode	Remote Node info such as MAC and IP address.\n
						If NULL, broadcast node address is used.
\param[in]	remotePort	UDP port number on remoteNode to connect to.
\return		\b Success: A UDP socket handle that can be used for 
						subsequent UDP API calls. \n
			\b Failure: INVALID_UDP_SOCKET.  This function fails when 
						no more UDP socket handles are available.  
						Increase MAX_UDP_SOCKETS to make more sockets 
						available.
\remarks	When finished using the UDP socket handle, call 
			the UDPClose() function to free the socket and 
			delete the handle.
*/
//////////////////////////////////////////////////////////////////////
UDP_SOCKET UDPOpen(UDP_PORT localPort,
                   NODE_INFO *remoteNode,
                   UDP_PORT remotePort)
{
    UDP_SOCKET s;
    UDP_SOCKET_INFO *p;

	// Local temp port numbers.
	static WORD NextPort __attribute__((persistent));


    p = UDPSocketInfo;
    for ( s = 0; s < MAX_UDP_SOCKETS; s++ )
    {
        if(p->localPort == INVALID_UDP_PORT)
        {
			p->localPort = localPort;	

			if(localPort == 0x0000u)
			{
				if(NextPort > LOCAL_UDP_PORT_END_NUMBER || NextPort < LOCAL_UDP_PORT_START_NUMBER)
					NextPort = LOCAL_UDP_PORT_START_NUMBER;
	
	            p->localPort    = NextPort++;
			}

            // If remoteNode is supplied, remember it.
            if(remoteNode)
            {
                memcpy((void*)&p->remoteNode,
                        (const void*)remoteNode,
                        sizeof(p->remoteNode));
            }
            else
			{
				// else Set broadcast address
				memset((void*)&p->remoteNode, 0xFF, sizeof(p->remoteNode));
			}

            p->remotePort   = remotePort;

            // Mark this socket as active.
            // Once an active socket is set, subsequent operation can be
            // done without explicitely supply socket identifier.
            activeUDPSocket = s;
            return s;
        }
        p++;
    }

    return (UDP_SOCKET)INVALID_UDP_SOCKET;
}




/*********************************************************************
 * Function:        void UDPClose(UDP_SOCKET s)
 *
 * PreCondition:    UDPOpen() is already called
 *
 * Input:           s       - Socket that is to be closed.
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Given socket is marked as available for future
 *                  new communcations.
 *
 * Note:            This function does not affect previous
 *                  active UDP socket designation.
  ********************************************************************/
void UDPClose(UDP_SOCKET s)
{
	if(s == INVALID_UDP_SOCKET)
		return;

	UDPSocketInfo[s].localPort = INVALID_UDP_PORT;
	UDPSocketInfo[s].remoteNode.IPAddr.Val = 0x00000000;
}


/*********************************************************************
 * Function:        void UDPSetTxBuffer(WORD wOffset)
 *
 * PreCondition:    None
 *
 * Input:           wOffset - Offset from begining of UDP packet data 
 *							  payload to set write pointer for 
 *							  UDPPut(), UDPPutArray(), etc.
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Updates memory access pointers
 *
 * Note:            None
  ********************************************************************/
void UDPSetTxBuffer(WORD wOffset)
{
	IPSetTxBuffer(wOffset+sizeof(UDP_HEADER));
	wPutOffset = wOffset;
}


/*********************************************************************
 * Function:        BOOL UDPIsPutReady(UDP_SOCKET s)
 *
 * PreCondition:
 *
 * Input:           s       - Socket that is to be loaded and made
 *                            an active UDP socket.
 *
 * Output:          WORD: Number of bytes that can be put, 0 if none
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            This call sets given socket as an active UDP socket.
 ********************************************************************/
WORD UDPIsPutReady(UDP_SOCKET s)
{
	if(LastPutSocket != s)
	{
		LastPutSocket = s;
		MACSetWritePtr(BASE_TX_ADDR + sizeof(ETHER_HEADER) + sizeof(IP_HEADER) + sizeof(UDP_HEADER));
		UDPTxCount = 0;
		wPutOffset = 0;
	}

	activeUDPSocket = s;
	if(!MACIsTxReady())
		return 0;

	return MAC_TX_BUFFER_SIZE - sizeof(IP_HEADER) - sizeof(UDP_HEADER) - UDPTxCount;
}

/*********************************************************************
 * Function:        BOOL UDPPut(BYTE v)
 *
 * PreCondition:    UDPIsPutReady() > 0 with desired UDP socket
 *                  that is to be loaded.
 *
 * Input:           v       - Data byte to loaded into transmit buffer
 *
 * Output:          TRUE if put successful
 *                  FALSE if transmit buffer is full and put failed
 *
 * Side Effects:    None
 *
 * Overview:        Given data byte is put into UDP transmit buffer
 *                  and active UDP socket buffer length is incremented
 *                  by one.
 *                  If buffer has become full, FALSE is returned.
 *                  Or else TRUE is returned.
 *
 * Note:            This function loads data into an active UDP socket
 *                  as determined by previous call to UDPIsPutReady()
 ********************************************************************/
BOOL UDPPut(BYTE v)
{
	// See if we are out of transmit space.
	if(wPutOffset >= (MAC_TX_BUFFER_SIZE - sizeof(IP_HEADER) - sizeof(UDP_HEADER)))
	{
		return FALSE;
	}

    // Load application data byte
    MACPut(v);
	wPutOffset++;
	if(wPutOffset > UDPTxCount)
		UDPTxCount = wPutOffset;

    return TRUE;
}

/*********************************************************************
 * Function:        WORD UDPPutArray(BYTE *cData, WORD wDataLen)
 *
 * PreCondition:    UDPIsPutReady() > 0 with desired UDP socket
 *                  that is to be loaded.
 *
 * Input:           *cData - Pointer to data to write
 *					wDataLen - Length of data @ *cData to write
 *
 * Output:          WORD: Number of bytes successfully placed in the 
 *						  UDP transmit buffer
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/
WORD UDPPutArray(BYTE *cData, WORD wDataLen)
{
	WORD wTemp;

	wTemp = (MAC_TX_BUFFER_SIZE - sizeof(IP_HEADER) - sizeof(UDP_HEADER)) - wPutOffset;
	if(wTemp < wDataLen)
		wDataLen = wTemp;

	wPutOffset += wDataLen;
	if(wPutOffset > UDPTxCount)
		UDPTxCount = wPutOffset;

    // Load application data bytes
    MACPutArray(cData, wDataLen);

    return wDataLen;
}

#if defined(__18CXX)
WORD UDPPutROMArray(ROM BYTE *cData, WORD wDataLen)
{
	WORD wTemp;

	wTemp = (MAC_TX_BUFFER_SIZE - sizeof(IP_HEADER) - sizeof(UDP_HEADER)) - wPutOffset;
	if(wTemp < wDataLen)
		wDataLen = wTemp;

	wPutOffset += wDataLen;
	if(wPutOffset > UDPTxCount)
		UDPTxCount = wPutOffset;

    // Load application data bytes
    MACPutROMArray(cData, wDataLen);

    return wDataLen;
}
#endif

BYTE* UDPPutString(BYTE *strData)
{
	return strData + UDPPutArray(strData, strlen((char*)strData));
}

#if defined(__18CXX)
ROM BYTE* UDPPutROMString(ROM BYTE *strData)
{
	return strData + UDPPutROMArray(strData, strlenpgm((ROM char*)strData));
}
#endif


/*********************************************************************
 * Function:        BOOL UDPFlush(void)
 *
 * PreCondition:    UDPPut() is already called and desired UDP socket
 *                  is set as an active socket by calling
 *                  UDPIsPutReady().
 *
 * Input:           None
 *
 * Output:          All and any data associated with active UDP socket
 *                  buffer is marked as ready for transmission.
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            This function transmit all data from
 *                  an active UDP socket.
 ********************************************************************/
void UDPFlush(void)
{
    UDP_HEADER      h;
    UDP_SOCKET_INFO *p;
    WORD 			wReadPtrSave;
    WORD			wChecksum;
    WORD			wUDPLength;

    p = &UDPSocketInfo[activeUDPSocket];

	wUDPLength = UDPTxCount + sizeof(UDP_HEADER);

	// Generate the correct UDP header
    h.SourcePort        = swaps(p->localPort);
    h.DestinationPort   = swaps(p->remotePort);
    h.Length            = swaps(wUDPLength);
	h.Checksum 			= 0x0000;
    
	// Calculate IP pseudoheader checksum if we are going to enable 
	// the checksum field
	#if defined(UDP_USE_TX_CHECKSUM)
	{
		PSEUDO_HEADER   pseudoHeader;
		
		pseudoHeader.SourceAddress	= AppConfig.MyIPAddr;
		pseudoHeader.DestAddress    = p->remoteNode.IPAddr;
		pseudoHeader.Zero           = 0x0;
		pseudoHeader.Protocol       = IP_PROT_UDP;
		pseudoHeader.Length			= wUDPLength;
		SwapPseudoHeader(pseudoHeader);
		h.Checksum = ~CalcIPChecksum((BYTE*)&pseudoHeader, sizeof(pseudoHeader));
	}
	#endif

	// Position the hardware write pointer where we will need to 
	// begin writing the IP header
	MACSetWritePtr(BASE_TX_ADDR + sizeof(ETHER_HEADER));
	
	// Write IP header to packet
	IPPutHeader(&p->remoteNode, IP_PROT_UDP, wUDPLength);

    // Write UDP header to packet
    MACPutArray((BYTE*)&h, sizeof(h));
    
	// Calculate the final UDP checksum and write it in, if enabled
	#if defined(UDP_USE_TX_CHECKSUM)
	{
		wReadPtrSave = MACSetReadPtr(BASE_TX_ADDR + sizeof(ETHER_HEADER) + sizeof(IP_HEADER));
		wChecksum = CalcIPBufferChecksum(wUDPLength);
		MACSetReadPtr(wReadPtrSave);
		MACSetWritePtr(BASE_TX_ADDR + sizeof(ETHER_HEADER) + sizeof(IP_HEADER) + 6);	// 6 is the offset to the Checksum field in UDP_HEADER
		MACPutArray((BYTE*)&wChecksum, sizeof(wChecksum));
	}
	#endif
    
	// Transmit the packet
    MACFlush();

	// Reset packet size counter for the next TX operation
    UDPTxCount = 0;
	LastPutSocket = INVALID_UDP_SOCKET;
}



/*********************************************************************
 * Function:        WORD UDPIsGetReady(UDP_SOCKET s)
 *
 * PreCondition:    UDPInit() is already called.
 *
 * Input:           A valid UDP socket that is already "Listen"ed on
 *                  or opened.
 *
 * Output:          WORD: Number of available bytes that can be 
 *					retrieved.  0 if no data received on this socket.
 *
 * Side Effects:    Given socket is set as an active UDP Socket.
 *
 * Overview:        None
 *
 * Note:            This function automatically sets supplied socket
 *                  as an active socket.  Caller need not call
 *                  explicit function UDPSetActiveSocket().  All
 *                  subsequent calls will us this socket as an
 *                  active socket.
 ********************************************************************/
WORD UDPIsGetReady(UDP_SOCKET s)
{
    activeUDPSocket = s;
	if(SocketWithRxData != s)
		return 0;

    return UDPRxCount;
}

/*********************************************************************
 * Function:        BOOL UDPGet(BYTE *v)
 *
 * PreCondition:    UDPInit() is already called     AND
 *                  UDPIsGetReady(s) > 0
 *
 * Input:           v       - Buffer to receive UDP data byte
 *
 * Output:          TRUE    if a data byte was read
 *                  FALSE   if no data byte was read or available
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            This function fetches data from an active UDP
 *                  socket as set by UDPIsGetReady() call.
 ********************************************************************/
BOOL UDPGet(BYTE *v)
{
	// Make sure that there is data to return
    if(UDPRxCount == 0u || (SocketWithRxData != activeUDPSocket))
        return FALSE;

    // If if this very first read to packet, set MAC Rx Pointer to
    // beginig of UDP data area.
    if(Flags.bFirstRead)
    {
        Flags.bFirstRead = 0;
        UDPSetRxBuffer(0);
    }

    *v = MACGet();
	UDPRxCount--;

    if(UDPRxCount == 0u)
    {
        UDPDiscard();
    }

    return TRUE;
}

/*********************************************************************
 * Function:        WORD UDPGetArray(BYTE *cData, WORD wDataLen)
 *
 * PreCondition:    UDPInit() is already called     AND
 *                  UDPIsGetReady(s) > 0
 *
 * Input:           *cData - Pointer to location to write retrieved bytes
 *					wDataLen - Number of bytes to retreive
 *				
 * Output:          WORD - Number of bytes written to cData
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            This function fetches data from an active UDP
 *                  socket as set by UDPIsGetReady()
 ********************************************************************/
WORD UDPGetArray(BYTE *cData, WORD wDataLen)
{
	// Make sure that there is data to return
    if(UDPRxCount == 0u || (SocketWithRxData != activeUDPSocket))
		return 0;

    // If this is the very first read of the packet, set MAC Rx Pointer to
    // beginig of UDP data area.
    if(Flags.bFirstRead)
    {
        Flags.bFirstRead = 0;
        UDPSetRxBuffer(0);
    }

	// Make sure we don't try to read more data than exists
	if(UDPRxCount < wDataLen)
		wDataLen = UDPRxCount;

	wDataLen = MACGetArray(cData, wDataLen);
    UDPRxCount -= wDataLen;

    if(UDPRxCount == 0u)
    {
        UDPDiscard();
    }

    return wDataLen;
}

/*********************************************************************
 * Function:        void UDPDiscard(void)
 *
 * PreCondition:    UDPInit() is already called
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function discards any UDP socket Rx content, 
 *					if any.
 *
 * Note:            It is safe to call this function more than needed.  
 *					If no data is available, this function does 
 *					nothing.
 ********************************************************************/
void UDPDiscard(void)
{
	if(!Flags.bWasDiscarded)
	{
		MACDiscardRx();
		UDPRxCount = 0;
		Flags.bWasDiscarded = 1;
	}
}


/*********************************************************************
 * Function:        BOOL UDPProcess(NODE_INFO* remoteNode,
 *                                  IP_ADDR *localIP,
 *                                  WORD len)
 *
 * PreCondition:    UDPInit() is already called     AND
 *                  UDP segment is ready in MAC buffer
 *
 * Input:           remoteNode      - Remote node info
 *					localIP			- Destination IP address of the packet
 *                  len             - Total length of UDP semgent.
 *
 * Output:          TRUE if a valid packet is waiting
 *                  FALSE if the packet was discarded
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/
BOOL UDPProcess(NODE_INFO *remoteNode, IP_ADDR *localIP, WORD len)
{
    UDP_HEADER		h;
    UDP_SOCKET		s;
    PSEUDO_HEADER	pseudoHeader;
    DWORD_VAL		checksums;

	SocketWithRxData = INVALID_UDP_SOCKET;
	UDPRxCount = 0;
	Flags.bFirstRead = 0;

    // Retrieve UDP header.
    MACGetArray((BYTE*)&h, sizeof(h));

    h.SourcePort        = swaps(h.SourcePort);
    h.DestinationPort   = swaps(h.DestinationPort);
    h.Length            = swaps(h.Length) - sizeof(UDP_HEADER);

	// See if we need to validate the checksum field (0x0000 is disabled)
	if(h.Checksum)
	{
	    // Calculate IP pseudoheader checksum.
	    pseudoHeader.SourceAddress		= remoteNode->IPAddr;
	    pseudoHeader.DestAddress.Val	= localIP->Val;
	    pseudoHeader.Zero				= 0x0;
	    pseudoHeader.Protocol			= IP_PROT_UDP;
	    pseudoHeader.Length				= len;

	    SwapPseudoHeader(pseudoHeader);
	
	    checksums.w[0] = ~CalcIPChecksum((BYTE*)&pseudoHeader,
	                                    sizeof(pseudoHeader));
	
	
	    // Now calculate UDP packet checksum in NIC RAM -- should match pseudoHeader
	    IPSetRxBuffer(0);
	    checksums.w[1] = CalcIPBufferChecksum(len);
	
	    if(checksums.w[0] != checksums.w[1])
	    {
	        MACDiscardRx();
	        return FALSE;
	    }
	}

    s = FindMatchingSocket(&h, remoteNode, localIP);
    if(s == INVALID_UDP_SOCKET)
    {
        // If there is no matching socket, There is no one to handle
        // this data.  Discard it.
        MACDiscardRx();
		return FALSE;
    }
    else
    {
		SocketWithRxData = s;
        UDPRxCount = h.Length;
        Flags.bFirstRead = 1;
		Flags.bWasDiscarded = 0;
    }


    return TRUE;
}


/*********************************************************************
 * Function:        UDP_SOCKET FindMatchingSocket(UDP_HEADER *h,
 *                                NODE_INFO *remoteNode,
 *                                IP_ADDR *localIP)
 *
 * PreCondition:    UDP Segment header has been retrieved from buffer
 *                  The IP header has also been retrieved
 *
 * Input:           remoteNode      - Remote node info from IP header
 *                  h               - header of UDP semgent.
 *
 * Output:          matching UDP socket or INVALID_UDP_SOCKET
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/
static UDP_SOCKET FindMatchingSocket(UDP_HEADER *h,
                                     NODE_INFO *remoteNode,
                                     IP_ADDR *localIP)
{
    UDP_SOCKET s;
    UDP_SOCKET partialMatch;
    UDP_SOCKET_INFO *p;

    partialMatch = INVALID_UDP_SOCKET;

    p = UDPSocketInfo;
    for ( s = 0; s < MAX_UDP_SOCKETS; s++ )
    {
        // This packet is said to be matching with current socket:
        // 1. If its destination port matches with our local port and
        // 2. Packet source IP address matches with socket remote IP address.
        //    OR this socket had transmitted packet with destination address as broadcast (subnet or limited broadcast).
        if ( p->localPort == h->DestinationPort )
        {
            if(p->remotePort == h->SourcePort)
            {
                if( (p->remoteNode.IPAddr.Val == remoteNode->IPAddr.Val) ||
                    (localIP->Val == 0xFFFFFFFFul) || 
					(localIP->Val == (AppConfig.MyIPAddr.Val | (~AppConfig.MyMask.Val))))
                {
                    return s;
                }
            }

            partialMatch = s;
        }
        p++;
    }

    if ( partialMatch != INVALID_UDP_SOCKET )
    {
        p = &UDPSocketInfo[partialMatch];

        memcpy((void*)&p->remoteNode,
                (const void*)remoteNode, sizeof(p->remoteNode) );

        p->remotePort = h->SourcePort;
    }
    return partialMatch;
}


#endif //#if defined(STACK_USE_UDP)
