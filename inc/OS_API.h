/*
 * OS_API.h
 *
 *  Created on: 17 Jul 2022
 *      Author: Santiago Esteva
 */

#ifndef OS_API_H_
#define OS_API_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "board.h"
#include "OS_Core.h"

#define MAX_TICKS_SEM 0xFFFFFFFF
struct _semaforo{
	bool estado; // estado del semáforo
	tarea* tarea_usa;	// tarea que hace uso del semáforo
};

typedef struct _semaforo semaforo;

#define N_MAX_COLA 5

struct _cola{
	uint8_t valor[N_MAX_COLA]; // vector con valores en la cola
	bool cola_completa;
};

typedef struct _cola cola;

/*==================[definicion de prototipos]=================================*/
void Delay(uint8_t );
void Init_Semaforo(semaforo* );
void Take_Semaforo(semaforo* );
void Give_Semaforo(semaforo* );
void Cola(uint8_t );
void CpuYield(void);

#endif /* ISO_I_2020_MSE_OS_INC_MSE_OS_CORE_H_ */
