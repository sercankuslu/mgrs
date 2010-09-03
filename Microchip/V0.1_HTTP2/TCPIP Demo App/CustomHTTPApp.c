/*********************************************************************
 *
 *  Application to Demo HTTP2 Server
 *  Support for HTTP2 module in Microchip TCP/IP Stack
 *	 -Implements the application 
 *	 -Reference: RFC 1002
 *
 *********************************************************************
 * FileName:        CustomHTTPApp.c
 * Dependencies:    TCP/IP stack
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
 * Elliott Wood     	6/18/07	Original
 ********************************************************************/
#define __CUSTOMHTTPAPP_C

#include "TCPIP Stack/TCPIP.h"

#if defined(STACK_USE_HTTP2_SERVER)

#if defined(HTTP_USE_POST)
	#if defined(USE_LCD)
	static HTTP_IO_RESULT HTTPPostLCD(void);
	#endif
	#if defined(STACK_USE_MD5)
	static HTTP_IO_RESULT HTTPPostMD5(void);
	#endif
	#if defined(STACK_USE_APP_RECONFIG)
	static HTTP_IO_RESULT HTTPPostConfig(void);
	#endif
	#if defined(STACK_USE_SMTP_CLIENT)
	static HTTP_IO_RESULT HTTPPostEmail(void);
	#endif
#endif

extern HTTP_CONN curHTTP;
extern HTTP_STUB httpStubs[MAX_HTTP_CONNECTIONS];
extern BYTE curHTTPID;

/*********************************************************************
 * Function:        BYTE HTTPAuthenticate(BYTE *user, BYTE *pass, BYTE *filename)
 *
 * PreCondition:    None
 *
 * Input:           *user: a pointer to the username string
 *					*pass: a pointer to the password string
 *					*filename: a pointer to the file name being requested
 *
 * Output:          0x00-0x79 if authentication fails
 *					0x80-0xff if authentication passes
 *
 * Side Effects:    None
 *
 * Overview:        The function is called at least once per 
 *					connection.  The first call has user and pass set
 *					to NULL, but the filename is set.  This call 
 *					determines if a password is later required, based
 *					only on filename.  Subsequent calls will have a 
 *					user and pass pointer, but the filename will be 
 *					NULL.  These calls occur every time that an
 *					Authorization: header is read and a user/pass
 *					is extracted.  The MSb of the return value 
 *					indicates whether or not authentication passes. 
 *					The remaining bits are application-defined, and 
 *					can be used by your application to manage groups, 
 *					multiple users with different permissions, etc.
 *
 * Note:            Return value is stored in curHTTP.isAuthorized
 ********************************************************************/
#if defined(HTTP_USE_AUTHENTICATION)
BYTE HTTPAuthenticate(BYTE *user, BYTE *pass, BYTE *filename)
{
	if(filename)
	{// This is the first round...just determine if auth is needed
		
		// If the filename begins with the folder "protect", then require auth
		if(memcmppgm2ram(filename, (ROM void*)"protect", 7) == 0)
			return 0x00;		// Authentication will be needed later

		#if defined(HTTP_MPFS_UPLOAD_REQUIRES_AUTH)
		if(memcmppgm2ram(filename, (ROM void*)"mpfsupload", 10) == 0)
			return 0x00;
		#endif

		// You can match additional strings here to password protect other files.
		// You could switch this and exclude files from authentication.
		// You could also always return 0x00 to require auth for all files.
		// You can return different values (0x00 to 0x79) to track "realms" for below.

		return 0x80;			// No authentication required
	}
	
	else
	{// This is a user/pass combination
		if(strcmppgm2ram((char *)user,(ROM char *)"admin") == 0
			&& strcmppgm2ram((char *)pass, (ROM char *)"microchip") == 0)
			return 0x80;		// We accept this combination
		
		// You can add additional user/pass combos here.
		// If you return specific "realm" values above, you can base this 
		//   decision on what specific file or folder is being accessed.
		// You could return different values (0x80 to 0xff) to indicate 
		//   various users or groups, and base future processing decisions
		//   in HTTPExecuteGet/Post or HTTPPrint callbacks on this value.
		
		return 0x00;			// Provided user/pass is invalid
	}
}
#endif

/*********************************************************************
 * Function:        HTTP_IO_RESULT HTTPExecuteGet(void)
 *
 * PreCondition:    curHTTP is loaded
 *
 * Input:           None
 *
 * Output:          HTTP_IO_DONE on success
 *					HTTP_IO_WAITING if waiting for asynchronous process
 *
 * Side Effects:    None
 *
 * Overview:        This function is called if data was read from the
 *					HTTP request from either the GET arguments, or
 *					any cookies sent.  curHTTP.data contains
 *					sequential pairs of strings representing the
 *					data received.  Any required authentication has 
 *					already been validated.
 *
 * Note:            In this simple example, HTTPGetROMArg is used to 
 *					search for values associated with argument names.
 *					At this point, the application may overwrite/modify
 *					curHTTP.data if additional storage associated with
 *					a connection is needed.  Cookies may be set; see
 *					HTTPExecutePostCookies for an example.  For 
 *					redirect functionality, set curHTTP.data to the 
 *					destination and change curHTTP.httpStatus to
 *					HTTP_REDIRECT.
 ********************************************************************/
HTTP_IO_RESULT HTTPExecuteGet(void)
{
	BYTE *ptr;
	BYTE filename[20];
	
	// Load the file name
	// Make sure BYTE filename[] above is large enough for your longest name
	MPFSGetFilename(curHTTP.file, filename, 20);
	
	// If its the forms.htm page
	if(!memcmppgm2ram(filename, "forms.htm", 9))
	{
		// Seek out each of the four LED strings, and if it exists set the LED states
		ptr = HTTPGetROMArg(curHTTP.data, (ROM BYTE *)"led4");
		//if(ptr)
			//2EI - comentei LED4_IO = (*ptr == '1');

		ptr = HTTPGetROMArg(curHTTP.data, (ROM BYTE *)"led3");
		//if(ptr)
			//2EI - comentei LED3_IO = (*ptr == '1');

		ptr = HTTPGetROMArg(curHTTP.data, (ROM BYTE *)"led2");
		//if(ptr)
			//2EI - comentei LED2_IO = (*ptr == '1');

		ptr = HTTPGetROMArg(curHTTP.data, (ROM BYTE *)"led1");
		//if(ptr)
			// 2EI - comentei LED1_IO = (*ptr == '1');
	}
	
	// If it's the LED updater file
	else if(!memcmppgm2ram(filename, "cookies.htm", 11))
	{
		// This is very simple.  The names and values we want are already in
		// the data array.  We just set the hasArgs value to indicate how many
		// name/value pairs we want stored as cookies.
		// To add the second cookie, just increment this value.
		// remember to also add a dynamic variable callback to control the printout.
		curHTTP.hasArgs = 0x01;
	}
		
	
	// If it's the LED updater file
	else if(!memcmppgm2ram(filename, "leds.cgi", 8))
	{
		// Determine which LED to toggle
		ptr = HTTPGetROMArg(curHTTP.data, (ROM BYTE *)"led");
		
		// Toggle the specified LED
		switch(*ptr) {
		case '1':
			//LED1_IO ^= 1;
			break;
		case '2':
			//LED2_IO ^= 1;
			break;
		case '3':
			//LED3_IO ^= 1;
			break;
		case '4':
			//LED4_IO ^= 1;
			break;
		case '5':
			//LED5_IO ^= 1;
			break;
		case '6':
			//LED6_IO ^= 1;
			break;
		case '7':
			//LED7_IO ^= 1;
			break;
		}
		
	}
	
	return HTTP_IO_DONE;
}

