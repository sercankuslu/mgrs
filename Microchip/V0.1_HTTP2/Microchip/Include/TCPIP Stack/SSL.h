/*********************************************************************
 *
 *                  SSLv3 / TLS 1.0 Module Headers
 *
 *********************************************************************
 * FileName:        SSL.h
 * Dependencies:    None
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
 * Elliott Wood			6/20/07	Original
********************************************************************/

#ifndef __SSL_H
#define __SSL_H

/*********************************************************************
 * Server Configuration Settings
 ********************************************************************/
	#define SSL_PORT               	(443u)		//listening port for SSL
	
	#define SSL_VERSION				(0x0300)	//SSL version number
	#define SSL_VERSION_HI			(0x03)		//SSL version number
	#define SSL_VERSION_LO			(0x00)		//SSL version number
	
/*********************************************************************
 * Commands and Server Responses
 ********************************************************************/
	//SSL Protocol Codes
	#define SSL_CHANGE_CIPHER_SPEC	20u
	#define SSL_ALERT				21u
	#define SSL_HANDSHAKE			22u
	#define SSL_APPLICATION			23u
	#define SSL_APPLICATION_DATA	0xFF
	
	typedef enum _SSL_HANDSHAKE_TYPE
	{
		SSL_HELLO_REQUEST		= 0u,
		SSL_CLIENT_HELLO		= 1u,
		SSL_ANTIQUE_CLIENT_HELLO = 100u,
		SSL_SERVER_HELLO		= 2u,
		SSL_CERTIFICATE 		= 11u,
		SSL_SERVER_HELLO_DONE	= 14u,
		SSL_CLIENT_KEY_EXCHANGE	= 16u,
		SSL_FINISHED			= 20u
	} SSL_HANDSHAKE_TYPE;
	
	typedef enum _SSL_ALERT_LEVEL
	{
		SSL_ALERT_WARNING 	= 1u,
		SSL_ALERT_FATAL		= 2u
	} SSL_ALERT_LEVEL;
	
	typedef enum _SSL_ALERT_DESCRIPTION
	{
		SSL_ALERT_CLOSE_NOTIFY			= 0u,
		SSL_ALERT_UNEXPECTED_MESSAGE	= 10u,
		SSL_ALERT_BAD_RECORD_MAC		= 20u,
		SSL_ALERT_HANDSHAKE_FAILURE		= 40u
	} SSL_ALERT_DESCRIPTION;
	
/*********************************************************************
 * Memory Space Definitions
 ********************************************************************/
 	//TODO: this ONLY works for a 512 bit key right now, but conserves
	//		hundreds of bytes of RAM
	typedef union _SSL_BUFFER
	{
		struct
		{								//used as RSA Buffers in HS
			BIGINT_DATA_TYPE A[2*RSA_KEY_WORDS];
			BIGINT_DATA_TYPE B[2*RSA_KEY_WORDS];
		} rsa;
		struct
		{								//used as handshake MAC calcs
			HASH_SUM hash;
			BYTE md5_hash[16];
			BYTE sha_hash[20];
			BYTE temp[256-sizeof(HASH_SUM)-16-20];
		} handshake;	
		BYTE full[256];					//used as ARCFOUR Sbox in app
	} SSL_BUFFER;		
	
	typedef struct _SSL_SESSION
	{
		BYTE sessionID[32];
		BIGINT_DATA_TYPE masterSecret[RSA_KEY_WORDS];
		BYTE age;
	} SSL_SESSION;

	#define _SSL_TEMP_A 	curBuffer.rsa.A
	#define _SSL_TEMP_B		curBuffer.rsa.B
	#define ARCFOURsbox		curBuffer.full
	#define SSL_INVALID_ID	(0xff)

/*********************************************************************
 * SSL State Machines
 ********************************************************************/

	//SSL State Machine
	typedef enum _SM_SSL
	{
		SM_SSL_IDLE = 0u,
		SM_SSL_HANDSHAKE,
		SM_SSL_HANDSHAKE_DONE,
		SM_SSL_APPLICATION,
		SM_SSL_APPLICATION_DONE,
		SM_SSL_CLEANUP
	} SM_SSL;

	//SSL Receive State Machine
	typedef enum _SM_SSL_RX
	{
		SM_SSL_RX_IDLE = 0u,
		SM_SSL_RX_READ_DATA,
		SM_SSL_RX_SET_UP_DECRYPT,
		SM_SSL_RX_DECRYPT_PREMASTER_SECRET,
		SM_SSL_RX_CALC_MASTER_SECRET,
		SM_SSL_RX_GENERATE_KEYS
	} SM_SSL_RX;
	
	//SSL Transmit State Machine
	typedef enum _SM_SSL_TX
	{
		SM_SSL_TX_IDLE = 0u,
		SM_SSL_TX_SEND_MESSAGE,
		SM_SSL_TX_SEND_DATA,
		SM_SSL_TX_DISCONNECT,
		SM_SSL_TX_WAIT
	} SM_SSL_TX;

