/*********************************************************************
 *
 *  HyperText Transfer Protocol (HTTP) Server
 *  Module for Microchip TCP/IP Stack
 *   -Serves dynamic pages to web browsers such as Microsoft Internet 
 *    Explorer, Mozilla Firefox, etc.
 *	 -Reference: RFC 2616
 *
 **********************************************************************
 * FileName:        HTTP2.c
 * Dependencies:    TCP, MPFS2, Tick, CustomHTTPApp.c callbacks
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
 * Nilesh Rajbharti     8/14/01     Original
 * Elliott Wood			6/4/07		Complete rewrite, known as HTTP2
 ********************************************************************/

#define __HTTP2_C

#include "TCPIP Stack/TCPIP.h"
#include "HTTPPrint.h"

#if defined(STACK_USE_HTTP2_SERVER)

// Only allow MPFS uploading if using EEPROM for storage
#if !defined(MPFS_USE_EEPROM) || !defined(HTTP_USE_POST)
	#undef HTTP_MPFS_UPLOAD
#endif

/*********************************************************************
 * String Constants
 ********************************************************************/
	static ROM BYTE HTTP_CRLF[] = "\r\n";	// New line sequence
	#define HTTP_CRLF_LEN	2				// Length of above string
		
/*********************************************************************
 * File and Content Type Settings
 ********************************************************************/
	
	// Corresponding File Type Extensions
	static ROM char *httpFileExtensions[HTTP_UNKNOWN+1] =
	{
	    "txt",          // HTTP_TXT
	    "htm",          // HTTP_HTM
	    "html",         // HTTP_HTML
	    "cgi",          // HTTP_CGI
	    "xml",          // HTTP_XML
	    "css",          // HTTP_CSS
	    "gif",          // HTTP_GIF
	    "png",          // HTTP_PNG
	    "jpg",          // HTTP_JPG
	    "cla",          // HTTP_JAVA
	    "wav",          // HTTP_WAV
		"\0\0\0"		// HTTP_UNKNOWN
	};
	
	// Corresponding Content Type Strings
	static ROM char *httpContentTypes[HTTP_UNKNOWN+1] =
	{
	    "text/plain",            // HTTP_TXT
	    "text/html",             // HTTP_HTM
	    "text/html",             // HTTP_HTML
	    "text/html",             // HTTP_CGI
	    "text/xml",              // HTTP_XML
	    "text/css",              // HTTP_CSS
	    "image/gif",             // HTTP_GIF
	    "image/png",             // HTTP_PNG
	    "image/jpeg",            // HTTP_JPG
	    "application/java-vm",   // HTTP_JAVA
	    "audio/x-wave",          // HTTP_WAV
		""						 // HTTP_UNKNOWN
	};
		
/*********************************************************************
 * Commands and Server Responses
 ********************************************************************/

	// Corresponding initial response strings (to HTTP_STATUS enum)
	static ROM char *HTTPResponseHeaders[] =
	{
		"HTTP/1.1 200 OK\r\nConnection: close\r\n",
		"HTTP/1.1 200 OK\r\nConnection: close\r\n",
		"HTTP/1.1 401 Unauthorized\r\nWWW-Authenticate: Basic realm=\"Protected\"\r\n\r\n401 Unauthorized: Password required\r\n",
		#if defined(HTTP_MPFS_UPLOAD)
		"HTTP/1.1 404 Not found\r\nContent-Type: text/html\r\n\r\n404: File not found<br>Use <a href=\"/mpfsupload\">MPFS Upload</a> to program web pages into EEPROM\r\n",
		#else		
		"HTTP/1.1 404 Not found\r\n\r\n404: File not found\r\n",
		#endif
		"HTTP/1.1 414 Request-URI Too Long\r\n\r\n414 Request-URI Too Long: Buffer overflow detected\r\n",
		"HTTP/1.1 500 Internal Server Error\r\n\r\n500 Internal Server Error: Expected data not present\r\n",
		"HTTP/1.1 501 Not Implemented\r\n\r\n501 Not Implemented: Only GET and POST supported\r\n",
		#if defined(HTTP_MPFS_UPLOAD)
		"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body style=\"margin:100px\"><form method=post action=\"/mpfsupload\" enctype=\"multipart/form-data\"><b>MPFS Image Upload</b><p><input type=file name=i size=40> &nbsp; <input type=submit value=\"Upload\"></form></body></html>",
		"",
		"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body style=\"margin:100px\"><b>MPFS Update Successful</b><p><a href=\"/\">Site main page</a></body></html>",
		"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body style=\"margin:100px\"><b>MPFS Image Corrupt or Wrong Version</b><p><a href=\"/mpfsupload\">Try again?</a></body></html>",
		#endif
		"HTTP/1.1 302 Found\r\nLocation: ",
		"HTTP/1.1 403 Forbidden\r\n\r\n403 Forbidden: SSL Required - use HTTPS\r\n"
	};
	
/*********************************************************************
 * Header Parsing Configuration
 ********************************************************************/
	#define HTTP_NUM_HEADERS		3
	
	// Header strings for which we'd like to parse
	static ROM char *HTTPRequestHeaders[HTTP_NUM_HEADERS] =
	{
		"Cookie:",
		"Authorization:",
		"Content-Length:"
	};
	
	// Set to length of longest string above
	#define HTTP_MAX_HEADER_LEN		(15u)

/*********************************************************************
 * HTTP Connection State Global Variable
 ********************************************************************/
	#pragma udata HTTP_CONNECTION_STATES
	HTTP_CONN curHTTP;							// Current HTTP connection state
	HTTP_STUB httpStubs[MAX_HTTP_CONNECTIONS];	// HTTP stubs with state machine and socket
	BYTE curHTTPID;								// Which HTTP stub is active
	#pragma udata

/*********************************************************************
 * Function Prototypes
 ********************************************************************/

	// Prototypes for header parsers
	static void HTTPHeaderParseLookup(BYTE i);
	#if defined(HTTP_USE_COOKIES)
	static void HTTPHeaderParseCookie(void);
	#endif
	#if defined(HTTP_USE_AUTHENTICATION)
	static void HTTPHeaderParseAuthorization(void);
	#endif
	#if defined(HTTP_USE_POST)
	static void HTTPHeaderParseContentLength(void);
	#endif
	
	// Internal function Prototypes
	static void HTTPProcess(void);
	static BOOL HTTPSendFile(void);
	static void HTTPLoadConn(BYTE connID);

	#if defined(HTTP_MPFS_UPLOAD)
	static HTTP_IO_RESULT HTTPMPFSUpload(void);
	#endif

	#define mMIN(a, b)	((a<b)?a:b)