#if defined(HTTP_USE_POST)

/*********************************************************************
 * Function:        HTTP_IO_RESULT HTTPExecutePost(void)
 *
 * PreCondition:    curHTTP is loaded
 *
 * Input:           None
 *
 * Output:          HTTP_IO_DONE on success
 *					HTTP_IO_NEED_DATA if more data is requested
 *					HTTP_IO_WAITING if waiting for asynchronous process
 *
 * Side Effects:    None
 *
 * Overview:        This function is called if the request method was
 *					POST.  It is called after HTTPExecuteGet and 
 *					after any required authentication has been validated.
 *
 * Note:            In this example, this function calls additional
 *					helpers depending on which file was requested.
 ********************************************************************/
HTTP_IO_RESULT HTTPExecutePost(void)
{
	// Resolve which function to use and pass along
	BYTE filename[20];
	
	// Load the file name
	// Make sure BYTE filename[] above is large enough for your longest name
	MPFSGetFilename(curHTTP.file, filename, 20);
	
#if defined(USE_LCD)
	if(!memcmppgm2ram(filename, "forms.htm", 9))
		return HTTPPostLCD();
#endif

#if defined(STACK_USE_MD5)
	if(!memcmppgm2ram(filename, "upload.htm", 10))
		return HTTPPostMD5();
#endif

#if defined(STACK_USE_APP_RECONFIG)
	if(!memcmppgm2ram(filename, "protect/config.htm", 18))
		return HTTPPostConfig();
#endif

#if defined(STACK_USE_SMTP_CLIENT)
	if(!strcmppgm2ram((char*)filename, "email/index.htm"))
		return HTTPPostEmail();
#endif

	return HTTP_IO_DONE;
}

/*********************************************************************
 * Function:        HTTP_IO_RESULT HTTPPostLCD(void)
 *
 * PreCondition:    curHTTP is loaded
 *
 * Input:           None
 *
 * Output:          HTTP_IO_DONE on success
 *					HTTP_IO_NEED_DATA if more data is requested
 *					HTTP_IO_WAITING if waiting for asynchronous process
 *
 * Side Effects:    None
 *
 * Overview:        This function reads an input parameter "lcd" from
 *					the POSTed data, and writes that string to the
 *					board's LCD display.
 *
 * Note:            None
 ********************************************************************/
#if defined(USE_LCD)
static HTTP_IO_RESULT HTTPPostLCD(void)
{
	BYTE *ptr;
	WORD len;

	// Look for the lcd string
	len = TCPFindROMArray(sktHTTP, (ROM BYTE *)"lcd=", 4, 0, FALSE);
	
	// If not found, then throw away almost all the data we have and ask for more
	if(len == 0xffff)
	{
		curHTTP.byteCount -= TCPGetArray(sktHTTP, NULL, TCPIsGetReady(sktHTTP) - 4);
		return HTTP_IO_NEED_DATA;
	}
	
	// Throw away all data preceeding the lcd string
	curHTTP.byteCount -= TCPGetArray(sktHTTP, NULL, len);
	
	// Look for end of LCD string
	len = TCPFind(sktHTTP, '&', 0, FALSE);
	if(len == 0xffff)
		len = curHTTP.byteCount;
	
	// If not found, ask for more data
	if(curHTTP.byteCount > TCPIsGetReady(sktHTTP))
		return HTTP_IO_NEED_DATA;
		
	// Prevent buffer overflows
	if(len > HTTP_MAX_DATA_LEN - 2)
		len = HTTP_MAX_DATA_LEN - 2;
		
	// Read entire LCD update string into buffer and parse it
	len = TCPGetArray(sktHTTP, curHTTP.data, len);
	curHTTP.byteCount -= len;
	curHTTP.data[len] = '\0';
	ptr = HTTPURLDecode(curHTTP.data);
	ptr = HTTPGetROMArg(curHTTP.data, (ROM BYTE *)"lcd");
	
	// Copy up to 32 characters to the LCD
	if(strlen((char*)curHTTP.data) < 32u)
	{
		memset(LCDText, ' ', 32);
		strcpy((char*)LCDText, (char*)ptr);
	}
	else
	{
		memcpy(LCDText, (void *)ptr, 32);
	}
	
	LCDUpdate();
	
	strcpypgm2ram((char*)curHTTP.data, (ROM void*)"forms.htm");
	curHTTP.httpStatus = HTTP_REDIRECT;
	
	return HTTP_IO_DONE;
}
#endif

/*********************************************************************
 * Function:        HTTP_IO_RESULT HTTPPostConfig(void)
 *
 * PreCondition:    curHTTP is loaded
 *
 * Input:           None
 *
 * Output:          HTTP_IO_DONE on success
 *					HTTP_IO_NEED_DATA if more data is requested
 *					HTTP_IO_WAITING if waiting for asynchronous process
 *
 * Side Effects:    None
 *
 * Overview:        This function reads an input parameter "lcd" from
 *					the POSTed data, and writes that string to the
 *					board's LCD display.
 *
 * Note:            None
 ********************************************************************/
