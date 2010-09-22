/*********************************************************************
 *
 *  Main Application Entry Point and TCP/IP Stack Demo
 *  Module for Microchip TCP/IP Stack
 *   -Demonstrates how to call and use the Microchip TCP/IP stack
 *	 -Reference: AN833
 *
 *********************************************************************
 * FileName:        MainDemo.c
 * Dependencies:    TCPIP.h
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
 * Author              Date         Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Nilesh Rajbharti		4/19/01		Original (Rev. 1.0)
 * Nilesh Rajbharti		2/09/02		Cleanup
 * Nilesh Rajbharti		5/22/02		Rev 2.0 (See version.log for detail)
 * Nilesh Rajbharti		7/9/02		Rev 2.1 (See version.log for detail)
 * Nilesh Rajbharti		4/7/03		Rev 2.11.01 (See version log for detail)
 * Howard Schlunder		10/1/04		Beta Rev 0.9 (See version log for detail)
 * Howard Schlunder		10/8/04		Beta Rev 0.9.1 Announce support added
 * Howard Schlunder		11/29/04	Beta Rev 0.9.2 (See version log for detail)
 * Howard Schlunder		2/10/05		Rev 2.5.0
 * Howard Schlunder		1/5/06		Rev 3.00
 * Howard Schlunder		1/18/06		Rev 3.01 ENC28J60 fixes to TCP, 
 *									UDP and ENC28J60 files
 * Howard Schlunder		3/01/06		Rev. 3.16 including 16-bit micro support
 * Howard Schlunder		4/12/06		Rev. 3.50 added LCD for Explorer 16
 * Howard Schlunder		6/19/06		Rev. 3.60 finished dsPIC30F support, added PICDEM.net 2 support
 * Howard Schlunder		8/02/06		Rev. 3.75 added beta DNS, NBNS, and HTTP client (GenericTCPClient.c) services
 * Howard Schlunder		12/28/06	Rev. 4.00RC added SMTP, Telnet, substantially modified TCP layer
 * Howard Schlunder		04/09/07	Rev. 4.02 added TCPPerformanceTest, UDPPerformanceTest, Reboot and fixed some bugs
 * Howard Schlunder		xx/xx/07	Rev. 4.03
 * HSchlunder & EWood	08/27/07	Rev. 4.11
 * HSchlunder & EWood	10/08/07	Rev. 4.13
 * HSchlunder & EWood	11/06/07	Rev. 4.16
 * HSchlunder & EWood	11/08/07	Rev. 4.17
 * HSchlunder & EWood	11/12/07	Rev. 4.18
 ********************************************************************/
/*
 * Following define uniquely deines this file as main
 * entry/application In whole project, there should only be one such
 * definition and application file must define AppConfig variable as
 * described below.
 */
#define THIS_IS_STACK_APPLICATION

#define BAUD_RATE       (9600)		// bps

// This header includes all headers for any enabled TCPIP Stack functions
#include "TCPIP Stack/TCPIP.h"

// envia um único email
static unsigned char uchar_envia_email = 0;

// This is used by other stack elements.
// Main application must define this and initialize it with proper values.
APP_CONFIG AppConfig;
BYTE AN0String[8];
BYTE myDHCPBindCount = 0xFF;

#if !defined(STACK_USE_DHCP_CLIENT)
	#define DHCPBindCount	(1)
#endif


/*
*	Início das definições do projeto
*/
#include "TCPIP Stack/Sensores.h"

unsigned char a;
unsigned char b;								// usadas no protocolo, transformam hexadecimal em decimal
unsigned char c;
unsigned char d;
unsigned char e;
unsigned char aaa;

int soma;

// ---------------------------------- USART -------------------------------------------------------
static TICK USART_Time_Out;							// Timeout para recebimento de dados serial
static unsigned char rec[14];						// buffer de mensagem recebida serial - interrupção
static unsigned char USARTString_rec[14];			// buffer de mensagem recebida serial para mostrar no navegador
static unsigned char *p;							// ponteiro para mensagem
static unsigned char var_muda;						// mudança de dados para Página Web
static unsigned char var_ser_tcp;					// indica que deve transmitir dados para Estação Remota
static unsigned char var_con_byt;					// contador de bytes da serial (substitui 0x0D do HyperTerminal)
static unsigned char con_esp;						// indicador usado para detectar espaço em branco proveniente da serial

// ---------------------------------- FIM USART -------------------------------------------------------


// Set configuration fuses
#if defined(__18CXX)
	#if defined(__EXTENDED18__)
		#pragma config XINST=ON
	#elif !defined(HI_TECH_C)
		#pragma config XINST=OFF
	#endif

	#if defined(__18F8722)
		// PICDEM HPC Explorer board
		#pragma config OSC=HSPLL, FCMEN=OFF, IESO=OFF, PWRT=OFF, WDT=OFF, LVP=OFF
	#elif defined(_18F8722)	// HI-TECH PICC-18 compiler
		// PICDEM HPC Explorer board
		__CONFIG(1, HSPLL);
		__CONFIG(2, WDTDIS);
		__CONFIG(3, MCLREN);
		__CONFIG(4, XINSTDIS & LVPDIS);
	#elif defined(__18F87J10) || defined(__18F86J15) || defined(__18F86J10) || defined(__18F85J15) || defined(__18F85J10) || defined(__18F67J10) || defined(__18F66J15) || defined(__18F66J10) || defined(__18F65J15) || defined(__18F65J10)
		// PICDEM HPC Explorer board
		#pragma config WDTEN=OFF, FOSC2=ON, FOSC=HSPLL
	#elif defined(__18F97J60) || defined(__18F96J65) || defined(__18F96J60) || defined(__18F87J60) || defined(__18F86J65) || defined(__18F86J60) || defined(__18F67J60) || defined(__18F66J65) || defined(__18F66J60) 
		// PICDEM.net 2 or any other PIC18F97J60 family device
		#pragma config WDT=OFF, FOSC2=ON, FOSC=HSPLL, ETHLED=ON
	#elif defined(_18F97J60) || defined(_18F96J65) || defined(_18F96J60) || defined(_18F87J60) || defined(_18F86J65) || defined(_18F86J60) || defined(_18F67J60) || defined(_18F66J65) || defined(_18F66J60) 
		// PICDEM.net 2 board with HI-TECH PICC-18 compiler
		__CONFIG(1, WDTDIS & XINSTDIS);
		__CONFIG(2, HSPLL);
		__CONFIG(3, ETHLEDEN);
	#elif defined(__18F4620)	
		// PICDEM Z board
		#pragma config OSC=HSPLL, WDT=OFF, MCLRE=ON, PBADEN=OFF, LVP=OFF
	#endif
#elif defined(__PIC24F__)
	// Explorer 16 board
	_CONFIG2(FNOSC_PRIPLL & POSCMOD_XT)		// Primary XT OSC with 4x PLL
	_CONFIG1(JTAGEN_OFF & FWDTEN_OFF)		// JTAG off, watchdog timer off
#elif defined(__dsPIC33F__) || defined(__PIC24H__)
	// Explorer 16 board
	_FOSCSEL(FNOSC_PRIPLL)			// PLL enabled
	_FOSC(OSCIOFNC_OFF & POSCMD_XT)	// XT Osc
	_FWDT(FWDTEN_OFF)				// Disable Watchdog timer
	// JTAG should be disabled as well
#elif defined(__dsPIC30F__)
	// dsPICDEM 1.1 board
	_FOSC(XT_PLL16)					// XT Osc + 16X PLL
	_FWDT(WDT_OFF)					// Disable Watchdog timer
	_FBORPOR(MCLR_EN & PBOR_OFF & PWRT_OFF)
#elif defined(__PIC32MX__)
	#pragma config FPLLODIV = DIV_1, FPLLMUL = MUL_18, FPLLIDIV = DIV_2, FWDTEN = OFF, FPBDIV = DIV_1, POSCMOD = XT, FNOSC = PRIPLL, CP = OFF
#endif


// Private helper functions.
// These may or may not be present in all applications.
static void InitAppConfig(void);
static void InitializeBoard(void);
static void ProcessIO(void);

#if !defined(STACK_USE_DHCP_CLIENT)
	#define DHCPBindCount	(1)
#endif

// Check sum dos pacotes que chegam pela serial
unsigned char Check_SUM(void);

// Tratamento de interrupções de alta prioridade
void HighISR(void);											

// Tratamento de interrupções de baixa prioridade
void LowISR(void);
void SerialISR(void);

static void DisplayIPValue(IP_ADDR IPVal);
void FormatNetBIOSName(BYTE Name[16]);

#if defined(MPFS_USE_EEPROM) && (defined(STACK_USE_MPFS) || defined(STACK_USE_MPFS2))
	void SaveAppConfig(void);
	#if defined(STACK_USE_UART) && defined(STACK_USE_MPFS)
		static BOOL DownloadMPFS(void);
	#endif
#else
	#define SaveAppConfig()

#endif

/*
#if defined(STACK_USE_SMTP_CLIENT)
static void SMTPDemo(void);
#endif
#if defined(STACK_USE_ICMP_CLIENT)
static void PingDemo(void);
#endif
#if defined(STACK_USE_UART)
	static void SetConfig(void);
#endif
*/

#if defined(STACK_USE_SNMP_SERVER) && !defined(SNMP_TRAP_DISABLED)
static void SNMPTrapDemo(SNMP_ID var, SNMP_VAL val, BYTE notificationCode);
#endif


// ---------------------------------Definição das interrupções---------------------------------//
#if defined(__18CXX) && !defined(HI_TECH_C)
#pragma code highVector=0x08
void HighVector(void){_asm goto HighISR _endasm}
#pragma code // Return to default code section
#endif

#if defined(__18CXX) && !defined(HI_TECH_C)
#pragma code lowVector=0x18
void LowVector(void){_asm goto LowISR _endasm}
#pragma code // Return to default code section
#endif

extern void MACISR(void);
#if defined(HI_TECH_C)
void interrupt low_priority LowISR(void)
#else
#pragma interruptlow LowISR
void LowISR(void)
#endif

{
#ifdef __18CXX
    TickUpdate();
#endif

#if defined(STACK_USE_SLIP)	
    MACISR();
#endif
}
#if defined(MCHP_C18)
    #pragma interruptlow HighISR save=section(".tmpdata"),section("MATH_DATA"),PROD
    void HighISR(void)
#elif defined(HITECH_C18)
    void interrupt HighISR(void)
#else
    void HighISR(void)
#endif
{
    SerialISR();
#if defined(STACK_USE_SLIP)
    MACISR();
#endif
}

// ---------------------------------FIM das Definição das interrupções---------------------------------//


/*

//
// PIC18 Interrupt Service Routines
// 
// NOTE: Several PICs, including the PIC18F4620 revision A3 have a RETFIE FAST/MOVFF bug
// The interruptlow keyword is used to work around the bug when using C18
#if defined(__18CXX)
	#if defined(HI_TECH_C)
	void interrupt low_priority LowISR(void)
	#else
	#pragma interruptlow LowISR
	void LowISR(void)
	#endif
	{
	    TickUpdate();
	}
	
	#if defined(HI_TECH_C)
	void interrupt HighISR(void)
	#else
	#pragma interruptlow HighISR
	void HighISR(void)
	#endif
	{
	    #if defined(STACK_USE_UART2TCP_BRIDGE)
		UART2TCPBridgeISR();
		#endif
	}
	
	#if !defined(HI_TECH_C)
	#pragma code lowVector=0x18
	void LowVector(void){_asm goto LowISR _endasm}
	#pragma code highVector=0x8
	void HighVector(void){_asm goto HighISR _endasm}
	#pragma code // Return to default code section
	#endif
	
#elif defined(__C30__)
	void _ISR __attribute__((__no_auto_psv__)) _AddressError(void)
	{
	    Nop();
		Nop();
	}
	void _ISR __attribute__((__no_auto_psv__)) _StackError(void)
	{
	    Nop();
		Nop();
	}
	
#elif defined(__C32__)
	void _general_exception_handler(unsigned cause, unsigned status)
	{
		Nop();
		Nop();
	}
#endif

*/

//
// Main application entry point.
//
#if defined(__18CXX)
void main(void)
#else
int main(void)
#endif
{
    static TICK t = 0;
	BYTE i;

    // Initialize any application specific hardware.
    InitializeBoard();

	// Inicializa Timeout da USART
	USART_Time_Out = TickGet(); // espera 10ms
		
	// inicializa indicador de espaço em branco proveniente da serial
	con_esp = 0x0;

	// contador de bytes da serial
	var_con_byt = 0;

	// Prepara para receber informação da serial
	rec[0]=0x7E;
	// p aponta para início do endereço da mensagem a ser ecebida
	p = &rec[0];												
	
	// Termina mensagem com caracter null
	//*p = '\0';													

	///USARTString_rec[0]  = '\0';
	
	// Limpando buffer serial
	RCSTAbits.CREN=0x1;								// Habilita recepção, limpando FLAG de ERRO
	a=RCREG1;

	// Habilita interrupçaõ serial
	// RCIE: EUSART Receive Interrupt Enable bit
	// 1 = Enables the EUSART receive interrupt
	// 0 = Disables de EUSART receive interrupt
	// Habilita interrupção de recepção serial
	PIE1bits.RCIE = 1;


#ifdef USE_LCD
	// Initialize and display the stack version on the LCD
	LCDInit();
	for(i = 0; i < 100; i++)
		DelayMs(1);
	strcpypgm2ram((char*)LCDText, "TCPStack " VERSION "  "
								  "                ");
	LCDUpdate();
#endif

    // Initialize all stack related components.
    // Following steps must be performed for all applications using
    // the Microchip TCP/IP Stack.
    TickInit();

#if defined(STACK_USE_MPFS) || defined(STACK_USE_MPFS2)
    // Initialize Microchip File System module
    MPFSInit();
#endif

    // Initialize Stack and application related NV variables into AppConfig.
    InitAppConfig();

	// Initialize core stack layers (MAC, ARP, TCP, UDP)
    StackInit();

#if defined(STACK_USE_UART2TCP_BRIDGE)
    UART2TCPBridgeInit();
#endif

#if defined(STACK_USE_HTTP_SERVER) || defined(STACK_USE_HTTP2_SERVER)
    HTTPInit();
#endif

#if defined(STACK_USE_SSL_SERVER)
    SSLInit();
#endif

#if defined(STACK_USE_FTP_SERVER) && defined(MPFS_USE_EEPROM) && defined(STACK_USE_MPFS)
    FTPInit();
#endif

#if defined(STACK_USE_SNMP_SERVER)
	SNMPInit();
#endif

#if defined(STACK_USE_DHCP_CLIENT)
    if(!AppConfig.Flags.bIsDHCPEnabled)
    {
        DHCPDisable();
    }
#endif

    // Once all items are initialized, go into infinite loop and let
    // stack items execute their tasks.
    // If application needs to perform its own task, it should be
    // done at the end of while loop.
    // Note that this is a "co-operative mult-tasking" mechanism
    // where every task performs its tasks (whether all in one shot
    // or part of it) and returns so that other tasks can do their
    // job.
    // If a task needs very long time to do its job, it must be broken
    // down into smaller pieces so that other tasks can have CPU time.
    while(1)
    {
        // Blink LED0 (right most one) every second.
        if(TickGet() - t >= TICK_SECOND/2ul)
        {
            t = TickGet();
            // original Microchip LED0_IO ^= 1;
			PORTJbits.RJ5 ^= 1;
        }

        // This task performs normal stack task including checking
        // for incoming packet, type of packet and calling
        // appropriate stack entity to process it.
        StackTask();

#if defined(STACK_USE_UART2TCP_BRIDGE)
        UART2TCPBridgeTask();
		//ProcessWSNData();
#endif

#if defined(STACK_USE_HTTP_SERVER) || defined(STACK_USE_HTTP2_SERVER)
        // This is a TCP application.  It listens to TCP port 80
        // with one or more sockets and responds to remote requests.
        HTTPServer();
#endif

#if defined(STACK_USE_SSL_SERVER)
		SSLServer();
#endif

#if defined(STACK_USE_FTP_SERVER) && defined(MPFS_USE_EEPROM) && defined(STACK_USE_MPFS)
        FTPServer();
#endif

#if defined(STACK_USE_SNMP_SERVER)
		SNMPTask();
#endif

#if defined(STACK_USE_ANNOUNCE)
		DiscoveryTask();
#endif

#if defined(STACK_USE_NBNS)
		NBNSTask();
#endif

#if defined(STACK_USE_DHCP_SERVER)
		DHCPServerTask();
#endif

#if defined(STACK_USE_GENERIC_TCP_CLIENT_EXAMPLE)
		GenericTCPClient();
#endif

#if defined(STACK_USE_GENERIC_TCP_SERVER_EXAMPLE)
		GenericTCPServer();
#endif

#if defined(STACK_USE_TELNET_SERVER)
		TelnetTask();
#endif

#if defined(STACK_USE_REBOOT_SERVER)
		RebootTask();
#endif

#if defined(STACK_USE_SNTP_CLIENT)
		SNTPClient();
#endif

#if defined(STACK_USE_UDP_PERFORMANCE_TEST)
		UDPPerformanceTask();
#endif

#if defined(STACK_USE_TCP_PERFORMANCE_TEST)
		TCPPerformanceTask();
#endif

#if defined(STACK_USE_SMTP_CLIENT)
		SMTPTask();
		//SMTPDemo();
#endif

#if defined(STACK_USE_ICMP_CLIENT)
		// 2EI - comentei abaixo
		// PingDemo();
#endif

//#if defined(STACK_USE_SNMP_SERVER) && !defined(SNMP_TRAP_DISABLED)
		//SNMPTrapDemo();
//#endif

        // Add your application specific tasks here.
        ProcessIO();


        // For DHCP information, display how many times we have renewed the IP
        // configuration since last reset.
        if(DHCPBindCount != myDHCPBindCount)
        {
            myDHCPBindCount = DHCPBindCount;

			#if defined(STACK_USE_UART)
				putrsUART((ROM char*)"New IP Address: ");
			#endif

            DisplayIPValue(AppConfig.MyIPAddr);	// Print to UART

			#if defined(STACK_USE_UART)
				putrsUART((ROM char*)"\r\n");
			#endif

			#if defined(STACK_USE_ANNOUNCE)
				AnnounceIP();
			#endif
        }

		// Processar dados recebidos do gateway da rede de sensores.
    }
}


