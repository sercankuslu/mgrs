#define __STRING2EI_C

#include "TCPIP Stack/TCPIP.h"


/********************************************************************
 * Fun��o:        void str2ram(static char *dest, static char rom *src)

 *
 * PreCondition:    
 *
 * Input:           
 *
 * Output:          
 *
 * Side Effects:    
 *
 * Overview:      Esta fun��o ler uma mensagem da mem�ria FLASH interna e escreve
 *                em uma string na  RAM
 *
 * Note:            
 *
 ********************************************************************/

void str2ram(static char *dest, static char rom *src){
	while ((*dest++ = *src++) != '\0');
}

/*****************************************************************************************************
 * Fun��o:       	 		void STRtoINT(unsigned char e, char *g)
 *
 * Pr�-Condi��o:    	
 *
 * Entrada:         		e constiutue um n�mero hexadecimal a ser transformado para uma string no formato inteiro
*							*g ponterio para a sa�da dos dados
 *
 * Sa�da:           		Nenhuma
 *
 * Outros Efeitos:  		Nenhum
 *
 * Fun��o:          		Transfoma um n�mero hexadecimalem uma stringo no formato inteiro.
 *
 * Notas:           		
*****************************************************************************************************/

void STRtoINT(unsigned char e, char* g){
	
	unsigned char a;
	unsigned char b;
	unsigned char c;

	// Em rela��o a centena
	a = (unsigned char)(e/100);													
	// Em rela��o a dezena
	b = (unsigned char)((e - a*100)/10);									
  	// Em rela��o a unidade
	c = (unsigned char) (e - a*100 - b*10);								

	if (a==0){
		*g=' ';
	}
	else{
		*g = 0x30 + a;																		
	}
	*++g = 0x30 + b;																		
	*++g = 0x30 + c;																		
}


/*****************************************************************************************************
 * Fun��o:       	 		void STRtoINT2(unsigned char e,char *g)
 *
 * Pr�-Condi��o:    	
 *
 * Entrada:         		e constiutue um n�mero hexadecimal a ser transformado para uma string inteira dividida por 2
*							*g ponteiro para a sa�da dos dados
 *
 * Sa�da:           		Nenhuma
 *
 * Outros Efeitos:  		Nenhum
 *
 * Fun��o:          		Transforma um n�mero hexadecimal(ab) para uma string no formato inteiro (cde) dividido por 2.
 *
 * Notas:           		
*****************************************************************************************************/

void STRtoINT2(unsigned char e, char* g){
	
	unsigned char a;
	unsigned char b;
	unsigned char c;
	unsigned char d;

	d = (unsigned char )(e / 2);
	// Em rela��o a centena
	a = (unsigned char)(d/100);													
	// Em rela��o a dezena
	b = (unsigned char)((d - a*100)/10);									
  	// Em rela��o a unidade
	c = (unsigned char) (d - a*100 - b*10);								

	if (a==0){
		*g=' ';
	}
	else{
		*g = 0x30+a;																		
	}
	*++g = 0x30+b;																		
	*++g = 0x30 + c;																		
}


/*****************************************************************************************************
 * Fun��o:       	 		void STRtoINT4(unsigned char e,unsigned char f, char *i)
 *
 * Pr�-Condi��o:    	
 *
 * Entrada:         		e(LSB) e f(MSB) constiutuem um n�mero hexadecimal a ser transformado para uma string inteira dividido por 2
*							*i ponteiro para a sa�da
 *
 * Sa�da:           		Nenhuma
 *
 * Outros Efeitos:  		Nenhum
 *
 * Fun��o:          		Transforma um n�mero hexadecimal(ef) em uma string inteira dividida por 2.
 *
 * Notas:           		
*****************************************************************************************************/

void STRtoINT4(unsigned char e,unsigned char f,  char* i){
	
	unsigned char a;
	unsigned char b;
	unsigned char c;
	unsigned char d;
	unsigned int  j;

	// juntando os dois n�meros hexa em um �nico n�mro
	j = 256*(int)f +(int)e ;																
	// Em rela��o a milhar
	a = (unsigned char)(j/1000);												
	// Em rela��o a centena
	b = (unsigned char)((j-a*1000)/100);									
	// Em rela��o a dezena
	c = (unsigned char)((j - a*1000 - b*100)/10);						
  	// Em rela��o a unidade
	d = (unsigned char) (j - a*1000 - b*100 - c*10);					

	if (b==0){
		*i=' ';
	}
	else{
		*i = 0x30+b;																			
	}
	*++i = 0x30 + c;																		
	*++i = 0x30 + d;																		
}



/*****************************************************************************************************
 * Fun��o:       	 		void STRtoINT2X(unsigned char e, char *g)
 *
 * Pr�-Condi��o:    	
 *
 * Entrada:         		e  constiutue um n�mero hexadecimal a ser transformado para uma string decimal multiplicado por 2
*							g � o ponteiro para a sa�da
 *
 * Sa�da:           		Nenhuma
 *
 * Outros Efeitos:  		Nenhum
 *
 * Fun��o:          		Transfoma um n�mero hexadecimal em uma string no formato decimal multiplicada por 2
 *
 * Notas:           		
*****************************************************************************************************/

void STRtoINT2X(unsigned char e, char* g){
	
	unsigned char a;
	unsigned char b;
	unsigned char c;
	int  j;

	j = 2*e;																						
	
	// em rela��o a centena
	a = (unsigned char)(j/100);
	// em rela��o a dezena
	b = (unsigned char)((j - a*100)/10);										
	// em rela��o a unidade
  	c = (unsigned char) (j - a*100 - b*10);									

	if (a==0){
		*g=' ';
	}
	else{
		// em rela��o a centena
		*g = 0x30+a;																			
	}
	// em rela��o a dezena
	*++g = 0x30+b;		
	// em rela��o a unidade																
	*++g = 0x30 + c;																		
}



/***********************************************************************
* Function:       	 	unsigned char STRING_to_int(unsigned char* str)
 *
 * Pr�-Condi��o:    	Nenhuma
 *
 * Entrada:         	String contendo o valor hexa a ser transformado para interiro
 *
 * Sa�da:           	Valor em inteiro
 *
 * Outros Efeitos:  	Nenhum
 *
 * Fun��o:          	Envia comando para celular.
 *
 * Notas:           	Nenhuma
 ********************************************************************/

unsigned char STRING_to_int (unsigned char *str){

	unsigned char val1;
	unsigned char val2;

	val1 = *str;

   	if ( (val1 >= 0x30) && (val1 <= 0x39) ) val1=val1-0x30;			// "0" a "9"
   	if ( (val1 >= 0x41) && (val1 <= 0x46) ) val1=val1-0x37;			// "A" a "F"
   	if ( (val1 >= 0x61) && (val1 <= 0x66) ) val1=val1-0x57;			// "a" a "f"
 
   	str = str + 1;
	val2 = *str;

   	if ( (val2 >= 0x30) && (val2 <= 0x39) ) val2=val2-0x30;
   	if ( (val2 >= 0x41) && (val2 <= 0x46) ) val2=val2-0x37;
   	if ( (val1 >= 0x61) && (val1 <= 0x66) ) val1=val1-0x57;			
   	return (0x10*val1 + val2);
}
