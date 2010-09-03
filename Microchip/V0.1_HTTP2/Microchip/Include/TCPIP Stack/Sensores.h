/* 
* Author              	Date        Comment
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Gregory Elias Miguel	02/09/2010	Versão Inicial
*/

/*
* Variáveis de controle para manipulação 
* dos sensores
*/
#pragma udata Sensor_1 

short long SENSOR_01_P_CONTROL; //Variável de controle de pressão do sensor 01
short long SENSOR_01_T_CONTROL; //Variável de controle de temperatura do sensor 01
short long SENSOR_01_L_CONTROL; //Variável de controle de luminosidade do sensor 01
short long SENSOR_01_U_CONTROL; //Variável de controle de umidade do sensor 01
short long SENSOR_02_P_CONTROL; //Variável de controle de pressão do sensor 02
short long SENSOR_02_T_CONTROL; //Variável de controle de temperatura do sensor 02
short long SENSOR_02_L_CONTROL; //Variável de controle de luminosidade do sensor 02
short long SENSOR_02_U_CONTROL; //Variável de controle de umidade do sensor 02
short long SENSOR_03_P_CONTROL; //Variável de controle de pressão do sensor 03
short long SENSOR_03_T_CONTROL; //Variável de controle de temperatura do sensor 03
short long SENSOR_03_L_CONTROL; //Variável de controle de luminosidade do sensor 03
short long SENSOR_03_U_CONTROL; //Variável de controle de umidade do sensor 03
short long SENSOR_04_P_CONTROL; //Variável de controle de pressão do sensor 04
short long SENSOR_04_T_CONTROL; //Variável de controle de temperatura do sensor 04
short long SENSOR_04_L_CONTROL; //Variável de controle de luminosidade do sensor 04
short long SENSOR_04_U_CONTROL; //Variável de controle de umidade do sensor 04
short long SENSOR_05_P_CONTROL; //Variável de controle de pressão do sensor 05
short long SENSOR_05_T_CONTROL; //Variável de controle de temperatura do sensor 05
short long SENSOR_05_L_CONTROL; //Variável de controle de luminosidade do sensor 05
short long SENSOR_05_U_CONTROL; //Variável de controle de umidade do sensor 05
short long SENSOR_06_P_CONTROL; //Variável de controle de pressão do sensor 06
short long SENSOR_06_T_CONTROL; //Variável de controle de temperatura do sensor 06
short long SENSOR_06_L_CONTROL; //Variável de controle de luminosidade do sensor 06
short long SENSOR_06_U_CONTROL; //Variável de controle de umidade do sensor 06 
short long SENSOR_07_P_CONTROL; //Variável de controle de pressão do sensor 07
short long SENSOR_07_T_CONTROL; //Variável de controle de temperatura do sensor 07
short long SENSOR_07_L_CONTROL; //Variável de controle de luminosidade do sensor 07
short long SENSOR_07_U_CONTROL; //Variável de controle de umidade do sensor 07
short long SENSOR_08_P_CONTROL; //Variável de controle de pressão do sensor 08
short long SENSOR_08_T_CONTROL; //Variável de controle de temperatura do sensor 08
short long SENSOR_08_L_CONTROL; //Variável de controle de luminosidade do sensor 08
short long SENSOR_08_U_CONTROL; //Variável de controle de umidade do sensor 08
short long SENSOR_09_P_CONTROL; //Variável de controle de pressão do sensor 09
short long SENSOR_09_T_CONTROL; //Variável de controle de temperatura do sensor 09
short long SENSOR_09_L_CONTROL; //Variável de controle de luminosidade do sensor 09
short long SENSOR_09_U_CONTROL; //Variável de controle de umidade do sensor 09
short long SENSOR_10_P_CONTROL; //Variável de controle de pressão do sensor 10
short long SENSOR_10_T_CONTROL; //Variável de controle de temperatura do sensor 10
short long SENSOR_10_L_CONTROL; //Variável de controle de luminosidade do sensor 10
short long SENSOR_10_U_CONTROL; //Variável de controle de umidade do sensor 10		


/*
* Variáveis de controle para manipulação 
* dos sensores como String
*/

#pragma udata Sensor_2 
BYTE SENSOR_01_P_STR[8]; //Variável de controle de pressão do sensor 01 como String
BYTE SENSOR_01_T_STR[8]; //Variável de controle de temperatura do sensor 01 como String
BYTE SENSOR_01_L_STR[8]; //Variável de controle de luminosidade do sensor 01 como String
BYTE SENSOR_01_U_STR[8]; //Variável de controle de umidade do sensor 01 como String
BYTE SENSOR_02_P_STR[8]; //Variável de controle de pressão do sensor 02 como String
BYTE SENSOR_02_T_STR[8]; //Variável de controle de temperatura do sensor 02 como String
BYTE SENSOR_02_L_STR[8]; //Variável de controle de luminosidade do sensor 02 como String
BYTE SENSOR_02_U_STR[8]; //Variável de controle de umidade do sensor 02 como String
BYTE SENSOR_03_P_STR[8]; //Variável de controle de pressão do sensor 03 como String
BYTE SENSOR_03_T_STR[8]; //Variável de controle de temperatura do sensor 03 como String
BYTE SENSOR_03_L_STR[8]; //Variável de controle de luminosidade do sensor 03 como String
BYTE SENSOR_03_U_STR[8]; //Variável de controle de umidade do sensor 03 como String

