/*********************************************************************
 *
 *               HTTP Headers for Microchip TCP/IP Stack
 *
 *********************************************************************
 * FileName:        HTTP2.h
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
 * Nilesh Rajbharti     8/14/01 Original
 * Elliott Wood			6/4/07	Complete rewrite (known as HTTP2)
********************************************************************/

#ifndef __HTTP2_H
#define __HTTP2_H

#include "TCPIP Stack/TCPIP.h"

#if defined(STACK_USE_HTTP2_SERVER)

/*********************************************************************
 * Server Configuration Settings
 ********************************************************************/
	#define HTTP_PORT               (80u)	// Listening port for HTTP server
	#define HTTP_MAX_DATA_LEN		(100u)	// Max bytes to store get and cookie args
	#define HTTP_MIN_CALLBACK_FREE	(16u)	// Min bytes free in TX FIFO before callbacks execute
	#define HTTP_CACHE_LEN			("600")	// Max lifetime (sec) of static responses as string
	#define HTTP_TIMEOUT			(45u)	// Max time (sec) to await more data before

	// Authentication requires Base64 decoding
	#if defined(HTTP_USE_AUTHENTICATION)
		#ifndef STACK_USE_BASE64_DECODE
			#define STACK_USE_BASE64_DECODE
		#endif
	#endif

/*********************************************************************
 * Commands and Server Responses
 ********************************************************************/
	// Supported Commands and Server Response Codes
	typedef enum _HTTP_STATUS
	{
	    HTTP_GET = 0u,					// GET command is being processed
	    HTTP_POST,						// POST command is being processed
		HTTP_UNAUTHORIZED,				// 401 Unauthorized will be returned
	    HTTP_NOT_FOUND,					// 404 Not Found will be returned
		HTTP_OVERFLOW,					// 414 Request-URI Too Long will be returned
		HTTP_INTERNAL_SERVER_ERROR,		// 500 Internal Server Error will be returned
		HTTP_NOT_IMPLEMENTED,			// 501 Not Implemented (not a GET or POST command)
		#if defined(HTTP_MPFS_UPLOAD)
		HTTP_MPFS_FORM,					// Show the MPFS Upload form
		HTTP_MPFS_UP,					// An MPFS Upload is being processed
		HTTP_MPFS_OK,					// An MPFS Upload was successful
		HTTP_MPFS_ERROR,				// An MPFS Upload was not a valid image
		#endif
		HTTP_REDIRECT,					// 302 Redirect will be returned
		HTTP_SSL_REQUIRED				// 403 Forbidden is returned, indicating SSL is required
	} HTTP_STATUS;
	