/*********************************************************************
 * Function:        void HTTPInit(void)
 *
 * PreCondition:    TCP must already be initialized.
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Sets all HTTP sockets to listening state.
 *                  Initialize state machine for each connection.
 *
 * Note:            This function is called only one during lifetime
 *                  of the application.
 ********************************************************************/
void HTTPInit(void)
{
	WORD oldPtr;

	// Make sure the file handles are invalidated
	curHTTP.file = MPFS_INVALID_HANDLE;
	curHTTP.offsets = MPFS_INVALID_HANDLE;
		
    for(curHTTPID = 0; curHTTPID < MAX_HTTP_CONNECTIONS; curHTTPID++)
    {
		smHTTP = SM_HTTP_IDLE;
		sktHTTP = TCPListen(HTTP_PORT);
		
	    // Save the default record (just invalid file handles)
    	oldPtr = MACSetWritePtr(BASE_HTTPB_ADDR + curHTTPID*sizeof(HTTP_CONN));
		MACPutArray((BYTE*)&curHTTP, sizeof(HTTP_CONN));
		MACSetWritePtr(oldPtr);
    }
}



/*********************************************************************
 * Function:        void HTTPServer(void)
 *
 * PreCondition:    HTTPInit() must already be called.
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Browse through each connection and try to bring
 *					it a step closer to completion.
 *
 * Note:            This function acts as a task (similar to one in
 *                  RTOS).  This function performs its task in
 *                  co-operative manner.  Main application must call
 *                  this function repeatdly to ensure all open
 *                  or new connections are served on time.
 ********************************************************************/
void HTTPServer(void)
{
	BYTE conn;

	for(conn = 0; conn < MAX_HTTP_CONNECTIONS; conn++)
	{
		if(httpStubs[conn].socket == INVALID_SOCKET)
			continue;
		
		// Determine if this connection is eligible for processing
		if(httpStubs[conn].sm != SM_HTTP_IDLE || TCPIsGetReady(httpStubs[conn].socket))
		{
			HTTPLoadConn(conn);
			HTTPProcess();
		}
	}
}

/*********************************************************************
 * Function:        static void HTTPLoadConn(BYTE connID)
 *
 * PreCondition:    None
 *
 * Input:           connID the connection ID to load
 *
 * Output:          curHTTP has a new connection loaded
 *
 * Side Effects:    None
 *
 * Overview:        Loads the current HTTP connection out of Ethernet
 *					buffer RAM and into local RAM for processing.
 *
 * Note:            None
 ********************************************************************/
static void HTTPLoadConn(BYTE connID)
{
    WORD oldPtr;
    
    // Return if already loaded
    if(connID == curHTTPID)
    	return;
    
    // Save the old one
    oldPtr = MACSetWritePtr(BASE_HTTPB_ADDR + curHTTPID*sizeof(HTTP_CONN));
	MACPutArray((BYTE*)&curHTTP, sizeof(HTTP_CONN));
	MACSetWritePtr(oldPtr);
	
	// Load the new one
    oldPtr = MACSetReadPtr(BASE_HTTPB_ADDR + connID*sizeof(HTTP_CONN));
	MACGetArray((BYTE*)&curHTTP, sizeof(HTTP_CONN));
	MACSetReadPtr(oldPtr);
	
	// Remember which one is loaded
	curHTTPID = connID;
			
}

/*********************************************************************
 * Function:        static void HTTPProcess(void)
 *
 * PreCondition:    HTTPInit() called and curHTTP loaded
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:		Serves the current HTTP connection in curHTTP
 *
 * Note:            None
 ********************************************************************/