#if defined(STACK_USE_APP_RECONFIG)
extern APP_CONFIG AppConfig;
#define HTTP_POST_CONFIG_MAX_LEN	(HTTP_MAX_DATA_LEN - sizeof(AppConfig) - 3)
static HTTP_IO_RESULT HTTPPostConfig(void)
{
	APP_CONFIG *app;
	BYTE *ptr;
	WORD len;

	// Set app config pointer to use data array
	app = (APP_CONFIG*)&curHTTP.data[HTTP_POST_CONFIG_MAX_LEN];
	
	// Use data[0] as a state machine.  0x01 is initialized, 0x02 is error, else uninit
	if(curHTTP.data[0] != 0x01 && curHTTP.data[0] != 0x02) 
	{
		// First run, so use current config as defaults
		memcpy((void*)app, (void*)&AppConfig, sizeof(AppConfig));
		app->Flags.bIsDHCPEnabled = 0;
		curHTTP.data[0] = 0x01;
	}

	// Loop over all parameters
	while(curHTTP.byteCount) 
	{	
		// Find end of next parameter string
		len = TCPFind(sktHTTP, '&', 0, FALSE);
		if(len == 0xffff && TCPIsGetReady(sktHTTP) == curHTTP.byteCount)
			len = TCPIsGetReady(sktHTTP);
		
		// If there's no end in sight, then ask for more data
		if(len == 0xffff)
			return HTTP_IO_NEED_DATA;
			
		// Read in as much data as we can
		if(len > HTTP_MAX_DATA_LEN-sizeof(AppConfig))
		{// If there's too much, read as much as possible
			curHTTP.byteCount -= TCPGetArray(sktHTTP, curHTTP.data+1, HTTP_POST_CONFIG_MAX_LEN);
			curHTTP.byteCount -= TCPGetArray(sktHTTP, NULL, len - HTTP_POST_CONFIG_MAX_LEN);
			curHTTP.data[HTTP_POST_CONFIG_MAX_LEN-1] = '\0';
		}
		else
		{// Otherwise, read as much as we wanted to
			curHTTP.byteCount -= TCPGetArray(sktHTTP, curHTTP.data+1, len);
			curHTTP.data[len+1] = '\0';
		}
	
		// Decode the string
		HTTPURLDecode(curHTTP.data+1);
	
		// Compare the string to those we're looking for
		if(!memcmppgm2ram(curHTTP.data+1, "ip\0", 3))
		{
			if(StringToIPAddress(&curHTTP.data[3+1], &(app->MyIPAddr)))
				memcpy((void*)&(app->DefaultIPAddr), (void*)&(app->MyIPAddr), sizeof(IP_ADDR));
			else
				curHTTP.data[0] = 0x02;
		}
		else if(!memcmppgm2ram(curHTTP.data+1, "gw\0", 3))
		{
			if(!StringToIPAddress(&curHTTP.data[3+1], &(app->MyGateway)))
				curHTTP.data[0] = 0x02;	
		}
		else if(!memcmppgm2ram(curHTTP.data+1, "subnet\0", 7))
		{
			if(StringToIPAddress(&curHTTP.data[7+1], &(app->MyMask)))
				memcpy((void*)&(app->DefaultMask), (void*)&(app->MyMask), sizeof(IP_ADDR));
			else
				curHTTP.data[0] = 0x02;
		}
		else if(!memcmppgm2ram(curHTTP.data+1, "dns1\0", 5))
		{
			if(!StringToIPAddress(&curHTTP.data[5+1], &(app->PrimaryDNSServer)))
				curHTTP.data[0] = 0x02;	
		}
		else if(!memcmppgm2ram(curHTTP.data+1, "dns2\0", 5))
		{
			if(!StringToIPAddress(&curHTTP.data[5+1], &(app->SecondaryDNSServer)))
				curHTTP.data[0] = 0x02;
		}
		else if(!memcmppgm2ram(curHTTP.data+1, "mac\0", 4))
		{
			WORD_VAL w;
			BYTE i, mac[12];

			ptr = &curHTTP.data[4+1];

			for(i = 0; i < 12; i++)
			{// Read the MAC address
				
				// Skip non-hex bytes
				while( *ptr != 0x00 && !(*ptr >= '0' && *ptr < '9') && !(*ptr >= 'A' && *ptr <= 'F') && !(*ptr >= 'a' && *ptr <= 'f') )
					ptr++;

				// MAC string is over, so zeroize the rest
				if(*ptr == 0x00)
				{
					for(; i < 12; i++)
						mac[i] = '0';
					break;
				}
				
				// Save the MAC byte
				mac[i] = *ptr++;
			}
			
			// Read MAC Address, one byte at a time
			for(i = 0; i < 6; i++)
			{				
				w.v[1] = mac[i*2];
				w.v[0] = mac[i*2+1];
				app->MyMACAddr.v[i] = hexatob(w);
			}
		}
		else if(!memcmppgm2ram(curHTTP.data+1, "host\0", 5))
		{
			memset(app->NetBIOSName, ' ', 15);
			app->NetBIOSName[15] = 0x00;
			memcpy((void*)app->NetBIOSName, (void*)&curHTTP.data[5+1], strlen((char*)&curHTTP.data[5+1]));
			strupr((char*)app->NetBIOSName);
		}
		else if(!memcmppgm2ram(curHTTP.data+1, "dhcpenabled\0", 12))
		{
			if(curHTTP.data[12+1] == '1')
				app->Flags.bIsDHCPEnabled = 1;
			else
				app->Flags.bIsDHCPEnabled = 0;
		}
		
		// Trash the separator character
		while( TCPFind(sktHTTP, '&', 0, FALSE)  == 0 || 
			   TCPFind(sktHTTP, '\r', 0, FALSE) == 0 || 
			   TCPFind(sktHTTP, '\n', 0, FALSE) == 0	 )
		{
			curHTTP.byteCount -= TCPGet(sktHTTP, NULL);
		}

	}
	
	// Check if all settings were successful
	if(curHTTP.data[0] == 0x01)
	{// Save the new AppConfig
		// If DCHP, then disallow editing of DefaultIP and DefaultMask
		if(app->Flags.bIsDHCPEnabled)
		{
			// If DHCP is enabled, then reset the default IP and mask
			app->DefaultIPAddr.v[0] = MY_DEFAULT_IP_ADDR_BYTE1;
			app->DefaultIPAddr.v[1] = MY_DEFAULT_IP_ADDR_BYTE2;
			app->DefaultIPAddr.v[2] = MY_DEFAULT_IP_ADDR_BYTE3;
			app->DefaultIPAddr.v[3] = MY_DEFAULT_IP_ADDR_BYTE4;
			app->DefaultMask.v[0] = MY_DEFAULT_MASK_BYTE1;
			app->DefaultMask.v[1] = MY_DEFAULT_MASK_BYTE2;
			app->DefaultMask.v[2] = MY_DEFAULT_MASK_BYTE3;
			app->DefaultMask.v[3] = MY_DEFAULT_MASK_BYTE4;
		}
		ptr = (BYTE*)app;
		#if defined(MPFS_USE_EEPROM)
	    XEEBeginWrite(0x0000);
	    XEEWrite(0x60);
	    for (len = 0; len < sizeof(AppConfig); len++ )
	        XEEWrite(*ptr++);
	    XEEEndWrite();
        while(XEEIsBusy());
        #endif
		
		// Set the board to reboot to the new address
		strcpypgm2ram((char*)curHTTP.data, (ROM void*)"/protect/reboot.htm?");
		memcpy((void*)(curHTTP.data+20), (void*)app->NetBIOSName, 16);
		ptr = curHTTP.data;
		while(*ptr != ' ' && *ptr != '\0')
			ptr++;
		*ptr = '\0';
	}
	else
	{// Error parsing IP, so don't save to avoid errors
		strcpypgm2ram((char*)curHTTP.data, (ROM void*)"/protect/config_error.htm");
	}
			
	curHTTP.httpStatus = HTTP_REDIRECT;
	
	return HTTP_IO_DONE;
}
#endif	// #if defined(STACK_USE_APP_RECONFIG)

