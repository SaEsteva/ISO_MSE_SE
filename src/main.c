/*
 * main.c
 *
 *  Created on: 17 Jul 2022
 *      Author: Santiago Esteva
 */
/*==================[inclusions]=============================================*/

#include <stdlib.h>
#include "main.h"
#include "board.h"
#include "OS_Core.h"
#include "OS_API.h"

/*==================[macros and definitions]=================================*/

#define MILISEC		1000
#define CANT_TAREAS 8
uint8_t Index_tareas;
/*==================[Global data declaration]==============================*/

tarea tareaStruct1;		//estructura tarea de la tarea 1
tarea tareaStruct2;		//estructura tarea de la tarea 2
tarea tareaStruct3;		//estructura tarea de la tarea 3
tarea tareaStruct4;		//estructura tarea de la tarea 4
tarea tareaStruct5;		//estructura tarea de la tarea 5
tarea tareaStruct6;		//estructura tarea de la tarea 6
tarea tareaStruct7;		//estructura tarea de la tarea 7
tarea tareaStruct8;		//estructura tarea de la tarea 8

semaforo_bin semaforo_binario;

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/** @brief hardware initialization function
 *	@return none
 */
static void initHardware(void)  {
	Board_Init();
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / MILISEC);		//systick 1ms
}


/*==================[Definicion de tareas para el OS]==========================*/
void tarea1(void)  {
	int i;
	while (1) {
		i++;
		Delay(10);
	}
}

void tarea2(void)  {
	int j;
	while (1) {
		j++;
		Delay(20);
	}
}

void tarea3(void)  {
	int k;
	while (1) {
		k++;
		Delay(10);
	}
}

void tarea4(void)  {
	int l;
	while (1) {
		l++;
		Delay(20);
	}
}

void tarea5(void)  {
	int l;
	while (1) {
		l++;
		Take_Semaforo_Bin(&semaforo_binario);
	}
}
void tarea6(void)  {
	int l;
	while (1) {
		l++;
	}
}
void tarea7(void)  {
	int l;
	while (1) {
		l++;
	}
}
void tarea8(void)  {
	int l;
	while (1) {
		l++;
		Give_Semaforo_Bin(&semaforo_binario);
	}
}
/*============================================================================*/

int main(void)  {

	initHardware();

	os_InitTarea(&tareaStruct1,tarea1,1,3,"tarea1");
	os_InitTarea(&tareaStruct5,tarea5,5,1,"tarea5");
	os_InitTarea(&tareaStruct2,tarea2,2,3,"tarea2");
	os_InitTarea(&tareaStruct6,tarea6,6,1,"tarea6");
	os_InitTarea(&tareaStruct3,tarea3,3,3,"tarea3");
	os_InitTarea(&tareaStruct7,tarea7,7,1,"tarea7");
	os_InitTarea(&tareaStruct4,tarea4,4,3,"tarea4");
	os_InitTarea(&tareaStruct8,tarea8,8,1,"tarea8");
	
	Init_Semaforo_Bin(&semaforo_binario);

	os_SistemInit();


	while (1) {
	}
}

/** @} doxygen end group definition */

/*==================[end of file]============================================*/