static void HTTPProcess(void)
{
    WORD lenA, lenB;
	BYTE c, i;
    BOOL isDone;
	BYTE *ext;
	BYTE buffer[HTTP_MAX_HEADER_LEN+1];

    do
    {
        isDone = TRUE;

        // If a socket is disconnected at any time 
        // forget about it and return to idle state.
        if(TCPWasReset(sktHTTP))
        {
            smHTTP = SM_HTTP_IDLE;

			// Make sure any opened files are closed
			if(curHTTP.file != MPFS_INVALID_HANDLE)
			{
				MPFSClose(curHTTP.file);
				curHTTP.file = MPFS_INVALID_HANDLE;
			}
			if(curHTTP.offsets != MPFS_INVALID_HANDLE)
			{
				MPFSClose(curHTTP.offsets);
				curHTTP.offsets = MPFS_INVALID_HANDLE;
			}

			// Adjust the TCP FIFOs for optimal reception of 
			// the next HTTP request from the browser
			TCPAdjustFIFOSize(sktHTTP, 1, 0, TCP_ADJUST_GIVE_REST_TO_RX | TCP_ADJUST_PRESERVE_RX);
        }


        switch(smHTTP)
        {

        case SM_HTTP_IDLE:

			// Check how much data is waiting
			lenA = TCPIsGetReady(sktHTTP);

			// If a connection has been made, then process the request
            if(lenA)
            {// Clear out state info and move to next state
				curHTTP.ptrData = curHTTP.data;
				smHTTP = SM_HTTP_PARSE_REQUEST;
				curHTTP.isAuthorized = 0xff;
				curHTTP.hasArgs = FALSE;
				curHTTP.callbackID = TickGet() + HTTP_TIMEOUT*TICK_SECOND;
				curHTTP.callbackPos = 0xffffffff;
				curHTTP.byteCount = 0;
			}
			
			// In all cases, we break
			// For new connections, this waits for the buffer to fill
			break;

		case SM_HTTP_PARSE_REQUEST:

			// Verify the entire first line is in the FIFO
			if(TCPFind(sktHTTP, '\n', 0, FALSE) == 0xffff)
			{// First line isn't here yet
				if(TCPGetRxFIFOFree(sktHTTP) == 0)
				{// If the FIFO is full, we overflowed
					curHTTP.httpStatus = HTTP_OVERFLOW;
					smHTTP = SM_HTTP_SERVE_HEADERS;
					isDone = FALSE;
				}
				if(TickGet() > curHTTP.callbackID)
				{// A timeout has occurred
					TCPDisconnect(sktHTTP);
					smHTTP = SM_HTTP_DISCONNECT;
					isDone = FALSE;
				}
				break;
			}

			// Reset the watchdog timer
			curHTTP.callbackID = TickGet() + HTTP_TIMEOUT*TICK_SECOND;

			// Determine the request method
			lenA = TCPFind(sktHTTP, ' ', 0, FALSE);
			if(lenA > 5)
				lenA = 5;
			TCPGetArray(sktHTTP, curHTTP.data, lenA+1);

		    if ( memcmppgm2ram(curHTTP.data, (ROM void*)"GET", 3) == 0)
			    curHTTP.httpStatus = HTTP_GET;
			#if defined(HTTP_USE_POST)
		    else if ( memcmppgm2ram(curHTTP.data, (ROM void*)"POST", 4) == 0)
			    curHTTP.httpStatus = HTTP_POST;
			#endif
		    else
			{// Unrecognized method, so return not implemented
		        curHTTP.httpStatus = HTTP_NOT_IMPLEMENTED;
				smHTTP = SM_HTTP_SERVE_HEADERS;
				isDone = FALSE;
				break;
			}

			// Find end of filename
			lenA = TCPFind(sktHTTP, ' ', 0, FALSE);
			lenB = TCPFindEx(sktHTTP, '?', 0, lenA, FALSE);
			lenA = mMIN(lenA, lenB);
			
			// If the file name is too long, then reject the request
			if(lenA > HTTP_MAX_DATA_LEN - HTTP_DEFAULT_LEN - 1)
			{
				curHTTP.httpStatus = HTTP_OVERFLOW;
				smHTTP = SM_HTTP_SERVE_HEADERS;
				isDone = FALSE;
				break;
			}

			// Read in the filename and decode
			lenB = TCPGetArray(sktHTTP, curHTTP.data, lenA);
			curHTTP.data[lenB] = '\0';
			HTTPURLDecode(curHTTP.data);
			
			// Check if this is an MPFS Upload
			#if defined(HTTP_MPFS_UPLOAD)
			if(memcmppgm2ram(&curHTTP.data[1], HTTP_MPFS_UPLOAD, strlenpgm(HTTP_MPFS_UPLOAD)) == 0)
			{// Read remainder of line, and bypass all file opening, etc.
				#if defined(HTTP_USE_AUTHENTICATION)
				curHTTP.isAuthorized = HTTPAuthenticate(NULL, NULL, &curHTTP.data[1]);
				#endif
				if(curHTTP.httpStatus == HTTP_GET)
					curHTTP.httpStatus = HTTP_MPFS_FORM;
				else
					curHTTP.httpStatus = HTTP_MPFS_UP;

				smHTTP = SM_HTTP_PARSE_HEADERS;
				isDone = FALSE;
				break;
			}
			#endif
			
			// If the last character is a not a directory delimiter, then try to open the file
			// String starts at 2nd character, because the first is always a '/'
			if(curHTTP.data[lenB-1] != '/')
				curHTTP.file = MPFSOpen(&curHTTP.data[1]);
				
			// If the open fails, then add our default name and try again
			if(curHTTP.file == MPFS_INVALID_HANDLE)
			{
				// Add the directory delimiter if needed
				if(curHTTP.data[lenB-1] != '/')
					curHTTP.data[lenB++] = '/';
				
				// Add our default file name			
				// If this is a loopback, then it's an SSL connection
				if(TCPIsLoopback(sktHTTP))
				{
					strcpypgm2ram((void*)&curHTTP.data[lenB], HTTPS_DEFAULT_FILE);
					lenB += strlenpgm(HTTPS_DEFAULT_FILE);
				}
				else
				{
					strcpypgm2ram((void*)&curHTTP.data[lenB], HTTP_DEFAULT_FILE);
					lenB += strlenpgm(HTTP_DEFAULT_FILE);
				}
					
				// Try to open again
				curHTTP.file = MPFSOpen(&curHTTP.data[1]);
			}
			
			// Find the extension in the filename
			for(ext = curHTTP.data + lenB-1; ext != curHTTP.data; ext--)
				if(*ext == '.')
					break;
					
			// Compare to known extensions to determine Content-Type
			ext++;
			for(curHTTP.fileType = HTTP_TXT; curHTTP.fileType < HTTP_UNKNOWN; curHTTP.fileType++)
				if(!stricmppgm2ram(ext, (ROM void*)httpFileExtensions[curHTTP.fileType]))
					break;
			
			// Perform first round authentication (pass file name only)
			#if defined(HTTP_USE_AUTHENTICATION)
			curHTTP.isAuthorized = HTTPAuthenticate(NULL, NULL, &curHTTP.data[1]);
			#endif
			
			// If the file was found, see if it has an index
			if(curHTTP.file != MPFS_INVALID_HANDLE &&
				(MPFSGetFlags(curHTTP.file) & MPFS2_FLAG_HASINDEX) )
			{
				curHTTP.data[lenB-1] = '#';
				curHTTP.offsets = MPFSOpen(&curHTTP.data[1]);
			}

			// Read GET args, up to buffer size - 1
			lenA = TCPFind(sktHTTP, ' ', 0, FALSE);
			if(lenA != 0)
			{
				curHTTP.hasArgs = TRUE;
				
				// Trash the '?'
				TCPGet(sktHTTP, &c);

				// Verify there's enough space
				lenA--;
				if(lenA >= HTTP_MAX_DATA_LEN - 2)
				{
			        curHTTP.httpStatus = HTTP_OVERFLOW;
					smHTTP = SM_HTTP_SERVE_HEADERS;
					isDone = FALSE;
					break;
				}

				// Read in the arguments and '&'-terminate in anticipation of cookies
				curHTTP.ptrData += TCPGetArray(sktHTTP, curHTTP.data, lenA);
				*(curHTTP.ptrData++) = '&';

			}

			// Clear the rest of the line
			lenA = TCPFind(sktHTTP, '\n', 0, FALSE);
			TCPGetArray(sktHTTP, NULL, lenA + 1);

			// Move to parsing the headers
			smHTTP = SM_HTTP_PARSE_HEADERS;
			
			// No break, continue to parsing headers

		case SM_HTTP_PARSE_HEADERS:

			// Loop over all the headers
			while(1)
			{
				// Make sure entire line is in the FIFO
				lenA = TCPFind(sktHTTP, '\n', 0, FALSE);
				if(lenA == 0xffff)
				{// If not, make sure we can receive more data
					if(TCPGetRxFIFOFree(sktHTTP) == 0)
					{// Overflow
						curHTTP.httpStatus = HTTP_OVERFLOW;
						smHTTP = SM_HTTP_SERVE_HEADERS;
						isDone = FALSE;
					}
					if(TickGet() > curHTTP.callbackID)
					{// A timeout has occured
						TCPDisconnect(sktHTTP);
						smHTTP = SM_HTTP_DISCONNECT;
						isDone = FALSE;
					}
					break;
				}
				
				// Reset the watchdog timer
				curHTTP.callbackID = TickGet() + HTTP_TIMEOUT*TICK_SECOND;
				
				// If a CRLF is immediate, then headers are done
				if(lenA == 1)
				{// Remove the CRLF and move to next state
					TCPGetArray(sktHTTP, NULL, 2);
					smHTTP = SM_HTTP_AUTHENTICATE;
					isDone = FALSE;
					break;
				}
	
				// Find the header name, and use isDone as a flag to indicate a match
				lenB = TCPFindEx(sktHTTP, ':', 0, lenA, FALSE) + 2;
				isDone = FALSE;
	
				// If name is too long or this line isn't a header, ignore it
				if(lenB > sizeof(buffer))
				{
					TCPGetArray(sktHTTP, NULL, lenA+1);
					continue;
				}
				
				// Read in the header name
				TCPGetArray(sktHTTP, buffer, lenB);
				buffer[lenB-1] = '\0';
				lenA -= lenB;
		
				// Compare header read to ones we're interested in
				for(i = 0; i < HTTP_NUM_HEADERS; i++)
				{
					if(strcmppgm2ram((char*)buffer, (ROM char *)HTTPRequestHeaders[i]) == 0)
					{// Parse the header and stop the loop
						HTTPHeaderParseLookup(i);
						isDone = TRUE;
						break;
					}
				}
				
				// Clear the rest of the line, and call the loop again
				if(isDone)
				{// We already know how much to remove unless a header was found
					lenA = TCPFind(sktHTTP, '\n', 0, FALSE);
				}
				TCPGetArray(sktHTTP, NULL, lenA+1);
			}
			
			break;

		case SM_HTTP_AUTHENTICATE:
		
			#if defined(HTTP_USE_AUTHENTICATION)
			// Check current authorization state
			if(curHTTP.isAuthorized < 0x80)
			{// 401 error
				curHTTP.httpStatus = HTTP_UNAUTHORIZED;
				smHTTP = SM_HTTP_SERVE_HEADERS;
				isDone = FALSE;
				
				#if defined(HTTP_NO_AUTH_WITHOUT_SSL)
				if(!TCPIsLoopback(sktHTTP))
					curHTTP.httpStatus = HTTP_SSL_REQUIRED;
				#endif

				break;
			}
			#endif

			// Parse the args string
			*curHTTP.ptrData = '\0';
			curHTTP.ptrData = HTTPURLDecode(curHTTP.data);

			// If this is an MPFS upload form request, bypass to headers
			#if defined(HTTP_MPFS_UPLOAD)
			if(curHTTP.httpStatus == HTTP_MPFS_FORM)
			{
				smHTTP = SM_HTTP_SERVE_HEADERS;
				isDone = FALSE;
				break;
			}
			#endif
			
			// Move on to GET args, unless there are none
			smHTTP = SM_HTTP_PROCESS_GET;
			if(!curHTTP.hasArgs)
				smHTTP = SM_HTTP_PROCESS_POST;
			isDone = FALSE;
			curHTTP.hasArgs = FALSE;
			break;

		case SM_HTTP_PROCESS_GET:

			// Run the application callback HTTPExecuteGet()
			if(HTTPExecuteGet() == HTTP_IO_WAITING)
			{// If waiting for asynchronous process, return to main app
				break;
			}

			// Move on to POST data
			smHTTP = SM_HTTP_PROCESS_POST;

		case SM_HTTP_PROCESS_POST:

			#if defined(HTTP_USE_POST)
			
			// See if we have any new data
			if(TCPIsGetReady(sktHTTP) == curHTTP.callbackPos)
			{
				if(TickGet() > curHTTP.callbackID)
				{// If a timeout has occured, disconnect
					TCPDisconnect(sktHTTP);
					smHTTP = SM_HTTP_DISCONNECT;
					isDone = FALSE;
					break;
				}
			}
			
			if(curHTTP.httpStatus == HTTP_POST 
				#if defined(HTTP_MPFS_UPLOAD)
				|| (curHTTP.httpStatus >= HTTP_MPFS_UP && curHTTP.httpStatus <= HTTP_MPFS_ERROR)
				#endif
				 )
			{
				// Run the application callback HTTPExecutePost()
				#if defined(HTTP_MPFS_UPLOAD)
				if(curHTTP.httpStatus >= HTTP_MPFS_UP && curHTTP.httpStatus <= HTTP_MPFS_ERROR)
				{
					c = HTTPMPFSUpload();
					if(c == HTTP_IO_DONE)
					{
						smHTTP = SM_HTTP_SERVE_HEADERS;
						isDone = FALSE;
						break;
					}
				}
				else
				#endif
				c = HTTPExecutePost();
				
				// If waiting for asynchronous process, return to main app
				if(c == HTTP_IO_WAITING)
				{// return to main app and make sure we don't get stuck by the watchdog
					curHTTP.callbackPos = TCPIsGetReady(sktHTTP) - 1;
					break;
				} else if(c == HTTP_IO_NEED_DATA)
				{// If waiting for more data
					curHTTP.callbackPos = TCPIsGetReady(sktHTTP);
					curHTTP.callbackID = TickGet() + HTTP_TIMEOUT*TICK_SECOND;
					// If more is expected and space is available, return to main app
					if(curHTTP.byteCount > 0 && TCPGetRxFIFOFree(sktHTTP) != 0)
						break;
					else
					{// Handle cases where application ran out of data or buffer space
						curHTTP.httpStatus = HTTP_INTERNAL_SERVER_ERROR;
						smHTTP = SM_HTTP_SERVE_HEADERS;
						isDone = FALSE;
						break;
					}	
				}
			}
			#endif

			// We're done with POST
			smHTTP = SM_HTTP_PROCESS_REQUEST;
			// No break, continue to sending request

		case SM_HTTP_PROCESS_REQUEST:

			// Check for 404
            if(curHTTP.file == MPFS_INVALID_HANDLE)
            {
                curHTTP.httpStatus = HTTP_NOT_FOUND;
                smHTTP = SM_HTTP_SERVE_HEADERS;
                isDone = FALSE;
                break;
            }

			// Set up the dynamic substitutions
			curHTTP.byteCount = 0;
			if(curHTTP.offsets == MPFS_INVALID_HANDLE)
            {// If no index file, then set next offset to huge
	            curHTTP.nextCallback = 0xffffffff;
            }
            else
            {// Read in the next callback index
	            MPFSGetLong(curHTTP.offsets, &(curHTTP.nextCallback));
			}
			
			// Move to next state
			smHTTP = SM_HTTP_SERVE_HEADERS;

		case SM_HTTP_SERVE_HEADERS:

			// We're in write mode now:
			// Adjust the TCP FIFOs for optimal transmission of 
			// the HTTP response to the browser
			TCPAdjustFIFOSize(sktHTTP, 1, 0, TCP_ADJUST_GIVE_REST_TO_TX);
				
			// Send headers
			TCPPutROMString(sktHTTP, (ROM BYTE*)HTTPResponseHeaders[curHTTP.httpStatus]);
			
			// If this is a redirect, print the rest of the Location: header			   
			if(curHTTP.httpStatus == HTTP_REDIRECT)
			{
				TCPPutString(sktHTTP, curHTTP.data);
				TCPPutROMString(sktHTTP, (ROM BYTE*)"\r\n\r\n304 Redirect: ");
				TCPPutString(sktHTTP, curHTTP.data);
				TCPPutROMString(sktHTTP, (ROM BYTE*)HTTP_CRLF);
			}

			// If not GET or POST, we're done
			if(curHTTP.httpStatus != HTTP_GET && curHTTP.httpStatus != HTTP_POST)
			{// Disconnect
				smHTTP = SM_HTTP_DISCONNECT;
				break;
			}

			// Output the content type, if known
			if(curHTTP.fileType != HTTP_UNKNOWN)
			{
				TCPPutROMString(sktHTTP, (ROM BYTE*)"Content-Type: ");
				TCPPutROMString(sktHTTP, (ROM BYTE*)httpContentTypes[curHTTP.fileType]);
				TCPPutROMString(sktHTTP, HTTP_CRLF);
			}
			
			// Output the gzip encoding header if needed
			if(MPFSGetFlags(curHTTP.file) & MPFS2_FLAG_ISZIPPED)
			{
				TCPPutROMString(sktHTTP, (ROM BYTE*)"Content-Encoding: gzip\r\n");
			}
						
			// Output the cache-control
			TCPPutROMString(sktHTTP, (ROM BYTE*)"Cache-Control: ");
			if(curHTTP.httpStatus == HTTP_POST || curHTTP.nextCallback != 0xffffffff)
			{// This is a dynamic page or a POST request, so no cache
				TCPPutROMString(sktHTTP, (ROM BYTE*)"no-cache");
			}
			else
			{// This is a static page, so save it for the specified amount of time
				TCPPutROMString(sktHTTP, (ROM BYTE*)"max-age=");
				TCPPutROMString(sktHTTP, (ROM BYTE*)HTTP_CACHE_LEN);
			}
			TCPPutROMString(sktHTTP, HTTP_CRLF);
			
			// Check if we should output cookies
			if(curHTTP.hasArgs)
				smHTTP = SM_HTTP_SERVE_COOKIES;
			else
			{// Terminate the headers
				TCPPutROMString(sktHTTP, HTTP_CRLF);
				smHTTP = SM_HTTP_SERVE_BODY;
			}
	
			// Move to next stage
			isDone = FALSE;
			break;

		case SM_HTTP_SERVE_COOKIES:

			#if defined(HTTP_USE_COOKIES)
			// If the TX FIFO runs out of space, the client will never get CRLFCRLF
			// Avoid writing huge cookies - keep it under a hundred bytes max

			// Write cookies one at a time as space permits
			for(curHTTP.ptrRead = curHTTP.data; curHTTP.hasArgs != 0; curHTTP.hasArgs--)
			{
				// Write the header
				TCPPutROMString(sktHTTP, (ROM BYTE*)"Set-Cookie: ");

				// Write the name, URL encoded, one character at a time
				while((c = *(curHTTP.ptrRead++)))
				{
					if(c == ' ')
						TCPPut(sktHTTP, '+');
					else if(c < '0' || (c > '9' && c < 'A') || (c > 'Z' && c < 'a') || c > 'z')
					{
						TCPPut(sktHTTP, '%');
						TCPPut(sktHTTP, btohexa_high(c));
						TCPPut(sktHTTP, btohexa_low(c));
					}
					else
						TCPPut(sktHTTP, c);
				}
				
				TCPPut(sktHTTP, '=');
				
				// Write the value, URL encoded, one character at a time
				while((c = *(curHTTP.ptrRead++)))
				{
					if(c == ' ')
						TCPPut(sktHTTP, '+');
					else if(c < '0' || (c > '9' && c < 'A') || (c > 'Z' && c < 'a') || c > 'z')
					{
						TCPPut(sktHTTP, '%');
						TCPPut(sktHTTP, btohexa_high(c));
						TCPPut(sktHTTP, btohexa_low(c));
					}
					else
						TCPPut(sktHTTP, c);
				}
				
				// Finish the line
				TCPPutROMString(sktHTTP, HTTP_CRLF);

			}
			#endif

			// We're done, move to next state
			TCPPutROMString(sktHTTP, HTTP_CRLF);
			smHTTP = SM_HTTP_SERVE_BODY;

		case SM_HTTP_SERVE_BODY:

			isDone = FALSE;

			// Try to send next packet
			if(HTTPSendFile())
			{// If EOF, then we're done so close and disconnect
				MPFSClose(curHTTP.file);
				curHTTP.file = MPFS_INVALID_HANDLE;
				smHTTP = SM_HTTP_DISCONNECT;
				isDone = TRUE;
			}
			
			// If the TX FIFO is full, then return to main app loop
			if(TCPIsPutReady(sktHTTP) == 0)
				isDone = TRUE;
            break;

		case SM_HTTP_SEND_FROM_CALLBACK:

			isDone = TRUE;

			// Check that at least the minimum bytes are free
			if(TCPIsPutReady(sktHTTP) < HTTP_MIN_CALLBACK_FREE)
				break;

			// Fill TX FIFO from callback
			HTTPPrint(curHTTP.callbackID);
			
			if(curHTTP.callbackPos == 0)
			{// Callback finished its output, so move on
				isDone = FALSE;
				smHTTP = SM_HTTP_SERVE_BODY;
			}// Otherwise, callback needs more buffer space, so return and wait
			
			break;

		case SM_HTTP_DISCONNECT:
		
			// Loopbacks have no wait state, so all data must be retrieved first
			if(TCPIsLoopback(sktHTTP) && TCPGetTxFIFOFull(sktHTTP) != 0)
				break;

			// Make sure any opened files are closed
			if(curHTTP.file != MPFS_INVALID_HANDLE)
			{
				MPFSClose(curHTTP.file);
				curHTTP.file = MPFS_INVALID_HANDLE;
			}
			if(curHTTP.offsets != MPFS_INVALID_HANDLE)
			{
				MPFSClose(curHTTP.offsets);
				curHTTP.offsets = MPFS_INVALID_HANDLE;
			}

			TCPDisconnect(sktHTTP);
            smHTTP = SM_HTTP_IDLE;
            break;
		}
	} while(!isDone);

}


