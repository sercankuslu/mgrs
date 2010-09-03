/*********************************************************************
 *
 *                  Tick Manager for Timekeeping
 *
 *********************************************************************
 * FileName:        Tick.c
 * Dependencies:    Timer 0 (PIC18) or Timer 1 (PIC24F, PIC24H, 
 *					dsPIC30F, dsPIC33F, PIC32MX)
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
 * Nilesh Rajbharti     6/28/01     Original        (Rev 1.0)
 * Nilesh Rajbharti     2/9/02      Cleanup
 * Nilesh Rajbharti     5/22/02     Rev 2.0 (See version.log for detail)
 * Howard Schlunder		6/13/07		Changed to use timer without 
 *									writing for perfect accuracy.
********************************************************************/
#define __TICK_C

#include "TCPIP Stack/TCPIP.h"

static DWORD dwInternalTicks = 0;
static BYTE vTickReading[6];

static void GetTickCopy(void);


/*********************************************************************
 * Function:        void TickInit(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          Tick manager is initialized.
 *
 * Side Effects:    None
 *
 * Overview:        Initializes Timer0 as a tick counter.
 *
 * Note:            None
 ********************************************************************/
void TickInit(void)
{
#if defined(__18CXX)
	// Use Timer0 for 8 bit processors
    // Initialize the time
    TMR0H = 0;
    TMR0L = 0;

	// Set up the timer interrupt
	INTCON2bits.TMR0IP = 0;		// Low priority
    INTCONbits.TMR0IF = 0;
    INTCONbits.TMR0IE = 1;		// Enable interrupt

    // Timer0 on, 16-bit, internal timer, 1:256 prescalar
    T0CON = 0x87;

#else
	// Use Timer 1 for 16-bit and 32-bit processors
	// 1:256 prescale
	T1CONbits.TCKPS = 3;
	// Base
	PR1 = 0xFFFF;
	// Clear counter
	TMR1 = 0;

	// Enable timer interrupt
	#if defined(__C30__)
		IPC0bits.T1IP = 2;	// Interrupt priority 2 (low)
	#else
		IPC1bits.T1IP = 2;	// Interrupt priority 2 (low)
	#endif
	IFS0bits.T1IF = 0;
	IEC0bits.T1IE = 1;

	// Start timer
	T1CONbits.TON = 1;
#endif
}

static void GetTickCopy(void)
{
	// Perform an Interrupt safe and synchronized read of the 48-bit 
	// tick value
#if defined(__18CXX)
	do
	{
		INTCONbits.TMR0IE = 1;		// Enable interrupt
		Nop();
		INTCONbits.TMR0IE = 0;		// Disable interrupt
		vTickReading[0] = TMR0L;
		vTickReading[1] = TMR0H;
		*((DWORD*)&vTickReading[2]) = dwInternalTicks;
	} while(INTCONbits.TMR0IF);
	INTCONbits.TMR0IE = 1;			// Enable interrupt
#else
	do
	{
		IEC0bits.T1IE = 1;			// Enable interrupt
		Nop();
		IEC0bits.T1IE = 0;			// Disable interrupt
		((WORD*)vTickReading)[0] = TMR1;
		vTickReading[2] = ((BYTE*)&dwInternalTicks)[0];
		vTickReading[3] = ((BYTE*)&dwInternalTicks)[1];
		vTickReading[4] = ((BYTE*)&dwInternalTicks)[2];
		vTickReading[5] = ((BYTE*)&dwInternalTicks)[3];
	} while(IFS0bits.T1IF);
	IEC0bits.T1IE = 1;				// Enable interrupt
#endif
}


/*********************************************************************
 * Function:        DWORD TickGet(void)
 *					DWORD TickGetDiv256(void)
 *					DWORD TickGetDiv64K(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          Current tick value is given.  Write timing 
 *					measurement code by calling this function and 
 *					comparing two successive results with TICK_SECOND, 
 *					TICK_SECOND*2, TICK_SECOND/1000, etc.
 *
 *					TickGet() returns the least significant 32 bits 
 *					of the internal tick counter.  It is useful for 
 *					measuring time differences up to a few hours.
 *
 *					TickGetDiv256() returns the same timer value, but 
 *					scaled by a factor of 256.  Useful for measuring 
 *					time diferences up to a few weeks.
 *
 *					TickGetDiv64K() returns the same timer value, but 
 *					scaled by a factor of 65536.  Useful for measuring 
 *					absolute time and time diferences up to a few 
 *					years.
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/
DWORD TickGet(void)
{
	GetTickCopy();
	return *((DWORD*)&vTickReading[0]);
}

DWORD TickGetDiv256(void)
{
	DWORD_VAL ret;

	GetTickCopy();
	ret.v[0] = vTickReading[1];	// Note: This copy must be done one 
	ret.v[1] = vTickReading[2];	// byte at a time to prevent misaligned 
	ret.v[2] = vTickReading[3];	// memory reads, which will reset the PIC.
	ret.v[3] = vTickReading[4];
	
	return ret.Val;
}

DWORD TickGetDiv64K(void)
{
	DWORD_VAL ret;

	GetTickCopy();
	ret.v[0] = vTickReading[2];	// Note: This copy must be done one 
	ret.v[1] = vTickReading[3];	// byte at a time to prevent misaligned 
	ret.v[2] = vTickReading[4];	// memory reads, which will reset the PIC.
	ret.v[3] = vTickReading[5];
	
	return ret.Val;
}


/*********************************************************************
 * Function:        DWORD TickConvertToMilliseconds(DWORD dwTickValue)
 *
 * PreCondition:    None
 *
 * Input:           Tick count or tick difference obtained from 
 *					calling TickGet().
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Converts a tick value or tick difference into 
 *					milliseconds.  
 *					Ex: TickConvertToMilliseconds(32768) returns 1000
 *					with a 32.768kHz clock driving the tick module (no 
 *					prescalar).
 *
 * Note:            This function does run time division on DWORDs, 
 *					which is rather slow.  Avoid using it unless you 
 *					are displaying the result to the user.
 ********************************************************************/
DWORD TickConvertToMilliseconds(DWORD dwTickValue)
{
	return (dwTickValue+(TICKS_PER_SECOND/500ul))/((DWORD)(TICKS_PER_SECOND/1000ul));
}


/*********************************************************************
 * Function:        void TickUpdate(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Internal Tick and Seconds count are updated.
 *
 * Note:            None
 ********************************************************************/
#if defined(__18CXX)
void TickUpdate(void)
{
    if(INTCONbits.TMR0IF)
    {
		// Increment internal high tick counter
		dwInternalTicks++;

		// Reset interrupt flag
        INTCONbits.TMR0IF = 0;
    }
}

/*********************************************************************
 * Function:        void _ISR _T1Interrupt(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          TickCount variable is updated
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/
#elif defined(__PIC32MX__)
void __attribute((interrupt(ipl2), vector(_TIMER_1_VECTOR), nomips16)) _T1Interrupt(void)
{
	// Increment internal high tick counter
	dwInternalTicks++;

	// Reset interrupt flag
	IFS0bits.T1IF = 0;
}
#else
#if __C30_VERSION__ >= 300
void _ISR __attribute__((__no_auto_psv__)) _T1Interrupt(void)
#else
void _ISR _T1Interrupt(void)
#endif
{
	// Increment internal high tick counter
	dwInternalTicks++;

	// Reset interrupt flag
	IFS0bits.T1IF = 0;
}
#endif
