/*********************************************************************
 *
 *	Microchip TCP/IP Stack Demo Application Configuration Header
 *
 *********************************************************************
 * FileName:        TCPIPConfig.h
 * Dependencies:    Microchip TCP/IP Stack
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
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Howard Schlunder		10/04/2006	Original
 ********************************************************************/
#ifndef __TCPIPCONFIG_H
#define __TCPIPCONFIG_H

#include "TCPIP Stack/TCPIP.h"

/*
 * Modules to include in this project
 */
#define STACK_USE_UART					// Application demo using UART for IP address display and stack configuration
#define STACK_USE_UART2TCP_BRIDGE		// UART to TCP Bridge application example
//#define STACK_USE_IP_GLEANING
#define STACK_USE_ICMP_SERVER
#define STACK_USE_ICMP_CLIENT
//#define STACK_USE_HTTP_SERVER			// Old HTTP server
#define STACK_USE_HTTP2_SERVER			// New HTTP server with POST, Cookies, Authentication, etc.
//#define STACK_USE_SSL_SERVER			// SSL / TLS server.  Not currently implemented.
//#define STACK_USE_DHCP_CLIENT
//#define STACK_USE_DHCP_SERVER
//#define STACK_USE_FTP_SERVER
#define STACK_USE_SMTP_CLIENT
#define STACK_USE_SNMP_SERVER
//#define STACK_USE_TFTP_CLIENT
//#define STACK_USE_GENERIC_TCP_CLIENT_EXAMPLE	// HTTP Client example in GenericTCPClient.c
//#define STACK_USE_GENERIC_TCP_SERVER_EXAMPLE	// ToUpper server example in GenericTCPServer.c
//#define STACK_USE_TELNET_SERVER			// Telnet server
#define STACK_USE_ANNOUNCE				// Microchip Embedded Ethernet Device Discoverer server/client
//#define STACK_USE_DNS					// Domain Name Service Client
//#define STACK_USE_NBNS					// NetBIOS Name Service Server
#define STACK_USE_REBOOT_SERVER			// Module for resetting this PIC remotely.  Primarily useful for a Bootloader.
//#define STACK_USE_SNTP_CLIENT			// Simple Network Time Protocol for obtaining current date/time from Internet
//#define STACK_USE_UDP_PERFORMANCE_TEST	// Module for testing UDP TX performance characteristics.  NOTE: Enabling this will cause a huge amount of UDP broadcast packets to flood your network on various ports.  Use care when enabling this on production networks, especially with VPNs (could tunnel broadcast traffic across a limited bandwidth connection).
//#define STACK_USE_TCP_PERFORMANCE_TEST	// Module for testing TCP TX performance characteristics

//
// MPFS Settings
//

/*
 * If html pages are stored in internal program memory,
 * comment MPFS_USE_EEPROM and include an MPFS image (.c file) 
 * in the project.  If html pages are stored in external EEPROM 
 * memory, uncomment MPFS_USE_EEPROM
 */
//#define MPFS_USE_EEPROM

/*
 * If using the 1Mbit EEPROM, uncomment this line
 */
//#define USE_EEPROM_25LC1024

/*
 * Number of EEPROM bytes to be reserved before MPFS storage starts.
 *
 * These bytes host application configurations such as IP Address,
 * MAC Address, and any other required variables.
 *
 * If using the DOS MPFS Utility, you must recreate your MPFS Image
 * using the correct reserve block size. See MPFS.exe /? for details.
 *
 * For MPFS2, no configuration in the MPFS2.exe utility is required.
 */
#define MPFS_RESERVE_BLOCK              (64)

/*
 * Maximum number of simultaneously open MPFS2 files.
 *
 * For MPFS Classic, this has no effect.
 */
#define MAX_MPFS_HANDLES				(7ul)




/*
 * Following low level modules are automatically enabled/disabled based on high-level
 * module selections.
 * If you need them with your custom application, enable it here.
 */
//#define STACK_USE_TCP
//#define STACK_USE_UDP

/*
 * MPFS is automatically included when required for other applications.
 * If your custom application requires it, uncomment the appropriate selection.
 */
//#define STACK_USE_MPFS
//#define STACK_USE_MPFS2

/*
 * Uncomment following line if this stack will be used in CLIENT
 * mode.  In CLIENT mode, some functions specific to client operation
 * are enabled.
 */
//#define STACK_CLIENT_MODE

// Make sure MPFS included for modules that require it
#if defined(STACK_USE_FTP_SERVER) || defined(STACK_USE_HTTP_SERVER)
	#define STACK_USE_MPFS
#endif