/*********************************************************************
 * Function:        static BOOL HTTPSendFile(void)
 *
 * PreCondition:    curHTTP.file and curHTTP.offsets have both been
 *					opened for reading.
 *
 * Input:           None
 *
 * Output:          TRUE if EOF was reached and reading is done
 *					FALSE if more data remains
 *
 * Side Effects:    None
 *
 * Overview:        This function serves the next chunk of curHTTP's
 *					file, up to a) available TX FIFO or b) up to
 *					the next recorded callback index, whichever comes
 *					first.
 *
 * Note:            None
 ********************************************************************/
static BOOL HTTPSendFile(void)
{
	WORD numBytes, len;
	BYTE c, data[64];
	
	// Determine how many bytes we can read right now
	numBytes = mMIN(TCPIsPutReady(sktHTTP), curHTTP.nextCallback - curHTTP.byteCount);
	
	// Get/put as many bytes as possible
	curHTTP.byteCount += numBytes;
	while(numBytes > 0)
	{
		len = MPFSGetArray(curHTTP.file, data, mMIN(numBytes, 64));
		if(len == 0)
			return TRUE;
		else
			TCPPutArray(sktHTTP, data, len);
		numBytes -= len;
	}
	
	// Check if a callback index was reached
	if(curHTTP.byteCount == curHTTP.nextCallback)
	{
		// Update the state machine
		smHTTP = SM_HTTP_SEND_FROM_CALLBACK;
		curHTTP.callbackPos = 0;

		// Read past the variable name and close the MPFS
		MPFSGet(curHTTP.file, NULL);
		do
		{
			if(!MPFSGet(curHTTP.file, &c))
				break;
			curHTTP.byteCount++;
		} while(c != '~');
		curHTTP.byteCount++;
		
		// Read in the callback address and next offset
		MPFSGetLong(curHTTP.offsets, &(curHTTP.callbackID));
		if(!MPFSGetLong(curHTTP.offsets, &(curHTTP.nextCallback)))
		{
			curHTTP.nextCallback = 0xffffffff;
			MPFSClose(curHTTP.offsets);
			curHTTP.offsets = MPFS_INVALID_HANDLE;
		}
	}

    // We are not done sending a file yet...
    return FALSE;
}