static void DisplayIPValue(IP_ADDR IPVal)
{
//	printf("%u.%u.%u.%u", IPVal.v[0], IPVal.v[1], IPVal.v[2], IPVal.v[3]);
    BYTE IPDigit[4];
	BYTE i;
#ifdef USE_LCD
	BYTE j;
	BYTE LCDPos=16;
#endif

	for(i = 0; i < sizeof(IP_ADDR); i++)
	{
	    uitoa((WORD)IPVal.v[i], IPDigit);

		#if defined(STACK_USE_UART)
			putsUART(IPDigit);
		#endif

		#ifdef USE_LCD
			for(j = 0; j < strlen((char*)IPDigit); j++)
			{
				LCDText[LCDPos++] = IPDigit[j];
			}
			if(i == sizeof(IP_ADDR)-1)
				break;
			LCDText[LCDPos++] = '.';
		#else
			if(i == sizeof(IP_ADDR)-1)
				break;
		#endif

		#if defined(STACK_USE_UART)
			while(BusyUART());
			WriteUART('.');
		#endif
	}

	#ifdef USE_LCD
		if(LCDPos < 32)
			LCDText[LCDPos] = 0;
		LCDUpdate();
	#endif
}

/**********************************************************************************************
* DEFINIÇÃO DOS MÉTODOS CRIADOS PARA O PROJETO
*
* Autor:Gregory Elias Miguel
* Data:	02/09/2010		
**********************************************************************************************/

/*	
*	Método:
*	static void ValidaLimitesSensor(BYTE sensor_id, BYTE valor_sensor, GRANDEZA grandeza);
*	Descrição:  Valida o valor recebido pela porta Serial para preenchimento do valor na MIB.
*				Caso o valor ultrapasse os limites, envia um trap para o gerente.
*/
void ValidaLimitesSensor(BYTE sensor_id, short long valor_sensor, GRANDEZA grandeza)
{	
	
	/*
	* NOTIFICATION CODE - CODIGO DE ERRO ENVIADO PARA O AGENTE
	* 10 - VALOR DO SENSOR ABAIXO DA PRESSAO MINIMA PERMITIDA
	* 11 - VALOR DO SENSOR ACIMA DA PRESSAO MAXIMA PERMITIDA
	* 21 - VALOR DO SENSOR ABAIXO DA TEMPERATURA MINIMA PERMITIDA
	* 22 - VALOR DO SENSOR ACIMA DA TEMPERATURA MAXIMA PERMITIDA
	* 31 - VALOR DO SENSOR ABAIXO DA LUMINOSIDADE MINIMA PERMITIDA
	* 32 - VALOR DO SENSOR ACIMA DA LUMINOSIDADE MAXIMA PERMITIDA
	* 41 - VALOR DO SENSOR ABAIXO DA UMIDADE MINIMA PERMITIDA
	* 42 - VALOR DO SENSOR ACIMA DA UMIDADE MAXIMA PERMITIDA
	*/

	SNMP_VAL val;

	val.word = (WORD)valor_sensor;

	switch(grandeza)
	{
		case PRESSAO:
			if (valor_sensor <= PRESSAO_MIN_CONTROL)
			{	
				SNMPTrapDemo(sensor_id, val,10);
			}
			if (valor_sensor >= PRESSAO_MAX_CONTROL)
			{
				SNMPTrapDemo(sensor_id, val,11);
			}
		break;				
		case TEMPERATURA:
			if (valor_sensor <= TEMPERATURA_MIN_CONTROL)
			{
				SNMPTrapDemo(sensor_id, val,21);
			}
			if (valor_sensor >= TEMPERATURA_MAX_CONTROL)
			{
				SNMPTrapDemo(sensor_id, val,22);
			}
		break;
		case LUMINOSIDADE:
			if (valor_sensor <= LUMINOSIDADE_MIN_CONTROL)
			{
				SNMPTrapDemo(sensor_id, val,31);
			}
			if (valor_sensor >= LUMINOSIDADE_MAX_CONTROL)
			{
				SNMPTrapDemo(sensor_id, val,32);
			}
		break;
		case UMIDADE:
			if (valor_sensor <= UMIDADE_MIN_CONTROL)
			{
				SNMPTrapDemo(sensor_id, val,41);
			}
			if (valor_sensor >= UMIDADE_MAX_CONTROL)
			{
				SNMPTrapDemo(sensor_id, val,42);
			}
		break;
		default:
			break;
	}
}


/**************************************
* DEFINIÇÃO DE BITS RECEBIDOS NO BARRAMENTO SERIAL
*
* POSICAO [5] = ID DO SENSOR
* POSICAO [6] = BIT MAIS SIGNIFICATIVO DA PRESSÃO MEDIDA PELO SENSOR
* POSICAO [7] = BIT MENOS SIGNIFICATIVO DA PRESSÃO MEDIDA PELO SENSOR
* POSICAO [8] = BIT MAIS SIGNIFICATIVO DA TEMPERATURA MEDIDA PELO SENSOR
* POSICAO [9] = BIT MENOS SIGNIFICATIVO DA TEMPERATURA MEDIDA PELO SENSOR
* POSICAO [10] = BIT MAIS SIGNIFICATIVO DA LUMINOSIDADE MEDIDA PELO SENSOR
* POSICAO [11] = BIT MENOS SIGNIFICATIVO DA LUMINOSIDADE MEDIDA PELO SENSOR
* POSICAO [12] = BIT MAIS SIGNIFICATIVO DA UMIDADE MEDIDA PELO SENSOR
* POSICAO [13] = BIT MENOS SIGNIFICATIVO DA UMIDADE MEDIDA PELO SENSOR
*
*
* OBS:
* LIMITES DE MÁXIMO E MÍNIMO DAS GRANDEZAS PODERÃO SER ALTERADOS APENAS
* VIA COMANDO SNMPSET.
*
*/

