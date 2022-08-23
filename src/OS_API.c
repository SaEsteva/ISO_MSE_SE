/*
 * OS_API.c
 *
 *  Created on: 17 Jul 2022
 *      Author: Santiago Esteva
 */

#include "../inc/OS_API.h"

/*************************************************************************************************
	 *  @brief Retardo de la tarea actual
     *
     *  @details
     *   Pone un retardo especificado en número de ticks a la tarea actual
	 * 	 Si la funcion se llama desde una IRQ, se ejecuta el errorHock correspondiente
     *
	 *  @param 		*tarea	Puntero a la estructura de la tarea.
	 *  @param 		ticks_delay	cantidad de ticks de sistema de retardo
	 *  @return     None
***************************************************************************************************/
void Delay(uint32_t ticks_delay){
	tarea * p_actualTask;

	// Si la tarea se llama desde un IRQ y se encuentra llena. Genera un error del OS
	if (os_Estado() == OS_INTERRUPT)os_Error(OS_ERR_DELAY_FROM_IRQ);

	p_actualTask = (tarea*)os_ActualTask();
	os_blockedTask(p_actualTask,ticks_delay);
}

/*************************************************************************************************
	 *  @brief Inicialización de semáforo binario
     *
     *  @details
     *   Inicializa un semáforo binario colocano el estado en tomado (true)
     *
	 *  @param 		semaforo_binario*	puntero al semaforo binario a inicializar
	 *  @return     None
***************************************************************************************************/
void Init_Semaforo_Bin(semaforo_bin* semaforo_binario){
	semaforo_binario->estado = true;
	semaforo_binario->tarea_usa = NULL;
}

/*************************************************************************************************
	 *  @brief Tomar el semáforo binario
     *
     *  @details
     *   Toma el semáforo, en caso de encontrarse tomado la tarea que toma el semáforo pasa a estar en ready
	 * 	 Si la funcion se llama desde una IRQ, y el semáforo está tomado, se ejecuta el errorHock correspondiente
	 * 	 En caso de tomar el semaforo sin problema desde una IRQ, se hace un rescheduling
     *
	 *  @param 		semaforo_binario*	puntero al semaforo binario a ser tomado
	 *  @return     None
***************************************************************************************************/
void Take_Semaforo_Bin(semaforo_bin* semaforo_binario){
	tarea * p_actualTask;
	p_actualTask = (tarea*)os_ActualTask();
	estadoOS EstadoActualOS;

	EstadoActualOS = os_Estado();
	/*
	 * Mientras el semáforo este tomado, la tarea debe permanecer bloqueada
	*/
	while(semaforo_binario->estado){
		// Si la tarea se llama desde un IRQ y se encuentra tomado. Genera un error del OS
		if (EstadoActualOS == OS_INTERRUPT)os_Error(OS_ERR_SEM_BIN_TOMADO);
		
		os_enter_critical();
		semaforo_binario->tarea_usa = p_actualTask;
		os_blockedTask(p_actualTask,MAX_TICKS_SEM);
		os_exit_critical();
		CpuYield();
	}
	/*
	 * Si el semaforo no esta tomado, la tarea lo toma
	*/
	semaforo_binario->estado = true;
	// Si la tarea que lo toma es una IRQ, se llama a rescheduling
	if (EstadoActualOS == OS_INTERRUPT)CpuYield();
}

/*************************************************************************************************
	 *  @brief Liberar el semáforo binario
     *
     *  @details
     *   Libera el semáforo, en caso de encontrarse tomado la tarea que toma el semáforo pasa a estar en ready
     *
	 *  @param 		semaforo_binario*	puntero al semaforo binario a ser liberado
	 *  @return     None
***************************************************************************************************/
void Give_Semaforo_Bin(semaforo_bin* semaforo_binario){
	if(semaforo_binario->estado) os_releaseTask(semaforo_binario->tarea_usa);
	semaforo_binario->estado = false;
	
	CpuYield();
}


/*************************************************************************************************
	 *  @brief Inicialización de semáforo contador
     *
     *  @details
     *   Inicializa un semáforo contador con el valor máximo de cuentas y el valor inicial
     *
	 *  @param 		semaforo_contador*	puntero al semaforo contador a inicializar
	 * 	@param 		max_count		cantidad máxima de cuentas del semáforo
	 * 	@param 		incial_count	valor inicial de la cuenta
	 *  @return     None
***************************************************************************************************/
void Init_Semaforo_Cont(semaforo_cont* semaforo_contador,uint16_t max_count, uint16_t incial_count){
	semaforo_contador->contador = incial_count;
	semaforo_contador->maximo = max_count;
}