/*********************************************************************
 * SSL Connection Struct Definition
 ********************************************************************/

	typedef struct _SSL_CONN
	{
	    SM_SSL_RX smRX;						//receiving state machine
	    SM_SSL_TX smTX;						//sending state machine
		
		WORD rxBytesRem;					//bytes left to read in current record
		WORD rxHSBytesRem;					//bytes left in current Handshake submessage
		WORD txBytesRem;					//bytes left to write in current record
		
		SSL_HANDSHAKE_TYPE rxHSType;		//handshake message being received
		BYTE rxProtocol;					//protocol for message being read
		
		BYTE idSession;						//SSL_SESSION reference
		BYTE idRXHash, idTXHash;			//SSH_HASH references for app mode
		BYTE idSHA1, idMD5;					//SSH_HASH references for app mode
		BYTE idRXSbox, idTXSbox;			//SSL_BUFFER references for S-Boxes

		SSL_ALERT_LEVEL reqAlertLevel;		//requested alert level
		SSL_ALERT_DESCRIPTION reqAlertDesc;	//requested alert type
		
		//flags relating to the client's current state
		union
		{
			struct
			{
				unsigned rxClientHello			: 1;
				unsigned rxClientKeyExchange	: 1;
				unsigned rxChangeCipherSpec		: 1;
				unsigned rxFinished				: 1;
				unsigned rxCloseNotify			: 1;
				unsigned okARCFOUR_128_MD5		: 1;
				unsigned okARCFOUR_40_MD5		: 1;
				unsigned reserved				: 1;
			} bits;
			BYTE byte;
		} clientFlags;

		//flags relating to the server's current state
		union
		{
			struct
			{
				unsigned reqServerHello			: 1;
				unsigned txServerHello			: 1;
				unsigned reqServerCertDone		: 1;
				unsigned txServerCertDone		: 1;
				unsigned reqCCSFin				: 1;
				unsigned txCCSFin				: 1;
				unsigned reqAlert				: 1;
				unsigned txAlert				: 1;
			} bits;
			BYTE byte;
		} serverFlags;
		
		//server area, switches usage for handshake vs application mode
		union
		{
			struct
			{
				BYTE MACSecret[16];				//Server's MAC write secret
				DWORD sequence;					//Server's write sequence number
				ARCFOUR_CTX cryptCtx;			//Server's write encryption context
				BYTE reserved[8];				//future expansion
			}app;
			BYTE random[32];					//Server.random value
		} Server;
		
		//client area, switches usage for handshake vs application mode
		union
		{
			struct
			{
				BYTE MACSecret[16];				//Client's MAC write secret
				DWORD sequence;					//Client's write sequence number
				ARCFOUR_CTX cryptCtx;			//Client's write encryption context
				BYTE reserved[8];				//future expansion
			}app;
			BYTE random[32];					//Client.random value
		} Client;
		
	} SSL_CONN;
	
	//stored in RAM for speed
	typedef struct _SSL_STUB
	{
		TCP_SOCKET SSLskt;					//socket being served
	    TCP_SOCKET FWDskt;					//socket to loop app data through
	    SM_SSL smSSL;						//SSL state machine
	} SSL_STUB;
	
	//define how much SSL memory we need to reserve in Ethernet buffer
	#define SSL_CONN_SIZE		((DWORD)sizeof(SSL_CONN))
	#define SSL_CONN_SPACE		(SSL_CONN_SIZE*MAX_SSL_CONNECTIONS)
	#define SSL_HASH_SIZE		((DWORD)sizeof(HASH_SUM))
	#define SSL_HASH_SPACE		((DWORD)(SSL_HASH_SIZE*MAX_SSL_HASHES))
	#define SSL_BUFFER_SIZE		((DWORD)sizeof(SSL_BUFFER))
	#define SSL_BUFFER_SPACE	(SSL_BUFFER_SIZE*MAX_SSL_BUFFERS)
	#define RESERVED_SSL_MEMORY ((DWORD)(SSL_CONN_SPACE + SSL_HASH_SPACE + SSL_BUFFER_SPACE))


void SSLInit(void);

void SSLServer(void);


#endif
