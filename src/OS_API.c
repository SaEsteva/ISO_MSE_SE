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
	 *  @brief Inicialización de semáforo
     *
     *  @details
     *   Inicializa un semáforo binario colocano el estado en tomado (true)
     *
	 *  @param 		semaforo*	puntero al semaforo binario a inicializar
	 *  @return     None
***************************************************************************************************/
void Init_Semaforo(semaforo* semaforo_binario){
	semaforo_binario->estado = true;
	semaforo_binario->tarea_usa = NULL;
}

/*************************************************************************************************
	 *  @brief Tomar el semáforo
     *
     *  @details
     *   Toma el semáforo, en caso de encontrarse tomado la tarea que toma el semáforo pasa a estar en ready
     *
	 *  @param 		semaforo*	puntero al semaforo binario a ser tomado
	 *  @return     None
***************************************************************************************************/
void Take_Semaforo(semaforo* semaforo_binario){
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
	 *  @brief Liberar el semáforo
     *
     *  @details
     *   Libera el semáforo, en caso de encontrarse tomado la tarea que toma el semáforo pasa a estar en ready
     *
	 *  @param 		semaforo*	puntero al semaforo binario a ser liberado
	 *  @return     None
***************************************************************************************************/
void Give_Semaforo(semaforo* semaforo_binario){
	if(semaforo_binario->estado) os_releaseTask(semaforo_binario->tarea_usa);
	semaforo_binario->estado = false;
	/*
	 * Cuando se aplique inversión de prioridades, se debe agregar abajo de esta linea para aumentar la prioridad
	 * de la tarea con el semaforo al valor máximo para que se ejecute
	*/

	CpuYield();
}

/*************************************************************************************************
	 *  @brief None
     *
     *  @details
     *   None
     *
	 *  @param 		None
	 *  @return     None
***************************************************************************************************/
void Cola(uint8_t ticks_delay){

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
