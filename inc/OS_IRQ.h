/*
 * OS_IRQ.h
 *
 *  Created on: 17 Jul 2022
 *      Author: Santiago Esteva
 */

#ifndef OS_IRQ_H_
#define OS_IRQ_H_

#include "board.h"
#include "OS_Core.h"
#include "OS_API.h"
#include "cmsis_43xx.h"

#define NUM_OF_IRQ 53 // dato extraído del enum LPC43XX_IRQn_Type en "cmsis_43xx.h"
/*==================[definicion de prototipos]=================================*/

bool os_AddIRQ(LPC43XX_IRQn_Type , void* );
bool os_RemoveIRQ(LPC43XX_IRQn_Type );
void os_IRQ_Handler(LPC43XX_IRQn_Type );

#endif
