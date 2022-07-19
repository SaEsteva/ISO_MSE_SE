/*
 * OS_Core.h
 *
 *  Created on: 17 Jul 2022
 *      Author: Santiago Esteva
 */

#ifndef OS_CORE_H_
#define OS_CORE_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "board.h"


/************************************************************************************
 * 			Tamano del stack predefinido para cada tarea expresado en bytes
 ***********************************************************************************/

#define STACK_SIZE 256

//----------------------------------------------------------------------------------



/************************************************************************************
 * 	Posiciones dentro del stack frame de los registros que conforman el stack frame
 ***********************************************************************************/

#define XPSR		1
#define PC_REG		2
#define LR			3
#define R12			4
#define R3			5
#define R2			6
#define R1			7
#define R0			8
#define LR_IRQ		9

//----------------------------------------------------------------------------------


/************************************************************************************
 * 			Valores necesarios para registros del stack frame inicial
 ***********************************************************************************/

#define INIT_XPSR 	1 << 24				//xPSR.T = 1
#define EXEC_RETURN	0xFFFFFFF9			//retornar a modo thread con MSP, FPU no utilizada

//----------------------------------------------------------------------------------



/************************************************************************************
 * 						Definiciones varias
 ***********************************************************************************/
#define STACK_FRAME_SIZE	8
#define TASK_NAME_SIZE		8
#define MAX_NUM_TASK		10

/************************************************************************************
 * 						Definiciones del OS
 ***********************************************************************************/
enum _estadoTarea{
	TAREA_READY,
	TAREA_RUNNING
	//TAREA_BLOKED,
	//TAREA_SUSPENDED
};

typedef enum _estadoTarea estadoTarea;

enum _estadoOS {
	OS_RESET,
	OS_RUNNING,
	OS_INTERRUPT
};

typedef enum _estadoOS estadoOS;

/*==================[Estructura para las tareas]=================================*/
struct _tarea{
 uint32_t		stack[STACK_SIZE/4];
 uint32_t		stack_pointer;
 void 			*entry_point;
 estadoTarea 	estado;
 uint8_t 		prioridad;
 uint8_t 		id;
 uint32_t 		ticks_bloqueada;
 char 			nombre[TASK_NAME_SIZE];
};

typedef struct _tarea tarea;

/*==================[Estructura de control del OS]=================================*/
struct _osControl{
	bool 		schedulerIRQ; 				//scheduling al volver de IRQ
	tarea 		*tarea_actual;
	tarea 		*tarea_siguiente;
	tarea 		*array_tareas[MAX_NUM_TASK];
	int32_t 	error; 						//ultimo error ocurrido
	estadoOS 	estado_sistema; 			//Estado actual del OS
	uint8_t 	cant_tareas;
};

typedef struct _osControl osControl;

/*==================[definicion de prototipos]=================================*/

void os_InitTarea(tarea *tarea,void *entryPoint,uint8_t id_tarea, uint8_t prioridad_tarea,char *Nombre);
void initTareaIdle(tarea* tareaToIdle);
void os_SistemInit(tarea* array[MAX_NUM_TASK],uint8_t numOfTask);
uint32_t getContextoSiguiente(uint32_t msp_ahora);
void SysTick_Handler(void);
static void scheduler(void);
static void setPendSV(void);

#endif /* ISO_I_2020_MSE_OS_INC_MSE_OS_CORE_H_ */
