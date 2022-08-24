# ISO_MSE_SE
Sistema operativo desarrollado por Santiago Esteva como trabajo de la materia Implementacion de sistemas operativos 7ma Coherte 2022 de la Maestría en Sistemas Embebidos de la Facultad de Ingeniería, Universidad de Buenos Aires.


# Sistema Operativo
El sistema operativo permite crear una cantidad máxima de **MAX_NUM_TASK** tareas que correran en modo round robin dependiendo los grupos de prioridad con tres tipos de estados:
- TAREA_READY 
- TAREA_RUNNING 
- TAREA_BLOCKED

El control del SO se realiza en el Core mediante la estructura **osControl**. 

## Prioridades
Las tareas del SO pueden contar con **MAX_PRIOR_TASK** = 4 niveles de prioridades de las cuales van desde el valor mínimo de **p_TaskIdle** al máximo que es 4. El scheduler ejecutará desde mayor prioridad a menor prioridad con un esquema de round robin entre las tareas de la misma prioridad e irán disminuyendo de escalones a medida que las tareas de mayor prioridad pasan a estar bloqueadas.

**NOTA: En caso de asignar una prioridad erronea, la tarea será asignada con la priodidad mínima.**

## Ejecución del Sistema Operativo
La ejecución del SO se implementa con interupciones del Systick. Realizando modificaciones en el Handler del systick se actualiza el estado del SO por cada tick, se llama al scheduler que define la nueva tarea a ejecutar y habilita la interrupcion del PendSV para el cambio de contexto

La interrpción PendSV se realiza en assembler utilizando el proceso de lazy stacking y en los casos correspondientes almacena y restaura los registros de propósito general **{r4-r11,lr}** y los registros FPU **{s16-s31}**.
Para realizar el cambio de contexto se implementa una funcion en C (**getContextoSiguiente()**, llamada desde el **PendSV_Habdler.sx**) que actualiza la estructura del SO y devuelve al MSP el stack de la nueva tarea a ejecutar. 

## Hooks
El SO operativo cuenta con los siguientes Hook:
- returnHook(): Tarea ejecutada luego de retornar una tarea del SO, en caso de no incorporar una loop infinito en las tareas creadas se finalizará en este hook a la espera de una nueva interrupcion del sistema.
- errorHook(): Tarea ejecutada cuando ocurre un error, abajo se encuentran los posibles errores de sistema que ejcutan esta tarea
- TickHook(): Tarea ejecutrada por el Systick cuando se encuentra definido en 1 el **configUSE_TICK_HOOK**. Por default ejecutará un no operation ("NOP").

### OS ERRORS
En caso de obtener un **errorHook()** el SO cuenta con una variable de error la cual me permite conocer la causa del mismo, los posibles errores son:
- OS_ERR_N_TAREAS (1) : se pretende crear una cantidad de tareas mayor a **MAX_NUM_TASK**.
- OS_ERR_COLA_COMPLETA (2) : se ejecuta la funcion **Enviar_aCola()** desde una IRQ y la cola se encuentra completa.
- OS_ERR_COLA_VACIA (3) : se ejecuta la funcion **Recibir_dCola()** desde una IRQ y la cola se encuentra vacia.
- OS_ERR_SEM_BIN_TOMADO (4) : se ejecuta la funcion **Take_Semaforo_Bin()** desde una IRQ y el semáforo se encuentra tomado.
- OS_ERR_DELAY_FROM_IRQ (5) : se ejecuta la funcion **Delay()** desde una IRQ.


## Tarea IDLE
Cuando el SO no cuenta con tareas a ejecutar debido a:
- No existen tareas creadas por el usuario
- Ninguna tarea se encuentra en estado READY o RUNNING

El SO cuenta con la tarea IDLE que se ejecutaría en estos casos, dando un uso de bajo consumo al CPU cuando no hay tareas del SO en estado de ejecución. Esta tarea sólo se encontrará en estado RUNNING o READY y cuenta con la prioridad mas baja. 
Puede ser redefinida para efectuar acciones de segundo plano (background) o definir perfiles de ejecución para aplicar técnicas de bajo consumo (frecuencia CPU, periféricos, etc). Por default la trea ejecuta la instrucción WFI (wait for interrupt).

