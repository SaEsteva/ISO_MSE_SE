/*
 * main.h
 *
 *  Created on: 17 Jul 2022
 *      Author: Santiago Esteva
 */

#ifndef _MAIN_H_
#define _MAIN_H_

/*==================[inclusions]=============================================*/

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros and definitions]=================================*/

#define MILISEC						1000
#define CANT_TAREAS 				4

#define TIMELED						1000

// #define LEDR 						40
// #define LEDG 						41
// #define LED1 						43
#define USB_HOST_ONLY

#define BUTTON_LOGIC 				BUTTON_ONE_IS_UP
#define SENSORESTRUSOR 				TEC1
#define SENSORDESCARTE 				TEC2
#define SENSORSCANTIME 				50 // ms
#define SENSORPRESSTIME 			100 // ms

#define CANT_BUFFER					30
#define StopMessage					10	// decimal number of LF new line

#define UART_BR						115200
// #define UART_USB					3

#define TIEMPOMUESTRASBOTELLAS 		10000 // ms -> 10 s
#define CANTMAXIMABOTELLASOK 		30000
#define CANTMAXIMABOTELLASDESCARTE	30000

#define CANTIDADCOLAOK				60 // 10 minutos de datos, como los datos son cada 10 ms serían 60 valores
#define CANTIDADCOLADESCARTE		60 // 10 minutos de datos, como los datos son cada 10 ms serían 60 valores
#define TIPODATOCOLA				4 // dato int -> 4 bytes


/*==================[Definicion de tareas para el OS]==========================*/

/** @brief main function
 * @return main function should never return
 */
int main(void);

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */
/*==================[end of file]============================================*/
#endif /* #ifndef _MAIN_H_ */
