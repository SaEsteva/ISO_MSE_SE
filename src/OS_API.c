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
     *
	 *  @param 		*tarea	Puntero a la estructura de la tarea.
	 *  @param 		ticks_delay	cantidad de ticks de sistema de retardo
	 *  @return     None
***************************************************************************************************/
void Delay(uint8_t ticks_delay){
	tarea * p_actualTask;
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
     *
	 *  @param 		semaforo_binario*	puntero al semaforo binario a ser tomado
	 *  @return     None
***************************************************************************************************/
void Take_Semaforo_Bin(semaforo_bin* semaforo_binario){
	tarea * p_actualTask;
	p_actualTask = (tarea*)os_ActualTask();
	/*
	 * Mientras el semáforo este tomado, la tarea debe permanecer bloqueada
	*/
	while(semaforo_binario->estado){
		semaforo_binario->tarea_usa = p_actualTask;
		os_blockedTask(p_actualTask,MAX_TICKS_SEM);
		CpuYield();
	}
	/*
	 * Si el semaforo no esta tomado, la tarea lo toma
	*/
	semaforo_binario->estado = true;
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
	/*
	 * Cuando se aplique inversión de prioridades, se debe agregar abajo de esta linea para aumentar la prioridad
	 * de la tarea con el semaforo al valor máximo para que se ejecute
	*/

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
void Init_Semaforo_Cont(semaforo_cont* semaforo_contador,uint8_t max_count, uint8_t incial_count){
	semaforo_contador->contador = incial_count;
	semaforo_contador->maximo = max_count;
}

/*************************************************************************************************
	 *  @brief Tomar el semáforo contador
     *
     *  @details
     *   Toma el semáforo, en caso de encontrarse tomado la tarea que toma el semáforo pasa a estar en ready
     *
	 *  @param 		semaforo_contador*	puntero al semaforo binario a ser tomado
	 *  @return     None
***************************************************************************************************/
void Take_Semaforo_Cont(semaforo_cont* semaforo_contador){
	// En caso de que el contador no sea 0, decremento
	if (semaforo_contador->contador>0)semaforo_contador->contador--;
}

/*************************************************************************************************
	 *  @brief Liberar el semáforo contador
     *
     *  @details
     *   Libera el semáforo
     *
	 *  @param 		semaforo_contador*	puntero al semaforo contador a ser liberado
	 *  @return     None
***************************************************************************************************/
void Give_Semaforo_Cont(semaforo_cont* semaforo_contador){
	// En caso de que el contador no haya llegado al maximo, aumento su valor
	if (semaforo_contador->contador <= semaforo_contador->maximo)semaforo_contador->contador++;
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
     *
	 *  @param 		p_cola*			puntero a la cola a enviar un dato
	 *  @param 		dato*			puntero al dato a enviar, la cantidad debe ser la que se definio en la cola 
	 *  @param 		ticks_wait		cantidad de ticks a esperar en caso de que la cola esté llena 
	 *  @return     estado de la cola al enviar el dato
***************************************************************************************************/
estadoCola Enviar_aCola(cola* p_cola, uint32_t	*dato, uint32_t ticks_wait){
	uint16_t i;
	tarea * p_actualTask;
	estadoCola status;
	uint16_t loop;

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
				os_releaseTask(p_cola->tarea_bloqueda);
				p_cola->tarea_bloqueda = NULL;
			}
		}else{
			if(loop == 0){
				p_actualTask = (tarea*)os_ActualTask();
				p_cola->tarea_bloqueda = p_actualTask;
				os_blockedTask(p_actualTask,ticks_wait);
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
estadoCola Recibir_dCola(cola* p_cola, uint32_t	*dato, uint32_t ticks_wait){
	uint16_t i;
	tarea * p_actualTask;
	estadoCola status;
	uint16_t loop;

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