/*********************************************************************
 * Function:        static void HTTPHeaderParseLookup(BYTE i)
 *
 * PreCondition:    None
 *
 * Input:           i: the index of the header parser found
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Calls the appropriate header parser
 *
 * Note:            None
 ********************************************************************/
static void HTTPHeaderParseLookup(BYTE i)
{
	// i corresponds to an index in HTTPRequestHeaders
	
	#if defined(HTTP_USE_COOKIES)
	if(i == 0u)
	{
		HTTPHeaderParseCookie();
		return;
	}
	#endif
	
	#if defined(HTTP_USE_AUTHENTICATION)	
	if(i == 1u)
	{
		HTTPHeaderParseAuthorization();
		return;
	}
	#endif
	
	#if defined(HTTP_USE_POST)
	if(i == 2u)
	{
		HTTPHeaderParseContentLength();
		return;
	}
	#endif
}

/*********************************************************************
 * Function:        static void HTTPHeaderParseAuthorization(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          myConn:	isAuthorized is updated
 * 
 * Side Effects:    None
 *
 * Overview:        Parses the "Authorization:" header.
 *					For example, "BASIC YWRtaW46cGFzc3dvcmQ=\r\n" is:
 *						admin(\0)password(\0)
 *						^        ^
 *					    buf      ptrBuf
 *					Once read, the user's authorization function is 
 *					called to process and store a result.
 *
 * Note:            Tries to read up to and including the end of the 
 *					current line.  If not successful, a return of
 *					FALSE indicates that this function should be 
 *					called again when more data is available.
 ********************************************************************/
