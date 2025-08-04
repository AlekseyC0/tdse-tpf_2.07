/********************** EXPLICACION BREVE *****************************/
/*
este módulo se encarga de controlar los LEDs del sistema de forma modular y no bloqueante, implemnta
funciones para encender, apagar y alternar LEDs, ademas de visualizar el estado de la FSM principal
mediante patrones, incluye una estructura de tarea periodica lista para integracion con un scheduler
basado en ticks. Tambien incorpora un autotest simple para verificar visualmente el funcionamiento
del hardware. Su diseno mantiene el codigo desacoplado y claro, facilitando el mantenimiento y pruebas.
*/
/**************************************************************************/

#include "task_actuator_interface.h"
#include "stm32f1xx_hal.h"  //HAL STM32 (GPIO, delays, etc.)
#include "main.h"           //def globales
#include "logger.h"         //imprimir mensajes por consola

/********************** variables globales*****************************/

/********************** macros y definiciones *******************************/
#define G_TASK_ACTUATOR_CNT_INI			    0ul
#define G_TASK_ACTUATOR_TICK_CNT_INI		0ul

#define DEL_ACTUATOR_MIN					0ul
#define DEL_ACTUATOR_MED					50ul
#define DEL_ACTUATOR_MAX					500ul


/********************** definicion data interna *****************************/

const char *p_task_actuator 		= "Task Actuator";
const char *p_task_actuator_		= "Non-Blocking & Update By Time Code";

/********************** declaracion data externa ****************************/
uint32_t g_task_actuator_cnt;
volatile uint32_t g_task_actuator_tick_cnt;

//usamos resistencias pull down --> led_on: apaga ; led_off:prende 

//led1 = rojo
//led2 = amarillo
//led3 = verde
//led4 = azul


void task_actuator_init(void *params) //inicializa contador
{
	led_off(1); led_off(2);led_off(3);led_off(4); //prende todas LEDs 
	HAL_Delay(500); //espera 500ms
	led_on(1); led_on(2);led_on(3);led_on(4); //apaga todas las LEDs para verificar que funcionen 
    LOGGER_INFO(" ");
    
    //inicializa contador global
    LOGGER_INFO("  %s is running - %s", GET_NAME(task_actuator_init), p_task_actuator);
    LOGGER_INFO("  %s is a %s", GET_NAME(task_actuator_update), p_task_actuator_);

    g_task_actuator_cnt = G_TASK_ACTUATOR_CNT_INI;
    LOGGER_INFO("   %s = %lu", GET_NAME(g_task_actuator_cnt), g_task_actuator_cnt);

}

void task_actuator_update(void *params) //actualiza contador
{
    bool b_time_update_required = false; //ver si es momento para actualizar 

    __asm("CPSID i"); //deshabilita interrupciones para codigo bloqueante
    if (G_TASK_ACTUATOR_CNT_INI < g_task_actuator_tick_cnt) //compara
    {
    	g_task_actuator_tick_cnt--; //si hay ticks pendientes lo decrementa
        b_time_update_required = true;
    }
    __asm("CPSIE i");// rehabilita interrupciones
  /*mientras haya ticks pendientes ejecutar tareas*/
    if (b_time_update_required)
    {
    	g_task_actuator_cnt++; //incremento contador de ciclo de tarea 

       __asm("CPSID i"); //dehabilito interrupciones
        if (G_TASK_ACTUATOR_CNT_INI < g_task_actuator_tick_cnt) //comparo
        {
        	g_task_actuator_tick_cnt--; //si hay ticks pendientes lo decrementa 
            b_time_update_required = true;
        }
        else
        {
            b_time_update_required = false; //si no quedan ticks pendientes no es necesario actualizar aun 
        }
        __asm("CPSIE i"); // rehabilito interrupciones 
    }

}

/********************** LED control *****************************/

void led_on(uint8_t led_num) //apaga led segun numero
{
    switch (led_num) {
        case 1: HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET); break;
        case 2: HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET); break;
        case 3: HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET); break;
        case 4: HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_RESET); break;
        default: break;
    }
}

void led_off(uint8_t led_num) //prendo led segun numero
{
    switch (led_num) {
        case 1: HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET); break;
        case 2: HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET); break;
        case 3: HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET); break;
        case 4: HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_SET); break;
        default: break;
    }
}

void led_toggle(uint8_t led_num) //cambio estado led segun numero
{
    switch (led_num) {
        case 1: HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin); break;
        case 2: HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin); break;
        case 3: HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin); break;
        case 4: HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin); break;
        default: break;
    }
}

bool test_leds(void) //testea 4 leds
{
    for (uint8_t i = 1; i <= 4; i++) {
        led_off(i);
        HAL_Delay(100);

        GPIO_TypeDef *port;
        uint16_t pin;

        switch (i) {
            case 1: port = LED1_GPIO_Port; pin = LED1_Pin; break;
            case 2: port = LED2_GPIO_Port; pin = LED2_Pin; break;
            case 3: port = LED3_GPIO_Port; pin = LED3_Pin; break;
            case 4: port = LED4_GPIO_Port; pin = LED4_Pin; break;
            default: return false;
        }

        if (HAL_GPIO_ReadPin(port, pin) != GPIO_PIN_SET) {
            return false;  // falla/error: el LED no se encendió correctamente
        }
        led_on(i);
    }
    return true;  // todos los LEDs pasaron la prueba
}

/********************** LED segun FSM *****************************/

void mostrar_estado_leds(estados est)//muestra estados de FSM a traves leds
{
    led_on(1); led_on(2); led_on(3); led_on(4); //apago todas las leds
    switch (est) {
        case estado_off:             led_off(1); break; // prendo rojo
        case estado_menu_principal:  led_off(2); break; // prendo amarrillo
        case estado_seleccion_cafe:  led_off(3); break; // prendo verde
        case estado_fabricante:      led_off(1); led_off(2); led_off(3); led_off(4); break; //prendo todos
        case estado_brewing:         led_on(1); led_on(2); led_on(3); led_on(4); break; //apago todos
        default: break;
    }
}
