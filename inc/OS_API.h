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

#define MAX_TICKS_SEM 	0xFFFFFFFF
#define N_MAX_COLA 		512 // Cantidad de bytes máximos de la cola

struct _semaforo_bin{
	bool	estado; // estado del semáforo
	tarea*	tarea_usa;	// tarea que hace uso del semáforo
};

typedef struct _semaforo_bin semaforo_bin;

struct _semaforo_cont{
	uint8_t contador; 	// contador del semáforo
	uint8_t	maximo; 	// valor máximo del contador
};

typedef struct _semaforo_cont semaforo_cont;

struct _cola{
	uint8_t		valor[N_MAX_COLA];	// vector con valores en la cola
	uint16_t	tamanio_dato;		// tamanio del dato a guardar en la cola en cantidad de bytes
	uint16_t	index;				// index que recorre la cola	
	uint16_t	posicion_fin;		// posicion en la cola con el ultimo dato	
	tarea*		tarea_bloqueda;		// puntero a la tarea bloqueado en caso de que exista
};

typedef struct _cola cola;

enum _estadoCola{
	ENVIO_DATO,
	ENVIO_DATO_TICK,
	NO_ENVIO_DATO,
	RECIBO_DATO,
	RECIBO_DATO_TICK,
	NO_RECIBO_DATO
};

typedef enum _estadoCola estadoCola;

/*==================[definicion de prototipos]=================================*/

void Delay(uint8_t );
void Init_Semaforo_Bin(semaforo_bin* );
void Take_Semaforo_Bin(semaforo_bin* );
void Give_Semaforo_Bin(semaforo_bin* );
void Init_Semaforo_Cont(semaforo_cont* ,uint8_t , uint8_t );
void Take_Semaforo_Cont(semaforo_cont* );
void Give_Semaforo_Cont(semaforo_cont* );
bool Init_Cola(	cola* , uint16_t	, uint16_t	);
estadoCola Enviar_aCola(cola* , uint32_t	*, uint32_t );
estadoCola Recibir_dCola(cola* , uint32_t	*, uint32_t );
void CpuYield(void);

#endif
