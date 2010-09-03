/*
 * This file was automatically generated on Wed Dec 09 12:11:29 2009
 * by mib2bib utility.

 * This file contains 'C' defines for dynamic OIDs and AgentID only.
 * Do not modify this file manually.
 * Include this file in your application source file that handles SNMP callbacks and TRAP.
 */
#define MICROCHIP (255)		// This is an Agent ID for use in SNMPNotify() only.
#define SYS_UP_TIME (250)			// 43.6.1.2.1.1.3: READONLY TIME_TICKS.
#define TRAP_RECEIVER_ID (25)			// 43.6.1.4.1.17095.2.1.1.1: READWRITE BYTE.
#define TRAP_RECEIVER_ENABLED (2)			// 43.6.1.4.1.17095.2.1.1.2: READWRITE BYTE.
#define TRAP_RECEIVER_IP (3)			// 43.6.1.4.1.17095.2.1.1.3: READWRITE IP_ADDRESS.
#define TRAP_COMMUNITY (4)			// 43.6.1.4.1.17095.2.1.1.4: READWRITE ASCII_STRING.
#define LED_D5 (5)			// 43.6.1.4.1.17095.3.1: READWRITE BYTE.
#define LED_D6 (6)			// 43.6.1.4.1.17095.3.2: READWRITE BYTE.
#define PUSH_BUTTON (7)			// 43.6.1.4.1.17095.3.3: READONLY BYTE.
#define ANALOG_POT0 (8)			// 43.6.1.4.1.17095.3.4: READONLY WORD.
#define LCD_DISPLAY (200)			// 43.6.1.4.1.17095.3.6: READWRITE ASCII_STRING. //Alterado ID do LCD_DISPLAY - Greg


/*

Sensores exclu�dos da MIB

//#define SENSOR_01_VALUE (71)			// 43.6.1.4.1.17095.4.11: READWRITE TIME_TICKS.
//#define SENSOR_02_VALUE (21)			// 43.6.1.4.1.17095.4.22: READWRITE TIME_TICKS.
//#define SENSOR_03_VALUE (31)			// 43.6.1.4.1.17095.4.33: READWRITE TIME_TICKS.
//#define SENSOR_04_VALUE (41)			// 43.6.1.4.1.17095.4.44: READWRITE TIME_TICKS.


	Definido por Gregory E. Miguel
	Nova MIB definida, contemplando 10 sensores.
	Cada sensor � capaz de armazenar 4 grandezas
	L = Valor de Luminosidade
	U = Valor de Umidade
	P = Valor de Press�o
	T = Valor de Temperatura

*/