static void ProcessIO(void)
{
	if (var_muda==0x01)	
	{

				// Se ocorrer outra mudança via serial volta ao começo
				//LED0_IO ^= 1;
				var_muda=0x00;
				soma = USARTString_rec[0];
				WriteUART(soma);	
				while(BusyUART());									
				soma=USARTString_rec[1];
				WriteUART(soma);										
				while(BusyUART());									
				soma=USARTString_rec[2];
				WriteUART(soma);										
				while(BusyUART());									
				soma=USARTString_rec[3];
				WriteUART(soma);										
				while(BusyUART());									
				soma=USARTString_rec[4];
				WriteUART(soma);										
				while(BusyUART());									
				soma=USARTString_rec[5];
				WriteUART(soma);										
				while(BusyUART());									
				soma=USARTString_rec[6];
				WriteUART(soma);										
				while(BusyUART());									
				soma=USARTString_rec[7];
				WriteUART(soma);										
				while(BusyUART());									
				soma=USARTString_rec[8];
				WriteUART(soma);										
				while(BusyUART());									
				soma=USARTString_rec[9];
				WriteUART(soma);										
				while(BusyUART());									
				soma=USARTString_rec[10];
				WriteUART(soma);										
				while(BusyUART());									
				soma=USARTString_rec[11];
				WriteUART(soma);										
				while(BusyUART());									
				soma=USARTString_rec[12];
				WriteUART(soma);										
				while(BusyUART());									
				soma=USARTString_rec[13];
				WriteUART(soma);										
				while(BusyUART());									
				// Se checksum OK pode atualizar variáveis
				//if (Check_SUM()==0x01) {	
				switch (USARTString_rec[5]){

					/******************************************************************************************************
					//INICIO DO CONTROLE DOS SENSORES
					/******************************************************************************************************/

					case SENSOR_01_ID:

						//Recebe dados de pressão do sensor
						soma = USARTString_rec[6];   // Recebe dados MSB			
						SENSOR_01_P_CONTROL = 256 * soma;
						soma = USARTString_rec[7];	 // Recebe dados LSB		
					    SENSOR_01_P_CONTROL = SENSOR_01_P_CONTROL + soma;

						//Recebe dados de temperatura do sensor
						soma = USARTString_rec[8];	 // Recebe dados MSB		
						SENSOR_01_T_CONTROL = 256 * soma;
						soma = USARTString_rec[9];	 // Recebe dados LSB		
					    SENSOR_01_T_CONTROL = SENSOR_01_T_CONTROL + soma;

						//Recebe dados de luminosidade do sensor
						soma = USARTString_rec[10];	 // Recebe dados MSB		
						SENSOR_01_L_CONTROL = 256 * soma;
						soma = USARTString_rec[11];	 // Recebe dados LSB		
					    SENSOR_01_L_CONTROL = SENSOR_01_L_CONTROL + soma;

						//Recebe dados de umidade do sensor
						soma = USARTString_rec[12];	 // Recebe dados MSB		
						SENSOR_01_U_CONTROL = 256 * soma;
						soma = USARTString_rec[13];	 // Recebe dados LSB		
					    SENSOR_01_U_CONTROL = SENSOR_01_U_CONTROL + soma;
					
						//Valida valores dos sensores para envio de TRAP
						ValidaLimitesSensor(SENSOR_01_ID, SENSOR_01_P_CONTROL, PRESSAO);		
						ValidaLimitesSensor(SENSOR_01_ID, SENSOR_01_T_CONTROL, TEMPERATURA);		
						ValidaLimitesSensor(SENSOR_01_ID, SENSOR_01_L_CONTROL, LUMINOSIDADE);		
						ValidaLimitesSensor(SENSOR_01_ID, SENSOR_01_U_CONTROL, UMIDADE);	

						//Transforma para string o resultado
						uitoa((WORD)(SENSOR_01_P_CONTROL), SENSOR_01_P_STR);  
						uitoa((WORD)(SENSOR_01_T_CONTROL), SENSOR_01_T_STR);  
						uitoa((WORD)(SENSOR_01_L_CONTROL), SENSOR_01_L_STR);  
						uitoa((WORD)(SENSOR_01_U_CONTROL), SENSOR_01_U_STR);  
	
						break;
				
				    case SENSOR_02_ID:

						//Recebe dados de pressão do sensor
						soma = USARTString_rec[6];   // Recebe dados MSB			
						SENSOR_02_P_CONTROL = 256 * soma;
						soma = USARTString_rec[7];	 // Recebe dados LSB		
					    SENSOR_02_P_CONTROL = SENSOR_02_P_CONTROL + soma;

						//Recebe dados de temperatura do sensor
						soma = USARTString_rec[8];	 // Recebe dados MSB		
						SENSOR_02_T_CONTROL = 256 * soma;
						soma = USARTString_rec[9];	 // Recebe dados LSB		
					    SENSOR_02_T_CONTROL = SENSOR_02_T_CONTROL + soma;

						//Recebe dados de luminosidade do sensor
						soma = USARTString_rec[10];	 // Recebe dados MSB		
						SENSOR_02_L_CONTROL = 256 * soma;
						soma = USARTString_rec[11];	 // Recebe dados LSB		
					    SENSOR_02_L_CONTROL = SENSOR_02_L_CONTROL + soma;

						//Recebe dados de umidade do sensor
						soma = USARTString_rec[12];	 // Recebe dados MSB		
						SENSOR_02_U_CONTROL = 256 * soma;
						soma = USARTString_rec[13];	 // Recebe dados LSB		
					    SENSOR_02_U_CONTROL = SENSOR_02_U_CONTROL + soma;
						
						//Valida valores dos sensores para envio de TRAP
						ValidaLimitesSensor(SENSOR_02_ID, SENSOR_02_P_CONTROL, PRESSAO);		
						ValidaLimitesSensor(SENSOR_02_ID, SENSOR_02_T_CONTROL, TEMPERATURA);		
						ValidaLimitesSensor(SENSOR_02_ID, SENSOR_02_L_CONTROL, LUMINOSIDADE);		
						ValidaLimitesSensor(SENSOR_02_ID, SENSOR_02_U_CONTROL, UMIDADE);		

						//Transforma para string o resultado
						uitoa((WORD)(SENSOR_02_P_CONTROL), SENSOR_02_P_STR);  
						uitoa((WORD)(SENSOR_02_T_CONTROL), SENSOR_02_T_STR);  
						uitoa((WORD)(SENSOR_02_L_CONTROL), SENSOR_02_L_STR);  
						uitoa((WORD)(SENSOR_02_U_CONTROL), SENSOR_02_U_STR);  

						break;
				
				    case SENSOR_03_ID:

						//Recebe dados de pressão do sensor
						soma = USARTString_rec[6];   // Recebe dados MSB			
						SENSOR_03_P_CONTROL = 256 * soma;
						soma = USARTString_rec[7];	 // Recebe dados LSB		
					    SENSOR_03_P_CONTROL = SENSOR_03_P_CONTROL + soma;

						//Recebe dados de temperatura do sensor
						soma = USARTString_rec[8];	 // Recebe dados MSB		
						SENSOR_03_T_CONTROL = 256 * soma;
						soma = USARTString_rec[9];	 // Recebe dados LSB		
					    SENSOR_03_T_CONTROL = SENSOR_03_T_CONTROL + soma;

						//Recebe dados de luminosidade do sensor
						soma = USARTString_rec[10];	 // Recebe dados MSB		
						SENSOR_03_L_CONTROL = 256 * soma;
						soma = USARTString_rec[11];	 // Recebe dados LSB		
					    SENSOR_03_L_CONTROL = SENSOR_03_L_CONTROL + soma;

						//Recebe dados de umidade do sensor
						soma = USARTString_rec[12];	 // Recebe dados MSB		
						SENSOR_03_U_CONTROL = 256 * soma;
						soma = USARTString_rec[13];	 // Recebe dados LSB		
					    SENSOR_03_U_CONTROL = SENSOR_03_U_CONTROL + soma;
						
						//Valida valores dos sensores para envio de TRAP
						ValidaLimitesSensor(SENSOR_03_ID, SENSOR_03_P_CONTROL, PRESSAO);		
						ValidaLimitesSensor(SENSOR_03_ID, SENSOR_03_T_CONTROL, TEMPERATURA);		
						ValidaLimitesSensor(SENSOR_03_ID, SENSOR_03_L_CONTROL, LUMINOSIDADE);		
						ValidaLimitesSensor(SENSOR_03_ID, SENSOR_03_U_CONTROL, UMIDADE);		

						//Transforma para string o resultado
						uitoa((WORD)(SENSOR_03_P_CONTROL), SENSOR_03_P_STR);  
						uitoa((WORD)(SENSOR_03_T_CONTROL), SENSOR_03_T_STR);  
						uitoa((WORD)(SENSOR_03_L_CONTROL), SENSOR_03_L_STR);  
						uitoa((WORD)(SENSOR_03_U_CONTROL), SENSOR_03_U_STR);  

						break;
				
				    case SENSOR_04_ID:

						//Recebe dados de pressão do sensor
						soma = USARTString_rec[6];   // Recebe dados MSB			
						SENSOR_04_P_CONTROL = 256 * soma;
						soma = USARTString_rec[7];	 // Recebe dados LSB		
					    SENSOR_04_P_CONTROL = SENSOR_04_P_CONTROL + soma;

						//Recebe dados de temperatura do sensor
						soma = USARTString_rec[8];	 // Recebe dados MSB		
						SENSOR_04_T_CONTROL = 256 * soma;
						soma = USARTString_rec[9];	 // Recebe dados LSB		
					    SENSOR_04_T_CONTROL = SENSOR_04_T_CONTROL + soma;

						//Recebe dados de luminosidade do sensor
						soma = USARTString_rec[10];	 // Recebe dados MSB		
						SENSOR_04_L_CONTROL = 256 * soma;
						soma = USARTString_rec[11];	 // Recebe dados LSB		
					    SENSOR_04_L_CONTROL = SENSOR_04_L_CONTROL + soma;

						//Recebe dados de umidade do sensor
						soma = USARTString_rec[12];	 // Recebe dados MSB		
						SENSOR_04_U_CONTROL = 256 * soma;
						soma = USARTString_rec[13];	 // Recebe dados LSB		
					    SENSOR_04_U_CONTROL = SENSOR_04_U_CONTROL + soma;
						
						//Valida valores dos sensores para envio de TRAP
						ValidaLimitesSensor(SENSOR_04_ID, SENSOR_04_P_CONTROL, PRESSAO);		
						ValidaLimitesSensor(SENSOR_04_ID, SENSOR_04_T_CONTROL, TEMPERATURA);		
						ValidaLimitesSensor(SENSOR_04_ID, SENSOR_04_L_CONTROL, LUMINOSIDADE);		
						ValidaLimitesSensor(SENSOR_04_ID, SENSOR_04_U_CONTROL, UMIDADE);			

						//Transforma para string o resultado
						uitoa((WORD)(SENSOR_04_P_CONTROL), SENSOR_04_P_STR);  
						uitoa((WORD)(SENSOR_04_T_CONTROL), SENSOR_04_T_STR);  
						uitoa((WORD)(SENSOR_04_L_CONTROL), SENSOR_04_L_STR);  
						uitoa((WORD)(SENSOR_04_U_CONTROL), SENSOR_04_U_STR);  

						break;
				
				    case SENSOR_05_ID:
						
						//Recebe dados de pressão do sensor
						soma = USARTString_rec[6];   // Recebe dados MSB			
						SENSOR_05_P_CONTROL = 256 * soma;
						soma = USARTString_rec[7];	 // Recebe dados LSB		
					    SENSOR_05_P_CONTROL = SENSOR_05_P_CONTROL + soma;

						//Recebe dados de temperatura do sensor
						soma = USARTString_rec[8];	 // Recebe dados MSB		
						SENSOR_05_T_CONTROL = 256 * soma;
						soma = USARTString_rec[9];	 // Recebe dados LSB		
					    SENSOR_05_T_CONTROL = SENSOR_05_T_CONTROL + soma;

						//Recebe dados de luminosidade do sensor
						soma = USARTString_rec[10];	 // Recebe dados MSB		
						SENSOR_05_L_CONTROL = 256 * soma;
						soma = USARTString_rec[11];	 // Recebe dados LSB		
					    SENSOR_05_L_CONTROL = SENSOR_05_L_CONTROL + soma;

						//Recebe dados de umidade do sensor
						soma = USARTString_rec[12];	 // Recebe dados MSB		
						SENSOR_05_U_CONTROL = 256 * soma;
						soma = USARTString_rec[13];	 // Recebe dados LSB		
					    SENSOR_05_U_CONTROL = SENSOR_05_U_CONTROL + soma;

						
						//Valida valores dos sensores para envio de TRAP
						ValidaLimitesSensor(SENSOR_05_ID, SENSOR_05_P_CONTROL, PRESSAO);		
						ValidaLimitesSensor(SENSOR_05_ID, SENSOR_05_T_CONTROL, TEMPERATURA);		
						ValidaLimitesSensor(SENSOR_05_ID, SENSOR_05_L_CONTROL, LUMINOSIDADE);		
						ValidaLimitesSensor(SENSOR_05_ID, SENSOR_05_U_CONTROL, UMIDADE);			

						//Transforma para string o resultado
						uitoa((WORD)(SENSOR_05_P_CONTROL), SENSOR_05_P_STR);  
						uitoa((WORD)(SENSOR_05_T_CONTROL), SENSOR_05_T_STR);  
						uitoa((WORD)(SENSOR_05_L_CONTROL), SENSOR_05_L_STR);  
						uitoa((WORD)(SENSOR_05_U_CONTROL), SENSOR_05_U_STR);  

						break;
				
				    case SENSOR_06_ID:

						//Recebe dados de pressão do sensor
						soma = USARTString_rec[6];   // Recebe dados MSB			
						SENSOR_06_P_CONTROL = 256 * soma;
						soma = USARTString_rec[7];	 // Recebe dados LSB		
					    SENSOR_06_P_CONTROL = SENSOR_06_P_CONTROL + soma;

						//Recebe dados de temperatura do sensor
						soma = USARTString_rec[8];	 // Recebe dados MSB		
						SENSOR_06_T_CONTROL = 256 * soma;
						soma = USARTString_rec[9];	 // Recebe dados LSB		
					    SENSOR_06_T_CONTROL = SENSOR_06_T_CONTROL + soma;

						//Recebe dados de luminosidade do sensor
						soma = USARTString_rec[10];	 // Recebe dados MSB		
						SENSOR_06_L_CONTROL = 256 * soma;
						soma = USARTString_rec[11];	 // Recebe dados LSB		
					    SENSOR_06_L_CONTROL = SENSOR_06_L_CONTROL + soma;

						//Recebe dados de umidade do sensor
						soma = USARTString_rec[12];	 // Recebe dados MSB		
						SENSOR_06_U_CONTROL = 256 * soma;
						soma = USARTString_rec[13];	 // Recebe dados LSB		
					    SENSOR_06_U_CONTROL = SENSOR_06_U_CONTROL + soma;						

						//Valida valores dos sensores para envio de TRAP
						ValidaLimitesSensor(SENSOR_06_ID, SENSOR_06_P_CONTROL, PRESSAO);		
						ValidaLimitesSensor(SENSOR_06_ID, SENSOR_06_T_CONTROL, TEMPERATURA);		
						ValidaLimitesSensor(SENSOR_06_ID, SENSOR_06_L_CONTROL, LUMINOSIDADE);		
						ValidaLimitesSensor(SENSOR_06_ID, SENSOR_06_U_CONTROL, UMIDADE);		

						//Transforma para string o resultado
						uitoa((WORD)(SENSOR_06_P_CONTROL), SENSOR_06_P_STR);  
						uitoa((WORD)(SENSOR_06_T_CONTROL), SENSOR_06_T_STR);  
						uitoa((WORD)(SENSOR_06_L_CONTROL), SENSOR_06_L_STR);  
						uitoa((WORD)(SENSOR_06_U_CONTROL), SENSOR_06_U_STR);  

						break;
				
				    case SENSOR_07_ID:

						//Recebe dados de pressão do sensor
						soma = USARTString_rec[6];   // Recebe dados MSB			
						SENSOR_07_P_CONTROL = 256 * soma;
						soma = USARTString_rec[7];	 // Recebe dados LSB		
					    SENSOR_07_P_CONTROL = SENSOR_07_P_CONTROL + soma;

						//Recebe dados de temperatura do sensor
						soma = USARTString_rec[8];	 // Recebe dados MSB		
						SENSOR_07_T_CONTROL = 256 * soma;
						soma = USARTString_rec[9];	 // Recebe dados LSB		
					    SENSOR_07_T_CONTROL = SENSOR_07_T_CONTROL + soma;

						//Recebe dados de luminosidade do sensor
						soma = USARTString_rec[10];	 // Recebe dados MSB		
						SENSOR_07_L_CONTROL = 256 * soma;
						soma = USARTString_rec[11];	 // Recebe dados LSB		
					    SENSOR_07_L_CONTROL = SENSOR_07_L_CONTROL + soma;

						//Recebe dados de umidade do sensor
						soma = USARTString_rec[12];	 // Recebe dados MSB		
						SENSOR_07_U_CONTROL = 256 * soma;
						soma = USARTString_rec[13];	 // Recebe dados LSB		
					    SENSOR_07_U_CONTROL = SENSOR_07_U_CONTROL + soma;
						
						//Valida valores dos sensores para envio de TRAP
						ValidaLimitesSensor(SENSOR_07_ID, SENSOR_07_P_CONTROL, PRESSAO);		
						ValidaLimitesSensor(SENSOR_07_ID, SENSOR_07_T_CONTROL, TEMPERATURA);		
						ValidaLimitesSensor(SENSOR_07_ID, SENSOR_07_L_CONTROL, LUMINOSIDADE);		
						ValidaLimitesSensor(SENSOR_07_ID, SENSOR_07_U_CONTROL, UMIDADE);		

						//Transforma para string o resultado
						uitoa((WORD)(SENSOR_07_P_CONTROL), SENSOR_07_P_STR);  
						uitoa((WORD)(SENSOR_07_T_CONTROL), SENSOR_07_T_STR);  
						uitoa((WORD)(SENSOR_07_L_CONTROL), SENSOR_07_L_STR);  
						uitoa((WORD)(SENSOR_07_U_CONTROL), SENSOR_07_U_STR);  

						break;
				
					case SENSOR_08_ID:

						//Recebe dados de pressão do sensor
						soma = USARTString_rec[6];   // Recebe dados MSB			
						SENSOR_08_P_CONTROL = 256 * soma;
						soma = USARTString_rec[7];	 // Recebe dados LSB		
					    SENSOR_08_P_CONTROL = SENSOR_08_P_CONTROL + soma;

						//Recebe dados de temperatura do sensor
						soma = USARTString_rec[8];	 // Recebe dados MSB		
						SENSOR_08_T_CONTROL = 256 * soma;
						soma = USARTString_rec[9];	 // Recebe dados LSB		
					    SENSOR_08_T_CONTROL = SENSOR_08_T_CONTROL + soma;

						//Recebe dados de luminosidade do sensor
						soma = USARTString_rec[10];	 // Recebe dados MSB		
						SENSOR_08_L_CONTROL = 256 * soma;
						soma = USARTString_rec[11];	 // Recebe dados LSB		
					    SENSOR_08_L_CONTROL = SENSOR_08_L_CONTROL + soma;

						//Recebe dados de umidade do sensor
						soma = USARTString_rec[12];	 // Recebe dados MSB		
						SENSOR_08_U_CONTROL = 256 * soma;
						soma = USARTString_rec[13];	 // Recebe dados LSB		
					    SENSOR_08_U_CONTROL = SENSOR_08_U_CONTROL + soma;						

						//Valida valores dos sensores para envio de TRAP
						ValidaLimitesSensor(SENSOR_08_ID, SENSOR_08_P_CONTROL, PRESSAO);		
						ValidaLimitesSensor(SENSOR_08_ID, SENSOR_08_T_CONTROL, TEMPERATURA);		
						ValidaLimitesSensor(SENSOR_08_ID, SENSOR_08_L_CONTROL, LUMINOSIDADE);		
						ValidaLimitesSensor(SENSOR_08_ID, SENSOR_08_U_CONTROL, UMIDADE);		

						//Transforma para string o resultado
						uitoa((WORD)(SENSOR_08_P_CONTROL), SENSOR_08_P_STR);  
						uitoa((WORD)(SENSOR_08_T_CONTROL), SENSOR_08_T_STR);  
						uitoa((WORD)(SENSOR_08_L_CONTROL), SENSOR_08_L_STR);  
						uitoa((WORD)(SENSOR_08_U_CONTROL), SENSOR_08_U_STR);  

						break;		
				
				    case SENSOR_09_ID:


						//Recebe dados de pressão do sensor
						soma = USARTString_rec[6];   // Recebe dados MSB			
						SENSOR_09_P_CONTROL = 256 * soma;
						soma = USARTString_rec[7];	 // Recebe dados LSB		
					    SENSOR_09_P_CONTROL = SENSOR_09_P_CONTROL + soma;

						//Recebe dados de temperatura do sensor
						soma = USARTString_rec[8];	 // Recebe dados MSB		
						SENSOR_09_T_CONTROL = 256 * soma;
						soma = USARTString_rec[9];	 // Recebe dados LSB		
					    SENSOR_09_T_CONTROL = SENSOR_09_T_CONTROL + soma;

						//Recebe dados de luminosidade do sensor
						soma = USARTString_rec[10];	 // Recebe dados MSB		
						SENSOR_09_L_CONTROL = 256 * soma;
						soma = USARTString_rec[11];	 // Recebe dados LSB		
					    SENSOR_09_L_CONTROL = SENSOR_09_L_CONTROL + soma;

						//Recebe dados de umidade do sensor
						soma = USARTString_rec[12];	 // Recebe dados MSB		
						SENSOR_09_U_CONTROL = 256 * soma;
						soma = USARTString_rec[13];	 // Recebe dados LSB		
					    SENSOR_09_U_CONTROL = SENSOR_09_U_CONTROL + soma;						

						//Valida valores dos sensores para envio de TRAP
						ValidaLimitesSensor(SENSOR_09_ID, SENSOR_09_P_CONTROL, PRESSAO);		
						ValidaLimitesSensor(SENSOR_09_ID, SENSOR_09_T_CONTROL, TEMPERATURA);		
						ValidaLimitesSensor(SENSOR_09_ID, SENSOR_09_L_CONTROL, LUMINOSIDADE);		
						ValidaLimitesSensor(SENSOR_09_ID, SENSOR_09_U_CONTROL, UMIDADE);		

						//Transforma para string o resultado
						uitoa((WORD)(SENSOR_09_P_CONTROL), SENSOR_09_P_STR);  
						uitoa((WORD)(SENSOR_09_T_CONTROL), SENSOR_09_T_STR);  
						uitoa((WORD)(SENSOR_09_L_CONTROL), SENSOR_09_L_STR);  
						uitoa((WORD)(SENSOR_09_U_CONTROL), SENSOR_09_U_STR);  

						break;
				
					case SENSOR_10_ID: 

						//Recebe dados de pressão do sensor
						soma = USARTString_rec[6];   // Recebe dados MSB			
						SENSOR_10_P_CONTROL = 256 * soma;
						soma = USARTString_rec[7];	 // Recebe dados LSB		
					    SENSOR_10_P_CONTROL = SENSOR_10_P_CONTROL + soma;

						//Recebe dados de temperatura do sensor
						soma = USARTString_rec[8];	 // Recebe dados MSB		
						SENSOR_10_T_CONTROL = 256 * soma;
						soma = USARTString_rec[9];	 // Recebe dados LSB		
					    SENSOR_10_T_CONTROL = SENSOR_10_T_CONTROL + soma;

						//Recebe dados de luminosidade do sensor
						soma = USARTString_rec[10];	 // Recebe dados MSB		
						SENSOR_10_L_CONTROL = 256 * soma;
						soma = USARTString_rec[11];	 // Recebe dados LSB		
					    SENSOR_10_L_CONTROL = SENSOR_10_L_CONTROL + soma;

						//Recebe dados de umidade do sensor
						soma = USARTString_rec[12];	 // Recebe dados MSB		
						SENSOR_10_U_CONTROL = 256 * soma;
						soma = USARTString_rec[13];	 // Recebe dados LSB		
					    SENSOR_10_U_CONTROL = SENSOR_10_U_CONTROL + soma;						

						//Valida valores dos sensores para envio de TRAP
						ValidaLimitesSensor(SENSOR_10_ID, SENSOR_10_P_CONTROL, PRESSAO);		
						ValidaLimitesSensor(SENSOR_10_ID, SENSOR_10_T_CONTROL, TEMPERATURA);		
						ValidaLimitesSensor(SENSOR_10_ID, SENSOR_10_L_CONTROL, LUMINOSIDADE);		
						ValidaLimitesSensor(SENSOR_10_ID, SENSOR_10_U_CONTROL, UMIDADE);		

						//Transforma para string o resultado
						uitoa((WORD)(SENSOR_10_P_CONTROL), SENSOR_10_P_STR);  
						uitoa((WORD)(SENSOR_10_T_CONTROL), SENSOR_10_T_STR);  
						uitoa((WORD)(SENSOR_10_L_CONTROL), SENSOR_10_L_STR);  
						uitoa((WORD)(SENSOR_10_U_CONTROL), SENSOR_10_U_STR);  

						break;

						/******************************************************************************************************
						//FIM DO CONTROLE DOS SENSORES
						/******************************************************************************************************/

					default:
						break;
					}
		}
}


