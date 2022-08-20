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
	 *  @param 	*tarea			Puntero a la estructura de la tarea.
	 *  @param 	entryPoint			Puntero a la tarea.
	 *  @param 	id_tarea			N�mero de identificaci�n de la tarea.
	 *  @param 	prioridad_tarea  Prioridad de la tarea.
	 *  @param 	Nombre			Puntero al nombre de la tarea.
	 *  @return	None.
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
		// La prioridad debe estar entre 0 y MAX_PRIOR_TASK, en caso de quedar fuera de eso se asigna la menor (P_TASKIDLE)
		switch (prioridad_tarea)
		{
		case 4:
			tarea->prioridad = P_TASKIDLE+prioridad_tarea;
			controlStrct_OS.prioridad_tareas[ARRAY_POS_P0]++;
			controlStrct_OS.cant_tareas_activas[ARRAY_POS_P0]++;
			break;
		case 3:
			tarea->prioridad = P_TASKIDLE+prioridad_tarea;
			controlStrct_OS.prioridad_tareas[ARRAY_POS_P1]++;
			controlStrct_OS.cant_tareas_activas[ARRAY_POS_P1]++;
			break;
		case 2:
			tarea->prioridad = P_TASKIDLE+prioridad_tarea;
			controlStrct_OS.prioridad_tareas[ARRAY_POS_P2]++;
			controlStrct_OS.cant_tareas_activas[ARRAY_POS_P2]++;
			break;
		case 1:
			tarea->prioridad = P_TASKIDLE+prioridad_tarea;
			controlStrct_OS.prioridad_tareas[ARRAY_POS_P3]++;
			controlStrct_OS.cant_tareas_activas[ARRAY_POS_P3]++;
			break;
		default:
			tarea->prioridad = P_TASKIDLE;
			controlStrct_OS.prioridad_tareas[ARRAY_POS_PIDLE]++;
			controlStrct_OS.cant_tareas_activas[ARRAY_POS_PIDLE]++;
			break;
		}		
		strcpy(tarea->nombre,Nombre);
		tarea->estado = TAREA_READY;
		controlStrct_OS.array_tareas[controlStrct_OS.cant_tareas]=tarea;
		controlStrct_OS.cant_tareas++;


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
	tareaIdle.id = ID_TASKIDLE;
	tareaIdle.prioridad = P_TASKIDLE-1;
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
	controlStrct_OS.criticalZone = false;
	controlStrct_OS.tarea_actual = NULL;
	controlStrct_OS.tarea_siguiente = NULL;
	controlStrct_OS.error = 0;
	controlStrct_OS.estado_sistema = OS_RESET;
	controlStrct_OS.array_tareas[ID_TASKIDLE]=&tareaIdle;
	controlStrct_OS.index_tareas=FIRST_INDEX_TASKS;
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
	uint8_t index_prioridad;
	tarea * p_tarea;

	index_tareas = 0;
	index_prioridad = 0;

	if (configUSE_TICK_HOOK) TickHook();

	controlStrct_OS.next_task = true;

	/*
	 * En caso de contar con tareas bloqueadas, dado que paso un tick de sistema de debería decrementar el contador
	 */
	for(index_tareas=FIRST_INDEX_TASKS;index_tareas<controlStrct_OS.cant_tareas;index_tareas++){
		p_tarea = controlStrct_OS.array_tareas[index_tareas];
		if (p_tarea->estado == TAREA_BLOKED){
			p_tarea->ticks_bloqueada--;
			// Si la cantidad de ticks ya llega a 0, la tarea vuelve a estar en ready
			if (p_tarea->ticks_bloqueada <= 0){
				os_releaseTask(p_tarea);
			}
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
	bool loop,prioridad_flag;
	uint8_t index_task_array;
	uint8_t index_actual_task;
	uint8_t index_inicial_p_actual;
	uint8_t index_prior;
	uint8_t prioridad_actual;
	uint8_t cant_misma_prioridad;
	tarea * p_tarea;
	estadoOS 	estado;

	loop = 0;
	prioridad_flag = 0;
	/*
	 * Si el estado del sistema se encuentra en RESET, El scheduler carga como primea tarea a la tareaIdle, 
	 * reinicia el contador de tareas y coloca como tarea siguiente a la primea del vector. 
	 */
	switch (controlStrct_OS.estado_sistema)
	{
	case OS_RESET:
		controlStrct_OS.tarea_actual = (tarea*) &tareaIdle;
		controlStrct_OS.estado_sistema = OS_RUNNING;
		controlStrct_OS.index_tareas = FIRST_INDEX_TASKS;
		controlStrct_OS.tarea_siguiente = controlStrct_OS.array_tareas[controlStrct_OS.index_tareas];
		controlStrct_OS.prioridad_actual = P_TASKIDLE;
		break;
	
	case OS_RUNNING:
		/*
		 * Si el estado del sistema se encuentra en RUNNING, el scheduler debe realizar el cambio hacia la tarea siguiente
		 * ubicada en el array de tareas. Para esto se debe tener en cuenta el arreglo de prioridades. Por esto se analiza 
		 * de mas prioridad a menos priodidad.
		*/
		index_prior = ARRAY_POS_P0;
		index_actual_task = controlStrct_OS.index_tareas;
		prioridad_actual = PRIORIDAD_0 - index_prior;
		index_inicial_p_actual = os_BuscarPosicion(prioridad_actual);
		while(!loop){
			cant_misma_prioridad = controlStrct_OS.prioridad_tareas[index_prior];
			if(controlStrct_OS.cant_tareas_activas[index_prior] > 0){
				if (prioridad_actual == controlStrct_OS.prioridad_actual){
					index_task_array=index_inicial_p_actual+(index_actual_task+1)%cant_misma_prioridad;
				}else{
					index_task_array=index_inicial_p_actual;
				}
				//Una vez que tengo el index de la posible tarea siguiente, veo si no esta bloqueada. Si lo esta, debo aumentar un index
				//y ver la siguiente
				p_tarea = controlStrct_OS.array_tareas[index_task_array];
				if (p_tarea->estado == TAREA_BLOKED){
					/*
					 * Esta parte se debe consultar en caso de que una tarea de mayor prioridad se pone el ready y por lo tanto el index inicial
					 * se utiliza como "buscador" de esta tarea ya que el flag prioridad_flag se encuentra activado.
					 *
					 */
					if(!prioridad_flag)index_inicial_p_actual = index_inicial_p_actual+(index_task_array+1)%cant_misma_prioridad;
					else index_inicial_p_actual = (index_task_array+1)%cant_misma_prioridad;
					prioridad_flag = 1;
				}else{
					controlStrct_OS.index_tareas = index_task_array;
					controlStrct_OS.tarea_siguiente = controlStrct_OS.array_tareas[controlStrct_OS.index_tareas];
					controlStrct_OS.prioridad_actual = prioridad_actual;
					if(controlStrct_OS.index_tareas == index_actual_task)controlStrct_OS.next_task = false;
					loop = 1;
				}
			}else{
				index_prior++;
				if (MAX_PRIOR_TASK < index_prior){
					// Ninguna tarea se encuentra en estado para ser ejecutada, se debe ejecutar la tarea Idle
					controlStrct_OS.index_tareas = ID_TASKIDLE;
					controlStrct_OS.tarea_siguiente = (tarea*) &tareaIdle;
					controlStrct_OS.prioridad_actual = P_TASKIDLE;
					loop = 1;
				}
			}
			prioridad_actual = PRIORIDAD_0 - index_prior;
			if(!prioridad_flag)index_inicial_p_actual = os_BuscarPosicion(prioridad_actual);
		}
		break;

	default:
		controlStrct_OS.estado_sistema = OS_RESET;
		break;
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
void os_blockedTask(tarea *tarea,uint32_t n_tick)  {

	uint8_t index_prioridad = 0; // posicion de la tarea en el vector de prioridades
	tarea->estado = TAREA_BLOKED;
	tarea->ticks_bloqueada = n_tick;
	index_prioridad = (PRIORIDAD_0-tarea->prioridad);
	// Saco la tarea del vector de activas, la posicion en el vector depende de su prioridad
	controlStrct_OS.cant_tareas_activas[index_prioridad]--;
	// La tarea se bloque por lo tanto se esperar� hasta una futura interupcion del Scheduler
	__WFI();
}

/*************************************************************************************************
	 *  @brief Pone en estado ready una tarea.
     *
     *  @details
     *   Pone en estado ready una tarea que estaba bloqueada
     *
	 *  @param 		*tarea	Puntero a la estructura de la tarea.
	 *  @return     None
***************************************************************************************************/
void os_releaseTask(tarea *tarea)  {
	uint8_t index_prioridad = 0;
	tarea->estado = TAREA_READY;
	tarea->ticks_bloqueada = 0;
	index_prioridad = (PRIORIDAD_0-tarea->prioridad);
	controlStrct_OS.cant_tareas_activas[index_prioridad]++;
}
/*************************************************************************************************
	 *  @brief Actualiza la estrucutra con el error y ejecutra el hook
     *
     *  @details
     *   Actualiza la estrucutra con el error que recibe como parametro, engtra a un estado crítico para
	 * 	que ninguna IRQ lo saque del estado y ejecuta el errorHook
     *
	 *  @param 		error	Error ocurrido
	 *  @return     None
***************************************************************************************************/
void os_Error(int32_t error)  {
	os_enter_critical();
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
	p0 = 0;
	p1 = controlStrct_OS.prioridad_tareas[ARRAY_POS_P0];
	p2 = controlStrct_OS.prioridad_tareas[ARRAY_POS_P3]+p1;
	p3 = controlStrct_OS.prioridad_tareas[ARRAY_POS_P2]+p2;
	p4 = controlStrct_OS.prioridad_tareas[ARRAY_POS_P1]+p3;

	// Genero un array auxiliar con las tareas desordenadas
	for(i=FIRST_INDEX_TASKS;i<controlStrct_OS.cant_tareas;i++){
		p_tarea[i] = controlStrct_OS.array_tareas[i];
	}
	for(i=FIRST_INDEX_TASKS;i<controlStrct_OS.cant_tareas;i++){
		switch (p_tarea[i]->prioridad)
		{
		case PRIORIDAD_0:
			controlStrct_OS.array_tareas[p0] = p_tarea[i];
			p0++;
			break;
		case PRIORIDAD_1:
			controlStrct_OS.array_tareas[p1] = p_tarea[i];
			p1++;
			break;
		case PRIORIDAD_2:
			controlStrct_OS.array_tareas[p2] = p_tarea[i];
			p2++;
			break;
		case PRIORIDAD_3:
			controlStrct_OS.array_tareas[p3] = p_tarea[i];
			p3++;
			break;
		default:
			controlStrct_OS.array_tareas[p4] = p_tarea[i];
			p0++;
			break;
		}
	}
}


/*************************************************************************************************
	 *  @brief Buscar la posicion en el vector de tareas con es prioridad
     *
     *  @details
     *   Buscar la posicion en el vector de tareas del OS la posicion en la cual se encuentran las tareas que tienen
     *   la prioridad recibida como par�metro
     *
	 *  @param 		prioridad: la prioridad a buscar
	 *  @return     posicion en el vector de tareas donde incia la prioridad actual
***************************************************************************************************/
static uint8_t os_BuscarPosicion(uint8_t prioridad) {
	uint8_t cant0 = controlStrct_OS.prioridad_tareas[ARRAY_POS_P0];
	uint8_t cant1 = controlStrct_OS.prioridad_tareas[ARRAY_POS_P1];
	uint8_t cant2 = controlStrct_OS.prioridad_tareas[ARRAY_POS_P2];
	uint8_t cant3 = controlStrct_OS.prioridad_tareas[ARRAY_POS_P3];
	uint8_t cant4 = cant0+cant1+cant2+cant3;	
	switch (prioridad)
	{
	case PRIORIDAD_0:
		return ARRAY_POS_P0;
		break;
	case PRIORIDAD_1:
		return ARRAY_POS_P0 + cant0;
		break;
	case PRIORIDAD_2:
		return cant0 + cant1;
		break;
	case PRIORIDAD_3:
		return cant0 + cant1 + cant2;
		break;
	default:
		return cant4;
		break;
	}
}

/*************************************************************************************************
	 *  @brief Indicar cual es la tarea actual
     *
     *  @details
     *   Indica cual es la tarea que se encuentra corriendo actualmente en el OS, permite a las tareas 
	 * de la API conocer el estado del OS.
     *
	 *  @param 		None
	 *  @return     Direccion de memoria de la tarea actual
***************************************************************************************************/
tarea* os_ActualTask(){
	return controlStrct_OS.tarea_actual;
}

/*************************************************************************************************
	 *  @brief Fuerza un scheduling del OS
     *
     *  @details
     *   Llama a la funcion de scheduler para generar un scheduling forzado del OS
     *
	 *  @param 		None
	 *  @return     None
***************************************************************************************************/
void os_Scheduling(){
	scheduler();
}

/*************************************************************************************************
	 *  @brief Consulta el estado del OS
     *
     *  @details
     *   Consulta el estado del OS para ser utilizado por otros recursos
     *
	 *  @param 		None
	 *  @return     estado actual del OS
***************************************************************************************************/
estadoOS os_Estado(){
	return controlStrct_OS.estado_sistema;
}

/*************************************************************************************************
	 *  @brief Modifica el estado del OS
     *
     *  @details
     *   Modifica el estado del OS para ser utilizado por otros recursos
     *
	 *  @param 		nuevo_estado
	 *  @return     None.
***************************************************************************************************/
void os_NuevoEstado(estadoOS nuevo_estado){
	controlStrct_OS.estado_sistema = nuevo_estado;
}


/*************************************************************************************************
	 *  @brief Entrar a zona crítica
     *
     *  @details
     *   Ingresa a una zona crítica por lo que debe desabilitar las interrupciones de hw
     *
	 *  @param 		None
	 *  @return     estado 
***************************************************************************************************/
inline bool os_enter_critical(){
	__disable_irq();
	controlStrct_OS.criticalZone = true;
}


/*************************************************************************************************
	 *  @brief Salid de zona crítica
     *
     *  @details
     *   Sale de una zona crítica por lo que debe habulitar las interrupciones de hw
     *
	 *  @param 		None
	 *  @return     None
***************************************************************************************************/
inline void os_exit_critical(){
	controlStrct_OS.criticalZone = true;
	__enable_irq();
}