#pragma udata Sensor_3 
BYTE SENSOR_04_P_STR[8]; //Variável de controle de pressão do sensor 04 como String
BYTE SENSOR_04_T_STR[8]; //Variável de controle de temperatura do sensor 04 como String
BYTE SENSOR_04_L_STR[8]; //Variável de controle de luminosidade do sensor 04 como String
BYTE SENSOR_04_U_STR[8]; //Variável de controle de umidade do sensor 04 como String
BYTE SENSOR_05_P_STR[8]; //Variável de controle de pressão do sensor 05 como String
BYTE SENSOR_05_T_STR[8]; //Variável de controle de temperatura do sensor 05 como String
BYTE SENSOR_05_L_STR[8]; //Variável de controle de luminosidade do sensor 05 como String
BYTE SENSOR_05_U_STR[8]; //Variável de controle de umidade do sensor 05 como String
BYTE SENSOR_06_P_STR[8]; //Variável de controle de pressão do sensor 06 como String
BYTE SENSOR_06_T_STR[8]; //Variável de controle de temperatura do sensor 06 como String
BYTE SENSOR_06_L_STR[8]; //Variável de controle de luminosidade do sensor 06 como String
BYTE SENSOR_06_U_STR[8]; //Variável de controle de umidade do sensor 06 como String

#pragma udata Sensor_4
BYTE SENSOR_07_P_STR[8]; //Variável de controle de pressão do sensor 07 como String
BYTE SENSOR_07_T_STR[8]; //Variável de controle de temperatura do sensor 07 como String
BYTE SENSOR_07_L_STR[8]; //Variável de controle de luminosidade do sensor 07 como String
BYTE SENSOR_07_U_STR[8]; //Variável de controle de umidade do sensor 07 como String
BYTE SENSOR_08_P_STR[8]; //Variável de controle de pressão do sensor 08 como String
BYTE SENSOR_08_T_STR[8]; //Variável de controle de temperatura do sensor 08 como String
BYTE SENSOR_08_L_STR[8]; //Variável de controle de luminosidade do sensor 08 como String
BYTE SENSOR_08_U_STR[8]; //Variável de controle de umidade do sensor 08 como String
BYTE SENSOR_09_P_STR[8]; //Variável de controle de pressão do sensor 09 como String
BYTE SENSOR_09_T_STR[8]; //Variável de controle de temperatura do sensor 09 como String
BYTE SENSOR_09_L_STR[8]; //Variável de controle de luminosidade do sensor 09 como String
BYTE SENSOR_09_U_STR[8]; //Variável de controle de umidade do sensor 09 como String
BYTE SENSOR_10_P_STR[8]; //Variável de controle de pressão do sensor 10 como String
BYTE SENSOR_10_T_STR[8]; //Variável de controle de temperatura do sensor 10 como String
BYTE SENSOR_10_L_STR[8]; //Variável de controle de luminosidade do sensor 10 como String
BYTE SENSOR_10_U_STR[8]; //Variável de controle de umidade do sensor 10	como String	 

/*
* Variáveis para controle de valores de máximo e mínimo para os sensores gerenciados.
* Através destes valores, o agente dispara TRAPs para o gerente.
*/

short long PRESSAO_MAX_CONTROL; 		//Variável que controla o valor máximo que pode ser atingido por um sensor de pressão sem disparar um TRAP
short long PRESSAO_MIN_CONTROL; 		//Variável que controla o valor mínimo que pode ser atingido por um sensor de pressão sem disparar um TRAP
short long TEMPERATURA_MAX_CONTROL; 	//Variável que controla o valor máximo que pode ser atingido por um sensor de temperatura sem disparar um TRAP
short long TEMPERATURA_MIN_CONTROL;	//Variável que controla o valor mínimo que pode ser atingido por um sensor de temperatura sem disparar um TRAP
short long LUMINOSIDADE_MAX_CONTROL;	//Variável que controla o valor máximo que pode ser atingido por um sensor de luminosidade sem disparar um TRAP
short long LUMINOSIDADE_MIN_CONTROL;  //Variável que controla o valor mínimo que pode ser atingido por um sensor de luminosidade sem disparar um TRAP
short long UMIDADE_MAX_CONTROL;		//Variável que controla o valor máximo que pode ser atingido por um sensor de umidade sem disparar um TRAP
short long UMIDADE_MIN_CONTROL;		//Variável que controla o valor mínimo que pode ser atingido por um sensor de umidade sem disparar um TRAP


/*
* Enum que relaciona todas as grandezas utilizadas pelos sensores
* Utilizado para validar o tipo de informação recebida pela porta Serial
*/

typedef enum _GRANDEZA
{
    PRESSAO = 0,
    TEMPERATURA,
    LUMINOSIDADE,
    UMIDADE
} GRANDEZA;