#if defined(STACK_USE_SMTP_CLIENT)
static void SMTPDemo(void)
{
	// Send an email once if someone pushes BUTTON2 and BUTTON3 at the same time
	// This is a simple message example, where the message 
	// body must already be in RAM.
	// LED1 will be used as a busy indicator
	// LED2 will be used as a mail sent successfully indicator
	static enum
	{
		MAIL_HOME = 0,
		MAIL_BEGIN,
		MAIL_SMTP_FINISHING,
		MAIL_DONE
	} MailState = MAIL_HOME;
	static TICK WaitTime;
       
	switch(MailState)
	{
		case MAIL_HOME:
			if (uchar_envia_email == 0) 
		    // 2EI - comentei if((BUTTON2_IO == 0u) && (BUTTON3_IO == 0u))
			{
				uchar_envia_email = 1;
				// Start sending an email
				// 2EI - comentei LED1_IO = 1;
				MailState++;
				// 2EI - comentei LED2_IO = 0;
			}
			break;

		case MAIL_BEGIN:
			if(SMTPBeginUsage())
			{
				// Note that these strings must stay allocated in 
				// memory until SMTPIsBusy() returns FALSE.  To 
				// guarantee that the C compiler does not reuse this 
				// memory, you must allocate the strings as static.

				static BYTE RAMStringTo[] = "franklin.silva.souza@terra.com.br";
				//static BYTE RAMStringCC[] = "foo@picsaregood.com, \"Jane Smith\" <jane.smith@picsaregood.com>";
				//static BYTE RAMStringBCC[] = "";
				static BYTE RAMStringBody[] = "Mensagem gerada pela PME10A \r\n\r\nBotões: 3210";
				RAMStringBody[sizeof(RAMStringBody)-2] = '0'; // 2EI - comentei + BUTTON0_IO;
				RAMStringBody[sizeof(RAMStringBody)-3] = '0'; // 2EI - comentei + BUTTON1_IO;
				RAMStringBody[sizeof(RAMStringBody)-4] = '0'; // 2EI - comentei + BUTTON2_IO;
				RAMStringBody[sizeof(RAMStringBody)-5] = '0'; // 2EI - comentei + BUTTON3_IO;

				SMTPClient.Server.szROM = (ROM BYTE*)"smtp.sao.terra.com.br";	// SMTP server address
				SMTPClient.ROMPointers.Server = 1;
				SMTPClient.Username.szROM = (ROM BYTE*)"franklin.silva.souza";
				SMTPClient.ROMPointers.Username = 1;
				SMTPClient.Password.szROM = (ROM BYTE*)"nova20";
				SMTPClient.ROMPointers.Password = 1;
				SMTPClient.To.szRAM = RAMStringTo;
				SMTPClient.From.szROM = (ROM BYTE*)"\"Franklin Silva de Souza\" <franklin.silva.souza@terra.com.br>";
				SMTPClient.ROMPointers.From = 1;
				SMTPClient.Subject.szROM = (ROM BYTE*)"Olá! Teste SMTP";
				SMTPClient.ROMPointers.Subject = 1;
				SMTPClient.Body.szRAM = RAMStringBody;
				SMTPSendMail();
				MailState++;
			}
			break;

		case MAIL_SMTP_FINISHING:
			if(!SMTPIsBusy())
			{
				// Finished sending mail
				//2EI - comentei LED1_IO = 0;
				MailState++;
				WaitTime = TickGet();
				// 2EI - comentei LED2_IO = (SMTPEndUsage() == SMTP_SUCCESS);
			}
			break;

		case MAIL_DONE:
			// Wait for the user to release BUTTON2 or BUTTON3 and for at 
			// least 1 second to pass before allowing another 
			// email to be sent.  This is merely to prevent 
			// accidental flooding of email boxes while 
			// developing code.  Your application may wish to 
			// remove this.
			if(BUTTON2_IO && BUTTON3_IO)
			{
				if(TickGet() - WaitTime > TICK_SECOND)
					MailState = MAIL_HOME;
			}
			break;
	}
}

/*
static void SMTPDemo(void)
{
	// Send an email once if someone pushes BUTTON2 and BUTTON3 simultaneously
	// This is a multi-part message example, where the message 
	// body is dynamically generated and need not fit in RAM.
	// LED1 will be used as a busy indicator
	// LED2 will be used as a mail sent successfully indicator
	static enum
	{
		MAIL_HOME = 0,
		MAIL_BEGIN,
		MAIL_PUT_DATA,
		MAIL_PUT_DATA2,
		MAIL_SMTP_FINISHING,
		MAIL_DONE
	} MailState = MAIL_HOME;
	static BYTE *MemPtr;
	static TICK WaitTime;
	     
	switch(MailState)
	{
		case MAIL_HOME:
		    if((BUTTON2_IO == 0u) && (BUTTON3_IO == 0u))
			{
				// Start sending an email
				LED1_IO = 1;
				MailState++;
				LED2_IO = 0;
			}
			break;
		
		case MAIL_BEGIN:
			if(SMTPBeginUsage())
			{
				// Note that these strings must stay allocated in 
				// memory until SMTPIsBusy() returns FALSE.  To 
				// guarantee that the C compiler does not reuse this 
				// memory, you must allocate the strings as static.

				static BYTE RAMStringTo[] = "joe@picsaregood.com";
				//static BYTE RAMStringCC[] = "foo@picsaregood.com, \"Jane Smith\" <jane.smith@picsaregood.com>";
		
				SMTPClient.Server.szROM = "mail";	// SMTP server address
				SMTPClient.ROMPointers.Server = 1;
				//SMTPClient.Username.szROM = (ROM BYTE*)"mchpboard";
				//SMTPClient.ROMPointers.Username = 1;
				//SMTPClient.Password.szROM = (ROM BYTE*)"secretpassword";
				//SMTPClient.ROMPointers.Password = 1;
				SMTPClient.To.szRAM = RAMStringTo;
				//SMTPClient.CC.szRAM = RAMStringCC;
				SMTPClient.From.szROM = (ROM BYTE*)"\"SMTP Service\" <mchpboard@picsaregood.com>";
				SMTPClient.ROMPointers.From = 1;
				SMTPClient.Subject.szROM = (ROM BYTE*)"Hello world!  SMTP Test.";
				SMTPClient.ROMPointers.Subject = 1;
				SMTPSendMail();
				MailState++;
			}
			break;
		
		case MAIL_PUT_DATA:
			// Check to see if a failure occured
			if(!SMTPIsBusy())
			{
				// Finished sending mail
				LED1_IO = 0;
				MailState = MAIL_DONE;
				WaitTime = TickGet();
				LED2_IO = (SMTPEndUsage() == SMTP_SUCCESS);
				break;
			}
		
			if(SMTPIsPutReady() >= 121u)
			{
				SMTPPutROMString((ROM BYTE*)"Hello!\r\n\r\nThis mail was automatically generated by Microchip TCP/IP Stack " VERSION ".\r\n\r\nThe following is a snapshot of RAM:\r\n");
				SMTPFlush();
				
				MemPtr = 0x0000;
				MailState++;
			}
			break;
		
		case MAIL_PUT_DATA2:
			// Check to see if a failure occured
			if(!SMTPIsBusy())
			{
				// Finished sending mail
				LED1_IO = 0;
				MailState = MAIL_DONE;
				WaitTime = TickGet();
				LED2_IO = (SMTPEndUsage() == SMTP_SUCCESS);
				break;
			}
		
			if(SMTPIsPutReady() >= 75u)
			{
				BYTE i, c;
				WORD_VAL w;
		
				// Write line address
				w.Val = (WORD)MemPtr;
				SMTPPut(btohexa_high(w.v[1]));
				SMTPPut(btohexa_low(w.v[1]));
				SMTPPut(btohexa_high(w.v[0]));
				SMTPPut(btohexa_low(w.v[0]));
				SMTPPut(' ');
		
				// Write data bytes in hex
				for(i = 0; i < 16u; i++)
				{
					SMTPPut(' ');
					#if defined(__C32__)		// PIC32 has memory protection, so you can't just read from any old address
						c = 'R';
						MemPtr++;
					#else
						c = *MemPtr++;
					#endif
					SMTPPut(btohexa_high(c));
					SMTPPut(btohexa_low(c));
					if(i == 7u)
						SMTPPut(' ');
				}
		
				SMTPPut(' ');
				SMTPPut(' ');
		
				// Write data bytes in ASCII
				MemPtr -= 16;
				for(i = 0; i < 16u; i++)
				{
					#if defined(__C32__)		// PIC32 has memory protection, so you can't just read from any old address
						c = 'R';
						MemPtr++;
					#else
						c = *MemPtr++;
					#endif
					if(c < ' ' || c > '~')
						c = '.';
					SMTPPut(c);
		
					if(i == 7u)
						SMTPPut(' ');
				}
		
				SMTPPut('\r');
				SMTPPut('\n');
				SMTPFlush();
		
				// Make sure not to read from memory above address 0x0E7F.
				// Doing so would disrupt volatile pointers, ERDPT, FSR0, FSR1, FSR2, etc.
				if((WORD)MemPtr >= 0xE7Fu)
				{
					SMTPPutDone();
					MailState++;
				}
			}
			break;
		
		case MAIL_SMTP_FINISHING:
			// Check to see if we are done communicating with the SMTP server
			if(!SMTPIsBusy())
			{
				// Finished sending mail
				LED1_IO = 0;
				MailState = MAIL_DONE;
				WaitTime = TickGet();
				LED2_IO = (SMTPEndUsage() == SMTP_SUCCESS);
			}
			break;
		
		case MAIL_DONE:
			// Wait for the user to release BUTTON2 or BUTTON3 for at 
			// least 1 second to pass before allowing another 
			// email to be sent.  This is merely to prevent 
			// accidental flooding of email boxes while 
			// developing code.  Your application may wish to 
			// remove this.
			if(BUTTON2_IO || BUTTON3_IO)
			{
				if(TickGet() - WaitTime > TICK_SECOND)
					MailState = MAIL_HOME;
			}
			break;
	}
}
*/
#endif //#if defined(STACK_USE_SMTP_CLIENT)


// ICMP Echo (Ping) example code
#if defined(STACK_USE_ICMP_CLIENT)
static void PingDemo(void)
{
	static enum
	{
		SM_HOME = 0,
		SM_GET_RESPONSE
	} PingState = SM_HOME;
	static TICK Timer;
	LONG ret;
	IP_ADDR RemoteIP;

	switch(PingState)
	{
		case SM_HOME:
			// Send a ping request out if the user pushes BUTTON0 (right-most one)
			if(BUTTON0_IO == 0)
			{
				// Don't ping flood: wait at least 1 second between ping requests
				if(TickGet() - Timer > 1ul*TICK_SECOND)
				{
					// Obtain ownership of the ICMP module
					if(ICMPBeginUsage())
					{
						Timer = TickGet();
						PingState = SM_GET_RESPONSE;
	
						// Send the ping request to 4.78.194.159 (ww1.microchip.com)
						RemoteIP.v[0] = 4;
						RemoteIP.v[1] = 78;
						RemoteIP.v[2] = 194;
						RemoteIP.v[3] = 159;
						ICMPSendPing(RemoteIP.Val);
					}
				}
			}
			break;

		case SM_GET_RESPONSE:
			// Get the status of the ICMP module
			ret = ICMPGetReply();					
			if(ret == -2)
			{
				// Do nothing: still waiting for echo
				break;
			}
			else if(ret == -1)
			{
				// Request timed out
				#if defined(USE_LCD)
				memcpypgm2ram((void*)&LCDText[16], (ROM void *)"Ping timed out", 15);
				LCDUpdate();
				#endif
				PingState = SM_HOME;
			}
			else
			{
				// Echo received.  Time elapsed is stored in ret (units of TICK).
				#if defined(USE_LCD)
				memcpypgm2ram((void*)&LCDText[16], (ROM void *)"Reply: ", 7);
				uitoa((WORD)TickConvertToMilliseconds((DWORD)ret), &LCDText[16+7]);
				strcatpgm2ram((char*)&LCDText[16+7], (ROM char*)"ms");
				LCDUpdate();
				#endif
				PingState = SM_HOME;
			}
			
			// Finished with the ICMP module, release it so other apps can begin using it
			ICMPEndUsage();
			break;
	}
}
#endif	//#if defined(STACK_USE_ICMP_CLIENT)

#if defined(STACK_USE_HTTP_SERVER)
//////////////////////////////////////////////////////////////////////////////////////////
// NOTE: The following HTTP code pretains to the old HTTP server.
//       Upgrading to HTTP2 is *strongly* recommended for all new designs.
//       Custom control of HTTP2 is implemented in CustomHTTPApp.c
//////////////////////////////////////////////////////////////////////////////////////////

// CGI Command Codes
#define CGI_CMD_DIGOUT      (0)
#define CGI_CMD_LCDOUT      (1)
#define CGI_CMD_RECONFIG	(2)

// CGI Variable codes. - There could be 00h-FFh variables.
// NOTE: When specifying variables in your dynamic pages (.cgi),
//       use the hexadecimal numbering scheme and always zero pad it
//       to be exactly two characters.  Eg: "%04", "%2C"; not "%4" or "%02C"
#define VAR_LED0			(0x00)	// LED Outputs
#define VAR_LED1			(0x01)
#define VAR_LED2			(0x10)
#define VAR_LED3			(0x11)
#define VAR_LED4			(0x12)
#define VAR_LED5			(0x13)
#define VAR_LED6			(0x14)
#define VAR_LED7			(0x15)
#define VAR_ANAIN_AN0       (0x02)	// Analog Inputs (POT, temp, etc)
#define VAR_ANAIN_AN1       (0x03)
#define VAR_DIGIN0       	(0x04)	// Momentary push button inputs
#define VAR_DIGIN1       	(0x0D)
#define VAR_DIGIN2       	(0x0E)
#define VAR_DIGIN3       	(0x0F)
#define VAR_STACK_VERSION	(0x16)	// Stack constants
#define VAR_STACK_DATE		(0x17)
#define VAR_STROUT_LCD      (0x05)	// LCD Display output
#define VAR_MAC_ADDRESS     (0x06)	// Stack configuration variables
#define VAR_SERIAL_NUMBER   (0x07)
#define VAR_IP_ADDRESS      (0x08)
#define VAR_SUBNET_MASK     (0x09)
#define VAR_GATEWAY_ADDRESS (0x0A)
#define VAR_DHCP	        (0x0B)	// Use this variable when the web page is updating us
#define VAR_DHCP_TRUE       (0x0B)	// Use this variable when we are generating the web page
#define VAR_DHCP_FALSE      (0x0C)	// Use this variable when we are generating the web page


// CGI Command codes (CGI_CMD_DIGOUT).
// Should be a one digit numerical value
#define CMD_LED1			(0x0)
#define CMD_LED2			(0x1)

#endif //STACK_USE_HTTP_SERVER

/*********************************************************************
 * Function:        void HTTPExecCmd(BYTE** argv, BYTE argc)
 *
 * PreCondition:    None
 *
 * Input:           argv        - List of arguments
 *                  argc        - Argument count.
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is a "callback" from HTTPServer
 *                  task.  Whenever a remote node performs
 *                  interactive task on page that was served,
 *                  HTTPServer calls this functions with action
 *                  arguments info.
 *                  Main application should interpret this argument
 *                  and act accordingly.
 *
 *                  Following is the format of argv:
 *                  If HTTP action was : thank.htm?name=Joe&age=25
 *                      argv[0] => thank.htm
 *                      argv[1] => name
 *                      argv[2] => Joe
 *                      argv[3] => age
 *                      argv[4] => 25
 *
 *                  Use argv[0] as a command identifier and rests
 *                  of the items as command arguments.
 *
 * Note:            THIS FUNCTION IS DEPRECATED BY HTTP2
 ********************************************************************/
#if defined(STACK_USE_HTTP_SERVER)
//////////////////////////////////////////////////////////////////////////////////////////
// NOTE: The following HTTP code pretains to the old HTTP server.
//       Upgrading to HTTP2 is *strongly* recommended for all new designs.
//       Custom control of HTTP2 is implemented in CustomHTTPApp.c
//////////////////////////////////////////////////////////////////////////////////////////
ROM char COMMANDS_OK_PAGE[] = "INDEX.CGI";
ROM char CONFIG_UPDATE_PAGE[] = "CONFIG.CGI";
ROM char CMD_UNKNOWN_PAGE[] = "INDEX.CGI";