/*********************************************************************
 * Function:        HTTP_IO_RESULT HTTPPostMD5(void)
 *
 * PreCondition:    curHTTP is loaded
 *
 * Input:           None
 *
 * Output:          HTTP_IO_DONE on success
 *					HTTP_IO_NEED_DATA if more data is requested
 *					HTTP_IO_WAITING if waiting for asynchronous process
 *
 * Side Effects:    None
 *
 * Overview:        This function accepts a POSTed file from the client
 *					and calculates its MD5 sum to be returned later.
 *
 * Note:            After the headers, the first line from the form
 *					will be the MIME separator.  Following that is 
 *					more headers about the file, which we discard. 
 *					After another CRLFCRLF, the file data begins, and 
 *					we read it 16 bytes at a time and add that to the 
 *					MD5 calculation.  The reading terminates when the
 *					separator string is encountered again on its own
 *					line.  Notice that the actual file data is trashed
 *					in this process, allowing us to accept files of 
 *					arbitrary size, not limited by RAM.  Also notice
 *					that the data buffer is used as an arbitrary 
 *					storage array for the result.  The %uploadedmd5%
 *					callback reads this data later to send back to 
 *					the client.
 ********************************************************************/
#if defined(STACK_USE_MD5)
static HTTP_IO_RESULT HTTPPostMD5(void)
{
	WORD lenA, lenB;
	static HASH_SUM md5;		// Assume only one simultaneous MD5
	
	#define SM_MD5_READ_SEPARATOR	(0u) // Processed as the "default" state
	#define SM_MD5_SKIP_TO_DATA		(1u)
	#define SM_MD5_READ_DATA		(2u)
	
	// We don't care about curHTTP.data at this point,
	// so we'll use that for our buffer
	
	// curHTTP.data[0] is always at least overwritten with the leading '/'
	// from the filename, so we'll use that as a state machine variable.  
	// If it's value isn't 0x01 or 0x02, then we haven't passed the separator.
	switch(curHTTP.data[0])
	{
	
	case SM_MD5_SKIP_TO_DATA:
		// Look for the CRLFCRLF
		lenA = TCPFindROMArray(sktHTTP, (ROM BYTE*)"\r\n\r\n", 4, 0, FALSE);

		if(lenA != 0xffff)
		{// Found it, so remove all data up to and including
			lenA = TCPGetArray(sktHTTP, NULL, lenA+4);
			curHTTP.byteCount -= lenA;
			curHTTP.data[0] = SM_MD5_READ_DATA;
		}
		else
		{// Otherwise, remove as much as possible
			lenA = TCPGetArray(sktHTTP, NULL, TCPIsGetReady(sktHTTP) - 4);
			curHTTP.byteCount -= lenA;
		
			// Return the need more data flag
			return HTTP_IO_NEED_DATA;
		}
		
		// No break if we found the header terminator
		
	case SM_MD5_READ_DATA:

		// Find out how many bytes are available to be read
		lenA = TCPIsGetReady(sktHTTP);
		if(lenA > curHTTP.byteCount)
			lenA = curHTTP.byteCount;

		while(lenA > 0)
		{// Add up to 64 bytes at a time to the sum
			lenB = TCPGetArray(sktHTTP, &(curHTTP.data[1]), (lenA < 64)?lenA:64);			
			curHTTP.byteCount -= lenB;
			lenA -= lenB;
			MD5AddData(&md5, &curHTTP.data[1], lenB);
		}
				
		// If we've read all the data
		if(curHTTP.byteCount == 0)
		{// Calculate and copy result to curHTTP.data for printout
			curHTTP.data[0] = 0x05;
			MD5Calculate(&md5, &(curHTTP.data[1]));
			return HTTP_IO_DONE;
		}
			
		// Ask for more data
		return HTTP_IO_NEED_DATA;
	
	// Just started, so try to find the separator string
	case SM_MD5_READ_SEPARATOR:
	default:
		// Reset the MD5 calculation
		MD5Initialize(&md5);
		
		// See if a CRLF is in the buffer
		lenA = TCPFindROMArray(sktHTTP, (ROM BYTE*)"\r\n", 2, 0, FALSE);
		if(lenA == 0xffff)
		{//if not, ask for more data
			return HTTP_IO_NEED_DATA;
		}
	
		// If so, figure out where the last byte of data is
		// Data ends if CRLFseparator--CRLF, so 6+len bytes
		curHTTP.byteCount -= lenA + 6;
		
		// Read past the CRLF
		curHTTP.byteCount -= TCPGetArray(sktHTTP, NULL, lenA+2);
		
		// Save the next state (skip to CRLFCRLF)
		curHTTP.data[0] = SM_MD5_SKIP_TO_DATA;
		
		// Ask for more data
		return HTTP_IO_NEED_DATA;
	}
	
}
#endif // #if defined(STACK_USE_MD5)

/*********************************************************************
 * Function:        HTTP_IO_RESULT HTTPPostEmail(void)
 *
 * PreCondition:    curHTTP is loaded
 *
 * Input:           None
 *
 * Output:          HTTP_IO_DONE on success
 *					HTTP_IO_NEED_DATA if more data is requested
 *					HTTP_IO_WAITING if waiting for asynchronous process
 *
 * Side Effects:    None
 *
 * Overview:        This function attempts to send an e-mail using
 *					input supplied as POST data
 *
 * Note:            None
 ********************************************************************/