#if defined(STACK_USE_HTTP2_SERVER)
	#define STACK_USE_MPFS2
#endif

#if defined(STACK_USE_SNMP_SERVER) && !defined(STACK_USE_MPFS) && !defined(STACK_USE_MPFS2)
	#define STACK_USE_MPFS2
#endif

// FTP is not supported in MPFS2 or when MPFS is stored in internal program 
// memory (instead of external EEPROM).
#if !defined(MPFS_USE_EEPROM) || defined(STACK_USE_MPFS2)
	#undef STACK_USE_FTP_SERVER
#endif

// Comment following line if SNMP TRAP support is needed
//#define SNMP_TRAP_DISABLED

// When IP Gleaning is enabled, ICMP must also be enabled.
#if defined(STACK_USE_IP_GLEANING)
    #if !defined(STACK_USE_ICMP_SERVER)
        #define STACK_USE_ICMP_SERVER
    #endif
#endif

// Make sure that the DNS client is enabled if services require it
#if defined(STACK_USE_GENERIC_TCP_CLIENT_EXAMPLE) || \
	defined(STACK_USE_SNTP_CLIENT) || \
	defined(STACK_USE_SMTP_CLIENT)
    #if !defined(STACK_USE_DNS)
        #define STACK_USE_DNS
    #endif
#endif

// Make sure that STACK_CLIENT_MODE is defined if a service 
// depends on it
#if defined(STACK_USE_FTP_SERVER) || \
	defined(STACK_USE_SNMP_SERVER) || \
	defined(STACK_USE_DNS) || \
	defined(STACK_USE_GENERIC_TCP_CLIENT_EXAMPLE) || \
	defined(STACK_USE_TFTP_CLIENT) || \
	defined(STACK_USE_SMTP_CLIENT) || \
	defined(STACK_USE_ICMP_CLIENT) || \
	defined(STACK_USE_SNTP_CLIENT)
	#if !defined(STACK_CLIENT_MODE)
	    #define STACK_CLIENT_MODE
	#endif
#endif

// Make sure that STACK_USE_TCP is defined if a service depends on 
// it
#if defined(STACK_USE_UART2TCP_BRIDGE) || \
	defined(STACK_USE_HTTP_SERVER) || \
	defined(STACK_USE_HTTP2_SERVER) || \
	defined(STACK_USE_FTP_SERVER) || \
	defined(STACK_USE_TELNET_SERVER) || \
	defined(STACK_USE_GENERIC_TCP_CLIENT_EXAMPLE) || \
	defined(STACK_USE_GENERIC_TCP_SERVER_EXAMPLE) || \
	defined(STACK_USE_SMTP_CLIENT) || \
	defined(STACK_USE_TCP_PERFORMANCE_TEST)
    #if !defined(STACK_USE_TCP)
        #define STACK_USE_TCP
    #endif
#endif

// Make sure that STACK_USE_UDP is defined if a service depends 
// on it
#if defined(STACK_USE_DHCP_CLIENT) || \
	defined(STACK_USE_DNS) || \
	defined(STACK_USE_NBNS) || \
	defined(STACK_USE_SNMP_SERVER) || \
	defined(STACK_USE_TFTP_CLIENT) || \
	defined(STACK_USE_ANNOUNCE) || \
	defined(STACK_USE_UDP_PERFORMANCE_TEST) || \
	defined(STACK_USE_SNTP_CLIENT)
    #if !defined(STACK_USE_UDP)
        #define STACK_USE_UDP
    #endif
#endif


#if defined(LCD_DATA_IO) || defined(LCD_DATA0_IO)
	#define USE_LCD
#endif


//
// Default Address information - If not found in data EEPROM.
//
#define MY_DEFAULT_HOST_NAME			"MCHPBOARD"

#define MY_DEFAULT_MAC_BYTE1            (0x00)
#define MY_DEFAULT_MAC_BYTE2            (0x16)
#define MY_DEFAULT_MAC_BYTE3            (0xA9)
#define MY_DEFAULT_MAC_BYTE4            (0x00)
#define MY_DEFAULT_MAC_BYTE5            (0x01)
#define MY_DEFAULT_MAC_BYTE6            (0x1C)

#define MY_DEFAULT_IP_ADDR_BYTE1        (172ul)
#define MY_DEFAULT_IP_ADDR_BYTE2        (16ul)
#define MY_DEFAULT_IP_ADDR_BYTE3        (1ul)
#define MY_DEFAULT_IP_ADDR_BYTE4        (253ul)

#define MY_DEFAULT_MASK_BYTE1           (255ul)
#define MY_DEFAULT_MASK_BYTE2           (255ul)
#define MY_DEFAULT_MASK_BYTE3           (252ul)
#define MY_DEFAULT_MASK_BYTE4           (0ul)