/*************************************************************************************************
	 *  @brief Tomar el semáforo contador
     *
     *  @details
     *   Toma el semáforo, en caso de encontrarse tomado la tarea que toma el semáforo pasa a estar en ready
	 * 	 En caso de ejecutar la funcion por una IRQ, se ejecuta un rescheduling al finalizar
     *
	 *  @param 		semaforo_contador*	puntero al semaforo binario a ser tomado
	 *  @return     None
***************************************************************************************************/
void Take_Semaforo_Cont(semaforo_cont* semaforo_contador){
	// En caso de que el contador no sea 0, decremento
	if (semaforo_contador->contador>0)semaforo_contador->contador--;

	// En caso de que se haya llamado desde una IRQ, hago un rescheduling
	if (os_Estado() == OS_INTERRUPT)CpuYield();
}

/*************************************************************************************************
	 *  @brief Liberar el semáforo contador
     *
     *  @details
     *   Libera el semáforo
     *   En caso de ejecutar la funcion por una IRQ, se ejecuta un rescheduling al finalizar
	 * 
	 *  @param 		semaforo_contador*	puntero al semaforo contador a ser liberado
	 *  @return     None
***************************************************************************************************/
void Give_Semaforo_Cont(semaforo_cont* semaforo_contador){
	// En caso de que el contador no haya llegado al maximo, aumento su valor
	if (semaforo_contador->contador <= semaforo_contador->maximo){
		semaforo_contador->contador++;
	}

	// En caso de que se haya llamado desde una IRQ, hago un rescheduling
	if (os_Estado() == OS_INTERRUPT)CpuYield();
}

/*************************************************************************************************
	 *  @brief Inicialización de una cola
     *
     *  @details
     *   Inicializa una cola de datos enteros 
     *
	 *  @param 		p_cola*			puntero a la cola a inicializar
	 *  @param 		tipo_dato		tipo de dato en cantidad de bytes
	 *  @param 		cant_datos		cantidad de datos que almacena la cola
	 *  @return     estado de la inicializacion:
	 * 							- True: la cantida de datos del tipo_dato no excede el tamaño de N_MAX_COLA
	 * 							- False: la cantida de datos del tipo_dato EXCEDE el tamaño de N_MAX_COLA 
***************************************************************************************************/
bool Init_Cola(	cola* p_cola, uint16_t	tipo_dato, uint16_t	cant_datos){		
	uint16_t cant_datos_total;
	
	cant_datos_total = tipo_dato*cant_datos;
	// Corroboro que el tamaño de la cola a crear sea correcto
	if (N_MAX_COLA < cant_datos_total)return false;
	p_cola->tamanio_dato 	= tipo_dato;
	p_cola->index 			= 0;
	p_cola->posicion_fin 	= cant_datos_total;
	p_cola->tarea_bloqueda	= NULL;
	return true;
}