#if defined(STACK_USE_SMTP_CLIENT)
static HTTP_IO_RESULT HTTPPostEmail(void)
{
	static BYTE smtpData[128];
	static BYTE *ptrData;
	static BYTE *szPort;
	WORD len, rem;

	#define SM_EMAIL_CLAIM_MODULE				(0u) // Processed as the "default" state
	#define SM_EMAIL_READ_PARAM_NAME			(1u)
	#define SM_EMAIL_READ_PARAM_VALUE			(2u)
	#define SM_EMAIL_PUT_IGNORED				(3u)
	#define SM_EMAIL_PUT_BODY					(4u)
	#define SM_EMAIL_PUT_ATTACHMENT_HEADER		(5u)
	#define SM_EMAIL_PUT_ATTACHMENT_DATA_BTNS	(6u)
	#define SM_EMAIL_PUT_ATTACHMENT_DATA_LEDS	(7u)
	#define SM_EMAIL_PUT_ATTACHMENT_DATA_POT	(8u)
	#define SM_EMAIL_PUT_TERMINATOR				(9u)
	#define SM_EMAIL_FINISHING					(10u)
	
	// Use curHTTP.data[0] as a state machine.  This is always overwritten by
	// the GET/POST request, so this is a safe memory location to rely upon.
	switch(curHTTP.data[0])
	{
		case SM_EMAIL_READ_PARAM_NAME:
			// Search for a parameter name in POST data
			len = TCPFind(sktHTTP, '=', 0, FALSE);
			if(len == 0xffff)
				return HTTP_IO_NEED_DATA;
				
			// Read in variable, assign SMTPClient.??.szRAM
			if(len > HTTP_MAX_DATA_LEN - 10)
			{
				curHTTP.byteCount -= TCPGetArray(sktHTTP, &curHTTP.data[1], HTTP_MAX_DATA_LEN - 10);
				curHTTP.data[HTTP_MAX_DATA_LEN - 10 + 1] = '\0';
				curHTTP.byteCount -= TCPGetArray(sktHTTP, NULL, len - (HTTP_MAX_DATA_LEN - 10));
			}
			else
			{
				curHTTP.byteCount -= TCPGetArray(sktHTTP, &curHTTP.data[1], len);
				curHTTP.data[len + 1] = '\0';
			}
			curHTTP.byteCount -= TCPGet(sktHTTP, NULL); // clear the '='
			
			// Try to match the name value
			if(!strcmppgm2ram((char*)&curHTTP.data[1], "server"))
			{// Read the server name
				SMTPClient.Server.szRAM = ptrData;
				curHTTP.data[0] = SM_EMAIL_READ_PARAM_VALUE;
			}
			else if(!strcmppgm2ram((char*)&curHTTP.data[1], "port"))
			{// Read the server port
				szPort = ptrData;
				curHTTP.data[0] = SM_EMAIL_READ_PARAM_VALUE;
			}
			else if(!strcmppgm2ram((char*)&curHTTP.data[1], "user"))
			{// Read the user name
				SMTPClient.Username.szRAM = ptrData;
				curHTTP.data[0] = SM_EMAIL_READ_PARAM_VALUE;
			}
			else if(!strcmppgm2ram((char*)&curHTTP.data[1], "pass"))
			{// Read the password
				SMTPClient.Password.szRAM = ptrData;
				curHTTP.data[0] = SM_EMAIL_READ_PARAM_VALUE;
			}
			else if(!strcmppgm2ram((char*)&curHTTP.data[1], "to"))
			{// Read the To string
				SMTPClient.To.szRAM = ptrData;
				curHTTP.data[0] = SM_EMAIL_READ_PARAM_VALUE;
			}
			else if(!strcmppgm2ram((char*)&curHTTP.data[1], "msg"))
			{// Done with headers, move on to the message
				// Delete paramters that are just null strings (no data from user) or illegal (ex: password without username)
				if(SMTPClient.Server.szRAM)
					if(*SMTPClient.Server.szRAM == 0x00)
						SMTPClient.Server.szRAM = NULL;
				if(SMTPClient.Username.szRAM)
					if(*SMTPClient.Username.szRAM == 0x00)
						SMTPClient.Username.szRAM = NULL;
				if(SMTPClient.Password.szRAM)
					if((*SMTPClient.Password.szRAM == 0x00) || (SMTPClient.Username.szRAM == NULL))
						SMTPClient.Password.szRAM = NULL;
				
				// Decode server port string if it exists
				if(szPort)
					if(*szPort)
						SMTPClient.ServerPort = (WORD)atol((char*)szPort);
				
				SMTPSendMail();
				curHTTP.data[0] = SM_EMAIL_PUT_IGNORED;
				return HTTP_IO_WAITING;
			}
			else
			{// Don't know what we're receiving
				curHTTP.data[0] = SM_EMAIL_READ_PARAM_VALUE;
			}
			
			// No break...continue to try reading the value
		
		case SM_EMAIL_READ_PARAM_VALUE:
			// Search for a parameter value in POST data
			len = TCPFind(sktHTTP, '&', 0, FALSE);
			if(len == 0xffff)
				return HTTP_IO_NEED_DATA;

			// Don't overflow our buffer, so restrict the length
			rem = sizeof(smtpData) - 2 - (WORD)(ptrData - smtpData);
			if(rem > sizeof(smtpData))
				rem = 0;

			// Read in variable, assign SMTPClient.??.szRAM
			if(len > rem)
			{
				curHTTP.byteCount -= TCPGetArray(sktHTTP, ptrData, rem);
				ptrData[rem] = '\0';
				curHTTP.byteCount -= TCPGetArray(sktHTTP, NULL, len - rem);
				ptrData = HTTPURLDecode(ptrData);
			}
			else
			{
				curHTTP.byteCount -= TCPGetArray(sktHTTP, ptrData, len);
				ptrData[len] = '\0';
				ptrData = HTTPURLDecode(ptrData);
			}
			curHTTP.byteCount -= TCPGet(sktHTTP, NULL); // clear the '&'
			
			// Try reading the next parameter
			curHTTP.data[0] = SM_EMAIL_READ_PARAM_NAME;
			return HTTP_IO_WAITING;
			
		case SM_EMAIL_PUT_IGNORED:
			// This section puts a message that is ignored by compatible clients.
			// This text will not display unless the receiving client is obselete 
			// and does not understand the MIME structure.
			// The "--frontier" indicates the start of a section, then any
			// needed MIME headers follow, then two CRLF pairs, and then
			// the actual content (which will be the body text in the next state).
			
			// Check to see if a failure occured
			if(!SMTPIsBusy())
			{
				curHTTP.data[0] = SM_EMAIL_FINISHING;
				return HTTP_IO_WAITING;
			}
		
			// See if we're ready to write data
			if(SMTPIsPutReady() < 90u)
				return HTTP_IO_WAITING;
				
			// Write the ignored text				
			SMTPPutROMString((ROM BYTE*)"This is a multi-part message in MIME format.\r\n");
			SMTPPutROMString((ROM BYTE*)"--frontier\r\nContent-type: text/plain\r\n\r\n");
			SMTPFlush();
			
			// Move to the next state
			curHTTP.data[0] = SM_EMAIL_PUT_BODY;
			
		case SM_EMAIL_PUT_BODY:
			// Write as much body text as is available from the TCP buffer
			// return HTTP_IO_NEED_DATA or HTTP_IO_WAITING
			// On completion, => PUT_ATTACHMENT_HEADER and continue
			
			// Check to see if a failure occurred
			if(!SMTPIsBusy())
			{
				curHTTP.data[0] = SM_EMAIL_FINISHING;
				return HTTP_IO_WAITING;
			}
			
			// Determine how much to write
			len = SMTPIsPutReady();
			rem = TCPIsGetReady(sktHTTP);
			
			if(rem < len)
				len = rem;
			if(len > HTTP_MAX_DATA_LEN - 3)
				len = HTTP_MAX_DATA_LEN - 3;
			
			// Read the data from HTTP POST buffer and send it to SMTP
			curHTTP.byteCount -= TCPGetArray(sktHTTP, &curHTTP.data[1], len);
			curHTTP.data[len+1] = '\0';
			HTTPURLDecode(&curHTTP.data[1]);
			SMTPPutString(&curHTTP.data[1]);
			SMTPFlush();
			
			// If we're done with the POST data, continue
			if(curHTTP.byteCount == 0)
				curHTTP.data[0] = SM_EMAIL_PUT_ATTACHMENT_HEADER;
			else
				return HTTP_IO_NEED_DATA;
						
		case SM_EMAIL_PUT_ATTACHMENT_HEADER:
			// This section writes the attachment to the message.
			// This portion generally will not display in the reader, but
			// will be downloadable to the local machine.  Use caution
			// when selecting the content-type and file name, as certain
			// types and extensions are blocked by virus filters.

			// The same structure as the message body is used.
			// Any attachment must not include high-bit ASCII characters or
			// binary data.  If binary data is to be sent, the data should
			// be encoded using Base64 and a MIME header should be added:
			// Content-transfer-encoding: base64
			
			// Check to see if a failure occurred
			if(!SMTPIsBusy())
			{
				curHTTP.data[0] = SM_EMAIL_FINISHING;
				return HTTP_IO_WAITING;
			}
			
			// See if we're ready to write data
			if(SMTPIsPutReady() < 100u)
				return HTTP_IO_WAITING;
			
			// Write the attachment header
			SMTPPutROMString((ROM BYTE*)"\r\n--frontier\r\nContent-type: text/csv\r\nContent-Disposition: attachment; filename=\"status.csv\"\r\n\r\n");
			SMTPFlush();
			
			// Move to the next state
			curHTTP.data[0] = SM_EMAIL_PUT_ATTACHMENT_DATA_BTNS;
			
		case SM_EMAIL_PUT_ATTACHMENT_DATA_BTNS:
			// The following states output the system status as a CSV file.
			
			// Check to see if a failure occurred
			if(!SMTPIsBusy())
			{
				curHTTP.data[0] = SM_EMAIL_FINISHING;
				return HTTP_IO_WAITING;
			}
			
			// See if we're ready to write data
			if(SMTPIsPutReady() < 36u)
				return HTTP_IO_WAITING;
				
			// Write the header and button strings
			SMTPPutROMString((ROM BYTE*)"SYSTEM STATUS\r\n");
			SMTPPutROMString((ROM BYTE*)"Buttons:,");
			SMTPPut(BUTTON0_IO + '0');
			SMTPPut(',');
			SMTPPut(BUTTON1_IO + '0');
			SMTPPut(',');
			SMTPPut(BUTTON2_IO + '0');
			SMTPPut(',');
			SMTPPut(BUTTON3_IO + '0');
			SMTPPut('\r');
			SMTPPut('\n');
			SMTPFlush();
			
			// Move to the next state
			curHTTP.data[0] = SM_EMAIL_PUT_ATTACHMENT_DATA_LEDS;

		case SM_EMAIL_PUT_ATTACHMENT_DATA_LEDS:
			// Check to see if a failure occurred
			if(!SMTPIsBusy())
			{
				curHTTP.data[0] = SM_EMAIL_FINISHING;
				return HTTP_IO_WAITING;
			}
			
			// See if we're ready to write data
			if(SMTPIsPutReady() < 30u)
				return HTTP_IO_WAITING;
				
			// Write the header and button strings
			SMTPPutROMString((ROM BYTE*)"LEDs:,");
			//2EI - comenta SMTPPut(LED0_IO + '0');
			SMTPPut('0');
			SMTPPut(',');
			//2EI - comenta SMTPPut(LED1_IO + '0');
			SMTPPut('0');
			SMTPPut(',');
			//2EI - comenta SMTPPut(LED2_IO + '0');
			SMTPPut('0');
			SMTPPut(',');
			//2EI - comenta SMTPPut(LED3_IO + '0');
			SMTPPut('0');
			SMTPPut(',');
			//2EI - comenta SMTPPut(LED4_IO + '0');
			SMTPPut('0');
			SMTPPut(',');
			//2EI - comenta SMTPPut(LED5_IO + '0');
			SMTPPut('0');
			SMTPPut(',');
			//2EI - comenta SMTPPut(LED6_IO + '0');
			SMTPPut('0');
			SMTPPut(',');
			//2EI - comenta SMTPPut(LED7_IO + '0');
			SMTPPut('0');
			SMTPPut('\r');
			SMTPPut('\n');
			SMTPFlush();

			// Move to the next state
			curHTTP.data[0] = SM_EMAIL_PUT_ATTACHMENT_DATA_POT;

		case SM_EMAIL_PUT_ATTACHMENT_DATA_POT:
			// Check to see if a failure occurred
			if(!SMTPIsBusy())
			{
				curHTTP.data[0] = SM_EMAIL_FINISHING;
				return HTTP_IO_WAITING;
			}
			
			// See if we're ready to write data
			if(SMTPIsPutReady() < 16u)
				return HTTP_IO_WAITING;

			// Do the A/D conversion
			#if defined(__18CXX)
			    // Wait until A/D conversion is done
			    ADCON0bits.GO = 1;
			    while(ADCON0bits.GO);
			    // Convert 10-bit value into ASCII string
			    len = (WORD)ADRES;
			    uitoa(len, (BYTE*)&curHTTP.data[1]);
			#else
				len = (WORD)ADC1BUF0;
			    uitoa(len, (BYTE*)&curHTTP.data[1]);
			#endif

			// Write the header and button strings
			SMTPPutROMString((ROM BYTE*)"Pot:,");
			SMTPPutString(&curHTTP.data[1]);
			SMTPPut('\r');
			SMTPPut('\n');
			SMTPFlush();
			
			// Move to the next state
			curHTTP.data[0] = SM_EMAIL_PUT_TERMINATOR;
			
		case SM_EMAIL_PUT_TERMINATOR:
			// This section finishes the message
			// This consists of two dashes, the boundary, and two more dashes
			// on a single line, followed by a CRLF pair to terminate the message.

			// Check to see if a failure occured
			if(!SMTPIsBusy())
			{
				curHTTP.data[0] = SM_EMAIL_FINISHING;
				return HTTP_IO_WAITING;
			}
		
			// See if we're ready to write data
			if(SMTPIsPutReady() < 16u)
				return HTTP_IO_WAITING;
				
			// Write the ignored text				
			SMTPPutROMString((ROM BYTE*)"--frontier--\r\n");
			SMTPPutDone();
			SMTPFlush();
			
			// Move to the next state
			curHTTP.data[0] = SM_EMAIL_FINISHING;
		
		case SM_EMAIL_FINISHING:
			// Wait for status
			if(!SMTPIsBusy())
			{
				// Release the module and check success
				// Redirect the user based on the result
				if(SMTPEndUsage() == SMTP_SUCCESS)
					strcpypgm2ram((char*)curHTTP.data, (ROM void*)"/email/ok.htm");
				else
					strcpypgm2ram((char*)curHTTP.data, (ROM void*)"/email/fail.htm");
									
				// Redirect to the page
				curHTTP.httpStatus = HTTP_REDIRECT;
				return HTTP_IO_DONE;
			}
			
			return HTTP_IO_WAITING;
		
		case SM_EMAIL_CLAIM_MODULE:
		default:
			// Try to claim module
			if(SMTPBeginUsage())
			{// Module was claimed, so set up static parameters
				SMTPClient.Subject.szROM = (ROM BYTE*)"Microchip TCP/IP Stack Status Update";
				SMTPClient.ROMPointers.Subject = 1;
				SMTPClient.From.szROM = (ROM BYTE*)"\"SMTP Service\" <mchpboard@picsaregood.com>";
				SMTPClient.ROMPointers.From = 1;
				
				// The following two lines indicate to the receiving client that 
				// this message has an attachment.  The boundary field *must not*
				// be included anywhere in the content of the message.  In real 
				// applications it is typically a long random string.
				SMTPClient.OtherHeaders.szROM = (ROM BYTE*)"MIME-version: 1.0\r\nContent-type: multipart/mixed; boundary=\"frontier\"\r\n";
				SMTPClient.ROMPointers.OtherHeaders = 1;
				
				// Move our state machine forward
				ptrData = smtpData;
				szPort = NULL;
				curHTTP.data[0] = SM_EMAIL_READ_PARAM_NAME;
			}
			return HTTP_IO_WAITING;			
	}

}
#endif	// #if defined(STACK_USE_SMTP_CLIENT)