#if defined(HTTP_USE_AUTHENTICATION)
static void HTTPHeaderParseAuthorization(void)
{
    WORD len;
    BYTE buf[40];
	BYTE *ptrBuf;
	
	// If auth processing is not required, return
	if(curHTTP.isAuthorized & 0x80)
		return;

	// Clear the auth type ("BASIC ")
	TCPGetArray(sktHTTP, NULL, 6);

	// Find the terminating CRLF and make sure it's a multiple of four
	len = TCPFindROMArray(sktHTTP, HTTP_CRLF, HTTP_CRLF_LEN, 0, FALSE);
	len += 3;
	len &= 0xfc;
	len = mMIN(len, sizeof(buf)-4);
	
	// Read in 4 bytes at a time and decode (slower, but saves RAM)
	for(ptrBuf = buf; len > 0; len-=4, ptrBuf+=3)
	{
		TCPGetArray(sktHTTP, ptrBuf, 4);
		Base64Decode(ptrBuf, 4, ptrBuf, 3);
	}

	// Null terminate both, and make sure there's at least two terminators
	*ptrBuf = '\0';
	for(len = 0, ptrBuf = buf; len < sizeof(buf); len++, ptrBuf++)
		if(*ptrBuf == ':')
			break;
	*(ptrBuf++) = '\0';
	
	// Verify credentials
	curHTTP.isAuthorized = HTTPAuthenticate(buf, ptrBuf, NULL);

	return;
}
#endif

