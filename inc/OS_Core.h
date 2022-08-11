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
 * 	Tick Hook 
 ***********************************************************************************/
#define configUSE_TICK_HOOK 0


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
#define STACK_FRAME_SIZE	17
#define TASK_NAME_SIZE		8
#define MAX_NUM_TASK		10
#define MAX_PRIOR_TASK		4
#define p_TaskIdle 			0 /*Prioridad minima de task idle*/
#define id_TaskIdle 		MAX_NUM_TASK /*ID de task idle*/
#define first_index_Tasks 	0 /*Index of the os taks*/


/************************************************************************************
 * 						Definiciones del OS
 ***********************************************************************************/

// Errores
#define OS_ERR_N_TAREAS			1

// Posicion en el array de prioridades
#define array_pos_p4			0
#define array_pos_p3			1
#define array_pos_p2			2
#define array_pos_p1			3
#define array_pos_pIdle			4

// Prioridades
#define Prioridad_1				1
#define Prioridad_2				2
#define Prioridad_3				3
#define Prioridad_4				4

typedef enum _prioridadTareas prioridadTareas;

enum _estadoTarea{
	TAREA_READY,
	TAREA_RUNNING,
	TAREA_BLOKED
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
 int8_t 		prioridad;
 uint8_t 		id;
 uint32_t 		ticks_bloqueada;
 char 			nombre[TASK_NAME_SIZE+1];
};

typedef struct _tarea tarea;

/*==================[Estructura de control del OS]=================================*/
struct _osControl{
	bool 		next_task; 								// flag tarea siguiente, se debe cambiar de contexto
	bool 		schedulerIRQ; 							// flag scheduling al volver de IRQ
	tarea 		*tarea_actual;
	tarea 		*tarea_siguiente;
	tarea 		*array_tareas[MAX_NUM_TASK+1];			// Arreglo con todas las tareas actuales
	int32_t 	error; 									// ultimo error ocurrido
	estadoOS 	estado_sistema; 						// Estado actual del OS
	uint8_t 	prioridad_tareas[MAX_PRIOR_TASK+1];		// Array con tareas en cada prioridad ordenado de mayor prioridad a menor
	uint8_t 	cant_tareas_activas[MAX_PRIOR_TASK+1];	// Array con la cantidad de tareas no bloqueadas en cada prioridad de mayor a menor
	int8_t 		prioridad_actual;
	uint8_t 	cant_tareas;
	uint8_t 	index_tareas;
};

typedef struct _osControl osControl;

/*==================[definicion de prototipos]=================================*/
__WEAK void TickHook(void);
__WEAK void errorHook(void);
__WEAK void returnHook(void);
__WEAK void os_Idle_task(void);
void os_InitTarea(tarea *,void *,uint8_t , uint8_t ,char *);
static void os_InitTareaIdle(void);
void os_SistemInit(void);
uint32_t getContextoSiguiente(uint32_t );
void SysTick_Handler(void);
static void scheduler(void);
static void setPendSV(void);
void os_blockedTask(tarea *,uint32_t );
void os_releaseTask(tarea *);
void os_Error(int32_t);
static void os_OrdenarPrioridades(void);
static uint8_t os_BuscarPosicion(uint8_t);
tarea* os_ActualTask(void);
void os_Scheduling(void);

#endif /* ISO_I_2020_MSE_OS_INC_MSE_OS_CORE_H_ */