// Copy string with NULL termination.
#define COMMANDS_OK_PAGE_LEN  	(sizeof(COMMANDS_OK_PAGE))
#define CONFIG_UPDATE_PAGE_LEN  (sizeof(CONFIG_UPDATE_PAGE))
#define CMD_UNKNOWN_PAGE_LEN    (sizeof(CMD_UNKNOWN_PAGE))
void HTTPExecCmd(BYTE** argv, BYTE argc)
{
    BYTE command;
    BYTE var;
#if defined(ENABLE_REMOTE_CONFIG)
	DWORD_VAL dwVal;
    BYTE CurrentArg;
    WORD_VAL TmpWord;
#endif
    /*
     * Design your pages such that they contain command code
     * as a one character numerical value.
     * Being a one character numerical value greatly simplifies
     * the job.
     */
    command = argv[0][0] - '0';

    /*
     * Find out the cgi file name and interpret parameters
     * accordingly
     */
    switch(command)
    {
    case CGI_CMD_DIGOUT:	// ACTION=0
        /*
         * Identify the parameters.
         * Compare it in upper case format.
         */
        var = argv[1][0] - '0';

        switch(var)
        {
        case CMD_LED1:	// NAME=0
            // Toggle LED.
			// 2EI - desabilitei pois ainda não fiz placa demo
            //LED1_IO ^= 1;
            break;

        case CMD_LED2:	// NAME=1
            // Toggle LED.
			// 2EI - desabilitei pois ainda não fiz placa demo
            //LED2_IO ^= 1;
            break;
         }

         memcpypgm2ram((void*)argv[0], (ROM void*)COMMANDS_OK_PAGE, COMMANDS_OK_PAGE_LEN);
         break;
#if defined(USE_LCD)
    case CGI_CMD_LCDOUT:	// ACTION=1
		if(argc > 2u)	// Text provided in argv[2]
		{
			// Convert %20 to spaces, and other URL transformations
			UnencodeURL(argv[2]);

			// Write 32 received characters or less to LCDText
			if(strlen((char*)argv[2]) < 32u)
			{
				memset(LCDText, ' ', 32);
				strcpy((char*)LCDText, (char*)argv[2]);
			}
			else
			{
				memcpy(LCDText, (void*)argv[2], 32);
			}

			// Write LCDText to the LCD
			LCDUpdate();
		}
		else			// No text provided
		{
			LCDErase();
		}
		memcpypgm2ram((void*)argv[0], (ROM void*)COMMANDS_OK_PAGE, COMMANDS_OK_PAGE_LEN);
        break;
#endif
#if defined(ENABLE_REMOTE_CONFIG)
// Possibly useful code for remotely reconfiguring the board through 
// HTTP
	case CGI_CMD_RECONFIG:	// ACTION=2
		// Loop through all variables that we've been given
		CurrentArg = 1;
		while(argc > CurrentArg)
		{
			// Get the variable identifier (HTML "name"), and 
			// increment to the variable's value
			TmpWord.v[1] = argv[CurrentArg][0];
			TmpWord.v[0] = argv[CurrentArg++][1];
	        var = hexatob(TmpWord);
	        
	        // Make sure the variable's value exists
	        if(CurrentArg >= argc)
	        	break;
	        
	        // Take action with this variable/value
	        switch(var)
	        {
	        case VAR_IP_ADDRESS:
	        case VAR_SUBNET_MASK:
	        case VAR_GATEWAY_ADDRESS:
	        	{
		        	// Convert the returned value to the 4 octect 
		        	// binary representation
			        if(!StringToIPAddress(argv[CurrentArg], (IP_ADDR*)&dwVal))
			        	break;

					// Reconfigure the App to use the new values
			        if(var == VAR_IP_ADDRESS)
			        {
				        // Cause the IP address to be rebroadcast
				        // through Announce.c or the RS232 port since
				        // we now have a new IP address
				        if(dwVal.Val != *(DWORD*)&AppConfig.MyIPAddr)
					        DHCPBindCount++;
					    
					    // Set the new address
			        	memcpy((void*)&AppConfig.MyIPAddr, (void*)&dwVal, sizeof(AppConfig.MyIPAddr));
			        }
			        else if(var == VAR_SUBNET_MASK)
			        	memcpy((void*)&AppConfig.MyMask, (void*)&dwVal, sizeof(AppConfig.MyMask));
			        else if(var == VAR_GATEWAY_ADDRESS)
			        	memcpy((void*)&AppConfig.MyGateway, (void*)&dwVal, sizeof(AppConfig.MyGateway));
		        }
	            break;
	
	        case VAR_DHCP:
	        	if(AppConfig.Flags.bIsDHCPEnabled)
	        	{
		        	if(!(argv[CurrentArg][0]-'0'))
		        	{
		        		AppConfig.Flags.bIsDHCPEnabled = FALSE;
		        	}
		        }
		        else
	        	{
		        	if(argv[CurrentArg][0]-'0')
		        	{
						AppConfig.MyIPAddr.Val = 0x00000000ul;
		        		AppConfig.Flags.bIsDHCPEnabled = TRUE;
				        AppConfig.Flags.bInConfigMode = TRUE;
			        	DHCPReset();
		        	}
		        }
	            break;
	    	}

			// Advance to the next variable (if present)
			CurrentArg++;	
        }
		
		// Save any changes to non-volatile memory
      	SaveAppConfig();


		// Return the same CONFIG.CGI file as a result.
        memcpypgm2ram((void*)argv[0],
             (ROM void*)CONFIG_UPDATE_PAGE, CONFIG_UPDATE_PAGE_LEN);
		break;
#endif

    default:
		memcpypgm2ram((void*)argv[0], (ROM void*)COMMANDS_OK_PAGE, COMMANDS_OK_PAGE_LEN);
        break;
    }

}
#endif


/*********************************************************************
 * Function:        WORD HTTPGetVar(BYTE var, WORD ref, BYTE* val)
 *
 * PreCondition:    None
 *
 * Input:           var         - Variable Identifier
 *                  ref         - Current callback reference with
 *                                respect to 'var' variable.
 *                  val         - Buffer for value storage.
 *
 * Output:          Variable reference as required by application.
 *
 * Side Effects:    None
 *
 * Overview:        This is a callback function from HTTPServer() to
 *                  main application.
 *                  Whenever a variable substitution is required
 *                  on any html pages, HTTPServer calls this function
 *                  8-bit variable identifier, variable reference,
 *                  which indicates whether this is a first call or
 *                  not.  Application should return one character
 *                  at a time as a variable value.
 *
 * Note:            Since this function only allows one character
 *                  to be returned at a time as part of variable
 *                  value, HTTPServer() calls this function
 *                  multiple times until main application indicates
 *                  that there is no more value left for this
 *                  variable.
 *                  On begining, HTTPGetVar() is called with
 *                  ref = HTTP_START_OF_VAR to indicate that
 *                  this is a first call.  Application should
 *                  use this reference to start the variable value
 *                  extraction and return updated reference.  If
 *                  there is no more values left for this variable
 *                  application should send HTTP_END_OF_VAR.  If
 *                  there are any bytes to send, application should
 *                  return other than HTTP_START_OF_VAR and
 *                  HTTP_END_OF_VAR reference.
 *
 *                  THIS FUNCTION IS DEPRECATED BY HTTP2
 ********************************************************************/
#if defined(STACK_USE_HTTP_SERVER)
//////////////////////////////////////////////////////////////////////////////////////////
// NOTE: The following HTTP code pretains to the old HTTP server.
//       Upgrading to HTTP2 is *strongly* recommended for all new designs.
//       Custom control of HTTP2 is implemented in CustomHTTPApp.c
//////////////////////////////////////////////////////////////////////////////////////////
WORD HTTPGetVar(BYTE var, WORD ref, BYTE* val)
{
	// Temporary variables designated for storage of a whole return 
	// result to simplify logic needed since one byte must be returned
	// at a time.
	static BYTE VarString[25];
#if defined(ENABLE_REMOTE_CONFIG)
	static BYTE VarStringLen;
	BYTE *VarStringPtr;

	BYTE i;
	BYTE *DataSource;
#endif
	
	// Identify variable
    switch(var)
    {
    case VAR_LED0:
        //*val = LED0_IO ? '1':'0';
		// 2EI - desabilitei acima e coloquei um valor fixo
		*val = '1';
    break;
    case VAR_LED1:
        //*val = LED1_IO ? '1':'0';
		// 2EI - desabilitei acima e coloquei um valor fixo
		*val = '1';
    break;
    case VAR_LED2:
        //*val = LED2_IO ? '1':'0';
		// 2EI - desabilitei acima e coloquei um valor fixo
		*val = '1';
    break;
    case VAR_LED3:
        //*val = LED3_IO ? '1':'0';
		// 2EI - desabilitei acima e coloquei um valor fixo
		*val = '1';
	break;
    case VAR_LED4:
        //*val = LED4_IO ? '1':'0';
		// 2EI - desabilitei acima e coloquei um valor fixo
		*val = '1';
    break;
    case VAR_LED5:
        //*val = LED5_IO ? '1':'0';
		// 2EI - desabilitei acima e coloquei um valor fixo
		*val = '1';
    break;
    case VAR_LED6:
        //*val = LED6_IO ? '1':'0';
		// 2EI - desabilitei acima e coloquei um valor fixo
		*val = '1';
	break;
    case VAR_LED7:
        //*val = LED7_IO ? '1':'0';
		// 2EI - desabilitei acima e coloquei um valor fixo
		*val = '1';
    break;

    case VAR_ANAIN_AN0:
        *val = AN0String[(BYTE)ref];
        if(AN0String[(BYTE)ref] == '\0')
            return HTTP_END_OF_VAR;
		else if(AN0String[(BYTE)++ref] == '\0' )
            return HTTP_END_OF_VAR;
        return ref;
//    case VAR_ANAIN_AN1:
//        *val = AN1String[(BYTE)ref];
//        if(AN1String[(BYTE)ref] == '\0')
//            return HTTP_END_OF_VAR;
//		else if(AN1String[(BYTE)++ref] == '\0' )
//            return HTTP_END_OF_VAR;
//        return ref;

    case VAR_DIGIN0:
        *val = BUTTON0_IO ? '1':'0';
        break;
    case VAR_DIGIN1:
        *val = BUTTON1_IO ? '1':'0';
        break;
    case VAR_DIGIN2:
        *val = BUTTON2_IO ? '1':'0';
        break;
    case VAR_DIGIN3:
        *val = BUTTON3_IO ? '1':'0';
        break;

	case VAR_STACK_VERSION:
        if(ref == HTTP_START_OF_VAR)
		{
			strncpypgm2ram((char*)VarString, (ROM char*)VERSION, sizeof(VarString));
		}
        *val = VarString[(BYTE)ref];
        if(VarString[(BYTE)ref] == '\0')
            return HTTP_END_OF_VAR;
		else if(VarString[(BYTE)++ref] == '\0' )
            return HTTP_END_OF_VAR;
        return ref;
	case VAR_STACK_DATE:
        if(ref == HTTP_START_OF_VAR)
		{
			strncpypgm2ram((char*)VarString, (ROM char*)(__DATE__ " " __TIME__), sizeof(VarString));
		}
        *val = VarString[(BYTE)ref];
        if(VarString[(BYTE)ref] == '\0')
            return HTTP_END_OF_VAR;
		else if(VarString[(BYTE)++ref] == '\0' )
            return HTTP_END_OF_VAR;
        return ref;

#if defined(ENABLE_REMOTE_CONFIG)
    case VAR_MAC_ADDRESS:
        if ( ref == HTTP_START_OF_VAR )
        {
            VarStringLen = 2*6+5;	// 17 bytes: 2 for each of the 6 address bytes + 5 octet spacers

	        // Format the entire string
            i = 0;
            VarStringPtr = VarString;
            while(1)
            {
	            *VarStringPtr++ = btohexa_high(AppConfig.MyMACAddr.v[i]);
	            *VarStringPtr++ = btohexa_low(AppConfig.MyMACAddr.v[i]);
	            if(++i == 6)
	            	break;
	            *VarStringPtr++ = '-';
	        }
        }

		// Send one byte back to the calling function (the HTTP Server)
		*val = VarString[(BYTE)ref];
		
        if ( (BYTE)++ref == VarStringLen )
            return HTTP_END_OF_VAR;

        return ref;
    		
    case VAR_IP_ADDRESS:
    case VAR_SUBNET_MASK:
    case VAR_GATEWAY_ADDRESS:
    	// Check if ref == 0 meaning that the first character of this 
    	// variable needs to be returned
        if ( ref == HTTP_START_OF_VAR )
        {
	        // Decide which 4 variable bytes to send back
	        if(var == VAR_IP_ADDRESS)
		    	DataSource = (BYTE*)&AppConfig.MyIPAddr;
		    else if(var == VAR_SUBNET_MASK)
		    	DataSource = (BYTE*)&AppConfig.MyMask;
		    else if(var == VAR_GATEWAY_ADDRESS)
		    	DataSource = (BYTE*)&AppConfig.MyGateway;
	        
	        // Format the entire string
	        VarStringPtr = VarString;
	        i = 0;
	        while(1)
	        {
		        uitoa((WORD)*DataSource++, VarStringPtr);
		        VarStringPtr += strlen(VarStringPtr);
		        if(++i == 4)
		        	break;
		        *VarStringPtr++ = '.';
		    }
		    VarStringLen = strlen(VarString);
        }

		// Send one byte back to the calling function (the HTTP Server)
		*val = VarString[(BYTE)ref];
		
		// If this is the last byte to be returned, return 
		// HTTP_END_OF_VAR so the HTTP server won't keep calling this 
		// application callback function
        if ( (BYTE)++ref == VarStringLen )
            return HTTP_END_OF_VAR;

        return ref;
    	
    case VAR_DHCP_TRUE:
    case VAR_DHCP_FALSE:
    	// Check if ref == 0 meaning that the first character of this 
    	// variable needs to be returned
        if ( ref == HTTP_START_OF_VAR )
        {
	        if((var == VAR_DHCP_TRUE) ^ AppConfig.Flags.bIsDHCPEnabled)
	        	return HTTP_END_OF_VAR;

            VarStringLen = 7;
			memcpypgm2ram(VarString, (rom void *)"checked", 7);
        }

		*val = VarString[(BYTE)ref];
		
        if ( (BYTE)++ref == VarStringLen )
            return HTTP_END_OF_VAR;

        return ref;
#endif
    }

    return HTTP_END_OF_VAR;
}
#endif



#if defined(STACK_USE_FTP_SERVER) && defined(MPFS_USE_EEPROM) && defined(STACK_USE_MPFS)
//////////////////////////////////////////////////////////////////////////////////////////
// NOTE: The following FTP code is deprecated.
//       If your application has upgraded to HTTP2, you can upload MPFS 
//       images directly through the MPFS2.exe utility without using FTP.
//       This upload will occur over HTTP, and therefore FTP is unnecessary.
//////////////////////////////////////////////////////////////////////////////////////////
ROM char FTP_USER_NAME[]    = "admin";
ROM char FTP_USER_PASS[]    = "microchip";
#undef FTP_USER_NAME_LEN
#define FTP_USER_NAME_LEN   (sizeof(FTP_USER_NAME)-1)
#define FTP_USER_PASS_LEN   (sizeof(FTP_USER_PASS)-1)

BOOL FTPVerify(BYTE *login, BYTE *password)
{
    if ( !memcmppgm2ram(login, (ROM void*)FTP_USER_NAME, FTP_USER_NAME_LEN) )
    {
        if ( !memcmppgm2ram(password, (ROM void*)FTP_USER_PASS, FTP_USER_PASS_LEN) )
            return TRUE;
    }
    return FALSE;
}
#endif




/*********************************************************************
 * Function:        void InitializeBoard(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Initialize board specific hardware.
 *
 * Note:            None
 ********************************************************************/
