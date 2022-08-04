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
	 *  @brief Tick hook.
     *
     *  @details
     *   
     *
	 *  @param 		None.
	 *  @return     None.
***************************************************************************************************/
__WEAK void TickHook(void)  {
	__asm volatile( "nop" );
}

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

	
	if(controlStrct_OS.cant_tareas < MAX_NUM_TASK){
		tarea->stack[STACK_SIZE/4 - XPSR] 	= INIT_XPSR;					//necesari para bit thumb
		tarea->stack[STACK_SIZE/4 - PC_REG] = (uint32_t)entryPoint;			//direccion de la tarea (ENTRY_POINT)
		tarea->stack[STACK_SIZE/4 - LR] 	= (uint32_t)returnHook;			/* LR */
		//tarea->stack[STACK_SIZE/4 - R0] 	= (uint32_t)argumento_tarea; 	/* R0 */
		tarea->stack[STACK_SIZE/4 - LR_IRQ] = EXEC_RETURN; 					/* LR IRQ */

		tarea->stack_pointer = (uint32_t) (tarea->stack + STACK_SIZE/4 - STACK_FRAME_SIZE);

		tarea->entry_point = entryPoint;
		tarea->id = id_tarea;
		// La prioridad debe estar entre 0 y MAX_PRIOR_TASK, en caso de quedar fuera de eso se asigna la menor (p_TaskIdle)
		switch (prioridad_tarea)
		{
		case 4:
			tarea->prioridad = p_TaskIdle+prioridad_tarea;		
			controlStrct_OS.prioridad_tareas[array_pos_p4]++;
			break;
		case 3:
			tarea->prioridad = p_TaskIdle+prioridad_tarea;		
			controlStrct_OS.prioridad_tareas[array_pos_p3]++;
			break;
		case 2:
			tarea->prioridad = p_TaskIdle+prioridad_tarea;		
			controlStrct_OS.prioridad_tareas[array_pos_p2]++;
			break;
		case 1:
			tarea->prioridad = p_TaskIdle+prioridad_tarea;		
			controlStrct_OS.prioridad_tareas[array_pos_p1]++;
			break;
		default:
			tarea->prioridad = p_TaskIdle;		
			controlStrct_OS.prioridad_tareas[array_pos_pIdle]++;
			break;
		}		
		strcpy(tarea->nombre,Nombre);
		tarea->estado = TAREA_READY;
		controlStrct_OS.cant_tareas++;
		controlStrct_OS.array_tareas[id_TaskIdle]=tarea;

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
void os_SistemInit()  {

	NVIC_SetPriority(PendSV_IRQn, (1 << __NVIC_PRIO_BITS)-1);
	controlStrct_OS.schedulerIRQ = false;
	controlStrct_OS.tarea_actual = NULL;
	controlStrct_OS.tarea_siguiente = NULL;
	controlStrct_OS.error = 0;
	controlStrct_OS.estado_sistema = OS_RESET;
	controlStrct_OS.array_tareas[id_TaskIdle]=&tareaIdle;
	os_InitTareaIdle();
	os_OrdenarPrioridades();
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
	
	uint8_t index_tareas = 0;
	tarea * p_tarea;

	if (configUSE_TICK_HOOK) TickHook();

	controlStrct_OS.next_task = true;

	/*
	 * En caso de contar con tareas bloqueadas, dado que paso un tick de sistema de debería decrementar el contador
	 */
	for(index_tareas=first_index_Tasks;index_tareas<controlStrct_OS.cant_tareas;index_tareas++){
		p_tarea = controlStrct_OS.array_tareas[index_tareas];
		if (p_tarea->estado == TAREA_BLOKED){
			p_tarea->ticks_bloqueada--;
			// Si la cantidad de ticks ya llega a 0, la tarea vuelve a estar en ready
			if (p_tarea->ticks_bloqueada <= 0)p_tarea->estado = TAREA_READY;
		}
	}
		
	/*
	 * Se llama al scheduler para analizar la tarea a seguir
	 */
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
	bool loop1,loop2 = 0;
	uint8_t index_task_array;
	uint8_t index_actual_task;
	uint8_t index_inicial_p_actual;
	uint8_t prioridad_actual;
	uint8_t prioridad_tarea_actual;
	uint8_t cant_misma_prioridad;
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
		controlStrct_OS.tarea_actual->prioridad = tareaIdle.prioridad;
	}else if(controlStrct_OS.estado_sistema == OS_RUNNING)  {
		/*
		 * Si el estado del sistema se encuentra en RUNNING, el scheduler debe realizar el cambio hacia la tarea siguiente
		 * ubicada en el array de tareas. Para esto se debe tener en cuenta el arreglo de prioridades. Por esto se analiza 
		 * de mas prioridad a menos priodidad.
		*/
		prioridad_actual = Prioridad_4;
		cant_misma_prioridad = os_ObtenerCantPrioridad(prioridad_actual,&index_inicial_p_actual);
		index_actual_task = controlStrct_OS.index_tareas;
	 	while(!loop1){
			if(cant_misma_prioridad>=1){
				/*
				 * Existen tareas con misma prioridad, hay que analizar si la prioridad de la tarea actual es esta
				*/
				prioridad_tarea_actual = controlStrct_OS.tarea_actual->prioridad;
				if (prioridad_tarea_actual == prioridad_actual){
					loop2=0;
					while(!loop2){
						index_task_array=index_inicial_p_actual + (index_actual_task-index_inicial_p_actual)%cant_misma_prioridad;
						p_tarea = controlStrct_OS.array_tareas[index_task_array];
						if (p_tarea->estado == TAREA_BLOKED){
							// Si la tarea esta bloqueada debo pasar a la otra
							index_inicial_p_actual = (index_inicial_p_actual+1)%cant_misma_prioridad;
							// Si el index siguiente da 0, significa que no hay mas tareas de esta prioridad y debo disminuir y salir de este loop
							if (index_inicial_p_actual == 0){
								prioridad_actual--;
								cant_misma_prioridad = os_ObtenerCantPrioridad(prioridad_actual,&index_inicial_p_actual);
								loop2 = 1;
							}
						}else{
							// La tarea no esta bloqueada asi que la asigno y sigo
							controlStrct_OS.index_tareas = index_inicial_p_actual;
							controlStrct_OS.tarea_siguiente = controlStrct_OS.array_tareas[controlStrct_OS.index_tareas];
							controlStrct_OS.tarea_actual->prioridad = prioridad_actual;
							controlStrct_OS.next_task = true;
							loop2 = loop1 = 1;
						}
					}
				}else{
					// Como no la tarea actual no es de esta prioridad, recorro el loop normal
					loop2=0;
					while(!loop2){
						p_tarea = controlStrct_OS.array_tareas[index_inicial_p_actual];
						if (p_tarea->estado == TAREA_BLOKED){
							// Si la tarea esta bloqueada debo pasar a la otra
							index_inicial_p_actual = (index_inicial_p_actual+1)%cant_misma_prioridad;
							// Si el index siguiente da 0, significa que no hay mas tareas de esta prioridad y debo disminuir y salir de este loop
							if (index_inicial_p_actual == 0){
								prioridad_actual--;
								cant_misma_prioridad = os_ObtenerCantPrioridad(prioridad_actual,&index_inicial_p_actual);
								loop2 = 1;
							}
						}else{
							controlStrct_OS.index_tareas = index_inicial_p_actual;
							controlStrct_OS.tarea_siguiente = controlStrct_OS.array_tareas[controlStrct_OS.index_tareas];
							controlStrct_OS.tarea_actual->prioridad = prioridad_actual;
							controlStrct_OS.next_task = true;
							loop2 = loop1 = 1;
						}
					}
				}					
			}else{
				/*
				 * No hay tareas con esta prioridad, debo bajar a la siguiente y volver al ciclo.
				*/
				prioridad_actual--;
				cant_misma_prioridad = os_ObtenerCantPrioridad(prioridad_actual,&index_inicial_p_actual);
				if (prioridad_actual < p_TaskIdle){
					// Ninguna tarea se encuentra en estado para ser ejecutada, se debe ejecutar la tarea Idle
					
					controlStrct_OS.index_tareas = id_TaskIdle;
					controlStrct_OS.tarea_siguiente = (tarea*) &tareaIdle;
					controlStrct_OS.tarea_actual->prioridad = tareaIdle.prioridad;
					controlStrct_OS.next_task = true;
					loop1 = 1;
		
				}
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

/*************************************************************************************************
	 *  @brief Ordena el vector de tareas dependiendo la prioridad
     *
     *  @details
     *   Ordena el vector de tareas de mas proritaria a menos prioritaria
     *
	 *  @param 		None
	 *  @return     None
***************************************************************************************************/
static void os_OrdenarPrioridades(void)  {
	uint16_t i,p0,p1,p2,p3,p4;
	tarea * p_tarea[MAX_NUM_TASK];
	p4 = 0;
	p3 = controlStrct_OS.prioridad_tareas[array_pos_p4];
	p2 = controlStrct_OS.prioridad_tareas[array_pos_p3];
	p1 = controlStrct_OS.prioridad_tareas[array_pos_p2];
	p0 = controlStrct_OS.prioridad_tareas[array_pos_p1];

	// Genero un array auxiliar con las tareas desordenadas
	for(i=first_index_Tasks;i<controlStrct_OS.cant_tareas;i++){
		p_tarea[i] = controlStrct_OS.array_tareas[i];
	}
	for(i=first_index_Tasks;i<controlStrct_OS.cant_tareas;i++){
		switch (p_tarea[i]->prioridad)
		{
		case Prioridad_4:
			controlStrct_OS.array_tareas[p4] = p_tarea[i];
			p4++;
			break;
		case Prioridad_3:
			controlStrct_OS.array_tareas[p3] = p_tarea[i];
			p3++;
			break;
		case Prioridad_2:
			controlStrct_OS.array_tareas[p2] = p_tarea[i];
			p2++;
			break;
		case Prioridad_1:
			controlStrct_OS.array_tareas[p1] = p_tarea[i];
			p1++;
			break;
		default:
			controlStrct_OS.array_tareas[p0] = p_tarea[i];
			p0++;
			break;
		}
	}
}

/*************************************************************************************************
	 *  @brief Buscar la cantidad de tareas que hay con una priridad
     *
     *  @details
     *   Buscar en el vector de control la cantidad de tareas que hay con una priridad determinada
     *
	 *  @param 		prioridad: la prioridad a buscar
	 * 	@param 		*posicion_inicial_vector: puntero a variable donde se guardará la posicion en el vector de tareas donde incia la prioridad actual
	 *  @return     la cantidad de tareas con esa prioridad
***************************************************************************************************/
static uint8_t os_ObtenerCantPrioridad(uint8_t prioridad,uint8_t * posicion_inicial_vector) {
	uint8_t cant4 = controlStrct_OS.prioridad_tareas[array_pos_p4];
	uint8_t cant3 = controlStrct_OS.prioridad_tareas[array_pos_p3];
	uint8_t cant2 = controlStrct_OS.prioridad_tareas[array_pos_p2];
	uint8_t cant1 = controlStrct_OS.prioridad_tareas[array_pos_p1];
	uint8_t cant0 = cant4+cant3+cant2+cant1;	
	switch (prioridad)
	{
	case Prioridad_4:
		*posicion_inicial_vector = array_pos_p4;
		return cant4;
		break;
	case Prioridad_3:
		*posicion_inicial_vector = array_pos_p4 + cant4;
		return cant3;
		break;
	case Prioridad_2:
		*posicion_inicial_vector = cant4 + cant3;
		return cant2;
		break;
	case Prioridad_1:
		*posicion_inicial_vector = cant4 + cant3 + cant2;
		return cant1;
		break;
	default:
		*posicion_inicial_vector = cant0;
		return 0;
		break;
	}
}
