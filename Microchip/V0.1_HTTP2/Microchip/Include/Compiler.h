/*********************************************************************
 *
 *                  Compiler and hardware specific definitions
 *
 *********************************************************************
 * FileName:        Compiler.h
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
 * Copyright � 2002-2007 Microchip Technology Inc.  All rights 
 * reserved.
 *
 * Microchip licenses to you the right to use, modify, copy, and 
 * distribute: 
 * (i)  the Software when embedded on a Microchip microcontroller or 
 *      digital signal controller product (�Device�) which is 
 *      integrated into Licensee�s product; or
 * (ii) ONLY the Software driver source files ENC28J60.c and 
 *      ENC28J60.h ported to a non-Microchip device used in 
 *      conjunction with a Microchip ethernet controller for the 
 *      sole purpose of interfacing with the ethernet controller. 
 *
 * You should refer to the license agreement accompanying this 
 * Software for additional information regarding your rights and 
 * obligations.
 *
 * THE SOFTWARE AND DOCUMENTATION ARE PROVIDED �AS IS� WITHOUT 
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
 * Howard Schlunder		10/03/2006	Original, copied from old Compiler.h
 * Howard Schlunder		11/07/2007	Reorganized and simplified
 ********************************************************************/
#ifndef __COMPILER_H
#define __COMPILER_H

// Include proper device header file
// INSTR_FREQ is used to calculate Tick counter value, set UART 
// baud rate, etc. based on CLOCK_FREQ, defined in 
// (HardwareProfile.h)
#if defined(__18CXX) || defined(HI_TECH_C)	
	// All PIC18 processors
	#if defined(HI_TECH_C)	// HI TECH PICC-18 compiler
		#define __18CXX
		#include <htc.h>
	#else					// Microchip C18 compiler
	    #include <p18cxxx.h>
	#endif
	#define INSTR_FREQ			((CLOCK_FREQ+2ul)/4ul)
#elif defined(__PIC24F__)	// Microchip C30 compiler
	// PIC24F processor
	#include <p24Fxxxx.h>
	#define INSTR_FREQ			((CLOCK_FREQ+1ul)/2ul)
#elif defined(__PIC24H__)	// Microchip C30 compiler
	// PIC24H processor
	#include <p24Hxxxx.h>
	#define INSTR_FREQ			((CLOCK_FREQ+1ul)/2ul)
#elif defined(__dsPIC33F__)	// Microchip C30 compiler
	// dsPIC33F processor
	#include <p33Fxxxx.h>
	#define INSTR_FREQ			((CLOCK_FREQ+1ul)/2ul)
#elif defined(__dsPIC30F__)	// Microchip C30 compiler
	// dsPIC30F processor
	#include <p30fxxxx.h>
	#define INSTR_FREQ			((CLOCK_FREQ+2ul)/4ul)
#elif defined(__PIC32MX__)	// Microchip C32 compiler
	#if !defined(__C32__)
		#define __C32__
	#endif
	#include <p32xxxx.h>
	#include <plib.h>
	#define INSTR_FREQ			(CLOCK_FREQ)
#else
	#error Unknown processor or compiler.  See Compiler.h
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Base RAM pointer type for given architecture
#if defined(__C32__)
	#define PTR_BASE	DWORD	
#else
	#define PTR_BASE	WORD	
#endif


// Definitions that apply to all compilers, except C18
#if !defined(__18CXX) || defined(HI_TECH_C)
	#define memcmppgm2ram(a,b,c)	memcmp(a,b,c)
	#define strcmppgm2ram(a,b)		strcmp(a,b)
	#define memcpypgm2ram(a,b,c)	memcpy(a,b,c)
	#define strcpypgm2ram(a,b)		strcpy(a,b)
	#define strncpypgm2ram(a,b,c)	strncpy(a,b,c)
	#define strstrrampgm(a,b)		strstr(a,b)
	#define	strlenpgm(a)			strlen(a)
	#define strchrpgm(a,b)			strchr(a,b)
	#define strcatpgm2ram(a,b)		strcat(a,b)
#endif


// Definitions that apply to all 8-bit products
// (PIC18)
#if defined(__18CXX)
	#define	__attribute__(a)

	// Microchip C18 specific defines
	#if !defined(HI_TECH_C)
	    #define ROM                 	rom
		#define strcpypgm2ram(a, b)		strcpypgm2ram(a,(far rom char*)b)
	#endif
	
	// HI TECH PICC-18 STD specific defines
	#if defined(HI_TECH_C)
	    #define ROM                 	const
		#define rom
	    #define Nop()               	asm("NOP");
		#define ClrWdt()				asm("CLRWDT");
	    #define Reset()					asm("RESET");
	#endif
    
// Definitions that apply to all 16-bit and 32-bit products
// (PIC24F, PIC24H, dsPIC30F, dsPIC33F, and PIC32MX)
#else
	#define	ROM						const

	// 16-bit specific defines (PIC24F, PIC24H, dsPIC30F, dsPIC33F)
	#if defined(__C30__)
		#define Reset()				asm("reset")
	#endif

	// 32-bit specific defines (PIC32MX)
	#if defined(__C32__)
		#define persistent
		#define far
		#define Reset()				SoftReset()
		#define ClrWdt()			(WDTCONSET = _WDTCON_WDTCLR_MASK)
		#define Nop()				asm("nop")
	#endif
#endif



#endif