/*********************************************************************
 * Function:        static void HTTPHeaderParseCookie(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          myConn:	data[] is updated
 * 
 * Side Effects:    None
 *
 * Overview:        Parses the "Cookie:" header.  For example, reads: 
 *					Cookie: name=Wile+E.+Coyote; order=ROCKET_LAUNCHER
 *					into the data buffer as:
 *					name=Wile+E.+Coyote&order=ROCKET_LAUNCHER&
 *
 * Note:            Tries to read up to and including the end of the 
 *					current line.  If not successful, a return of 
 *					FALSE indicates that this function should be 
 *					called again when more data is available.
 ********************************************************************/
#if defined(HTTP_USE_COOKIES)
static void HTTPHeaderParseCookie(void)
{
	WORD lenA, lenB;

	// Verify there's enough space
	lenB = TCPFindROMArray(sktHTTP, HTTP_CRLF, HTTP_CRLF_LEN, 0, FALSE);
	if(lenB >= curHTTP.data + HTTP_MAX_DATA_LEN - curHTTP.ptrData - 2)
	{// If not, overflow
		curHTTP.httpStatus = HTTP_OVERFLOW;
		smHTTP = SM_HTTP_SERVE_HEADERS;
		return;
	}

	// While a CRLF is not immediate, grab a cookie value
	while(lenB != 0)
	{
		// Look for a ';' and use the shorter of that or a CRLF
		lenA = TCPFind(sktHTTP, ';', 0, FALSE);
		
		// Read to the terminator
		curHTTP.ptrData += TCPGetArray(sktHTTP, curHTTP.ptrData, mMIN(lenA, lenB));
		
		// Insert an & to anticipate another cookie
		*(curHTTP.ptrData++) = '&';
		
		// If semicolon, trash it and whitespace
		if(lenA < lenB)
		{
			TCPGet(sktHTTP, NULL);
			while(TCPFind(sktHTTP, ' ', 0, FALSE) == 0)
				TCPGet(sktHTTP, NULL);
		}
		
		// Find the new distance to the CRLF
		lenB = TCPFindROMArray(sktHTTP, HTTP_CRLF, HTTP_CRLF_LEN, 0, FALSE);
	}

	return;

}
#endif

/*********************************************************************
 * Function:        static void HTTPHeaderParseContentLength(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          curHTTP.byteCount is updated
 * 
 * Side Effects:    None
 *
 * Overview:        Parses the "Content-Length" header to determine 
 *					how many bytes of POSTED data to expect.
 *
 * Note:            Tries to read up to and including the end of the 
 *					current line.  If not successful, a return of 
 *					FALSE indicates that this function should be 
 *					called again when more data is available.
 ********************************************************************/
#if defined(HTTP_USE_POST)
static void HTTPHeaderParseContentLength(void)
{
	WORD len;
	BYTE buf[10];

	// Read up to the CRLF (max 9 bytes or ~1GB)
	len = TCPFindROMArray(sktHTTP, HTTP_CRLF, HTTP_CRLF_LEN, 0, FALSE);
	len = TCPGetArray(sktHTTP, buf, len);
	buf[len] = '\0';
	
	curHTTP.byteCount = atol((char*)buf);
	
	return;

}
#endif

/*********************************************************************
 * Function:        BYTE* HTTPURLDecode(BYTE *data)
 *
 * PreCondition:    data has at least one extra byte free
 *
 * Input:           *data the string to decode
 *
 * Output:          A pointer to the end of the data
 *
 * Side Effects:    *data is URL decoded
 *
 * Overview:        Parses a string, converting:
 *					'=' to '\0'
 *					'&' to '\0'
 *					'+' to ' '
 *					"%xx" to a single byte, as defined by xx (hex)
 *					After completion, the data is de-escaped, and 
 *					each null terminator signifies the end of a name
 *					or value.
 *
 * Note:            This function is called by the stack to parse
 *					GET arguments and modified cookie data.  User
 *					applications can use this function to decode POST
 *					data, but need to verify that the string is
 *					null terminated before passing.
 ********************************************************************/
BYTE* HTTPURLDecode(BYTE *data)
{
	BYTE *read, *write;
	WORD len;
	BYTE c;
	WORD_VAL d;
	 
	// Determine length of input
	len = strlen((char*)data);
	 
	// Read all characters in the string
	for(read = write = data; len != 0; len--, read++, write++)
	{
		c = *read;
		if(c == '=' || c == '&')
			*write = '\0';
		else if(c == '+')
			*write = ' ';
		else if(c == '%')
		{
			if(len < 2)
				len = 0;
			else
			{
				read++;
				d.v[1] = *read;
				read++;
				d.v[0] = *read;
				*write = hexatob(d);
			}
		}
		else
			*write = c;
	}
	
	// Double null terminate the last value
	*write++ = '\0';
	*write = '\0';
	
	return write;
}

/*********************************************************************
 * Function:        BYTE* HTTPGetArg(BYTE *data, BYTE* arg)
 *
 * PreCondition:    None
 *
 * Input:           *data: the buffer to search
 *					*arg: the name of the argument to find
 *
 * Output:          Pointer to the argument value, NULL if not found
 *
 * Side Effects:    None
 *
 * Overview:        Searches a BYTE array for a named argument and
 *					returns its value.
 *
 * Note:            This function is provided as a utility to access
 *					data passed via an HTTP GET or POST request.
 ********************************************************************/
BYTE* HTTPGetArg(BYTE *data, BYTE* arg)
{
	// Search through the array while bytes remain
	while(*data != '\0')
	{ 
		// Look for arg at current position
		if(!strcmp((char*)arg, (char*)data))
		{// Found it, so return parameter
			return data + strlen((char*)arg) + 1;
		}
		
		// Skip past two strings (NUL bytes)
		data += strlen((char*)data) + 1;
		data += strlen((char*)data) + 1;
	}
	 	
	// Return NULL if not found
	return NULL;
}

#if defined(__18CXX)
BYTE* HTTPGetROMArg(BYTE *data, ROM BYTE* arg)
{
	// Search through the array while bytes remain
	while(*data != '\0')
	{
		// Look for arg at current position
		if(!memcmppgm2ram(data, (ROM void*)arg, strlenpgm((ROM char*)arg) + 1))
		{// Found it, so skip to next string
			return data + strlenpgm((ROM char*)arg) + 1;
		}
		
		// Skip past two strings (NUL bytes)
		data += strlen((char*)data) + 1;
		data += strlen((char*)data) + 1;
	}
	 	
	// Return NULL if not found
	return NULL;
}
#endif