/*************************************************************************************************
	 *  @brief Enviar un dato a la cola
     *
     *  @details
     *   Enviar el dato a la cola, en caso de no contar con tamaño en la cola se bloque la tarea en la cantidad de ticks
	 * 	enviada como parametro  
	 *  En caso de ejecutar la funcion por una IRQ, se ejecuta un rescheduling al finalizar
     *
	 *  @param 		p_cola*			puntero a la cola a enviar un dato
	 *  @param 		dato*			puntero al dato a enviar, la cantidad debe ser la que se definio en la cola 
	 *  @param 		ticks_wait		cantidad de ticks a esperar en caso de que la cola esté llena 
	 *  @return     estado de la cola al enviar el dato
***************************************************************************************************/
estadoCola Enviar_aCola(cola* p_cola, uint8_t	*dato, uint32_t ticks_wait){
	uint16_t i;
	tarea * p_actualTask;
	estadoCola status;
	uint16_t loop;
	estadoOS EstadoActualOS;

	EstadoActualOS = os_Estado();
	loop = 0;
	while (loop<2)
	{
		if(p_cola->index < p_cola->posicion_fin){
			for (i = 0; i < p_cola->tamanio_dato; i++)
			{
				p_cola->valor[p_cola->index+i] = dato[i];
			}
			p_cola->index = p_cola->index + p_cola->tamanio_dato;
			// Veo cuando se envio y actualiza el estado
			if(loop == 0)status = ENVIO_DATO;
			else status = ENVIO_DATO_TICK;
			//salgo del while
			loop=2;
			// En caso de que haya una tarea bloqueada por la cola, la desbloqueo
			if (p_cola->tarea_bloqueda != NULL){
				os_enter_critical();
				os_releaseTask(p_cola->tarea_bloqueda);
				p_cola->tarea_bloqueda = NULL;
				os_exit_critical();
			}
		}else{
			// Si la tarea se llama desde un IRQ y se encuentra llena. Genera un error del OS
			if (EstadoActualOS == OS_INTERRUPT)os_Error(OS_ERR_COLA_COMPLETA);
			
			// Si el ticks_wait enviado vale 0, la tarea sale.
			if (ticks_wait == 0) loop = 1;
			
			if(loop == 0){
				os_enter_critical();
				p_actualTask = (tarea*)os_ActualTask();
				p_cola->tarea_bloqueda = p_actualTask;
				os_blockedTask(p_actualTask,ticks_wait);
				os_exit_critical();
				CpuYield();
				// Retorno al finalizar el tick, vuelvo a intentar enviar el dato
				loop++;
			}else{
				// En este caso la tarea ya fue bloquead una vez y no se logra enviar el dato
				status = NO_ENVIO_DATO;
				loop = 2;
				p_cola->tarea_bloqueda = NULL;
			}			
		}
	}

	// En caso de que se haya llamado desde una IRQ, hago un rescheduling
	if (EstadoActualOS == OS_INTERRUPT)CpuYield();

	return status;
}

/*************************************************************************************************
	 *  @brief Recibir un dato de la cola
     *
     *  @details
     *   Recibir un dato de la cola
     *
	 *  @param 		p_cola*			puntero a la cola a recibir un dato
	 *  @param 		dato*			puntero a la variable para almacenar el dato recibido, la cantidad debe la que se definio en la cola
	 *  @param 		ticks_wait		cantidad de ticks a esperar en caso de que la cola no tenga datos 
	 *  @return     None
***************************************************************************************************/
estadoCola Recibir_dCola(cola* p_cola, uint8_t	*dato, uint32_t ticks_wait){
	uint16_t i;
	tarea * p_actualTask;
	estadoCola status;
	uint16_t loop;
	estadoOS EstadoActualOS;

	EstadoActualOS = os_Estado();

	loop = 0;
	while (loop<2)
	{
		if(p_cola->index > 0){
			for (i = 1; i <= p_cola->tamanio_dato; i++)
			{
				dato[p_cola->tamanio_dato-i] = p_cola->valor[p_cola->index-i];
			}
			p_cola->index = p_cola->index - p_cola->tamanio_dato;
			// Veo cuando se envio y actualiza el estado
			if(loop == 0)status = RECIBO_DATO;
			else status = RECIBO_DATO_TICK;
			//salgo del while
			loop=2;
			// En caso de que haya una tarea bloqueada por la cola, la desbloqueo
			if (p_cola->tarea_bloqueda != NULL){
				os_releaseTask(p_cola->tarea_bloqueda);
				p_cola->tarea_bloqueda = NULL;
			}
		}else{
			// Si la tarea se llama desde un IRQ y se encuentra vacia. Genera un error del OS
			if (EstadoActualOS == OS_INTERRUPT)os_Error(OS_ERR_COLA_VACIA);

			// Si el ticks_wait enviado vale 0, la tarea sale.
			if (ticks_wait == 0) loop = 1;

			if(loop == 0){
				p_actualTask = (tarea*)os_ActualTask();
				p_cola->tarea_bloqueda = p_actualTask;
				os_blockedTask(p_actualTask,ticks_wait);
				CpuYield();
				// Retorno al finalizar el tick, vuelvo a intentar enviar el dato
				loop++;
			}else{
				// En este caso la tarea ya fue bloquead una vez y no se logra enviar el dato
				status = NO_RECIBO_DATO;
				loop = 2;
			}			
		}
	}

	// En caso de que se haya llamado desde una IRQ, hago un rescheduling
	if (EstadoActualOS == OS_INTERRUPT)CpuYield();
	return status;
}

/*************************************************************************************************
	 *  @brief Forzar el scheduling del OS
     *
     *  @details
     *   Fuersa el scheduling del OS con uso de la funcion en el core que permite ello
     *
	 *  @param 		None
	 *  @return     None
***************************************************************************************************/
void CpuYield(void){
	os_Scheduling();
}
