/*
 * main.c
 *
 *  Created on: 17 Jul 2022
 *      Author: Santiago Esteva
 */
/*==================[inclusions]=============================================*/

#include <stdlib.h>
#include "main.h"
#include "board.h"
#include "OS_Core.h"
#include "OS_API.h"
#include "sapi.h"
#include "sapi_button.h"
#include "sapi_gpio.h"
#include "sapi_uart.h"
#include "sapi_peripheral_map.h"

#define TEC1_PORT_NUM   1
#define TEC1_BIT_VAL    4

#define TEC2_PORT_NUM   3
#define TEC2_BIT_VAL    8

/*==================[macros and definitions]=================================*/

char BufferUart[CANT_BUFFER];
uint8_t bufferSizeUart;
/*==================[Global data declaration]==============================*/

tarea tareaStructLed;		//estructura tarea de la tarea tareaLed
tarea tareaStructContador1;	//estructura tarea de la tarea ContarBotellasEstrusor
tarea tareaStructContador2;	//estructura tarea de la tarea ContarBotellasDescarte
tarea tareaStructUart;		//estructura tarea de la tarea ResponderUART

semaforo_bin semaforo_UART;
semaforo_cont semaforo_sensor_descarte;
semaforo_cont semaforo_sensor_estrusor;

cola botellas_fabricadas;
cola botellas_desechadas;
/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[Definicion de tareas para el OS]==========================*/
void tareaLed(void)  {
	uint16_t n_loop;
	n_loop = 0;
	while (1) {
		Delay(TIMELED);
		gpioToggle(LED1);
		n_loop++;
		if(n_loop >= TIEMPOUART){
			Give_Semaforo_Bin(&semaforo_UART);
			n_loop = 0;
		}
	}
}

void ContarBotellasEstrusor(void)  {
	uint32_t cantidad;
	estadoCola status;
	cantidad = 0;
	while (1) {
		Delay(TIEMPOMUESTRASBOTELLAS);
		os_enter_critical();
		cantidad = semaforo_sensor_estrusor.contador;
		if (cantidad > 0) {
			while (semaforo_sensor_estrusor.contador>0)
					{
						Take_Semaforo_Cont(&semaforo_sensor_estrusor);
					}
					status = Enviar_aCola(&botellas_fabricadas, &cantidad, 0);
					if (status == NO_ENVIO_DATO){
						// Error, la cola se lleno. Se prende el LED Rojo
						gpioWrite(LEDG,false);
						gpioWrite(LEDR,true);
					}
		}
		os_exit_critical();
	}
}

void ContarBotellasDescarte(void)  {
	uint32_t cantidad;
	estadoCola status;
	cantidad = 0;
	while (1) {
		Delay(TIEMPOMUESTRASBOTELLAS);
		os_enter_critical();
		cantidad = semaforo_sensor_descarte.contador;
		if (cantidad > 0) {
			while (semaforo_sensor_descarte.contador>0)
			{
				Take_Semaforo_Cont(&semaforo_sensor_descarte);
			}
			status = Enviar_aCola(&botellas_desechadas, &cantidad, 0);
			if (status == NO_ENVIO_DATO){
				// Error, la cola se lleno. Se prende el LED Rojo
				gpioWrite(LEDG,false);
				gpioWrite(LEDR,true);
			}
		}
		os_exit_critical();
	}
}

void ResponderUART(void)  {
	int Cant_botellas_totales,Cant_botellas_ok,Cant_botellas_no;
	uint32_t auxiliar;
	estadoCola status;

	Cant_botellas_totales = Cant_botellas_ok = Cant_botellas_no = 0;
	auxiliar = 0;
	while (1) {
		//Delay(TIEMPOUART);
		Take_Semaforo_Bin(&semaforo_UART);
		/* 
		 * El semaforo de la UART fue liberado por lo tanto debo extraer las cantidades de las colas y enviar el mensaje
		*/
		// Leo la cola con datos fabricados
		status = RECIBO_DATO;		
		auxiliar = 0;
		while (status != NO_RECIBO_DATO)
		{
			status = Recibir_dCola(&botellas_fabricadas, &auxiliar, 0);
			Cant_botellas_ok = Cant_botellas_ok + auxiliar;
		}
		// Leo la cola con datos descartados
		status = RECIBO_DATO;		
		auxiliar = 0;
		while (status != NO_RECIBO_DATO)
		{
			status = Recibir_dCola(&botellas_desechadas, &auxiliar, 0);
			Cant_botellas_no = Cant_botellas_no + auxiliar;
		}
		// Calculo las botellas totales
		Cant_botellas_totales = Cant_botellas_ok - Cant_botellas_no;
		// Imprimo el mensaje
		uartWriteByte(UART_USB,(uint8_t)'T');
		uartWriteByteArray(UART_USB,&Cant_botellas_totales,4);
		uartWriteByte(UART_USB,(uint8_t)'F');
		uartWriteByteArray(UART_USB,&Cant_botellas_ok,4);
		uartWriteByte(UART_USB,(uint8_t)'D');
		uartWriteByteArray(UART_USB,&Cant_botellas_no,4);
		uartWriteByte(UART_USB,'\n');

		// Envio el mensaje
		// Por alg�n motivo el uso de las funcioens printf o sprintf llevan a hard fault
		//printf( "Cant Totales: %d\t Cant Fabricadas: %d\t Cant Descartadas: %d\n",Cant_botellas_totales,Cant_botellas_ok,Cant_botellas_no );
		// Reseteo los contadores
		Cant_botellas_totales = Cant_botellas_ok = Cant_botellas_no = 0; 
	}
}