/*********************************************************************
 * HTTP State Definitions
 ********************************************************************/

	// Basic HTTP Connection State Machine
	typedef enum _SM_HTTP2
	{
		SM_HTTP_IDLE = 0u,				// Socket is idle
		SM_HTTP_PARSE_REQUEST,			// Parses the first line for a file name and GET args
		SM_HTTP_PARSE_HEADERS,			// Reads and parses headers one at a time
		SM_HTTP_AUTHENTICATE,			// Validates the current authorization state
		SM_HTTP_PROCESS_GET,			// Invokes user callback for GET args or cookies
		SM_HTTP_PROCESS_POST,			// Invokes user callback for POSTed data
		SM_HTTP_PROCESS_REQUEST,		// Begins the process of returning data
		SM_HTTP_SERVE_HEADERS,			// Sends any required headers for the response
		SM_HTTP_SERVE_COOKIES,			// Adds any cookies to the response
		SM_HTTP_SERVE_BODY,				// Serves the actual content
		SM_HTTP_SEND_FROM_CALLBACK,		// Invokes a dynamic variable callback
		SM_HTTP_DISCONNECT,				// Disconnects the server and closes all files
		SM_HTTP_WAIT					// Unused state
	} SM_HTTP2;
	
	// Result States for Read and Write Operations
	typedef enum _HTTP_IO_RESULT
	{
		HTTP_IO_DONE = 0u,	// Finished with procedure
		HTTP_IO_NEED_DATA,	// More data needed to continue, call again later
		HTTP_IO_WAITING,	// Waiting for asynchronous process to complete, call again later
	} HTTP_IO_RESULT;

	// File Type Definitions
	typedef enum _HTTP_FILE_TYPE
	{
		HTTP_TXT = 0u,		// File is a text document
		HTTP_HTM,			// File is HTML (extension .htm)
		HTTP_HTML,			// File is HTML (extension .html)
		HTTP_CGI,			// File is HTML (extension .cgi)
		HTTP_XML,			// File is XML (extension .xml)
		HTTP_CSS,			// File is stylesheet (extension .css)
		HTTP_GIF,			// File is GIF image (extension .gif)
		HTTP_PNG,			// File is PNG image (extension .png)
		HTTP_JPG,			// File is JPG image (extension .jpg)
		HTTP_JAVA,			// File is java (extension .java)
		HTTP_WAV,			// File is audio (extension .wav)
		HTTP_UNKNOWN		// File type is unknown
	} HTTP_FILE_TYPE;

	// HTTP Connection Struct
	// Stores partial state data for each connection
	// Meant for storage in fast access RAM
	typedef struct _HTTP_STUB
	{
	    SM_HTTP2 sm;						// Current connection state
	    TCP_SOCKET socket;					// Socket being served
	} HTTP_STUB;

	#define sktHTTP		httpStubs[curHTTPID].socket		// Access the current socket
	#define smHTTP		httpStubs[curHTTPID].sm			// Access the current state machine

	// Stores extended state data for each connection
	typedef struct _HTTP_CONN
	{
		DWORD byteCount;					// How many bytes have been read so far
		DWORD nextCallback;					// Byte index of the next callback
		DWORD callbackID;					// Callback ID to execute, also used as watchdog timer
		DWORD callbackPos;					// Callback position indicator
		BYTE *ptrData;						// Points to first free byte in data
		BYTE *ptrRead;						// Points to current read location
		MPFS_HANDLE file;					// File pointer for the file being served
	    MPFS_HANDLE offsets;				// File pointer for any offset info being used
		BYTE hasArgs;						// True if there were get or cookie arguments	
		BYTE isAuthorized;					// 0x00-0x79 on fail, 0x80-0xff on pass
		HTTP_STATUS httpStatus;				// Request method/status
	    HTTP_FILE_TYPE fileType;			// File type to return with Content-Type
		BYTE data[HTTP_MAX_DATA_LEN];		// General purpose data buffer
	} HTTP_CONN;
	
	#define RESERVED_HTTP_MEMORY ( (DWORD)MAX_HTTP_CONNECTIONS * (DWORD)sizeof(HTTP_CONN))


void HTTPInit(void);
void HTTPServer(void);

    /*
     * Main application must implement these callback functions
     * to complete HTTP2 implementation.
	 * These functions may set cookies by setting curHTTP.hasArgs to
	 *   indicate the number of cookies, and curHTTP.data with null terminated
	 *   name and value pairs.  (Both name and value are null terminated.)
	 * These functions may store info in curHTTP.data, but will be overwriting
	 * any GET or cookie arguments.
     */
	HTTP_IO_RESULT HTTPExecuteGet(void);
#ifdef HTTP_USE_POST
	HTTP_IO_RESULT HTTPExecutePost(void);
#endif

#ifdef HTTP_USE_AUTHENTICATION
	extern BYTE HTTPAuthenticate(BYTE *user, BYTE *pass, BYTE *filename);
#endif

	BYTE* HTTPURLDecode(BYTE *data);
	BYTE* HTTPGetArg(BYTE *data, BYTE* arg);
	void HTTPIncFile(ROM BYTE* file);

	#if defined(__18CXX)
		// ROM function variants for PIC18
		BYTE* HTTPGetROMArg(BYTE *data, ROM BYTE* arg);
	#else
		// Identical macro for C30
		#define HTTPGetROMArg(a,b)	HTTPGetArg(a,(BYTE*)b)
	#endif

#endif

#endif