## Tarea BLOCKED
Para pasar una tarea a bloqueado se implementa la funcion **os_blockedTask()** que recibe como puntero la estructura de la tarea y la cantidad de ticks que se pretende bloquear la tarea. Actualiza el estado, almacena el número de ticks recibidos y se queda en espera a una nueva interrupcion con __WFI()

## Tarea RELEASE
Para liberar una tarea del estado bloqueado se implementa la funcion **os_releaseTask()** que recibe como puntero la estructura de la tarea que se pretende liberar. Actualiza el estado, resetea el número de ticks recibidos.

## Zona Crítica
Se implementan las funciones para ingresar (**os_enter_critical()**) y salir (**os_exit_critical()**) de una zona crítica que desabilitan y habilitan las interrupciones respectivamente. 

Se agrega el ingreso a la zona crítica en la función **PendSV_Handler.sx** con los respectivos comandos en Assembly.


# API
Se dearrolla la biblioteca de apis con las funcionalidades que se presentan en esta seccion.
## Delay
Genera un delay durante una cantidad de ticks de sistema indicado como parámetro de la funcion sobre la tarea que la invoca, el estado de la tarea durante este tiempo es **TAREA_BLOCKED**. 

Hace uso de la funcion **os_blockedTask()** del core.

**Nota: Esta funcion no puede ser llamada desde una IRQ, en caso de hacerlo se ejecutra el errorHook correspondiente.**

## Semaforo
### Binario
Se implementa un semáforo binario con la posiblidad de ser tomado y liberado utilizando las respectivas funciones, en caso de que el semaforo se encuentre tomado durante el llamado a la funcion **Take_Semaforo_Bin()**, la tarea pasará al estado **TAREA_BLOCKED** hasta que el semáforo sea liberado. En caso de que nunca sea liberado el semáforo, la tareá volverá al estado READY luego de **4294967295** ticks de sistema (0xFFFFFFFF, valor máximo de la variable uint32_t ticks_bloqueada de la estructura de la tarea)..

La funcion que libera el semáforo **Give_Semaforo_Bin()** libera la tarea (usando **os_releaseTask()**) , actualiza el estado del semáforo y llama a la api **CpuYield()**

**Nota: En caso de utilizar los semáforos binarios en una IRQ, se debe considerar los posibles errores del SO. Si se obtiene un errorHook al utilizar esta funcionalidad con IRQ, se recomineda leer el estado del error proporcionado por el SO.**

### Contador
Se implementa un semáforo contador con un valor inicial y un valor máximo de cuentas. Se desarrollan las funciones de tomar y liberar el semáforo que suman o restan el contador respectivamente. En caso de encontrarse el semáforo completo o vacío, las funciones no modifican el semáforo.

Si se ejecuta desde una IRQ, se realiza un rescheduling del SO llamando a la tarea **CpuYield()**

## Forzado de Scheduling
Fuerza un llamado al scheduler del SO por parte de la API. La funcion correspondiente es **CpuYield()**

## Colas
Se implementan colas de número enteros con un tamaño de 4 bytes máximo (futuras versiones contemplarán otro tipo de dato) y un tamaño máximo total de **N_MAX_COLA** bytes.

En caso de no contar con datos a recibir o espacio en la cola para enviar datos, la tarea pasará al estado **TAREA_BLOCKED** hasta lograr su objetivo, el tiempo que permanece bloqueado se especifica en números de ticks de sistema cuando se llama a la tarea **Recibir_dCola()** o **Enviar_aCola()** . Si los ticks de sistemas concluyen y la tarea no logra enviar o recibir un dato, la tarea finaliza con el flag de estado correspondiente.

En caso de quere que la tarea retorne cuando no logra enviar o recibir un valor de la cola, sin estar bloqueada, se debe colocar el valor **0** en los ticks. En este caso se recibirá el flag de **NO_ENVIO_DATO o NO_RECIBO_DATO**

Al finalizar la tarea de enviar o recibir cola se devuelve una variable de estado que puede ser:
- **ENVIO_DATO** : envió el dato luego de llamar a la función
- **ENVIO_DATO_TICK**: envió el dato luego de alguna cantidad de ticks en los cuales la tarea permanecio bloqueada
- **NO_ENVIO_DATO**: no logró enviar el dato luego de cumplirse los ticks de sistema enviados como parámetro
- **RECIBO_DATO**: recibió el dato luego de llamar a la función
- **RECIBO_DATO_TICK**: recibió el dato luego de alguna cantidad de ticks en los cuales la tarea permanecio bloqueada
- **NO_RECIBO_DATO**: no logró recibir el dato luego de cumplirse los ticks de sistema enviados como parámetro


