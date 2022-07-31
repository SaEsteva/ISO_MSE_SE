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


/*************************************************************************************************
	 *  @brief error hook.
     *
     *  @details
     *   
     *
	 *  @param 		None.
	 *  @return     None.
***************************************************************************************************/
__WEAK void errorHook(void)  {
		while (1);
}

/*************************************************************************************************
	 *  @brief return hook.
     *
     *  @details
     *   
     *
	 *  @param 		None.
	 *  @return     None.
***************************************************************************************************/
__WEAK void returnHook(void)  {
		while (1);
}


/*************************************************************************************************
	 *  @brief Tarea Idle del OS.
     *
     *  @details
     *   Tarea Idle con prioridad m�nima de ejecuci�n.
     *
	 *  @param 		None.
	 *  @return     None.
***************************************************************************************************/
__WEAK void os_Idle_task(void)  {
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
	 *  @param id_tarea			N�mero de identificaci�n de la tarea.
	 *  @param prioridad_tarea  Prioridad de la tarea.
	 *  @param Nombre			Puntero al nombre de la tarea.
	 *  @return     None.
***************************************************************************************************/
void os_InitTarea(tarea *tarea,void *entryPoint,uint8_t id_tarea, uint8_t prioridad_tarea,char *Nombre)  {

	
	if(control_OS.cantidad_Tareas < MAX_NUM_TASK){
		tarea->stack[STACK_SIZE/4 - XPSR] 	= INIT_XPSR;					//necesari para bit thumb
		tarea->stack[STACK_SIZE/4 - PC_REG] = (uint32_t)entryPoint;			//direccion de la tarea (ENTRY_POINT)
		tarea->stack[STACK_SIZE/4 - LR] 	= (uint32_t)returnHook;			/* LR */
		//tarea->stack[STACK_SIZE/4 - R0] 	= (uint32_t)argumento_tarea; 	/* R0 */
		tarea->stack[STACK_SIZE/4 - LR_IRQ] = EXEC_RETURN; 					/* LR IRQ */

		tarea->stack_pointer = (uint32_t) (tarea->stack + STACK_SIZE/4 - STACK_FRAME_SIZE);

		tarea->entry_point = entryPoint;
		tarea->id = id_tarea;
		tarea->prioridad = p_TaskIdle+prioridad_tarea;
		strcpy(tarea->nombre,Nombre);
		tarea->estado = TAREA_READY;

	}else{
		os_Error(OS_ERR_N_TAREAS);
	}
	
	
}

/*************************************************************************************************
	 *  @brief Inicializa la tarea Idle del OS.
     *
     *  @details
	 * 	Inicializa la tarea IDLE del OS modificando la estructura 
     *
	 *  @param 		None.
	 *  @return     None.
***************************************************************************************************/
static void os_InitTareaIdle(void){

	
	tareaIdle.stack[STACK_SIZE/4 - XPSR] 	= INIT_XPSR;					//necesari para bit thumb
	tareaIdle.stack[STACK_SIZE/4 - PC_REG] = (uint32_t)os_Idle_task;		//direccion de la tarea (ENTRY_POINT)
	tareaIdle.stack[STACK_SIZE/4 - LR] 	= (uint32_t)returnHook;	/* LR */
	tareaIdle.stack[STACK_SIZE/4 - LR_IRQ] = EXEC_RETURN; 					/* LR IRQ */

	tareaIdle.stack_pointer = (uint32_t) (tareaIdle.stack + STACK_SIZE/4 - STACK_FRAME_SIZE);

	tareaIdle.entry_point = os_Idle_task;
	tareaIdle.id = id_TaskIdle;
	tareaIdle.prioridad = p_TaskIdle-1;
	strcpy(tareaIdle.nombre,"IdleTask");
	tareaIdle.estado = TAREA_READY;
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
	for(uint8_t i = first_index_Tasks; i<MAX_NUM_TASK+1;i++){
		if (i<numOfTask+1)controlStrct_OS.array_tareas[i]=array[i-1];
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

	controlStrct_OS.next_task = true;
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
	bool finish = 0;
	uint32_t index_task_array;
	uint32_t index_actual_task;
	uint32_t num_of_bloqued_task = 0;
	tarea * p_tarea;

	/*
	 * Si el estado del sistema se encuentra en RESET, El scheduler carga como primea tarea a la tareaIdle, 
	 * reinicia el contador de tareas y coloca como tarea siguiente a la primea del vector. 
	 */
	if (controlStrct_OS.estado_sistema == OS_RESET)  {
		controlStrct_OS.tarea_actual = (tarea*) &tareaIdle;
		controlStrct_OS.estado_sistema = OS_RUNNING;
		controlStrct_OS.index_tareas = first_index_Tasks;
		controlStrct_OS.tarea_siguiente = controlStrct_OS.array_tareas[controlStrct_OS.index_tareas];
	}else if(controlStrct_OS.estado_sistema == OS_RUNNING)  {
		/*
		 * Si el estado del sistema se encuentra en RUNNING, el scheduler debe realizar el cambio hacia la tarea siguiente
		 * ubicada en el array de tareas
		 */
		controlStrct_OS.tarea_actual = controlStrct_OS.tarea_siguiente;
		index_actual_task = controlStrct_OS.index_tareas;
		controlStrct_OS.index_tareas++;
		if(controlStrct_OS.index_tareas>controlStrct_OS.cant_tareas)controlStrct_OS.index_tareas=first_index_Tasks;
		controlStrct_OS.tarea_siguiente = controlStrct_OS.array_tareas[controlStrct_OS.index_tareas];

		/*
	 	* Se realiza una actualización del array de tareas, en caso de contar con tareas bloqueadas se decrementa 
		* el indicador de ticks bloqueada de la tarea, si lo requiere, se actualiza el estado de la tarea a Ready.
		* Si el estado de la tarea siguiente es bloqueado, se pasará a la siguiente tarea de la lista que se encuentre en
		* estado ready, en caso de que ninguna lo esté, pasaría nuevamente a la tarea actual.
		* En caso de que todas las tareas estén en estado bloqueado, se ejecutaría la tarea Idle.
		* 
	 	*/

		/*
		* Indice que recorrerá el array con las tareas del OS. Si la tarea es la Idle, toma el siguiente.
		*/
		index_task_array = controlStrct_OS.index_tareas;
	 	while(!finish){

			p_tarea = controlStrct_OS.array_tareas[index_task_array];
			if (p_tarea->estado == TAREA_BLOKED){
				p_tarea->ticks_bloqueada--;
				// Si la cantidad de ticks ya llega a 0, la tarea vuelve a estar en ready
				if (p_tarea->ticks_bloqueada <= 0)p_tarea->estado = TAREA_READY;
				// Si la tarea siguiente se encuentra en estado bloqueado, se debe elegir la tarea siguiente del array
				if (p_tarea == controlStrct_OS.tarea_siguiente)controlStrct_OS.tarea_siguiente = controlStrct_OS.array_tareas[(index_task_array+1)%controlStrct_OS.cant_tareas];
				num_of_bloqued_task++;
			}
			index_task_array++;
			if(index_task_array>controlStrct_OS.cant_tareas)index_task_array=first_index_Tasks;
			/*
	 		* Cuando el index llega a la tarea actual, debería salir del while. En este punto se analiza si la 
			* todas la tareas están bloqueadas.
			*/		
			if (index_task_array == index_actual_task){
				/*
	 			* Si todas las tareas están bloqueadas, debería pasar nuevamente a la tarea actual.
				*/	
				if (num_of_bloqued_task >= controlStrct_OS.cant_tareas-1){
					/*
					* Si todas las tareas estan bloqueadas y la tarea que corre es la Idle, se debe salir del while
					* sin cambio de contexto.
					*/
					if(controlStrct_OS.tarea_actual->id == id_TaskIdle){
						controlStrct_OS.next_task = false;
						finish = 1;
						break;
					}

					p_tarea = controlStrct_OS.array_tareas[index_task_array];
					/*
	 				* Si la tarea actual también está bloqueada, se pasa a la tarea idle.
					*/
					if (p_ tarea->estado == TAREA_BLOKED){
						controlStrct_OS.tarea_siguiente = controlStrct_OS.array_tareas[id_TaskIdle];
						//controlStrct_OS.index_tareas = id_TaskIdle;
						/*
						* Se actualiza el contador de ticks bloqueados de la tarea.
						*/
						p_tarea->ticks_bloqueada--;
						if (p_tarea->ticks_bloqueada <= 0) p_tarea->estado  = TAREA_READY;
					}else{
						/*
						* La unica tarea corriendo es la actual, se coloca como tarea siguiente y no debe haber un cambio de contexto
						*/
						controlStrct_OS.tarea_siguiente = controlStrct_OS.tarea_actual;
						controlStrct_OS.next_task = false;
					}					
				}
				finish = 1;
			}
		}
	}


	if(controlStrct_OS.next_task)setPendSV();
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


/*************************************************************************************************
	 *  @brief Pone en estado bloqueado una tarea.
     *
     *  @details
     *   Esta funcion simplemente es a efectos de prueba de tarea bloqueada, pone la tarea que se
	 * 	pasa como argumento en estado bloquedo la cantidad de ticks que se informa en el segundo argumento
     *
	 *  @param 		*tarea	Puntero a la estructura de la tarea.
	 *  @param 		n_tick	cantidad de ticks a permanecer bloqueada
	 *  @return     None
***************************************************************************************************/
void bloqued_Task(tarea *tarea,uint32_t n_tick)  {

	tarea->estado = TAREA_BLOKED;
	tarea->ticks_bloqueada = n_tick;	
}

/*************************************************************************************************
	 *  @brief Actualiza la estrucutra con el error y ejecutra el hook
     *
     *  @details
     *   Actualiza la estrucutra con el error que recibe como parametro y ejecutra el errorHook
     *
	 *  @param 		error	Error ocurrido
	 *  @return     None
***************************************************************************************************/
void os_Error(int32_t error)  {
	controlStrct_OS.error = error;
	errorHook();
}