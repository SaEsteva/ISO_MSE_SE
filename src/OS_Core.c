/*
 * OS_Core.c
 *
 *  Created on: 17 Jul 2022
 *      Author: Santiago Esteva
 */

#include "../inc/OS_Core.h"

/*==================[definicion de variables globales]=================================*/
static osControl controlStrct_OS;
static tarea tareaIdle;
extern uint8_t Index_tareas;

/*************************************************************************************************
	 *  @brief Tarea Idle del OS.
     *
     *  @details
     *   Tarea Idle con prioridad mínima de ejecución.
     *
	 *  @param 		None.
	 *  @return     None.
***************************************************************************************************/
void os_Idle_task(void)  {
		while (1) {
			__WFI();
		}
}

/*************************************************************************************************
	 *  @brief Inicializa las tareas que correran en el OS.
     *
     *  @details
     *   Inicializa una tarea para que pueda correr en el OS implementado.
     *   Es necesario llamar a esta funcion para cada tarea antes que inicie
     *   el OS.
     *
	 *  @param *tarea			Puntero a la estructura de la tarea.
	 *  @param entryPoint			Puntero a la tarea.
	 *  @param id_tarea			Número de identificación de la tarea.
	 *  @param prioridad_tarea  Prioridad de la tarea.
	 *  @param Nombre			Puntero al nombre de la tarea.
	 *  @return     None.
***************************************************************************************************/
void os_InitTarea(tarea *tarea,void *entryPoint,uint8_t id_tarea, uint8_t prioridad_tarea,char *Nombre)  {

	tarea->stack[STACK_SIZE/4 - XPSR] 	= INIT_XPSR;					//necesari para bit thumb
	tarea->stack[STACK_SIZE/4 - PC_REG] = (uint32_t)entryPoint;			//direccion de la tarea (ENTRY_POINT)
	//tarea->stack[STACK_SIZE/4 - LR] 	= (uint32_t)task_return_hook;	/* LR */
	//tarea->stack[STACK_SIZE/4 - R0] 	= (uint32_t)argumento_tarea; 	/* R0 */
	tarea->stack[STACK_SIZE/4 - LR_IRQ] = EXEC_RETURN; 					/* LR IRQ */

	tarea->stack_pointer = (uint32_t) (tarea->stack + STACK_SIZE/4 - STACK_FRAME_SIZE);

	tarea->entry_point = entryPoint;
	tarea->id = id_tarea;
	tarea->prioridad = p_TaskIdle+prioridad_tarea;
	strcpy(tarea->nombre,Nombre);
	tarea->estado = TAREA_READY;
}

/*************************************************************************************************
	 *  @brief Inicializa la tarea Idle del OS.
     *
     *  @details
     *
	 *  @param 		None.
	 *  @return     None.
***************************************************************************************************/
void os_InitTareaIdle(void){

	os_InitTarea(&tareaIdle,os_Idle_task,id_TaskIdle,p_TaskIdle,"IdleTask");
}

/*************************************************************************************************
	 *  @brief Inicializa el OS.
     *
     *  @details
     *   Inicializa el OS modificando la estructura
     *
	 *  @param 		None.
	 *  @return     None.
***************************************************************************************************/
void os_SistemInit(tarea* array[MAX_NUM_TASK],uint8_t numOfTask)  {

	NVIC_SetPriority(PendSV_IRQn, (1 << __NVIC_PRIO_BITS)-1);
	controlStrct_OS.schedulerIRQ = false;
	controlStrct_OS.tarea_actual = NULL;
	controlStrct_OS.tarea_siguiente = NULL;
	controlStrct_OS.error = 0;
	controlStrct_OS.estado_sistema = OS_RESET;
	controlStrct_OS.cant_tareas = numOfTask;
	controlStrct_OS.array_tareas[id_TaskIdle]=&tareaIdle;
	for(uint8_t i = 1; i<MAX_NUM_TASK+1;i++){
		if (i<numOfTask)controlStrct_OS.array_tareas[i]=array[i-1];
		else controlStrct_OS.array_tareas[i]=NULL;
	}
	os_InitTareaIdle();
}