#endif //(use_post)

/*********************************************************************
 * Function:        void HTTPPrint_varname(TCP_SOCKET sktHTTP, 
 *							DWORD callbackPos, BYTE *data)
 *
 * PreCondition:    None
 *
 * Input:           sktHTTP: the TCP socket to which to write
 *					callbackPos: 0 initially
 *						return value of last call for subsequent callbacks
 *					data: this connection's data buffer
 *
 * Output:          0 if output is complete
 *					application-defined otherwise
 *
 * Side Effects:    None
 *
 * Overview:        Outputs a variable to the HTTP client.
 *
 * Note:            Return zero to indicate that this callback function 
 *					has finished writing data to the TCP socket.  A 
 *					non-zero return value indicates that more data 
 *					remains to be written, and this callback should 
 *					be called again when more space is available in 
 *					the TCP TX FIFO.  This non-zero return value will 
 *					be the value of the parameter callbackPos for the 
 *					next call.
 ********************************************************************/

ROM BYTE HTML_UP_ARROW[] = "up";
ROM BYTE HTML_DOWN_ARROW[] = "dn";

void HTTPPrint_btn(WORD num)
{
	// Determine which button
	switch(num)
	{
		case 0:
			num = BUTTON0_IO;
			break;
		case 1:
			num = BUTTON1_IO;
			break;
		case 2:
			num = BUTTON2_IO;
			break;
		case 3:
			num = BUTTON3_IO;
			break;
		default:
			num = 0;
	}

	// Print the output
	TCPPutROMString(sktHTTP, (num?HTML_UP_ARROW:HTML_DOWN_ARROW));
	return;
}
	