#define MY_DEFAULT_GATE_BYTE1           MY_DEFAULT_IP_ADDR_BYTE1
#define MY_DEFAULT_GATE_BYTE2           MY_DEFAULT_IP_ADDR_BYTE2
#define MY_DEFAULT_GATE_BYTE3           MY_DEFAULT_IP_ADDR_BYTE3
#define MY_DEFAULT_GATE_BYTE4           (100ul)

#define MY_DEFAULT_PRIMARY_DNS_BYTE1	MY_DEFAULT_GATE_BYTE1
#define MY_DEFAULT_PRIMARY_DNS_BYTE2	MY_DEFAULT_GATE_BYTE2
#define MY_DEFAULT_PRIMARY_DNS_BYTE3	MY_DEFAULT_GATE_BYTE3
#define MY_DEFAULT_PRIMARY_DNS_BYTE4	MY_DEFAULT_GATE_BYTE4

#define MY_DEFAULT_SECONDARY_DNS_BYTE1	MY_DEFAULT_GATE_BYTE1
#define MY_DEFAULT_SECONDARY_DNS_BYTE2	MY_DEFAULT_GATE_BYTE2
#define MY_DEFAULT_SECONDARY_DNS_BYTE3	MY_DEFAULT_GATE_BYTE3
#define MY_DEFAULT_SECONDARY_DNS_BYTE4	MY_DEFAULT_GATE_BYTE4

//
// TCP and UDP protocol options
//

#if defined(STACK_USE_TCP)
	// Allocate how much total RAM (in bytes) you want to allocate 
	// for use by your TCP TCBs, RX FIFOs, and TX FIFOs.  
	// Sockets can be scattered across several different storage 
	// mediums if you are out of space on one medium.
	#define TCP_ETH_RAM	0
	#define TCP_ETH_RAM_BASE_ADDRESS			(BASE_TCB_ADDR)
	#define TCP_ETH_RAM_SIZE					3600
	#define TCP_PIC_RAM	1
	#define TCP_PIC_RAM_BASE_ADDRESS			((PTR_BASE)&TCPBufferInPIC[0])
	#define TCP_PIC_RAM_SIZE					0
	#define TCP_SPI_RAM	2
	#define TCP_SPI_RAM_BASE_ADDRESS			(0x0000u)
	#define TCP_SPI_RAM_SIZE					0

	// These are the different types of TCP sockets 
	// that you want to use.  Each different type can have a 
	// different RX FIFO size, TX FIFO size, and even be stored in 
	// a different physical memory medium, optimizing storage
	#define TCP_PURPOSE_DEFAULT					0
	#define TCP_PURPOSE_GENERIC_TCP_CLIENT		1
	#define TCP_PURPOSE_GENERIC_TCP_SERVER		2
	#define TCP_PURPOSE_TELNET					3
	#define TCP_PURPOSE_FTP_COMMAND				4
	#define TCP_PURPOSE_FTP_DATA				5
	#define TCP_PURPOSE_TCP_PERFORMANCE_TX		6
	#define TCP_PURPOSE_TCP_PERFORMANCE_RX		7
	#define TCP_PURPOSE_UART_2_TCP_BRIDGE		8
	#define TCP_PURPOSE_MP3_CLIENT				9
	
	#if defined(__TCP_C)
		// Define how many sockets are needed, what type they are,
		// where their TCB, TX FIFO, and RX FIFO should be stored, 
		// and how big the RX and TX FIFOs should be.  Making this 
		// initializer bigger or smaller defines how many total TCP
		// sockets are available.
		// Each socket requires up to 48 bytes of PIC RAM and 
		// 40+(TX FIFO size)+(RX FIFO size) bytes bytes of 
		// TCP_*_RAM each.
		// Note: The RX FIFO must be at least 1 byte in order to 
		// receive SYN and FIN messages required by TCP.  The TX 
		// FIFO can be zero if desired.
		ROM struct
		{
			BYTE vSocketPurpose;
			BYTE vMemoryMedium;
			WORD wTXBufferSize;
			WORD wRXBufferSize;
		} TCPSocketInitializer[] = 
		{
			{TCP_PURPOSE_GENERIC_TCP_CLIENT, 	TCP_ETH_RAM, 125, 200}, 
			{TCP_PURPOSE_GENERIC_TCP_SERVER, 	TCP_ETH_RAM, 20, 20}, 
			{TCP_PURPOSE_TELNET, 				TCP_ETH_RAM, 150, 20}, 
			//{TCP_PURPOSE_FTP_COMMAND, 			TCP_ETH_RAM, 100, 40}, 
			//{TCP_PURPOSE_FTP_DATA, 				TCP_ETH_RAM, 0, 128}, 
			{TCP_PURPOSE_TCP_PERFORMANCE_TX,	TCP_ETH_RAM, 256, 1}, 
			//{TCP_PURPOSE_TCP_PERFORMANCE_RX,	TCP_PIC_RAM, 40, 2560}, 
			{TCP_PURPOSE_UART_2_TCP_BRIDGE, 	TCP_ETH_RAM, 256, 256}, 
			//{TCP_PURPOSE_MP3_CLIENT, 			TCP_SPI_RAM, 50, 31000}, 
			{TCP_PURPOSE_DEFAULT,				TCP_ETH_RAM, 200, 200},
			{TCP_PURPOSE_DEFAULT,				TCP_ETH_RAM, 200, 200},
			{TCP_PURPOSE_DEFAULT,				TCP_ETH_RAM, 200, 200},
			{TCP_PURPOSE_DEFAULT,				TCP_ETH_RAM, 200, 200},
			//{TCP_PURPOSE_DEFAULT,				TCP_ETH_RAM, 200, 200},
			//{TCP_PURPOSE_DEFAULT,				TCP_ETH_RAM, 200, 200},
		};

		// If PIC RAM is used to store TCP socket FIFOs and TCBs, 
		// let's allocate it so the linker dynamically chooses 
		// where to locate it and prevents other variables from 
		// overlapping with it
		#if TCP_PIC_RAM_SIZE > 0
			static BYTE TCPBufferInPIC[TCP_PIC_RAM_SIZE] __attribute__((far));
		#endif
	#endif