#define SENSOR_01_ID (10)			// 43.6.1.4.1.17095.3.7.1: READONLY BYTE.
#define SENSOR_02_ID (20)			// 43.6.1.4.1.17095.3.7.2: READONLY BYTE.
#define SENSOR_03_ID (30)			// 43.6.1.4.1.17095.3.7.3: READONLY BYTE.
#define SENSOR_04_ID (40)			// 43.6.1.4.1.17095.3.7.4: READONLY BYTE.
#define SENSOR_05_ID (50)			// 43.6.1.4.1.17095.3.7.5: READONLY BYTE.
#define SENSOR_06_ID (60)			// 43.6.1.4.1.17095.3.7.6: READONLY BYTE.
#define SENSOR_07_ID (70)			// 43.6.1.4.1.17095.3.7.7: READONLY BYTE.
#define SENSOR_08_ID (80)			// 43.6.1.4.1.17095.3.7.8: READONLY BYTE.
#define SENSOR_09_ID (90)			// 43.6.1.4.1.17095.3.7.9: READONLY BYTE.
#define SENSOR_10_ID (100)			// 43.6.1.4.1.17095.3.7.10: READONLY BYTE.
#define SENSOR_01_L (11)			// 43.6.1.4.1.17095.3.7.11: READWRITE BYTE.
#define SENSOR_01_U (12)			// 43.6.1.4.1.17095.3.7.12: READWRITE BYTE.
#define SENSOR_01_P (13)			// 43.6.1.4.1.17095.3.7.13: READWRITE BYTE.
#define SENSOR_01_T (14)			// 43.6.1.4.1.17095.3.7.14: READWRITE BYTE.
#define SENSOR_02_L (21)			// 43.6.1.4.1.17095.3.7.21: READWRITE BYTE.
#define SENSOR_02_U (22)			// 43.6.1.4.1.17095.3.7.22: READWRITE BYTE.
#define SENSOR_02_P (23)			// 43.6.1.4.1.17095.3.7.23: READWRITE BYTE.
#define SENSOR_02_T (24)			// 43.6.1.4.1.17095.3.7.24: READWRITE BYTE.
#define SENSOR_03_L (31)			// 43.6.1.4.1.17095.3.7.31: READWRITE BYTE.
#define SENSOR_03_U (32)			// 43.6.1.4.1.17095.3.7.32: READWRITE BYTE.
#define SENSOR_03_P (33)			// 43.6.1.4.1.17095.3.7.33: READWRITE BYTE.
#define SENSOR_03_T (34)			// 43.6.1.4.1.17095.3.7.34: READWRITE BYTE.
#define SENSOR_04_L (41)			// 43.6.1.4.1.17095.3.7.41: READWRITE BYTE.
#define SENSOR_04_U (42)			// 43.6.1.4.1.17095.3.7.42: READWRITE BYTE.
#define SENSOR_04_P (43)			// 43.6.1.4.1.17095.3.7.43: READWRITE BYTE.
#define SENSOR_04_T (44)			// 43.6.1.4.1.17095.3.7.44: READWRITE BYTE.
#define SENSOR_05_L (51)			// 43.6.1.4.1.17095.3.7.51: READWRITE BYTE.
#define SENSOR_05_U (52)			// 43.6.1.4.1.17095.3.7.52: READWRITE BYTE.
#define SENSOR_05_P (53)			// 43.6.1.4.1.17095.3.7.53: READWRITE BYTE.
#define SENSOR_05_T (54)			// 43.6.1.4.1.17095.3.7.54: READWRITE BYTE.
#define SENSOR_06_L (61)			// 43.6.1.4.1.17095.3.7.61: READWRITE BYTE.
#define SENSOR_06_U (62)			// 43.6.1.4.1.17095.3.7.62: READWRITE BYTE.
#define SENSOR_06_P (63)			// 43.6.1.4.1.17095.3.7.63: READWRITE BYTE.
#define SENSOR_06_T (64)			// 43.6.1.4.1.17095.3.7.64: READWRITE BYTE.
#define SENSOR_07_L (71)			// 43.6.1.4.1.17095.3.7.71: READWRITE BYTE.
#define SENSOR_07_U (72)			// 43.6.1.4.1.17095.3.7.72: READWRITE BYTE.
#define SENSOR_07_P (73)			// 43.6.1.4.1.17095.3.7.73: READWRITE BYTE.
#define SENSOR_07_T (74)			// 43.6.1.4.1.17095.3.7.74: READWRITE BYTE.
#define SENSOR_08_L (81)			// 43.6.1.4.1.17095.3.7.81: READWRITE BYTE.
#define SENSOR_08_U (82)			// 43.6.1.4.1.17095.3.7.82: READWRITE BYTE.
#define SENSOR_08_P (83)			// 43.6.1.4.1.17095.3.7.83: READWRITE BYTE.
#define SENSOR_08_T (84)			// 43.6.1.4.1.17095.3.7.84: READWRITE BYTE.
#define SENSOR_09_L (91)			// 43.6.1.4.1.17095.3.7.91: READWRITE BYTE.
#define SENSOR_09_U (92)			// 43.6.1.4.1.17095.3.7.92: READWRITE BYTE.
#define SENSOR_09_P (93)			// 43.6.1.4.1.17095.3.7.93: READWRITE BYTE.
#define SENSOR_09_T (94)			// 43.6.1.4.1.17095.3.7.94: READWRITE BYTE.
#define SENSOR_10_L (101)			// 43.6.1.4.1.17095.3.7.101: READWRITE BYTE.
#define SENSOR_10_U (102)			// 43.6.1.4.1.17095.3.7.102: READWRITE BYTE.
#define SENSOR_10_P (103)			// 43.6.1.4.1.17095.3.7.103: READWRITE BYTE.
#define SENSOR_10_T (104)			// 43.6.1.4.1.17095.3.7.104: READWRITE BYTE.
#define LUMINOSIDADE_MAX (202)			// 43.6.1.4.1.17095.3.8.1: READWRITE BYTE.
#define UMIDADE_MAX (212)			// 43.6.1.4.1.17095.3.8.2: READWRITE BYTE.
#define TEMPERATURA_MAX (222)			// 43.6.1.4.1.17095.3.8.3: READWRITE BYTE.
#define PRESSAO_MAX (232)			// 43.6.1.4.1.17095.3.8.4: READWRITE BYTE.
#define LUMINOSIDADE_MIN (201)			// 43.6.1.4.1.17095.3.8.11: READWRITE BYTE.
#define UMIDADE_MIN (211)			// 43.6.1.4.1.17095.3.8.22: READWRITE BYTE.
#define TEMPERATURA_MIN (221)			// 43.6.1.4.1.17095.3.8.33: READWRITE BYTE.
#define PRESSAO_MIN (231)			// 43.6.1.4.1.17095.3.8.44: READWRITE BYTE.