void HTTPPrint_led(WORD num)
{
	// Determine which LED
	switch(num)
	{
		case 0:
			num = 1;
			// 2EI - comenta num = LED0_IO;
			break;
		case 1:
			num = 1;
			// 2EI - comentanum = LED1_IO;
			break;
		case 2:
			num = 1;
			// 2EI - comenta num = LED2_IO;
			break;
		case 3:
			num = 1;
			// 2EI - comenta num = LED3_IO;
			break;
		case 4:
			num = 1;
			// 2EI - comentanum = LED4_IO;
			break;
		case 5:
			num = 1;
			// 2EI - comenta num = LED5_IO;
			break;
		case 6:
			num = 1;
			// 2EI - comenta num = LED6_IO;
			break;
		case 7:
			num = 1;
			// 2EI - comenta num = LED7_IO;
			break;

		default:
			num = 0;
	}

	// Print the output
	TCPPut(sktHTTP, (num?'1':'0'));
	return;
}

void HTTPPrint_ledSelected(WORD num, WORD state)
{
	// Determine which LED to check
	switch(num)
	{
		case 0:
			num = 1;
			//2EI - comenta num = LED0_IO;
			break;
		case 1:
			num = 1;
			// 2EI - comenta num = LED1_IO;
			break;
		case 2:
			num = 1;
			// 2EI - comenta num = LED2_IO;
			break;
		case 3:
			num = 1;
			// 2EI - comenta num = LED3_IO;
			break;
		case 4:
			num = 1;
			// 2EI - comenta num = LED4_IO;
			break;
		case 5:
			num = 1;
			// 2EI - comenta num = LED5_IO;
			break;
		case 6:
			num = 1;
			// 2EI - comenta num = LED6_IO;
			break;
		case 7:
			num = 1;
			// 2EI - comenta num = LED7_IO;
			break;

		default:
			num = 0;
	}
	
	// Print output if TRUE and ON or if FALSE and OFF
	if((state && num) || (!state && !num))
		TCPPutROMString(sktHTTP, (ROM BYTE*)"SELECTED");
	return;
}