static void InitializeBoard(void)
{	
	// LEDs
	//LED0_TRIS = 0;
	//LED1_TRIS = 0;
	//LED2_TRIS = 0;
	//LED3_TRIS = 0;
	LED4_TRIS = 0;
	LED5_TRIS = 0;
	//LED6_TRIS = 0;
#if !defined(EXPLORER_16)	// Pin multiplexed with a button on EXPLORER_16 
	//LED7_TRIS = 0;
#endif
	//LED0_IO = 0;
	//LED1_IO = 0;
	//LED2_IO = 0;
	//LED3_IO = 0;
	LED4_IO = 0;
	LED5_IO = 0;
	//LED6_IO = 0;
	//LED7_IO = 0;

#if defined(__18CXX)
	// Enable 4x/5x PLL on PIC18F87J10, PIC18F97J60, etc.
    OSCTUNE = 0x40;

	// Set up analog features of PORTA

	// PICDEM.net 2 board has POT on AN2, Temp Sensor on AN3
	#if defined(PICDEMNET2)
		ADCON0 = 0x09;		// ADON, Channel 2
		ADCON1 = 0x0B;		// Vdd/Vss is +/-REF, AN0, AN1, AN2, AN3 are analog
	#elif defined(PICDEMZ)
		ADCON0 = 0x81;		// ADON, Channel 0, Fosc/32
		ADCON1 = 0x0F;		// Vdd/Vss is +/-REF, AN0, AN1, AN2, AN3 are all digital
	#else
		ADCON0 = 0x01;		// ADON, Channel 0
		ADCON1 = 0x0E;		// Vdd/Vss is +/-REF, AN0 is analog
	#endif
	#if defined(__18F87J50) || defined(_18F87J50)
		#define ADCON2		ADCON1
	#endif
	ADCON2 = 0xBE;			// Right justify, 20TAD ACQ time, Fosc/64 (~21.0kHz)


    // Enable internal PORTB pull-ups
    INTCON2bits.RBPU = 0;

	// Configure USART
    TXSTA = 0x20;
    RCSTA = 0x90;

	// See if we can use the high baud rate setting
	#if ((INSTR_FREQ+2*BAUD_RATE)/BAUD_RATE/4 - 1) <= 255
		SPBRG = (INSTR_FREQ+2*BAUD_RATE)/BAUD_RATE/4 - 1;
		TXSTAbits.BRGH = 1;
	#else	// Use the low baud rate setting
		SPBRG = (INSTR_FREQ+8*BAUD_RATE)/BAUD_RATE/16 - 1;
	#endif


	// Enable Interrupts
	RCONbits.IPEN = 1;		// Enable interrupt priorities
    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;

    // Do a calibration A/D conversion
	#if defined(__18F87J10) || defined(__18F86J15) || defined(__18F86J10) || defined(__18F85J15) || defined(__18F85J10) || defined(__18F67J10) || defined(__18F66J15) || defined(__18F66J10) || defined(__18F65J15) || defined(__18F65J10) || defined(__18F97J60) || defined(__18F96J65) || defined(__18F96J60) || defined(__18F87J60) || defined(__18F86J65) || defined(__18F86J60) || defined(__18F67J60) || defined(__18F66J65) || defined(__18F66J60) || \
	defined(_18F87J10)  || defined(_18F86J15)  || defined(_18F86J10)  || defined(_18F85J15)  || defined(_18F85J10)  || defined(_18F67J10)  || defined(_18F66J15)  || defined(_18F66J10)  || defined(_18F65J15)  || defined(_18F65J10)  || defined(_18F97J60)  || defined(_18F96J65)  || defined(_18F96J60)  || defined(_18F87J60)  || defined(_18F86J65)  || defined(_18F86J60)  || defined(_18F67J60)  || defined(_18F66J65)  || defined(_18F66J60)
		ADCON0bits.ADCAL = 1;
	    ADCON0bits.GO = 1;
		while(ADCON0bits.GO);
		ADCON0bits.ADCAL = 0;
	#endif

#else	// 16-bit C30 and and 32-bit C32
	#if defined(__PIC32MX__)
	{
		BYTE i;

		// Enable multi-vectored interrupts
		INTEnableSystemMultiVectoredInt();
		
		// Enable optimal performance
		SYSTEMConfigPerformance(INSTR_FREQ);
		mOSCSetPBDIV(OSC_PB_DIV_1);				// Use 1:1 CPU Core:Peripheral clocks
		
		// Disable JTAG port so we get our I/O pins back, but first
		// wait 50ms so if you want to reprogram the part with 
		// JTAG, you'll still have a tiny window before JTAG goes away
		for(i = 0; i < 50; i++)
			DelayMs(1);
		DDPCONbits.JTAGEN = 0;
		LED_IO = 0x00;				// Turn the LEDs off
	}
	#endif

	#if defined(__dsPIC33F__) || defined(__PIC24H__)
		// Crank up the core frequency
		PLLFBD = 38;				// Multiply by 40 for 160MHz VCO output (8MHz XT oscillator)
		CLKDIV = 0x0000;			// FRC: divide by 2, PLLPOST: divide by 2, PLLPRE: divide by 2
	
		// Port I/O
		AD1PCFGHbits.PCFG23 = 1;	// Make RA7 (BUTTON1) a digital input

		// ADC
	    AD1CHS0 = 0;				// Input to AN0 (potentiometer)
		AD1PCFGLbits.PCFG5 = 0;		// Disable digital input on AN5 (potentiometer)
		AD1PCFGLbits.PCFG4 = 0;		// Disable digital input on AN4 (TC1047A temp sensor)
	#else	//defined(__PIC24F__)
		// ADC
	    AD1CHS = 0;					// Input to AN0 (potentiometer)
		AD1PCFGbits.PCFG5 = 0;		// Disable digital input on AN5 (potentiometer)
		AD1PCFGbits.PCFG4 = 0;		// Disable digital input on AN4 (TC1047A temp sensor)
	#endif

	// ADC
	AD1CON1 = 0x84E4;			// Turn on, auto sample start, auto-convert, 12 bit mode (on parts with a 12bit A/D)
	AD1CON2 = 0x0404;			// AVdd, AVss, int every 2 conversions, MUXA only, scan
	AD1CON3 = 0x1003;			// 16 Tad auto-sample, Tad = 3*Tcy
	AD1CSSL = 1<<5;				// Scan pot

	// UART
	#if defined(STACK_USE_UART)
		UARTTX_TRIS = 0;
		UARTRX_TRIS = 1;
		UMODE = 0x8000;			// Set UARTEN.  Note: this must be done before setting UTXEN

		#if defined(__C30__)
			USTA = 0x0400;		// UTXEN set
			#define CLOSEST_UBRG_VALUE ((INSTR_FREQ+8ul*BAUD_RATE)/16/BAUD_RATE-1)
			#define BAUD_ACTUAL (INSTR_FREQ/16/(CLOSEST_UBRG_VALUE+1))
		#else	//defined(__C32__)
			USTA = 0x00001400;		// RXEN set, TXEN set
			#define CLOSEST_UBRG_VALUE ((PERIPHERAL_FREQ+8ul*BAUD_RATE)/16/BAUD_RATE-1)
			#define BAUD_ACTUAL (PERIPHERAL_FREQ/16/(CLOSEST_UBRG_VALUE+1))
		#endif
	
		#define BAUD_ERROR ((BAUD_ACTUAL > BAUD_RATE) ? BAUD_ACTUAL-BAUD_RATE : BAUD_RATE-BAUD_ACTUAL)
		#define BAUD_ERROR_PRECENT	((BAUD_ERROR*100+BAUD_RATE/2)/BAUD_RATE)
		#if (BAUD_ERROR_PRECENT > 3)
			#warning UART frequency error is worse than 3%
		#elif (BAUD_ERROR_PRECENT > 2)
			#warning UART frequency error is worse than 2%
		#endif
	
		UBRG = CLOSEST_UBRG_VALUE;
	#endif

#endif

#if defined(PIC24FJ64GA004_PIM)
	// Remove some LED outputs to regain other functions
	LED1_TRIS = 1;		// Multiplexed with BUTTON0
	LED5_TRIS = 1;		// Multiplexed with EEPROM CS
	LED7_TRIS = 1;		// Multiplexed with BUTTON1
	
	// Inputs
	RPINR19bits.U2RXR = 19;			//U2RX = RP19
	RPINR22bits.SDI2R = 20;			//SDI2 = RP20
	RPINR20bits.SDI1R = 17;			//SDI1 = RP17
	
	// Outputs
	RPOR12bits.RP25R = U2TX_IO;		//RP25 = U2TX  
	RPOR12bits.RP24R = SCK2OUT_IO; 	//RP24 = SCK2
	RPOR10bits.RP21R = SDO2_IO;		//RP21 = SDO2
	RPOR7bits.RP15R = SCK1OUT_IO; 	//RP15 = SCK1
	RPOR8bits.RP16R = SDO1_IO;		//RP16 = SDO1
	
	AD1PCFG = 0xFFFF;				//All digital inputs - POT and Temp are on same pin as SDO1/SDI1, which is needed for ENC28J60 commnications

	// Lock the PPS
	asm volatile (	"mov #OSCCON,w1 \n"
					"mov #0x46, w2 \n"
					"mov #0x57, w3 \n"
					"mov.b w2,[w1] \n"
					"mov.b w3,[w1] \n"
					"bset OSCCON, #6");

#elif defined(DSPICDEM11)
	// Deselect the LCD controller (PIC18F252 onboard) to ensure there is no SPI2 contention
	LCDCTRL_CS_TRIS = 0;
	LCDCTRL_CS_IO = 1;

	// Hold the codec in reset to ensure there is no SPI2 contention
	CODEC_RST_TRIS = 0;
	CODEC_RST_IO = 0;

#elif defined(DSPICDEMNET1) || defined(DSPICDEMNET2)
	// Hold Si3021 in reset to keep the speaker as quiet as possible
	TRISGbits.TRISG8 = 0;
	LATGbits.LATG8 = 0;

	// Ensure that the SRAM does not interfere with RTL8019AS communications
	SRAM_CE_ADPCFG = 1;
	SRAM_OE_ADPCFG = 1;
	SRAM_CE_IO = 1;
	SRAM_OE_IO = 1;
	SRAM_CE_TRIS = 0;
	SRAM_OE_TRIS = 0;

#endif

#if defined(SPIRAM_CS_TRIS)
	SPIRAMInit();
#endif
#if defined(SPIFLASH_CS_TRIS)
	SPIFlashInit();
#endif
}

/*********************************************************************
 * Function:        void InitAppConfig(void)
 *
 * PreCondition:    MPFSInit() is already called.
 *
 * Input:           None
 *
 * Output:          Write/Read non-volatile config variables.
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/
// Uncomment these two pragmas for production MAC address 
// serialization if using C18. The MACROM=0x1FFF0 statement causes 
// the MAC address to be located at aboslute program memory address 
// 0x1FFF0 for easy auto-increment without recompiling the stack for 
// each device made.  Note, other compilers/linkers use a different 
// means of allocating variables at an absolute address.  Check your 
// compiler documentation for the right method.
//#pragma romdata MACROM=0x1FFF0
static ROM BYTE SerializedMACAddress[6] = {MY_DEFAULT_MAC_BYTE1, MY_DEFAULT_MAC_BYTE2, MY_DEFAULT_MAC_BYTE3, MY_DEFAULT_MAC_BYTE4, MY_DEFAULT_MAC_BYTE5, MY_DEFAULT_MAC_BYTE6};
//#pragma romdata

static void InitAppConfig(void)
{
#if defined(MPFS_USE_EEPROM) && (defined(STACK_USE_MPFS) || defined(STACK_USE_MPFS2))
    BYTE c;
    BYTE *p;
#endif

	AppConfig.Flags.bIsDHCPEnabled = TRUE;
	AppConfig.Flags.bInConfigMode = TRUE;
	memcpypgm2ram((void*)&AppConfig.MyMACAddr, (ROM void*)SerializedMACAddress, sizeof(AppConfig.MyMACAddr));
	AppConfig.MyIPAddr.Val = MY_DEFAULT_IP_ADDR_BYTE1 | MY_DEFAULT_IP_ADDR_BYTE2<<8ul | MY_DEFAULT_IP_ADDR_BYTE3<<16ul | MY_DEFAULT_IP_ADDR_BYTE4<<24ul;
	AppConfig.DefaultIPAddr.Val = AppConfig.MyIPAddr.Val;
	AppConfig.MyMask.Val = MY_DEFAULT_MASK_BYTE1 | MY_DEFAULT_MASK_BYTE2<<8ul | MY_DEFAULT_MASK_BYTE3<<16ul | MY_DEFAULT_MASK_BYTE4<<24ul;
	AppConfig.DefaultMask.Val = AppConfig.MyMask.Val;
	AppConfig.MyGateway.Val = MY_DEFAULT_GATE_BYTE1 | MY_DEFAULT_GATE_BYTE2<<8ul | MY_DEFAULT_GATE_BYTE3<<16ul | MY_DEFAULT_GATE_BYTE4<<24ul;
	AppConfig.PrimaryDNSServer.Val = MY_DEFAULT_PRIMARY_DNS_BYTE1 | MY_DEFAULT_PRIMARY_DNS_BYTE2<<8ul  | MY_DEFAULT_PRIMARY_DNS_BYTE3<<16ul  | MY_DEFAULT_PRIMARY_DNS_BYTE4<<24ul;
	AppConfig.SecondaryDNSServer.Val = MY_DEFAULT_SECONDARY_DNS_BYTE1 | MY_DEFAULT_SECONDARY_DNS_BYTE2<<8ul  | MY_DEFAULT_SECONDARY_DNS_BYTE3<<16ul  | MY_DEFAULT_SECONDARY_DNS_BYTE4<<24ul;

	// Load the default NetBIOS Host Name
	memcpypgm2ram(AppConfig.NetBIOSName, (ROM void*)MY_DEFAULT_HOST_NAME, 16);
	FormatNetBIOSName(AppConfig.NetBIOSName);

#if defined(MPFS_USE_EEPROM) && (defined(STACK_USE_MPFS) || defined(STACK_USE_MPFS2))
    p = (BYTE*)&AppConfig;


    XEEBeginRead(0x0000);
    c = XEERead();
    XEEEndRead();

    // When a record is saved, first byte is written as 0x60 to indicate
    // that a valid record was saved.  Note that older stack versions 
	// used 0x57.  This change has been made to so old EEPROM contents 
	// will get overwritten.  The AppConfig() structure has been changed,
	// resulting in parameter misalignment if still using old EEPROM 
	// contents.
    if(c == 0x60u)
    {
        XEEBeginRead(0x0001);
        for ( c = 0; c < sizeof(AppConfig); c++ )
            *p++ = XEERead();
        XEEEndRead();
    }
    else
        SaveAppConfig();
#endif
}

#if defined(MPFS_USE_EEPROM) && (defined(STACK_USE_MPFS) || defined(STACK_USE_MPFS2))
void SaveAppConfig(void)
{
    BYTE c;
    BYTE *p;

    p = (BYTE*)&AppConfig;
    XEEBeginWrite(0x0000);
    XEEWrite(0x60);
    for ( c = 0; c < sizeof(AppConfig); c++ )
    {
        XEEWrite(*p++);
    }

    XEEEndWrite();
}
#endif



#if defined(STACK_USE_UART)
#define MAX_USER_RESPONSE_LEN   (20u)
static void SetConfig(void)
{
    BYTE response[MAX_USER_RESPONSE_LEN];
    IP_ADDR tempIPValue;
    IP_ADDR *destIPValue;
	WORD_VAL wvTemp;
    BOOL bQuit = FALSE;

	while(!bQuit)
	{
		// Display the menu
	    putrsUART("\r\n\r\n\rMicrochip TCP/IP Config Application ("VERSION", " __DATE__ ")\r\n\r\n");
	    putrsUART("\t1: Change serial number:\t\t");
		wvTemp.v[1] = AppConfig.MyMACAddr.v[4];
		wvTemp.v[0] = AppConfig.MyMACAddr.v[5];
		uitoa(wvTemp.Val, response);
		putsUART(response);
		putrsUART("\r\n\t2: Change host name:\t\t\t");
		putsUART(AppConfig.NetBIOSName);
	    putrsUART("\r\n\t3: Change static IP address:\t\t");
	    DisplayIPValue(AppConfig.MyIPAddr);
	    putrsUART("\r\n\t4: Change static gateway address:\t");
	    DisplayIPValue(AppConfig.MyGateway);
	    putrsUART("\r\n\t5: Change static subnet mask:\t\t");
	    DisplayIPValue(AppConfig.MyMask);
		putrsUART("\r\n\t6: Change static primary DNS server:\t");
	    DisplayIPValue(AppConfig.PrimaryDNSServer);
		putrsUART("\r\n\t7: Change static secondary DNS server:\t");
	    DisplayIPValue(AppConfig.SecondaryDNSServer);
	    putrsUART("\r\n\t8: ");
		putrsUART((ROM BYTE*)(AppConfig.Flags.bIsDHCPEnabled ? "Dis" : "En"));
		putrsUART("able DHCP & IP Gleaning:\t\tDHCP is currently ");
		putrsUART((ROM BYTE*)(AppConfig.Flags.bIsDHCPEnabled ? "enabled" : "disabled"));
	    putrsUART("\r\n\t9: Download MPFS image.");
	    putrsUART("\r\n\t0: Save & Quit.");
	    putrsUART("\r\nEnter a menu choice: ");
	
	
		// Wait for the user to press a key
	    while(!DataRdyUART());
	
		putrsUART((ROM char*)"\r\n");
	
		// Execute the user selection
	    switch(ReadUART())
	    {
		    case '1':
				putrsUART("New setting: ");
				if(ReadStringUART(response, sizeof(response)))
				{
					wvTemp.Val = atoi((char*)response);
			        AppConfig.MyMACAddr.v[4] = wvTemp.v[1];
		    	    AppConfig.MyMACAddr.v[5] = wvTemp.v[0];
				}
		        break;
		
			case '2':
				putrsUART("New setting: ");
		        ReadStringUART(response, sizeof(response) > sizeof(AppConfig.NetBIOSName) ? sizeof(AppConfig.NetBIOSName) : sizeof(response));
				if(response[0] != '\0')
				{
					memcpy(AppConfig.NetBIOSName, (void*)response, sizeof(AppConfig.NetBIOSName));
			        FormatNetBIOSName(AppConfig.NetBIOSName);
				}
				break;
		
		    case '3':
		        destIPValue = &AppConfig.MyIPAddr;
		        goto ReadIPConfig;
		
		    case '4':
		        destIPValue = &AppConfig.MyGateway;
		        goto ReadIPConfig;
		
		    case '5':
		        destIPValue = &AppConfig.MyMask;
		        goto ReadIPConfig;
		
		    case '6':
		        destIPValue = &AppConfig.PrimaryDNSServer;
		        goto ReadIPConfig;
	
			case '7':
		        destIPValue = &AppConfig.SecondaryDNSServer;
		        goto ReadIPConfig;
		
ReadIPConfig:
				putrsUART("New setting: ");
		        ReadStringUART(response, sizeof(response));
		
		        if(StringToIPAddress(response, &tempIPValue))
		            destIPValue->Val = tempIPValue.Val;
				else
		            putrsUART("Invalid input.\r\n");

		        break;
		
		
		    case '8':
		        AppConfig.Flags.bIsDHCPEnabled = !AppConfig.Flags.bIsDHCPEnabled;
		        break;
		
		    case '9':
				#if defined(MPFS_USE_EEPROM) && defined(STACK_USE_MPFS)
		        	DownloadMPFS();
				#endif
		        break;
		
		    case '0':
			    bQuit = TRUE;
				#if defined(MPFS_USE_EEPROM) && (defined(STACK_USE_MPFS) || defined(STACK_USE_MPFS2))
		        	SaveAppConfig();
					putrsUART("Settings saved.\r\n");
				#else
					putrsUART("External MPFS not enabled -- settings will be lost at reset.\r\n");
				#endif
		        break;
		}
	}
}
#endif //#if defined(STACK_USE_UART)


#if defined(MPFS_USE_EEPROM) && defined(STACK_USE_MPFS) && defined(STACK_USE_UART)
/*********************************************************************
 * Function:        BOOL DownloadMPFS(void)
 *
 * PreCondition:    MPFSInit() is already called.
 *
 * Input:           None
 *
 * Output:          TRUE if successful
 *                  FALSE otherwise
 *
 * Side Effects:    This function uses 128 bytes of Bank 4 using
 *                  indirect pointer.  This requires that no part of
 *                  code is using this block during or before calling
 *                  this function.  Once this function is done,
 *                  that block of memory is available for general use.
 *
 * Overview:        This function implements XMODEM protocol to
 *                  be able to receive a binary file from PC
 *                  applications such as HyperTerminal.
 *
 * Note:            In current version, this function does not
 *                  implement user interface to set IP address and
 *                  other informations.  User should create their
 *                  own interface to allow user to modify IP
 *                  information.
 *                  Also, this version implements simple user
 *                  action to start file transfer.  User may
 *                  evaulate its own requirement and implement
 *                  appropriate start action.
 *
 ********************************************************************/