void Handler_sensor2(void)  {

	if ( Chip_PININT_GetRiseStates( LPC_GPIO_PIN_INT ) & PININTCH3 )
	{
		Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH3 );
		gpioToggle(LED3);
		Give_Semaforo_Cont(&semaforo_sensor_descarte);
	}

}
void Handler_sensor1(void)  {

	if ( Chip_PININT_GetRiseStates( LPC_GPIO_PIN_INT ) & PININTCH1 )
	{
		Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH1 );
		gpioToggle(LED2);
		Give_Semaforo_Cont(&semaforo_sensor_estrusor);
	}
}

/** @brief hardware initialization function
 *	@return none
 */
static void initHardware(void)  {
	Board_Init();
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / MILISEC);		//systick 1ms

	// Inicializar UART_USB a 115200 baudios
    uartInit( UART_USB, UART_BR );     


	//Inicializamos las interrupciones (LPCopen)
	Chip_PININT_Init( LPC_GPIO_PIN_INT );

	//Inicializamos de cada evento de interrupcion (LPCopen)

	/* Machete:
	GLOBAL! extern pinInitGpioLpc4337_t gpioPinsInit[];
	Chip_SCU_GPIOIntPinSel( j,  gpioPinsInit[i].gpio.port, gpioPinsInit[i].gpio.pin );   // TECi
	Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( j ) );                      // INTj (canal j -> hanlder GPIOj)       //Borra el pending de la IRQ
	Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH( j ) );                      // INTj //Selecciona activo por flanco
	Chip_PININT_EnableIntLow( LPC_GPIO_PIN_INT, PININTCH( j ) );                        // INTj //Selecciona activo por flanco descendente
	Chip_PININT_EnableIntHigh( LPC_GPIO_PIN_INT, PININTCH( j ) );                       // INTj //Selecciona activo por flanco ascendente
	*/

	/*Seteo la interrupción para el flanco descendente
						channel, GPIOx,                        [y]    <- no es la config del pin, sino el nombre interno de la señal
							|       |                           |
							|       |                           |    */

	// // TEC1 FALL
	// Chip_SCU_GPIOIntPinSel( 0, 0, 4 ); 	//(Canal 0 a 7, Puerto GPIO, Pin GPIO)
	// Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH0 ); //Se configura el canal para que se active por flanco
	// Chip_PININT_EnableIntLow( LPC_GPIO_PIN_INT, PININTCH0 ); //Se configura para que el flanco sea el de bajada

	// TEC1 RISE
	Chip_SCU_GPIOIntPinSel( TEC1_PORT_NUM, 0, TEC1_BIT_VAL );	//(Canal 0 a 7, Puerto GPIO, Pin GPIO)
	Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH1 ); //Se configura el canal para que se active por flanco
	Chip_PININT_EnableIntHigh( LPC_GPIO_PIN_INT, PININTCH1 ); //En este caso el flanco es de subida

	// // TEC2 FALL
	// Chip_SCU_GPIOIntPinSel( 2, 0, 8 );
	// Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH2 );
	// Chip_PININT_EnableIntLow( LPC_GPIO_PIN_INT, PININTCH2 );

	// TEC2 RISE
	Chip_SCU_GPIOIntPinSel( TEC2_PORT_NUM, 0, TEC2_BIT_VAL );
	Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH3 );
	Chip_PININT_EnableIntHigh( LPC_GPIO_PIN_INT, PININTCH3 );
}

/*============================================================================*/

int main(void)  {

	initHardware();// Inicializar la placa

    bool status;
	// Tarea de baja prioridad que toglea el LED Azul
	os_InitTarea(&tareaStructLed,tareaLed,1,1,"tareaLed");

	// Tarea que realiza el conteo de la cantidad de botellas que se contador por el sensor del estrusor
	os_InitTarea(&tareaStructContador1,ContarBotellasEstrusor,2,4,"tareaContadorEstrusor");

	// Tarea que realiza el conteo de la cantidad de botellas que se contador por el sensor del descarte
	os_InitTarea(&tareaStructContador2,ContarBotellasDescarte,3,4,"tareaContadorDescarte");

	// Tarea que responde el mensaje por UART
	os_InitTarea(&tareaStructUart,ResponderUART,6,3,"tareaUart");

	// status = os_AddIRQ(USART2_IRQn, Handler_Comunicacion);
	// if (!status)errorHook();

	status = os_AddIRQ(PIN_INT1_IRQn, Handler_sensor1);
	if (!status)errorHook();
	
	status = os_AddIRQ(PIN_INT3_IRQn, Handler_sensor2);
	if (!status)errorHook();

	// Inicializao el semaforo binario que habilitará la UART
	Init_Semaforo_Bin(&semaforo_UART);

	// Inicializao los semaforos contares para los sensores
	Init_Semaforo_Cont(&semaforo_sensor_estrusor,CANTMAXIMABOTELLASOK,0);
	Init_Semaforo_Cont(&semaforo_sensor_descarte,CANTMAXIMABOTELLASDESCARTE,0);

	// Inicializo las colas que guardan los datos cada el tiempo definido
	status = Init_Cola(	&botellas_fabricadas, TIPODATOCOLA, CANTIDADCOLAOK);
	if (!status)errorHook();
	status = Init_Cola(	&botellas_desechadas, TIPODATOCOLA, CANTIDADCOLADESCARTE);
	if (!status)errorHook();

	gpioToggle(LEDG);
	// Inicializo el SO
	os_SistemInit();

	while (1) {
	}
}

/** @} doxygen end group definition */

/*==================[end of file]============================================*/
