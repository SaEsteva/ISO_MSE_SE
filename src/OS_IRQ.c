/*
 * OS_IRQ.c
 *
 *  Created on: 17 Jul 2022
 *      Author: Santiago Esteva
 */

#include "../inc/OS_IRQ.h"

static void* irq_handler_vecto[NUM_OF_IRQ];

/*************************************************************************************************
	 *  @brief Agregar una IRQ
     *
     *  @details
     *   Agregar una peticion de interrupcion al OS 
     *
	 *  @param 		IRQ 		número del periférico a habilitar la IRQ
	 *  @param 		irq_handler	puntero al handler de la IRQ a agregar
	 *  @return     Estado de la operacion:
	 * 						True: se logra agregar la IRQ
	 * 						False: ocurre un problmea al intentar añadir la IRQ
***************************************************************************************************/
bool os_AddIRQ(LPC43XX_IRQn_Type IRQ, void* irq_handler){
	bool status;
	// Los IRQ que se pueden agregar corresponden a los de hardware que cuentan con un número positivo de id
	if (IRQ >= 0 && IRQ < NUM_OF_IRQ && irq_handler_vecto[IRQ] == NULL){
		irq_handler_vecto[IRQ] =  irq_handler;
		NVIC_ClearPendingIRQ(IRQ);
		NVIC_EnableIRQ(IRQ);
		status = true;
	}else{
		status = false;
	}
	return status;
}

/*************************************************************************************************
	 *  @brief Remover una IRQ
     *
     *  @details
     *   Remover una peticion de interrupcion del OS 
     *
	 *  @param 		IRQ número del periférico a habilitar la IRQ
	 *  @return     Estado de la operacion:
	 * 						True: se logra remover la IRQ
	 * 						False: ocurre un problmea al intentar añadir la IRQ
***************************************************************************************************/
bool os_RemoveIRQ(LPC43XX_IRQn_Type IRQ){
	bool status;
	// Los IRQ que se pueden agregar corresponden a los de hardware que cuentan con un número positivo de id
	if (IRQ >= 0 && IRQ < NUM_OF_IRQ && irq_handler_vecto[IRQ] != NULL){
		irq_handler_vecto[IRQ] =  NULL;
		NVIC_ClearPendingIRQ(IRQ);
		NVIC_DisableIRQ(IRQ);
		status = true;
	}else{
		status = false;
	}
	return status;
}

/*************************************************************************************************
	 *  @brief Handler del IRQ
     *
     *  @details
     *   Handler del IRQ para todo el sistema operativo
     *
	 *  @param 		IRQ número del periférico al cual especifica el Handler
	 *  @return     None.
***************************************************************************************************/
void os_IRQ_Handler(LPC43XX_IRQn_Type IRQ){
	estadoOS estado_actualOS;
	void (*Handler)(void);
	// Tomo el estado acutal del sistema
	estado_actualOS = os_Estado();
	// Cambio el estado del sistema
	os_NuevoEstado(OS_INTERRUPT);

	//Busco el puntoro al handler correspondiente para ejecutarlo
	Handler = irq_handler_vecto[IRQ];
	// Ejecuto la función
	Handler();
	
	// Restructuro el estado anteriore del OS
	os_NuevoEstado(estado_actualOS);
	
	// Limpio el vector de IRQ que acabo de atender
	NVIC_ClearPendingIRQ(IRQ);	
}


/*************************************************************************************************
	 *  FUNCIONES HANDLER EXTRAIDAS DEL ARCHIVO cr_startup_lpc43xx.c
***************************************************************************************************/

