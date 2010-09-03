/*********************************************************************
 *
 *				RSA Public Key Encryption Library Header
 *
 *********************************************************************
 * FileName:        RSA.h
 * Dependencies:    BigInt.h
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
 * IMPORTANT:  The implementation and use of third party algorithms, 
 * specifications and/or other technology may require a license from 
 * various third parties.  It is your responsibility to obtain 
 * information regarding any applicable licensing obligations.
 *
 *
 * Author               Date		Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Elliott Wood	        2/14/07		Original
 * Elliott Wood			10/15/07	Modified API to support both ops
 ********************************************************************/

#ifndef __RSA_H
#define __RSA_H

#define RSA_KEY_WORDS	(SSL_RSA_KEY_SIZE/BIGINT_DATA_SIZE)		// Words
#define RSA_PRIME_WORDS	(SSL_RSA_KEY_SIZE/BIGINT_DATA_SIZE/2)	// Words

// TODO: properly select these options
//#define STACK_USE_RSA_DECRYPT
#define STACK_USE_RSA_ENCRYPT
#define STACK_USE_RSA

typedef enum _SM_RSA
{
	SM_RSA_IDLE = 0u,
	SM_RSA_ENCRYPT_START,
	SM_RSA_ENCRYPT,
	SM_RSA_DECRYPT_START,
	SM_RSA_DECRYPT_FIND_M1,
	SM_RSA_DECRYPT_FIND_M2,
	SM_RSA_DECRYPT_FINISH,
	SM_RSA_DONE
} SM_RSA;

typedef enum _RSA_STATUS
{
	RSA_WORKING = 0u,
	RSA_FINISHED_M1,
	RSA_FINISHED_M2,
	RSA_DONE
} RSA_STATUS;

typedef enum _RSA_DATA_FORMAT
{
	RSA_BIG_ENDIAN = 0u,
	RSA_LITTLE_ENDIAN
} RSA_DATA_FORMAT;

typedef enum _RSA_OP
{
	RSA_OP_ENCRYPT = 0u,
	RSA_OP_DECRYPT
} RSA_OP;

#define RSABeginDecrypt()	RSABeginUsage(RSA_OP_DECRYPT, 0)
#define RSABeginEncrypt(a)	RSABeginUsage(RSA_OP_ENCRYPT, a)
#define RSAEndDecrypt()		RSAEndUsage()
#define RSAEndEncrypt()		RSAEndUsage()

void RSAInit(void);

BOOL RSABeginUsage(RSA_OP op, BYTE keyBytes);
void RSAEndUsage(void);

void RSASetData(BYTE *data, BYTE len, RSA_DATA_FORMAT format);
void RSASetE(BYTE *data, BYTE len, RSA_DATA_FORMAT format);
void RSASetN(BYTE *data, RSA_DATA_FORMAT format);
void RSASetResult(BYTE *data, RSA_DATA_FORMAT format);

RSA_STATUS RSAStep(void);

#endif