#define XMODEM_SOH      0x01u
#define XMODEM_EOT      0x04u
#define XMODEM_ACK      0x06u
#define XMODEM_NAK      0x15u
#define XMODEM_CAN      0x18u
#define XMODEM_BLOCK_LEN 128u
//////////////////////////////////////////////////////////////////////////////////////////
// NOTE: The following XMODEM code pretains to MPFS Classic.
//       Upgrading to HTTP2 and MPFS2 is *strongly* recommended for all new designs.
//       MPFS2 images can be uploaded directly using the MPFS2.exe tool.
//////////////////////////////////////////////////////////////////////////////////////////
static BOOL DownloadMPFS(void)
{
    enum SM_MPFS
    {
        SM_MPFS_SOH,
        SM_MPFS_BLOCK,
        SM_MPFS_BLOCK_CMP,
        SM_MPFS_DATA,
    } state;

    BYTE c;
    MPFS handle;
    BOOL lbDone;
    BYTE blockLen;
    BYTE lResult;
    BYTE tempData[XMODEM_BLOCK_LEN];
    TICK lastTick;
    TICK currentTick;

    state = SM_MPFS_SOH;
    lbDone = FALSE;

    handle = MPFSFormat();

    // Notify the host that we are ready to receive...
    lastTick = TickGet();
    do
    {
        currentTick = TickGet();
        if ( TickGetDiff(currentTick, lastTick) >= (TICK_SECOND/2) )
        {
            lastTick = TickGet();
			while(BusyUART());
            WriteUART(XMODEM_NAK);

            /*
             * Blink LED to indicate that we are waiting for
             * host to send the file.
             */
            //2EI - comentei LED6_IO ^= 1;
        }

    } while(!DataRdyUART());


    while(!lbDone)
    {
        if(DataRdyUART())
        {
            // Toggle LED as we receive the data from host.
            //2EI - comentei LED6_IO ^= 1;
            c = ReadUART();
        }
        else
        {
            // Real application should put some timeout to make sure
            // that we do not wait forever.
            continue;
        }

        switch(state)
        {
        default:
            if ( c == XMODEM_SOH )
            {
                state = SM_MPFS_BLOCK;
            }
            else if ( c == XMODEM_EOT )
            {
                // Turn off LED when we are done.
                //2EI - comentei LED6_IO = 1;

                MPFSClose();
				while(BusyUART());
                WriteUART(XMODEM_ACK);
                lbDone = TRUE;
            }
            else
			{
				while(BusyUART());
				WriteUART(XMODEM_NAK);
			}

            break;

        case SM_MPFS_BLOCK:

            // We do not use block information.
            lResult = XMODEM_ACK;
            blockLen = 0;
            state = SM_MPFS_BLOCK_CMP;
            break;

        case SM_MPFS_BLOCK_CMP:

            // We do not use 1's comp. block value.
            state = SM_MPFS_DATA;
            break;

        case SM_MPFS_DATA:

            // Buffer block data until it is over.
            tempData[blockLen++] = c;
            if ( blockLen > XMODEM_BLOCK_LEN )
            {

                // We have one block data. Write it to EEPROM.
                MPFSPutBegin(handle);

                lResult = XMODEM_ACK;
                for ( c = 0; c < XMODEM_BLOCK_LEN; c++ )
                    MPFSPut(tempData[c]);

                handle = MPFSPutEnd();

				while(BusyUART());
                WriteUART(lResult);
                state = SM_MPFS_SOH;
            }
            break;

        }

    }

    return TRUE;
}
#endif	// #if defined(MPFS_USE_EEPROM) && defined(STACK_USE_MPFS)

// NOTE: Name[] must be at least 16 characters long.
// It should be exactly 16 characters, as defined by NetBIOS spec.
void FormatNetBIOSName(BYTE Name[])
{
	BYTE i;

	Name[15] = '\0';
	strupr((char*)Name);
	i = 0;
	while(i < 15u)
	{
		if(Name[i] == '\0')
		{
			while(i < 15u)
			{
				Name[i++] = ' ';
			}
			break;
		}
		i++;
	}
}



#if defined(STACK_USE_SNMP_SERVER)

#if !defined(SNMP_TRAP_DISABLED)


static DWORD SNMPGetTimeStamp(void)
{

	DWORD_VAL dwvHigh, dwvLow;
    DWORD dw;
    DWORD timeStamp;
	
	//TimeStamp
	// Get all 48 bits of the internal Tick timer
    do
   	{
	   	dwvHigh.Val = TickGetDiv64K();
	   	dwvLow.Val = TickGet();
	} while(dwvHigh.w[0] != dwvLow.w[1]);
    dwvHigh.Val = dwvHigh.w[1];
    
	// Find total contribution from lower DWORD
    dw = dwvLow.Val/(DWORD)TICK_SECOND;
    timeStamp = dw*100ul;
    dw = (dwvLow.Val - dw*(DWORD)TICK_SECOND)*100ul;		// Find fractional seconds and convert to 10ms ticks
    timeStamp += (dw+((DWORD)TICK_SECOND/2ul))/(DWORD)TICK_SECOND;

	// Itteratively add in the contribution from upper WORD
	while(dwvHigh.Val >= 0x1000ul)
	{
		timeStamp += (0x100000000000ull*100ull+(TICK_SECOND/2ull))/TICK_SECOND;
		dwvHigh.Val -= 0x1000;
	}	
	while(dwvHigh.Val >= 0x100ul)
	{
		timeStamp += (0x010000000000ull*100ull+(TICK_SECOND/2ull))/TICK_SECOND;
		dwvHigh.Val -= 0x100;
	}	
	while(dwvHigh.Val >= 0x10ul)
	{
		timeStamp += (0x001000000000ull*100ull+(TICK_SECOND/2ull))/TICK_SECOND;
		dwvHigh.Val -= 0x10;
	}	
	while(dwvHigh.Val)
	{
		timeStamp += (0x000100000000ull*100ull+(TICK_SECOND/2ull))/TICK_SECOND;
		dwvHigh.Val--;
	}
    
    return timeStamp;
}


/*
* Trap information.
* This table maintains list of intereseted receivers
* who should receive notifications when some interesting
* event occurs.
*/
#define TRAP_TABLE_SIZE         (2)
#define MAX_COMMUNITY_LEN       (8)
typedef struct _TRAP_INFO
{
   BYTE Size;
   struct
   {
       BYTE communityLen;
       char community[MAX_COMMUNITY_LEN];
       IP_ADDR IPAddress;
       struct
       {
           unsigned int bEnabled : 1;
       } Flags;
   } table[TRAP_TABLE_SIZE];
} TRAP_INFO;

/*
* Initialize trap table with no entries.
*/
TRAP_INFO trapInfo = { TRAP_TABLE_SIZE };

/**************************************************************************
  Function:
 	void SNMPSendTrap(void)
 
  Summary:	
  	 Prepare, validate remote node which will receive trap and send trap pdu.
 	 	  
  Description:		
     This function is used to send trap notification to previously 
     configured ip address if trap notification is enabled. There are
     different trap notification code. The current implementation
     sends trap for authentication failure (4).
  
  PreCondition:
 	 If application defined event occurs to send the trap.
 
  parameters:
     None.
 
  Returns:          
 	 None.
 
  Remarks:
     This is a callback function called by the application on certain 
     predefined events. This routine only implemented to send a 
     authentication failure Notification-type macro with PUSH_BUTTON
     oid stored in MPFS. If the ARP is no resolved i.e. if 
     SNMPIsNotifyReady() returns FALSE, this routine times 
     out in 5 seconds. This routine should be modified according to 
     event occured and should update corrsponding OID and notification
     type to the trap pdu.
 *************************************************************************/
void SNMPSendTrap(BYTE receiverIndex, SNMP_ID var, SNMP_VAL val, BYTE notificationCode)
{
	static BYTE timeLock=FALSE;
	//static BYTE receiverIndex=0; ///is application specific
	IP_ADDR remHostIPAddress,* remHostIpAddrPtr;
	//SNMP_VAL val;
	static DWORD TimerRead;

	static enum 
	{
		SM_PREPARE,
		SM_NOTIFY_WAIT 
	} smState = SM_PREPARE;



	if(trapInfo.table[receiverIndex].Flags.bEnabled)
	{
		remHostIPAddress.v[0] = trapInfo.table[receiverIndex].IPAddress.v[3];
		remHostIPAddress.v[1] = trapInfo.table[receiverIndex].IPAddress.v[2];
		remHostIPAddress.v[2] = trapInfo.table[receiverIndex].IPAddress.v[1];
		remHostIPAddress.v[3] = trapInfo.table[receiverIndex].IPAddress.v[0];
		remHostIpAddrPtr = &remHostIPAddress;
		if(timeLock==(BYTE)FALSE)
		{
			TimerRead=TickGet();
			timeLock=TRUE;
		}
	}	
	else
	{
		receiverIndex++;
		if((receiverIndex == (BYTE)TRAP_TABLE_SIZE))
		{
			receiverIndex=0;
			timeLock=FALSE;
//			gSendTrapFlag=FALSE;	
			UDPDiscard();
		}
		return;
		
	}
		
	switch(smState)
	{
	
		case SM_PREPARE:

			SNMPNotifyPrepare(remHostIpAddrPtr,trapInfo.table[receiverIndex].community,
						trapInfo.table[receiverIndex].communityLen,
						MICROCHIP,			  // Agent ID Var
						notificationCode, //gSpecificTrapNotification,					  // Notification code.
						SNMPGetTimeStamp());
			smState++;
			break;
			
		case SM_NOTIFY_WAIT:
			if(SNMPIsNotifyReady(remHostIpAddrPtr))
			{
				smState = SM_PREPARE;
		 		val.byte = 0;
				receiverIndex++;

				//application has to decide on which SNMP var OID to send. Ex. PUSH_BUTTON	
				SNMPNotify(var, val, 0);
            	smState = SM_PREPARE;
				UDPDiscard();
				break;
			}
	}	
		
	//Try for max 5 seconds to send TRAP, do not get block in while()
	if((TickGet()-TimerRead)>(5*TICK_SECOND)|| (receiverIndex == (BYTE)TRAP_TABLE_SIZE))
	{
		UDPDiscard();
		smState = SM_PREPARE;
		receiverIndex=0;
		timeLock=FALSE;
//		gSendTrapFlag=FALSE;
		return;
	}

}


/**************************************************************************
  Function:
 	void SNMPTrapDemo(SNMP_ID var, SNMP_VAL val, BYTE notificationCode)
 
  Summary:	
  	Send trap pdu demo application.
 	  	  
  Description:		
	This routine sends a trap pdu for the predefined ip addresses with the
	agent. The "event" to generate this trap pdu is "BUTTON_PUSH_EVENT". Whenever
	there occurs a specific button push, this routine is called and sends
	a trap containing PUSH_BUTTON mib var OID and notification type 
	as authentication failure. 
       
  PreCondition:
 	Application defined event occurs to send the trap.
 	
  parameters:
     None.
 
  Returns:          
 	 None.
 
  Remarks:
    This routine guides how to build a event generated trap notification.
    The application should make use of SNMPSendTrap() routine to generate 
    and send trap.
 *************************************************************************/
static void SNMPTrapDemo(SNMP_ID var, SNMP_VAL val, BYTE notificationCode)
{
    static BYTE i = 0;

    if ( trapInfo.table[i].Flags.bEnabled )
    {
    	val.byte = 0;
        SNMPSendTrap(i, var , val,notificationCode);
    }
}


#endif

BOOL SNMPValidate(SNMP_ACTION SNMPAction, char* community)
{
    return TRUE;
}



// Only dynamic variables with ASCII_STRING or OCTET_STRING data type
// needs to be handled.
BOOL SNMPIsValidSetLen(SNMP_ID var, BYTE len)
{
    switch(var)
    {
    case TRAP_COMMUNITY:
        if ( len < MAX_COMMUNITY_LEN+1 )
            return TRUE;
        break;

#if defined(USE_LCD)
    case LCD_DISPLAY:
        if ( len < sizeof(LCDText)+1 )
            return TRUE;
        break;
#endif
    }
    return FALSE;
}


