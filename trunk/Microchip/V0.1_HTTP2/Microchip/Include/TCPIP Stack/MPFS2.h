/*********************************************************************
 *
 *               Microchip File System
 *
 *********************************************************************
 * FileName:        MPFS.h
 * Dependencies:    StackTsk.H
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
 * Elliott Wood			x/x/x		Complete Rewrite as MPFS2
********************************************************************/
#ifndef __MPFS2_H
#define __MPFS2_H

typedef DWORD MPFS_PTR;
#define MPFS_INVALID			(0xffffffffu)

#if defined(MPFS_USE_EEPROM)
	#if defined(USE_EEPROM_25LC1024)
		#define MPFS_WRITE_PAGE_SIZE		(256u)
	#else
		#define MPFS_WRITE_PAGE_SIZE		(64u)
	#endif
#endif

#define MPFS_INVALID_HANDLE 		(0xffu)
typedef BYTE MPFS_HANDLE;

// MPFS Flags
#define MPFS2_FLAG_ISZIPPED		((WORD)0x0001)
#define MPFS2_FLAG_HASINDEX		((WORD)0x0002)

// Stores each file handle's information
// Handles are free when addr = MPFS_INVALID
typedef struct _MPFS_STUB
{
	MPFS_PTR addr;
	DWORD bytesRem;
	WORD fatID;
} MPFS_STUB;

// Indicates the method for MPFSSeek
typedef enum _MPFS_SEEK_MODE
{
	MPFS_SEEK_START		= 0u,
	MPFS_SEEK_END,
	MPFS_SEEK_FORWARD,
	MPFS_SEEK_REWIND
} MPFS_SEEK_MODE;	

    //C30 routine to read program memory
    #if defined(__C30__)
		extern DWORD ReadProgramMemory(DWORD address);
	#endif

void MPFSInit(void);

// MPFS Handle Management Functions
MPFS_HANDLE MPFSOpen(BYTE* name);
#if defined(__18CXX)
	MPFS_HANDLE MPFSOpenROM(ROM BYTE* name);
#else
	#define MPFSOpenROM(a)	MPFSOpen((BYTE*) a);
#endif
void MPFSClose(MPFS_HANDLE hMPFS);
MPFS_HANDLE MPFSOpenID(WORD fatID);

// MPFS Metadata Accessors
DWORD MPFSGetTimestamp(MPFS_HANDLE hMPFS);
DWORD MPFSGetMicrotime(MPFS_HANDLE hMPFS);
WORD MPFSGetFlags(MPFS_HANDLE hMPFS);
DWORD MPFSGetSize(MPFS_HANDLE hMPFS);
DWORD MPFSGetBytesRem(MPFS_HANDLE hMPFS);
MPFS_PTR MPFSGetStartAddr(MPFS_HANDLE hMPFS);
MPFS_PTR MPFSGetEndAddr(MPFS_HANDLE hMPFS);
BOOL MPFSGetFilename(MPFS_HANDLE hMPFS, BYTE *name, WORD len);
DWORD MPFSGetPosition(MPFS_HANDLE hMPFS);
#define MPFSTell(a)	MPFSGetPosition(a)
WORD MPFSGetID(MPFS_HANDLE hMPFS);

// MPFS Data Accessors
BOOL MPFSGet(MPFS_HANDLE hMPFS, BYTE *c);
WORD MPFSGetArray(MPFS_HANDLE hMPFS, BYTE *data, WORD len);
BOOL MPFSGetLong(MPFS_HANDLE hMPFS, DWORD *ul);
BOOL MPFSSeek(MPFS_HANDLE hMPFS, DWORD offset, MPFS_SEEK_MODE mode);

// MPFS Writing Functions
MPFS_HANDLE MPFSFormat(void);
void MPFSPutEnd(void);
WORD MPFSPutArray(MPFS_HANDLE hMPFS, BYTE *data, WORD len);


#endif
