# ISO_MSE_SE
Sistema operativo desarrollado por Santiago Esteva como trabajo de la materia Implementacion de sistemas operativos 7ma Coherte 2022 de la Maestría en Sistemas Embebidos de la Facultad de Ingeniería, Universidad de Buenos Aires.

# Sistema Operativo
El sistema operativo permite crear una cantidad máxima de **MAX_NUM_TASK** tareas que correran en modo round robin dependiendo los grupos de prioridad con tres tipos de estados:
- TAREA_READY 
- TAREA_RUNNING 
- TAREA_BLOKED

El controlar del SO se realiza en el Core mediante la estructura **osControl**. 

## Prioridades
Las tareas del SO pueden contar con **MAX_PRIOR_TASK** = 4 niveles de prioridades de las cuales van desde el valor mínimo de **p_TaskIdle** al máximo que es 4. El scheduler ejecutará desde mayor prioridad a menor prioridad con un esquema de round robin entre las tareas de la misma prioridad e irán disminuyendo de escalones a medida que las tareas de mayor prioridad pasan a estar bloqueadas.

**NOTA: En caso de asignar una prioridad erronea, la tarea será asignada con la priodidad mínima.**

## Ejecución del Sistema Operativo
La ejecución del SO se implementa con interupciones del Systick. Realizando modificaciones en el Handler del systick se actualiza el estado del SO por cada tick, se llama al scheduler que define la nueva tarea a ejecutar y habilita la interrupcion del PendSV para el cambio de contexto

La interrpción PendSV se realiza en assembler utilizando el proceso de lazy stacking y en los casos correspondientes almacena y restaura los registros de propósito general **{r4-r11,lr}** y los registros FPU **{s16-s31}**.
Para realizar el cambio de contexto se implementa una funcion en C (**getContextoSiguiente**, llamada desde el **PendSV_Habdler.sx**) que actualiza la estructura del SO y devuelve al MSP el stack de la nueva tarea a ejecutar. 

## Hooks
El SO operativo cuenta con los siguientes Hook:
- returnHook: Tarea ejecutada luego de retornar una tarea del SO, en caso de no incorporar una loop infinito en las tareas creadas se finalizará en este hook a la espera de una nueva interrupcion del sistema.
- errorHook: Tarea ejecutada cuando se pretende crear una cantidad de tareas mayor a **MAX_NUM_TASK**.
- TickHook: Tarea ejecutrada por el Systick cuando se encuentra definido en 1 el **configUSE_TICK_HOOK**. Por default ejecutra un no operation "NOP".

## Tarea IDLE
Cuando el SO no cuenta con tareas a ejecutar debido a:
- No existen tareas creadas por el usuario
- Ninguna tarea se encuentra en estado READY o RUNNING

El SO cuenta con la tarea IDLE que se ejecutaría en estos casos, dando un uso de bajo consumo al CPU cuando no hay tareas del SO en estado de ejecución. Esta tarea sólo se encontrará en estado RUNNING o READY y cuenta con la prioridad mas baja. 
Puede ser redefinida para efectuar acciones de segundo plano (background) o definir perfiles de ejecución para aplicar técnicas de bajo consumo (frecuencia CPU, periféricos, etc). Por default la trea ejecuta la instrucción WFI (wait for interrupt).

## Tarea BLOCKED
Actualmente para pasar una tarea a bloqueado se implementa la funcion **bloqued_Task** que recibe como puntero la estructura de la tarea y la cantidad de ticks que se pretende bloquear la tarea. Actualiza el estado, almacena el número de ticks recibidos y se queda en espera a una nueva interrupcion con __WFI()