// Only dynamic read-write variables needs to be handled.
BOOL SNMPSetVar(SNMP_ID var, SNMP_INDEX index, BYTE ref, SNMP_VAL val)
{
    switch(var)
    {
    case LED_D5:
        //LED1_IO = val.byte;
        return TRUE;

    case LED_D6:
        //LED2_IO = val.byte;
        return TRUE;

    case TRAP_RECEIVER_IP:
        // Make sure that index is within our range.
        if ( index < trapInfo.Size )
        {
            // This is just an update to an existing entry.
            trapInfo.table[index].IPAddress.Val = val.dword;
            return TRUE;
        }
        else if ( index < TRAP_TABLE_SIZE )
        {
            // This is an addition to table.
            trapInfo.table[index].IPAddress.Val = val.dword;
            trapInfo.table[index].communityLen = 0;
            trapInfo.Size++;
            return TRUE;
        }
        break;

    case TRAP_RECEIVER_ENABLED:
        // Make sure that index is within our range.
        if ( index < trapInfo.Size )
        {
            // Value of '1' means Enabled".
            if ( val.byte == 1 )
                trapInfo.table[index].Flags.bEnabled = 1;
            // Value of '0' means "Disabled.
            else if ( val.byte == 0 )
                trapInfo.table[index].Flags.bEnabled = 0;
            else
                // This is unknown value.
                return FALSE;
            return TRUE;
        }
        // Given index is more than our current table size.
        // If it is within our range, treat it as an addition to table.
        else if ( index < TRAP_TABLE_SIZE )
        {
            // Treat this as an addition to table.
            trapInfo.Size++;
            trapInfo.table[index].communityLen = 0;
        }

        break;

    case TRAP_COMMUNITY:
        // Since this is a ASCII_STRING data type, SNMP will call with
        // SNMP_END_OF_VAR to indicate no more bytes.
        // Use this information to determine if we just added new row
        // or updated an existing one.
        if ( ref ==  SNMP_END_OF_VAR )
        {
            // Index equal to table size means that we have new row.
            if ( index == trapInfo.Size )
                trapInfo.Size++;

            // Length of string is one more than index.
            trapInfo.table[index].communityLen++;

            return TRUE;
        }

        // Make sure that index is within our range.
        if ( index < trapInfo.Size )
        {
            // Copy given value into local buffer.
            trapInfo.table[index].community[ref] = val.byte;
            // Keep track of length too.
            // This may not be NULL terminate string.
            trapInfo.table[index].communityLen = (BYTE)ref;
            return TRUE;
        }
        break;

#if defined(USE_LCD)
    case LCD_DISPLAY:
        // Copy all bytes until all bytes are transferred
        if ( ref != SNMP_END_OF_VAR )
        {
            LCDText[ref] = val.byte;
            LCDText[ref+1] = 0;
        }
        else
        {
			LCDUpdate();
        }

        return TRUE;
#endif

	/******************************************************
	* Início do controle para alteração dos dados de Máximo e Mínimo 
	* dos sensores a partir do comando SNMPSet
	******************************************************/

	case LUMINOSIDADE_MAX:
		
		LUMINOSIDADE_MAX_CONTROL = val.byte;
		return TRUE;

	case LUMINOSIDADE_MIN:

		LUMINOSIDADE_MIN_CONTROL = val.byte;
		return TRUE;

	case UMIDADE_MAX:

		UMIDADE_MAX_CONTROL = val.byte;
		return TRUE;

	case UMIDADE_MIN:

		UMIDADE_MIN_CONTROL = val.byte;
		return TRUE;

	case TEMPERATURA_MAX:

		TEMPERATURA_MAX_CONTROL = val.byte;
		return TRUE;

	case TEMPERATURA_MIN:

		TEMPERATURA_MIN_CONTROL = val.byte;
		return TRUE;

	case PRESSAO_MAX:

		PRESSAO_MAX_CONTROL = val.byte;
		return TRUE;

	case PRESSAO_MIN:

		PRESSAO_MIN_CONTROL = val.byte;
		return TRUE;

	/******************************************************
	* Fim do controle para alteração dos dados de Máximo e Mínimo 
	* dos sensores a partir do comando SNMPSet
	******************************************************/

    }

    return FALSE;
}

// Only sequence index needs to be handled in this function.
BOOL SNMPGetNextIndex(SNMP_ID var, SNMP_INDEX *index)
{
    SNMP_INDEX tempIndex;

    tempIndex = *index;

    switch(var)
    {
    case TRAP_RECEIVER_ID:
        // There is no next possible index if table itself is empty.
        if ( trapInfo.Size == 0 )
            return FALSE;

        // INDEX_INVALID means start with first index.
        if ( tempIndex == SNMP_INDEX_INVALID )
        {
            *index = 0;
            return TRUE;
        }
        else if ( tempIndex < (trapInfo.Size-1) )
        {
            *index = tempIndex+1;
            return TRUE;
        }
        break;
    }
    return FALSE;
}


BOOL SNMPGetVar(SNMP_ID var, SNMP_INDEX index, BYTE *ref, SNMP_VAL* val)
{

    BYTE myRef;

    myRef = *ref;

    switch(var)
    {

    case SYS_UP_TIME:
        val->dword = TickGet();
        return TRUE;

    case LED_D5:
        //val->byte = LED1_IO;
        return TRUE;

    case LED_D6:
        //val->byte = LED2_IO;
        return TRUE;

    case PUSH_BUTTON:
        // There is only one button - meaning only index of 0 is allowed.
        val->byte = BUTTON0_IO;
        return TRUE;

    case ANALOG_POT0:
        val->word = atoi((char*)AN0String);
        return TRUE;

    case TRAP_RECEIVER_ID:
        if ( index < trapInfo.Size )
        {
            val->byte = index;
            return TRUE;
        }
        break;

    case TRAP_RECEIVER_ENABLED:
        if ( index < trapInfo.Size )
        {
            val->byte = trapInfo.table[index].Flags.bEnabled;
            return TRUE;
        }
        break;

    case TRAP_RECEIVER_IP:
        if ( index < trapInfo.Size )
        {
            val->dword = trapInfo.table[index].IPAddress.Val;
            return TRUE;
        }
        break;

    case TRAP_COMMUNITY:
        if ( index < trapInfo.Size )
        {
            if ( trapInfo.table[index].communityLen == 0 )
                *ref = SNMP_END_OF_VAR;
            else
            {
                val->byte = trapInfo.table[index].community[myRef];

                myRef++;

                if ( myRef == trapInfo.table[index].communityLen )
                    *ref = SNMP_END_OF_VAR;
                else
                    *ref = myRef;
            }
            return TRUE;
        }
        break;

#if defined(USE_LCD)
    case LCD_DISPLAY:
        if ( LCDText[0] == 0 )
            myRef = SNMP_END_OF_VAR;
        else
        {
            val->byte = LCDText[myRef++];
            if ( LCDText[myRef] == 0 )
                myRef = SNMP_END_OF_VAR;
        }

        *ref = myRef;
        return TRUE;
#endif

	/******************************************************************************************************
	* INICIO DO CONTROLE DOS SENSORES
	* RETORNA OS VALORES ARMAZENADOS NA MIB PARA OS SENSORES SOLICITADOS 
	/******************************************************************************************************/		

	case SENSOR_01_P:
		val->byte = SENSOR_01_P_CONTROL;
		return TRUE;

	case SENSOR_01_T:
		val->byte = SENSOR_01_T_CONTROL;		
		return TRUE;

	case SENSOR_01_L:
		val->byte = SENSOR_01_L_CONTROL;
		return TRUE;

	case SENSOR_01_U:
		val->byte = SENSOR_01_U_CONTROL;
		return TRUE;

	case SENSOR_02_P:
		val->byte = SENSOR_02_P_CONTROL;
		return TRUE;

	case SENSOR_02_T:
		val->byte = SENSOR_02_T_CONTROL;
		return TRUE;

	case SENSOR_02_L:
		val->byte = SENSOR_02_L_CONTROL;
		return TRUE;

	case SENSOR_02_U:
		val->byte = SENSOR_02_U_CONTROL;
		return TRUE;

	case SENSOR_03_P:
		val->byte = SENSOR_03_P_CONTROL;
		return TRUE;

	case SENSOR_03_T:
		val->byte = SENSOR_03_T_CONTROL;
		return TRUE;

	case SENSOR_03_L:
		val->byte = SENSOR_03_L_CONTROL;
		return TRUE;

	case SENSOR_03_U:
		val->byte = SENSOR_03_U_CONTROL;
		return TRUE;

	case SENSOR_04_P:
		val->byte = SENSOR_04_P_CONTROL;
		return TRUE;

	case SENSOR_04_T:
		val->byte = SENSOR_04_T_CONTROL;
		return TRUE;

	case SENSOR_04_L:
		val->byte = SENSOR_04_L_CONTROL;
		return TRUE;

	case SENSOR_04_U:
		val->byte = SENSOR_04_U_CONTROL;
		return TRUE;		

	case SENSOR_05_P:
		val->byte = SENSOR_05_P_CONTROL;
		return TRUE;

	case SENSOR_05_T:
		val->byte = SENSOR_05_T_CONTROL;
		return TRUE;

	case SENSOR_05_L:
		val->byte = SENSOR_05_L_CONTROL;
		return TRUE;

	case SENSOR_05_U:
		val->byte = SENSOR_05_U_CONTROL;
		return TRUE;		

	case SENSOR_06_P:
		val->byte = SENSOR_06_P_CONTROL;
		return TRUE;

	case SENSOR_06_T:
		val->byte = SENSOR_06_T_CONTROL;
		return TRUE;

	case SENSOR_06_L:
		val->byte = SENSOR_06_L_CONTROL;
		return TRUE;

	case SENSOR_06_U:
		val->byte = SENSOR_06_U_CONTROL;
		return TRUE;		

	case SENSOR_07_P:
		val->byte = SENSOR_07_P_CONTROL;
		return TRUE;

	case SENSOR_07_T:
		val->byte = SENSOR_07_T_CONTROL;
		return TRUE;

	case SENSOR_07_L:
		val->byte = SENSOR_07_L_CONTROL;
		return TRUE;

	case SENSOR_07_U:
		val->byte = SENSOR_07_U_CONTROL;
		return TRUE;		

	case SENSOR_08_P:
		val->byte = SENSOR_08_P_CONTROL;
		return TRUE;

	case SENSOR_08_T:
		val->byte = SENSOR_08_T_CONTROL;
		return TRUE;

	case SENSOR_08_L:
		val->byte = SENSOR_08_L_CONTROL;
		return TRUE;

	case SENSOR_08_U:
		val->byte = SENSOR_08_U_CONTROL;
		return TRUE;		

	case SENSOR_09_P:
		val->byte = SENSOR_09_P_CONTROL;
		return TRUE;

	case SENSOR_09_T:
		val->byte = SENSOR_09_T_CONTROL;
		return TRUE;

	case SENSOR_09_L:
		val->byte = SENSOR_09_L_CONTROL;
		return TRUE;

	case SENSOR_09_U:
		val->byte = SENSOR_09_U_CONTROL;
		return TRUE;		

	case SENSOR_10_P:
		val->byte = SENSOR_10_P_CONTROL;
		return TRUE;

	case SENSOR_10_T:
		val->byte = SENSOR_10_T_CONTROL;
		return TRUE;

	case SENSOR_10_L:
		val->byte = SENSOR_10_L_CONTROL;
		return TRUE;

	case SENSOR_10_U:
		val->byte = SENSOR_10_U_CONTROL;
		return TRUE;


	/*  
	*	VERIFICA SE É ALTERAÇÃO DOS LIMITES MÁXIMOS E 
	*	MINIMOS DOS SENSORES.
	*/

	case LUMINOSIDADE_MAX:
		val->byte = LUMINOSIDADE_MAX_CONTROL;
		return TRUE;

	case LUMINOSIDADE_MIN:
		val->byte = LUMINOSIDADE_MIN_CONTROL;
		return TRUE;

	case UMIDADE_MAX:
		val->byte = UMIDADE_MAX_CONTROL;
		return TRUE;

	case UMIDADE_MIN:
		val->byte = UMIDADE_MIN_CONTROL;
		return TRUE;

	case TEMPERATURA_MAX:
		val->byte = TEMPERATURA_MAX_CONTROL;
		return TRUE;

	case TEMPERATURA_MIN:
		val->byte = TEMPERATURA_MIN_CONTROL;
		return TRUE;

	case PRESSAO_MAX:
		val->byte = PRESSAO_MAX_CONTROL;
		return TRUE;

	case PRESSAO_MIN:
		val->byte = PRESSAO_MIN_CONTROL;
		return TRUE;

	/******************************************************************************************************
	* FIM DO CONTROLE DOS SENSORES
	* RETORNA OS VALORES ARMAZENADOS NA MIB PARA OS SENSORES SOLICITADOS 
	/******************************************************************************************************/
    }

    return FALSE;

}
#endif	//#if defined(STACK_USE_SNMP_SERVER)

/*****************************************************************************************************
 * Função:       	 		void SerialISR(void)
 *
 * Pré-Condição:    	Verificar habilitação de interrupções
 *
 * Entrada:         		Nenhuma
 *
 * Saída:           		Nenhuma
 *
 * Outros Efeitos:  	Nenhum
 *
 * Função:          		Carrega em um buffer o que a USART recebeu.
 *
 * Notas:           		Vetor de interrupção de baixa prioridade, no Hyperterminal deve-se teclar ENTER para finalizar o buffer
*****************************************************************************************************/
#pragma interruptlow HighISR
void SerialISR(void){


	unsigned char a;
	unsigned char *u;
	unsigned char *w;


//	TEMP_NODE1 = 2;

   	USART_Time_Out = TickGet();												// Time-out serial re-inicializado
	// RCIF: EUSART Receive Interrupt Flag bit
	// 1: The EUSART receive buffer, RCREG is full (cleared when RCREG is read)
	// 0: The EUSART receive buffer is empty
	if (PIR1bits.RCIF==1) {  																// checa flag de interrupção recepção serial
		a=RCREG1;
	//	WriteUART(a);
		*p = a;																					// Armazenar no buffer serial temporário
		p++;																						// prepara próxima posiçaõ de armazenamento do dado
		var_con_byt++;																	// contador de bytes deve contar até 21 bytes
		// se rec[0]  for diferente de FF volta ao começo o ponteiro para buffer
		if ( rec[0] != 0x7E ) {		
				p=&rec[0];																	// Volta para posiçâo inicial do buffer
				rec[0]=0x7E;																	// Prepara para receber nova informação da serial
				var_con_byt = 0;
		}
		if (var_con_byt == 14) {
				//putrsUART("Conta 14");					  										// Recebeu os 21 bytes 
				//*--p = '\0';																	// Substitui CR da mensagem por NULL
				//*p = '\0';																		// Fim da mensagem com  NULL
				u = &USARTString_rec[0]; 										// Inicio da informação para navegador
				w = &rec[0];																	// In/icio da informação que recebeu da serial
				for (a=0;a<14;a++){													// Transferencia do buffer da serial para buffer da página Web
 	 			 	*u++ = *w++ ;															// copia o que recebeu da serial para ser mostrado no navegador
				}
				//*u = '\0';																		// terminar string com caracter nulo
				p=&rec[0];																	// Volta para posiçâo inicial do buffer
				rec[0]=0x7E;																	// Prepara para receber nova informação da serial
				var_con_byt = 0;															// contador de bytes da serial volta a zero
				var_muda=0x01;															// mudança de dados da Página Web
		}
	}
		//WriteUART(var_con_byt);
}// retorno através de instrução RETFIE


/*****************************************************************************************************
 * Função:       	 		unsigned char Check_SUM(void)
 *
 * Pré-Condição:    	
 *
 * Entrada:         		
 *
 * Saída:           		1 - check Sum 0K, 0 - Erro no Nenhuma
 *
 * Outros Efeitos:  	Nenhum
 *
 * Função:          		Calcula check-sum do vetor USARTString_rec.
 *
 * Notas:           		
*****************************************************************************************************/

unsigned char Check_SUM(void){
/*
unsigned char i;
unsigned char a;
unsigned char b;
int soma;
int check;

a=0;
soma = 0;

// valor do check sum (tirei o primeiro byte do MSB só para diminuir sobre-carga no microcontrolador)
b = USARTString_rec[20];
check = 256*b;

b = USARTString_rec[19];
check = check+b;

// vou varrer do byte 02 até o byte 19, ou seja, 18 bytes
for (i=1; i < 19 ; i++){
	b = USARTString_rec[i];
	soma = soma + b;
}

if (check==soma) 
	return 1;

return 0;
*/
	return 1;
}

