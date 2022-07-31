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

/*==================[macros and definitions]=================================*/

#define MILISEC		1000
#define CANT_TAREAS 4
uint8_t Index_tareas;
/*==================[Global data declaration]==============================*/

tarea tareaStruct1;		//estructura tarea de la tarea 1
tarea tareaStruct2;		//estructura tarea de la tarea 2
tarea tareaStruct3;		//estructura tarea de la tarea 3
tarea tareaStruct4;		//estructura tarea de la tarea 4

tarea* Tareas[MAX_NUM_TASK];
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
		bloqued_Task(&tareaStruct1,10);
	}
}

void tarea2(void)  {
	int j;
	while (1) {
		j++;
		bloqued_Task(&tareaStruct2,10);
	}
}

void tarea3(void)  {
	int k;
	while (1) {
		k++;
		bloqued_Task(&tareaStruct3,10);
	}
}

void tarea4(void)  {
	int l;
	while (1) {
		l++;
		bloqued_Task(&tareaStruct4,10);
	}
}
/*============================================================================*/

int main(void)  {

	initHardware();

	os_InitTarea(&tareaStruct1,tarea1,1,1,"tarea1");
	Tareas[0] = &tareaStruct1;
	os_InitTarea(&tareaStruct2,tarea2,2,1,"tarea2");
	Tareas[1] = &tareaStruct2;
	os_InitTarea(&tareaStruct3,tarea3,3,1,"tarea3");
	Tareas[2] = &tareaStruct3;
	os_InitTarea(&tareaStruct4,tarea4,4,1,"tarea4");
	Tareas[3] = &tareaStruct4;

	for(uint8_t i = CANT_TAREAS; i<MAX_NUM_TASK;i++){
		Tareas[i]=NULL;
	}

	os_SistemInit(Tareas,CANT_TAREAS);

	while (1) {
	}
}

/** @} doxygen end group definition */

/*==================[end of file]============================================*/