void HTTPPrint_pot(void)
{    
	char AN0String[8];
	WORD ADval;

#if defined(__18CXX)
    // Wait until A/D conversion is done
    ADCON0bits.GO = 1;
    while(ADCON0bits.GO);

    // Convert 10-bit value into ASCII string
    ADval = (WORD)ADRES;
    //ADval *= (WORD)10;
    //ADval /= (WORD)102;
    uitoa(ADval, AN0String);
#else
	ADval = (WORD)ADC1BUF0;
	//ADval *= (WORD)10;
	//ADval /= (WORD)102;
    uitoa(ADval, (BYTE*)AN0String);
#endif

   	TCPPutArray(sktHTTP,(void *)AN0String, strlen((char*)AN0String));
}

void HTTPPrint_version(void)
{
	TCPPutROMArray(sktHTTP,(ROM void*)VERSION, strlenpgm((ROM char*)VERSION));
}

void HTTPPrint_builddate(void)
{
	TCPPutROMArray(sktHTTP,(ROM void*)__DATE__" "__TIME__, strlenpgm((ROM char*)__DATE__" "__TIME__));
}

void HTTPPrint_lcdtext(void)
{
	WORD len;

	// Determine how many bytes we can write
	len = TCPIsPutReady(sktHTTP);
	
	#if defined(USE_LCD)
	// If just starting, set callbackPos
	if(curHTTP.callbackPos == 0)
		curHTTP.callbackPos = 32;
	
	// Write a byte at a time while we still can
	// It may take up to 12 bytes to write a character
	// (spaces and newlines are longer)
	while(len > 12 && curHTTP.callbackPos)
	{
		// After 16 bytes write a newline
		if(curHTTP.callbackPos == 16)
			len -= TCPPutROMArray(sktHTTP, (ROM BYTE*)"<br />", 6);

		if(LCDText[32-curHTTP.callbackPos] == ' ' || LCDText[32-curHTTP.callbackPos] == '\0')
			len -= TCPPutROMArray(sktHTTP, (ROM BYTE*)"&nbsp;", 6);
		else
			len -= TCPPut(sktHTTP, LCDText[32-curHTTP.callbackPos]);

		curHTTP.callbackPos--;
	}
	#else
	TCPPutROMArray(sktHTTP, (ROM BYTE*)"No LCD Present", 14);
	#endif

	return;
}

void HTTPPrint_hellomsg(void)
{
	BYTE *ptr;
	
	ptr = HTTPGetROMArg(curHTTP.data, (ROM BYTE*)"name");
	
	// We omit checking for space because this is the only data being written
	if(ptr != NULL)
	{
		TCPPutROMArray(sktHTTP, (ROM BYTE*)"Hello, ", 7);
		TCPPutArray(sktHTTP, ptr, strlen((char*)ptr));
	}

	return;
}

void HTTPPrint_cookiename(void)
{
	BYTE *ptr;
	
	ptr = HTTPGetROMArg(curHTTP.data, (ROM BYTE*)"name");
	
	if(ptr)
		TCPPutArray(sktHTTP, ptr, strlen((char*)ptr));
	else
		TCPPutROMArray(sktHTTP, (ROM BYTE*)"not set", 7);
	
	return;
}

void HTTPPrint_uploadedmd5(void)
{
	BYTE i;

	// Set a flag to indicate not finished
	curHTTP.callbackPos = 1;
	
	// Make sure there's enough output space
	if(TCPIsPutReady(sktHTTP) < 32 + 37 + 5)
		return;

	// Check for flag set in HTTPPostMD5
	if(curHTTP.data[0] != 0x05)
	{// No file uploaded, so just return
		TCPPutROMArray(sktHTTP, (ROM BYTE*)"<b>Upload a File</b>", 20);
		curHTTP.callbackPos = 0;
		return;
	}
	
	TCPPutROMArray(sktHTTP, (ROM BYTE*)"<b>Uploaded File's MD5 was:</b><br />", 37);
	
	// Write a byte of the md5 sum at a time
	for(i = 1; i <= 16; i++)
	{
		TCPPut(sktHTTP, btohexa_high(curHTTP.data[i]));
		TCPPut(sktHTTP, btohexa_low(curHTTP.data[i]));
		if((i & 0x03) == 0)
			TCPPut(sktHTTP, ' ');
	}
	
	curHTTP.callbackPos = 0x00;
	return;
}

extern APP_CONFIG AppConfig;

void HTTPPrintIP(IP_ADDR ip)
{
	BYTE digits[4];
	BYTE i;
	
	for(i = 0; i < 4; i++)
	{
		if(i != 0)
			TCPPut(sktHTTP, '.');
		uitoa(ip.v[i], digits);
		TCPPutArray(sktHTTP, digits, strlen((char*)digits));
	}
}

void HTTPPrint_config_hostname(void)
{
	TCPPutArray(sktHTTP, AppConfig.NetBIOSName, strlen((char*)AppConfig.NetBIOSName));
	return;
}

void HTTPPrint_config_dhcpchecked(void)
{
	if(AppConfig.Flags.bIsDHCPEnabled)
		TCPPutROMArray(sktHTTP, (ROM BYTE*)"checked", 7);
	return;
}

void HTTPPrint_config_ip(void)
{
	HTTPPrintIP(AppConfig.MyIPAddr);
	return;
}

void HTTPPrint_config_gw(void)
{
	HTTPPrintIP(AppConfig.MyGateway);
	return;
}

void HTTPPrint_config_subnet(void)
{
	HTTPPrintIP(AppConfig.MyMask);
	return;
}

void HTTPPrint_config_dns1(void)
{
	HTTPPrintIP(AppConfig.PrimaryDNSServer);
	return;
}

void HTTPPrint_config_dns2(void)
{
	HTTPPrintIP(AppConfig.SecondaryDNSServer);
	return;
}

void HTTPPrint_config_mac(void)
{
	BYTE i;
	
	if(TCPIsPutReady(sktHTTP) < 18)
	{//need 17 bytes to write a MAC
		curHTTP.callbackPos = 0x01;
		return;
	}	
	
	// Write each byte
	for(i = 0; i < 6; i++)
	{
		if(i != 0)
			TCPPut(sktHTTP, ':');
		TCPPut(sktHTTP, btohexa_high(AppConfig.MyMACAddr.v[i]));
		TCPPut(sktHTTP, btohexa_low(AppConfig.MyMACAddr.v[i]));
	}
	
	// Indicate that we're done
	curHTTP.callbackPos = 0x00;
	return;
}

void HTTPPrint_reboot(void)
{
	// This is not so much a print function, but causes the board to reboot
	// when the configuration is changed.  If called via an AJAX call, this
	// will gracefully reset the board and bring it back online immediately
	Reset();
}

void HTTPPrint_rebootaddr(void)
{// This is the expected address of the board upon rebooting
	TCPPutArray(sktHTTP, curHTTP.data, strlen((char*)curHTTP.data));	
}

#endif
