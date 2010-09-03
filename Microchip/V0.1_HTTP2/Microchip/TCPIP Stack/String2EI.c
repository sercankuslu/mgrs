#define __STRING2EI_C

#include "TCPIP Stack/TCPIP.h"


/********************************************************************
 * Função:        void str2ram(static char *dest, static char rom *src)

 *
 * PreCondition:    
 *
 * Input:           
 *
 * Output:          
 *
 * Side Effects:    
 *
 * Overview:      Esta função ler uma mensagem da memória FLASH interna e escreve
 *                em uma string na  RAM
 *
 * Note:            
 *
 ********************************************************************/

void str2ram(static char *dest, static char rom *src){
	while ((*dest++ = *src++) != '\0');
}

/*****************************************************************************************************
 * Função:       	 		void STRtoINT(unsigned char e, char *g)
 *
 * Pré-Condição:    	
 *
 * Entrada:         		e constiutue um número hexadecimal a ser transformado para uma string no formato inteiro
*							*g ponterio para a saída dos dados
 *
 * Saída:           		Nenhuma
 *
 * Outros Efeitos:  		Nenhum
 *
 * Função:          		Transfoma um número hexadecimalem uma stringo no formato inteiro.
 *
 * Notas:           		
*****************************************************************************************************/

void STRtoINT(unsigned char e, char* g){
	
	unsigned char a;
	unsigned char b;
	unsigned char c;

	// Em relação a centena
	a = (unsigned char)(e/100);													
	// Em relação a dezena
	b = (unsigned char)((e - a*100)/10);									
  	// Em relação a unidade
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
 * Função:       	 		void STRtoINT2(unsigned char e,char *g)
 *
 * Pré-Condição:    	
 *
 * Entrada:         		e constiutue um número hexadecimal a ser transformado para uma string inteira dividida por 2
*							*g ponteiro para a saída dos dados
 *
 * Saída:           		Nenhuma
 *
 * Outros Efeitos:  		Nenhum
 *
 * Função:          		Transforma um número hexadecimal(ab) para uma string no formato inteiro (cde) dividido por 2.
 *
 * Notas:           		
*****************************************************************************************************/

void STRtoINT2(unsigned char e, char* g){
	
	unsigned char a;
	unsigned char b;
	unsigned char c;
	unsigned char d;

	d = (unsigned char )(e / 2);
	// Em relação a centena
	a = (unsigned char)(d/100);													
	// Em relação a dezena
	b = (unsigned char)((d - a*100)/10);									
  	// Em relação a unidade
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
 * Função:       	 		void STRtoINT4(unsigned char e,unsigned char f, char *i)
 *
 * Pré-Condição:    	
 *
 * Entrada:         		e(LSB) e f(MSB) constiutuem um número hexadecimal a ser transformado para uma string inteira dividido por 2
*							*i ponteiro para a saída
 *
 * Saída:           		Nenhuma
 *
 * Outros Efeitos:  		Nenhum
 *
 * Função:          		Transforma um número hexadecimal(ef) em uma string inteira dividida por 2.
 *
 * Notas:           		
*****************************************************************************************************/

void STRtoINT4(unsigned char e,unsigned char f,  char* i){
	
	unsigned char a;
	unsigned char b;
	unsigned char c;
	unsigned char d;
	unsigned int  j;

	// juntando os dois números hexa em um único númro
	j = 256*(int)f +(int)e ;																
	// Em relação a milhar
	a = (unsigned char)(j/1000);												
	// Em relação a centena
	b = (unsigned char)((j-a*1000)/100);									
	// Em relação a dezena
	c = (unsigned char)((j - a*1000 - b*100)/10);						
  	// Em relação a unidade
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
 * Função:       	 		void STRtoINT2X(unsigned char e, char *g)
 *
 * Pré-Condição:    	
 *
 * Entrada:         		e  constiutue um número hexadecimal a ser transformado para uma string decimal multiplicado por 2
*							g é o ponteiro para a saída
 *
 * Saída:           		Nenhuma
 *
 * Outros Efeitos:  		Nenhum
 *
 * Função:          		Transfoma um número hexadecimal em uma string no formato decimal multiplicada por 2
 *
 * Notas:           		
*****************************************************************************************************/

void STRtoINT2X(unsigned char e, char* g){
	
	unsigned char a;
	unsigned char b;
	unsigned char c;
	int  j;

	j = 2*e;																						
	
	// em relação a centena
	a = (unsigned char)(j/100);
	// em relação a dezena
	b = (unsigned char)((j - a*100)/10);										
	// em relação a unidade
  	c = (unsigned char) (j - a*100 - b*10);									

	if (a==0){
		*g=' ';
	}
	else{
		// em relação a centena
		*g = 0x30+a;																			
	}
	// em relação a dezena
	*++g = 0x30+b;		
	// em relação a unidade																
	*++g = 0x30 + c;																		
}



/***********************************************************************
* Function:       	 	unsigned char STRING_to_int(unsigned char* str)
 *
 * Pré-Condição:    	Nenhuma
 *
 * Entrada:         	String contendo o valor hexa a ser transformado para interiro
 *
 * Saída:           	Valor em inteiro
 *
 * Outros Efeitos:  	Nenhum
 *
 * Função:          	Envia comando para celular.
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