#else
	// Don't allocate any RAM for TCP if TCP module isn't enabled
	#define TCP_ETH_RAM_SIZE 0
	#define TCP_PIC_RAM_SIZE 0
	#define TCP_SPI_RAM_SIZE 0
#endif


// Maximum avaialble UDP Sockets
#define MAX_UDP_SOCKETS     (9ul)
#define UDP_USE_TX_CHECKSUM		// This slows UDP TX performance by nearly 50%


// 
// HTTP2 Server options
//

// Maximum numbers of simultaneous HTTP connections allowed.
// Each connection consumes 2 bytes of RAM and a TCP socket
#define MAX_HTTP_CONNECTIONS	(3ul)

// Indicate what file to serve when no specific one is requested
#define HTTP_DEFAULT_FILE		"index.htm"
#define HTTPS_DEFAULT_FILE		"index.htm"
#define HTTP_DEFAULT_LEN		10			//for buffer overrun protection
											//set to longest length of above two strings

// Configure MPFS over HTTP updating
// Comment this line to disable updating via HTTP
#define HTTP_MPFS_UPLOAD		"mpfsupload"
//#define HTTP_MPFS_UPLOAD_REQUIRES_AUTH	//Uncomment to require auth on MPFS uploads
										// Certain firewall and router combinations cause
										// the MPFS2 Utility to fail when uploading.  If
										// this happens, comment out this definition.

// Define which HTTP modules to use
#define HTTP_USE_POST					// Enable POST support (~0.5k ROM)
#define HTTP_USE_COOKIES				// Enable cookie support (~0.9k ROM)
#define HTTP_USE_AUTHENTICATION			// Enable basic authentication support (~0.9k ROM)

//#define HTTP_NO_AUTH_WITHOUT_SSL		// Uncomment to require SSL before requesting a password
#define HTTP_SSL_ONLY_CHAR		0xff	// Files beginning with this character will only be served over HTTPS
										// Set to NUL to require for all files
										// Set to 0xff to require for no files

//
// SSL Server Options
//
#define MAX_SSL_CONNECTIONS		(3ul)	//maximum connections via SSL
#define MAX_SSL_SESSIONS		(1ul)	//max # of cached SSL sessions
#define MAX_SSL_BUFFERS			(4ul)	//max # of SSL buffers (handshake needs 1, app mode needs 2)
#define MAX_SSL_HASHES			(4ul)	//each transmitting connection needs 2

#define SSL_RSA_KEY_SIZE		(512ul)	//bits in SSL RSA key


					
// This enables the Custom HTTP Demo App to use the MD5 function
// Your application likely does not need this functionality, and so
// this should be commented out to save ROM and RAM.
// Impact: ~5KB ROM, ~160B RAM
#define STACK_USE_MD5

// This enables the Custom HTTP Demo App to use the AppConfig 
// configuration web page.
// Impact: ~2.4KB ROM, ~0B RAM
#define STACK_USE_APP_RECONFIG

#endif