/*********************************************************************
 * Function:        HTTP_IO_RESULT HTTPMPFSUpload(void)
 *
 * PreCondition:    MPFSFormat() has been called
 *
 * Input:           None
 *
 * Output:          HTTP_IO_DONE on success
 *					HTTP_IO_NEED_DATA if more data is requested
 *
 * Side Effects:    None
 *
 * Overview:        Writes all POSTed data to the MPFS EEPROM
 *
 * Note:            After the headers, the first line from the form
 *					will be the MIME separator.  Following that is 
 *					more headers about the file, which we discard. 
 *					After another CRLFCRLF, the file data begins, and 
 *					we read it 16 bytes at a time and write it to 
 *					the EEPROM.
 ********************************************************************/
#if defined(HTTP_MPFS_UPLOAD)
static HTTP_IO_RESULT HTTPMPFSUpload(void)
{
	BYTE c[16];
	WORD lenA, lenB;
	
	switch(curHTTP.httpStatus)
	{
		// New upload, so look for the CRLFCRLF
		case HTTP_MPFS_UP:
		
			lenA = TCPFindROMArray(sktHTTP, (ROM BYTE*)"\r\n\r\n", 4, 0, FALSE);
		
			if(lenA != 0xffff)
			{// Found it, so remove all data up to and including
				lenA = TCPGetArray(sktHTTP, NULL, lenA);
				curHTTP.byteCount -= lenA;
				
				// Make sure first 6 bytes are also in
				if(TCPIsGetReady(sktHTTP) < (4 + 6) )
				{
					lenA++;
					return HTTP_IO_NEED_DATA;
				}
				
				// Make sure it's an MPFS of the correct version
				lenA = TCPGetArray(sktHTTP, c, 10);
				curHTTP.byteCount -= lenA;
				if(memcmppgm2ram(c, (ROM void*)"\r\n\r\nMPFS\x02\x00", 10) == 0)
				{// Read as Ver 2.0
					curHTTP.httpStatus = HTTP_MPFS_OK;
					
					// Format MPFS storage and put 6 byte tag
					curHTTP.file = MPFSFormat();
					MPFSPutArray(curHTTP.file, &c[4], 6);
				}
				else
				{// Version is wrong
					curHTTP.httpStatus = HTTP_MPFS_ERROR;
				}
			}
			else
			{// Otherwise, remove as much as possible
				lenA = TCPGetArray(sktHTTP, NULL, TCPIsGetReady(sktHTTP) - 4);
				curHTTP.byteCount -= lenA;
			}
			
			break;
		
		// Received file is invalid
		case HTTP_MPFS_ERROR:
			curHTTP.byteCount -= TCPIsGetReady(sktHTTP);
			TCPDiscard(sktHTTP);
			if(curHTTP.byteCount < 100 || curHTTP.byteCount > 0x80000000)
			{// If almost all data was read, or if we overflowed, then return
				smHTTP = SM_HTTP_SERVE_HEADERS;
				return HTTP_IO_DONE;
			}
			break;
		
		// File is verified, so write the data
		case HTTP_MPFS_OK:
			// Determine how much to read
			lenA = TCPIsGetReady(sktHTTP);
			if(lenA > curHTTP.byteCount)
				lenA = curHTTP.byteCount;
				
			while(lenA > 0)
			{
				lenB = TCPGetArray(sktHTTP, c, mMIN(lenA,16));
				curHTTP.byteCount -= lenB;
				lenA -= lenB;
				MPFSPutArray(curHTTP.file, c, lenB);
			}
				
			// If we've read all the data
			if(curHTTP.byteCount == 0)
			{
				MPFSPutEnd();
				smHTTP = SM_HTTP_SERVE_HEADERS;
				return HTTP_IO_DONE;
			}
	}
		
	// Ask for more data
	return HTTP_IO_NEED_DATA;
	
}
#endif

/*********************************************************************
 * Function:        DWORD HTTPIncFile(TCP_SOCKET skt, 
 *						DWORD callbackPos, ROM BYTE* file)
 *
 * PreCondition:    curHTTP is loaded
 *
 * Input:           None
 *
 * Output:          Updates curHTTP.callbackPos
 *
 * Side Effects:    None
 *
 * Overview:        Writes an MPFS file to the socket and returns
 *
 * Note:            Provides rudimentary include support for dynamic
 *					files which allows them to use header, footer, 
 *					and/or menu inclusion files rather than
 *					duplicating code across all files.
 ********************************************************************/
void HTTPIncFile(ROM BYTE* file)
{
	WORD count, len;
	BYTE data[64];
	MPFS_HANDLE fp;
	
	// Check if this is a first round call
	if(curHTTP.callbackPos == 0x00)
	{// On initial call, open the file and save its ID
		fp = MPFSOpenROM(file);
		if(fp == MPFS_INVALID_HANDLE)
		{// File not found, so abort
			return;
		}
		((DWORD_VAL*)&curHTTP.callbackPos)->w[0] = MPFSGetID(fp);
	}
	else
	{// The file was already opened, so load up it's ID and seek
		fp = MPFSOpenID(((DWORD_VAL*)&curHTTP.callbackPos)->w[0]);
		if(fp == MPFS_INVALID_HANDLE)
		{// File not found, so abort
			curHTTP.callbackPos = 0x00;
			return;
		}
		MPFSSeek(fp, ((DWORD_VAL*)&curHTTP.callbackPos)->w[1], MPFS_SEEK_FORWARD);
	}
	
	// Get/put as many bytes as possible
	count = TCPIsPutReady(sktHTTP);
	while(count > 0)
	{
		len = MPFSGetArray(fp, data, mMIN(count, 64));
		if(len == 0)
		{// If no bytes were read, an EOF was reached
			MPFSClose(fp);
			curHTTP.callbackPos = 0x00;
			return;
		}
		else
		{// Write the bytes to the socket
			TCPPutArray(sktHTTP, data, len);
			count -= len;
		}
	}
	
	// Save the new address and close the file
	((DWORD_VAL*)&curHTTP.callbackPos)->w[1] = MPFSTell(fp);
	MPFSClose(fp);
	
	return;
}

#endif
