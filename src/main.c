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

#define TEC1_PORT_NUM   0
#define TEC1_BIT_VAL    4

#define TEC2_PORT_NUM   0
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
	while (1) {
		Delay(TIMELED);
		gpioToggle(LED1);
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
		while (semaforo_sensor_estrusor.contador>0)
		{
			Take_Semaforo_Cont(&semaforo_sensor_estrusor);
		}
		status = Enviar_aCola(&botellas_desechadas, &cantidad, 0);
		if (status == NO_ENVIO_DATO){
			// Error, la cola se lleno. Se prende el LED Rojo
			gpioWrite(LEDG,false);
			gpioWrite(LEDR,true);
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
		while (semaforo_sensor_descarte.contador>0)
		{
			Take_Semaforo_Cont(&semaforo_sensor_descarte);
		}
		status = Enviar_aCola(&botellas_fabricadas, &cantidad, 0);
		if (status == NO_ENVIO_DATO){
			// Error, la cola se lleno. Se prende el LED Rojo
			gpioWrite(LEDG,false);
			gpioWrite(LEDR,true);
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
		Take_Semaforo_Bin(&semaforo_UART);
		/* 
		 * El semaforo de la UART fue liberado por lo tanto debo extraer las cantidades de las colas y enviar el mensaje
		*/
		// Leo la cola con datos fabricados
		status = RECIBO_DATO;		
		auxiliar = 0;		
		os_enter_critical();
		while (status != NO_RECIBO_DATO)
		{
			status = Recibir_dCola(&botellas_fabricadas, &auxiliar, 0);
			Cant_botellas_ok = Cant_botellas_ok + auxiliar;
		}
		os_exit_critical();		
		// Leo la cola con datos descartados
		status = RECIBO_DATO;		
		auxiliar = 0;
		os_enter_critical();
		while (status != NO_RECIBO_DATO)
		{
			status = Recibir_dCola(&botellas_desechadas, &auxiliar, 0);
			Cant_botellas_no = Cant_botellas_no + auxiliar;
		}
		os_exit_critical();		
		// Calculo las botellas totales
		Cant_botellas_totales = Cant_botellas_ok - Cant_botellas_no;
		// Envio el mensaje
		printf( "Cant Totales: %d\t Cant Fabricadas: %d\t Cant Descartadas: %d\n",Cant_botellas_totales,Cant_botellas_ok,Cant_botellas_no );
		// Reseteo los contadores
		Cant_botellas_totales = Cant_botellas_ok = Cant_botellas_no = 0; 
	}
}

void Handler_sensor2(void)  {
	gpioToggle(LED2);
	Give_Semaforo_Cont(&semaforo_sensor_estrusor);
}
void Handler_sensor1(void)  {
	gpioToggle(LED3);
	Give_Semaforo_Cont(&semaforo_sensor_descarte);
}

void Handler_Comunicacion(void)  {
	static uint8_t BufferCounter = 0;
	uint8_t readByte;
	readByte = uartRxRead( UART_USB );
	if (BufferCounter < CANT_BUFFER){
		BufferUart[BufferCounter] = (char)readByte;
		if(readByte == StopMessage){
			bufferSizeUart = BufferCounter;
			BufferCounter = 0;
			Give_Semaforo_Bin(&semaforo_UART);
		}else{
			BufferCounter++;
		}
	}else{
		BufferCounter = 0;
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
    // // Seteo un callback al evento de recepcion y habilito su interrupcion
    // uartCallbackSet(UART_USB, UART_RECEIVE, onRx, NULL);
    // // Habilito todas las interrupciones de UART_USB
    // uartInterrupt(UART_USB, true);
	// Button objects
	// button_t Sensor1;
   	// button_t Sensor2;

    // // Sensor 0 is handled with callbacks
    // buttonInit( &Sensor1,                  // Button structure (object)
    //            SENSORESTRUSOR, BUTTON_LOGIC,       // Pin and electrical connection
    //            SENSORSCANTIME,                          // Button scan time [ms]
    //            FALSE,                        // checkPressedEvent
    //            FALSE,                        // checkReleasedEvent
    //            TRUE,                        // checkHoldPressedEvent
    //            SENSORPRESSTIME,                        // holdPressedTime [ms]
    //            0,    // pressedCallback
    //            0,   // releasedCallback
    //            Handler_sensor1 // holdPressedCallback
    //          );

	// // Sensor 1 is handled with callbacks
    // buttonInit( &Sensor2,SENSORDESCARTE, BUTTON_LOGIC,SENSORSCANTIME,FALSE,FALSE,TRUE,SENSORPRESSTIME,0,0,Handler_sensor2);

	/*
	 * Seteamos la interrupcion 0 para el flanco descendente en la tecla 1
	 */
	Chip_SCU_GPIOIntPinSel( 0, TEC1_PORT_NUM, TEC1_BIT_VAL );
	Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 6 ) ); // INT0 flanco descendente
	Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH( 6 ) );
	Chip_PININT_EnableIntLow( LPC_GPIO_PIN_INT, PININTCH( 6 ) );

	// /*
	//  * Seteamos la interrupcion 1 para el flanco ascendente en la tecla 1
	//  */
	// Chip_SCU_GPIOIntPinSel( 1, TEC1_PORT_NUM, TEC1_BIT_VAL );
	// Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 1 ) ); // INT1 flanc
	// Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH( 1 ) );
	// Chip_PININT_EnableIntHigh( LPC_GPIO_PIN_INT, PININTCH( 1 ) );
	
	/*
	 * Seteamos la interrupcion 0 para el flanco descendente en la tecla 1
	 */
	Chip_SCU_GPIOIntPinSel( 0, TEC2_PORT_NUM, TEC2_BIT_VAL );
	Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 7 ) ); // INT0 flanco descendente
	Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH( 7 ) );
	Chip_PININT_EnableIntLow( LPC_GPIO_PIN_INT, PININTCH( 7 ) );

	// /*
	//  * Seteamos la interrupcion 1 para el flanco ascendente en la tecla 1
	//  */
	// Chip_SCU_GPIOIntPinSel( 1, TEC1_PORT_NUM, TEC1_BIT_VAL );
	// Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 1 ) ); // INT1 flanc
	// Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH( 1 ) );
	// Chip_PININT_EnableIntHigh( LPC_GPIO_PIN_INT, PININTCH( 1 ) );

	// gpioObtainPinInit( pin, &pinNamePort, &pinNamePin, &func,
    //                   &gpioPort, &gpioPin );
	// { {1, 0}, FUNC0, {0, 4} },   // 36   TEC1    TEC_1  
    // { {1, 1}, FUNC0, {0, 8} },   // 37   TEC2    TEC_2 
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

	status = os_AddIRQ(USART2_IRQn, Handler_Comunicacion);
	if (status) gpioWrite(LEDG,true);
	else errorHook();
	status = os_AddIRQ(PIN_INT6_IRQn, Handler_sensor1);
	if (status) gpioToggle(LEDG);
	else errorHook();
	status = os_AddIRQ(PIN_INT7_IRQn, Handler_sensor2);
	if (status) gpioToggle(LEDG);
	else errorHook();

	// Inicializao el semaforo binario que habilitará la UART
	Init_Semaforo_Bin(&semaforo_UART);

	// Inicializao los semaforos contares para los sensores
	Init_Semaforo_Cont(&semaforo_sensor_estrusor,CANTMAXIMABOTELLASOK,0);
	Init_Semaforo_Cont(&semaforo_sensor_descarte,CANTMAXIMABOTELLASDESCARTE,0);

	// Inicializo las colas que guardan los datos cada el tiempo definido
	status = Init_Cola(	&botellas_fabricadas, TIPODATOCOLA, CANTIDADCOLAOK);
	if (status) gpioToggle(LEDG);
	else errorHook();
	status = Init_Cola(	&botellas_desechadas, TIPODATOCOLA, CANTIDADCOLADESCARTE);
	if (status) gpioToggle(LEDG);
	else errorHook();

	// Inicializo el SO
	os_SistemInit();

	while (1) {
	}
}

/** @} doxygen end group definition */

/*==================[end of file]============================================*/