void DAC_IRQHandler(void) os_IRQ_Handler(DAC_IRQn);
void M0APP_IRQHandler(void) os_IRQ_Handler(M0APP_IRQn);
void DMA_IRQHandler(void) os_IRQ_Handler(DMA_IRQn);
void FLASH_EEPROM_IRQHandler(void) os_IRQ_Handler(RESERVED1_IRQn);
void ETH_IRQHandler(void) os_IRQ_Handler(ETHERNET_IRQn);
void SDIO_IRQHandler(void) os_IRQ_Handler(SDIO_IRQn);
void LCD_IRQHandler(void) os_IRQ_Handler(LCD_IRQn);
void USB0_IRQHandler(void) os_IRQ_Handler(USB0_IRQn);
void USB1_IRQHandler(void) os_IRQ_Handler(USB1_IRQn);
void SCT_IRQHandler(void) os_IRQ_Handler(SCT_IRQn);
void RIT_IRQHandler(void) os_IRQ_Handler(RITIMER_IRQn);
void TIMER0_IRQHandler(void) os_IRQ_Handler(TIMER0_IRQn);
void TIMER1_IRQHandler(void) os_IRQ_Handler(TIMER1_IRQn);
void TIMER2_IRQHandler(void) os_IRQ_Handler(TIMER2_IRQn);
void TIMER3_IRQHandler(void) os_IRQ_Handler(TIMER3_IRQn);
void MCPWM_IRQHandler(void) os_IRQ_Handler(MCPWM_IRQn);
void ADC0_IRQHandler(void) os_IRQ_Handler(ADC0_IRQn);
void I2C0_IRQHandler(void) os_IRQ_Handler(I2C0_IRQn);
void SPI_IRQHandler(void) os_IRQ_Handler(SPI_INT_IRQn);
void I2C1_IRQHandler(void) os_IRQ_Handler(I2C1_IRQn);
void ADC1_IRQHandler(void) os_IRQ_Handler(ADC1_IRQn);
void SSP0_IRQHandler(void) os_IRQ_Handler(SSP0_IRQn);
void SSP1_IRQHandler(void) os_IRQ_Handler(SSP1_IRQn);
void UART0_IRQHandler(void) os_IRQ_Handler(USART0_IRQn);
void UART1_IRQHandler(void) os_IRQ_Handler(UART1_IRQn);
void UART2_IRQHandler(void) os_IRQ_Handler(USART2_IRQn);
void UART3_IRQHandler(void) os_IRQ_Handler(USART3_IRQn);
void I2S0_IRQHandler(void) os_IRQ_Handler(I2S0_IRQn);
void I2S1_IRQHandler(void) os_IRQ_Handler(I2S1_IRQn);
void SPIFI_IRQHandler(void) os_IRQ_Handler(RESERVED4_IRQn);
void SGPIO_IRQHandler(void) os_IRQ_Handler(SGPIO_INT_IRQn);
void GPIO0_IRQHandler(void) os_IRQ_Handler(PIN_INT0_IRQn);
void GPIO1_IRQHandler(void) os_IRQ_Handler(PIN_INT1_IRQn);
void GPIO2_IRQHandler(void) os_IRQ_Handler(PIN_INT2_IRQn);
void GPIO3_IRQHandler(void) os_IRQ_Handler(PIN_INT3_IRQn);
void GPIO4_IRQHandler(void) os_IRQ_Handler(PIN_INT4_IRQn);
void GPIO5_IRQHandler(void) os_IRQ_Handler(PIN_INT5_IRQn);
void GPIO6_IRQHandler(void) os_IRQ_Handler(PIN_INT6_IRQn);
void GPIO7_IRQHandler(void) os_IRQ_Handler(PIN_INT7_IRQn);
void GINT0_IRQHandler(void) os_IRQ_Handler(GINT0_IRQn);
void GINT1_IRQHandler(void) os_IRQ_Handler(GINT1_IRQn);
void EVRT_IRQHandler(void) os_IRQ_Handler(EVENTROUTER_IRQn);
void CAN1_IRQHandler(void) os_IRQ_Handler(C_CAN1_IRQn);
void ADCHS_IRQHandler(void) os_IRQ_Handler(ADCHS_IRQn);
void ATIMER_IRQHandler(void) os_IRQ_Handler(ATIMER_IRQn);
void RTC_IRQHandler(void) os_IRQ_Handler(RTC_IRQn);
void WDT_IRQHandler(void) os_IRQ_Handler(WWDT_IRQn);
void M0SUB_IRQHandler(void) os_IRQ_Handler(M0SUB_IRQn);
void CAN0_IRQHandler(void) os_IRQ_Handler(C_CAN0_IRQn);
void QEI_IRQHandler(void) os_IRQ_Handler(QEI_IRQn);