**Nota: En caso de utilizar las colas en una IRQ, se debe considerar los posibles errores del SO. Si se obtiene un errorHook al utilizar esta funcionalidad con IRQ, se recomineda leer el estado del error proporcionado por el SO.**

# IRQ
Se implementas las IRQ de HW del sistema mediante la utilizacion de un vector de punteros, por cada interrupcion se debería habilitar o desabilitar indicando la funcion handler cuando sea correpondiente.

El sistema operativo se encontrará en el estado **OS_INTERRUPT** cuando ocurra la ejecucion de las interrupciones y para el uso de colas y semáforos, de pueden utilziar las funciones de la API pero se deben considerar los errores posibles del SO detallados en la documentacion. 

**Nota: En caso de utilizar funciones que modifican el estado de las tareas asociasdas dentro de una IRQ, el SO generará un errorHook. Es por esto que se recomienda analizar con atención qué APIs se utilizarán dentro de un handler de IRQ.** 

# EJEMPLO DE USO   
Para demostrar la funcionalidad del SO se decide desarrollar el siguiente ejemplo de uso:

Se pretende contar la cantidad de botellas fabricadas en una linea, para esto, se cuenta con un sensor inductor ubicado a la salida del estrusor que se pone en alto cada vez que una botella pasa. A su vez se cuenta con un segundo sensor ubicado en la linea de productos descartados que nos permite conocer el número total de botellas fabricadas sin errores.

El sistema debe realizar lo siguiente:
- Contar la cantidad de botellas que salen del estrusor.
- Contar la cantidad de botellas que se descartan.
- Contar la cantidad de botellas fabricadas exitosamente.
- Enviar cada un tiempo determinado por UART, al operario, las tres cantidades de botellas en el último tiempo.
- Contar con leds de estado por cada uno de los dos sensores inductivos.

Los sensores inductivos serán representados por los botones de la Placa EDU-CIAA, contarán con un proceso de rebote similar a lo que se tendría con una señal digital de un sensor inductivo.

La trama de comunicación entre el la PC y la EDU-CIAA será "TXXXXFYYYYDZZZZ\n" con XXXX, YYYY y ZZZZ los últimos valores registados.

Luego de cada mensaje enviado al operario, se resetearán los valores de conteo y se inicia con un nuevo batch de datos.

## Solucion del Ejemplo
El ejercicio puede ser resuelto de infinidad de maneras, sin embargo la que se plantea en este caso, pretende utilizar la mayor cantidad de funciones para corroborar su funcionamiento. Se crean tras interrupciones de Hardware:
- **Handler_sensor1**: Lecuta de la tecla 1 al ser presionada para simular el sensor 1, aumenta el contador **semaforo_sensor_estrusor** cada vez que se ejcuta.
- **Handler_sensor2**: Lecuta de la tecla 2 al ser presionada para simular el sensor 2, aumenta el contador **semaforo_sensor_descarte** cada vez que se ejcuta.

Para el ciclo normal de funcionamiento se cuenta con 4 tareas:
- **tareaLed** : Tarea de baja prioridad que togglea un led en señal de Heart Beat y luego de un cierto tiempo, libera el semáforo binario **semaforo_UART** para que se envíe el dato.
- **ContarBotellasEstrusor** : Tarea de alta prioridad que cada **TIEMPOMUESTRASBOTELLAS** ticks de sistema optiene el valor del semáforo contador correspondinete y lo carga en la cola **botellas_fabricadas**
- **ContarBotellasDescarte** : Tarea de alta prioridad que cada **TIEMPOMUESTRASBOTELLAS** ticks de sistema optiene el valor del semáforo contador correspondinete y lo carga en la cola **botellas_desechadas**
- **ResponderUART** : Tarea de prioridad media que envía por la UART los ultimos datos procesados cada vez que se habilita el semáforo binario **semaforo_UART**. Lee los datos de las dos colas (dejandolas vacias), calcula la cantidad de botellas totales y envia el mensaje.

El ejemplo se encuentra totalmente funcinal como se puede observar en el siguiente video:


https://user-images.githubusercontent.com/44267876/186425029-cb5b7d82-5c07-4e37-9ace-3879a21131ed.mp4