/*************************************************************************************************
	 *  @brief Funcion para determinar el proximo contexto.
     *
     *  @details
     *   Esta funcion obtiene el siguiente contexto a ser cargado. El cambio de contexto se
     *   ejecuta en el handler de PendSV, dentro del cual se llama a esta funcion
     *
	 *  @param 		msp_ahora	copia del contenido del MSP a momento de llamar a la funcion.
	 *  @return     valor a cargarse en el MSP que apunta al nuevo contexto.
***************************************************************************************************/
uint32_t getContextoSiguiente(uint32_t msp_ahora)  {
	uint32_t msp_siguiente;

	/*
	 * Esta funcion efectua el cambio de contexto. Se guarda el MSP (msp_ahora) en la variable
	 * correspondiente de la estructura de la tarea corriendo actualmente.
	 * Se carga en la variable msp_siguiente el stack pointer de la tarea siguiente
	 * Se actualiza la tarea a estado RUNNING y se retorna al handler de PendSV
	 */

	controlStrct_OS.tarea_actual->stack_pointer = msp_ahora;

	if (controlStrct_OS.tarea_actual->estado == TAREA_RUNNING)
		controlStrct_OS.tarea_actual->estado = TAREA_READY;

	msp_siguiente = controlStrct_OS.tarea_siguiente->stack_pointer;

	controlStrct_OS.tarea_actual = controlStrct_OS.tarea_siguiente;
	controlStrct_OS.tarea_actual->estado = TAREA_RUNNING;

	controlStrct_OS.estado_sistema = OS_RUNNING;

	return msp_siguiente;
}

/*************************************************************************************************
	 *  @brief SysTick Handler.
     *
     *  @details
     *   Handler del Systick para llamar al scheduler y ejecutar la tarea siguiente seteando el PendSV
     *
	 *  @param 		None.
	 *  @return     None.
***************************************************************************************************/
void SysTick_Handler(void)  {

	scheduler();
}


/*************************************************************************************************
	 *  @brief Funcion de scheduling.
     *
     *  @details
     *   Determina que tarea se va a ejecutar dando el puntero necesario para realizar el cambio de contexto
     *
	 *  @param	None.
	 *  @return	None.
***************************************************************************************************/
static void scheduler(void)  {

	if (controlStrct_OS.estado_sistema == OS_RESET)  {
		controlStrct_OS.tarea_actual = (tarea*) &tareaIdle;
		controlStrct_OS.estado_sistema = OS_RUNNING;
		Index_tareas = 1;
		controlStrct_OS.tarea_siguiente = controlStrct_OS.array_tareas[Index_tareas];
	}else{
		controlStrct_OS.tarea_actual = controlStrct_OS.tarea_siguiente;
		Index_tareas++;
		if(Index_tareas>=controlStrct_OS.cant_tareas)Index_tareas=0;
		controlStrct_OS.tarea_siguiente = controlStrct_OS.array_tareas[Index_tareas];
	}


	setPendSV();
}

/*************************************************************************************************
	 *  @brief Setea la bandera correspondiente para lanzar PendSV.
     *
     *  @details
     *   Esta funcion simplemente es a efectos de simplificar la lectura del programa. Setea
     *   la bandera comrrespondiente para que se ejucute PendSV
     *
	 *  @param 		None
	 *  @return     None
***************************************************************************************************/
static void setPendSV(void)  {

	/**
	 * Se setea el bit correspondiente a la excepcion PendSV
	 */
	SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;

	/**
	 * Instruction Synchronization Barrier; flushes the pipeline and ensures that
	 * all previous instructions are completed before executing new instructions
	 */
	__ISB();

	/**
	 * Data Synchronization Barrier; ensures that all memory accesses are
	 * completed before next instruction is executed
	 */
	__DSB();
}
