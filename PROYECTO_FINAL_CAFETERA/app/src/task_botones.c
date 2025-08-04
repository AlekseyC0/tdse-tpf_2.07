/********************** EXPLICACION BREVE *****************************/
/*
 * Este modulo implementa una maquina de estados finito para el manejo robusto de botones fisicos,
 * incluyendo antirrebote por software. Lee tres botones conectados a GPIOa y detecta sus eventos
 * (presionado, liberado) de forma no bloqueante mediante un scheduler por ticks. Cada boton es tratado
 * como una estructura independiente (Button) que guarda sus estado actual, ultima lectura, timestamp
 * del ultimo cambio. La funcion task_botones_update() actualiza periodicamente el estado de cada boton
 * usando HAL_GetTIck() y registra los eventos con LOGGER_INFO. Adema, se ofrecen funciones publicas
 * para consultar el estado (botonX_presionado) y detectar flancos descendentes (boton1_falling_edge)
 * facilitando la integracion con la FSM principal.
*/
/**************************************************************************/

#include "main.h"
#include "logger.h"
#include "stm32f1xx_hal.h"
#include "task_botones_attribute.h"
#include <stdbool.h>
#include <stdint.h>
#include "stm32f1xx_it.h"
#include "main.h"
#include "logger.h"
#include <stdio.h>
#define DEBOUNCE_TIME_MS 20 //cuanto tiempo de estabilidad se necesita para validar cambio
#define NUM_BOTONES 3

#define G_TASK_BOTONES_CNT_INI        0ul
#define G_TASK_BOTONES_TICK_CNT_INI   0ul

/********************** etiquetas de depuración ******************************/
const char *p_task_botones   = "Task BOTONES";
const char *p_task_botones_  = "Non-Blocking & Update By Time Code";

/********************** variables globales públicas **************************/
uint32_t g_task_botones_cnt;
volatile uint32_t g_task_botones_tick_cnt;

/********************** instancias de botones **************************/
static Button botones[NUM_BOTONES] = {
    {BOTON1_GPIO_Port, BOTON1_Pin, BUTTON_RELEASED, false, 0},
    {BOTON2_GPIO_Port, BOTON2_Pin, BUTTON_RELEASED, false, 0},
    {BOTON3_GPIO_Port, BOTON3_Pin, BUTTON_RELEASED, false, 0}
};

/********************** FSM proceso de botón individual ******************/
static button_event_t button_process(Button* btn) //fsm individual por boton
{
    uint32_t now = HAL_GetTick(); 
    bool lectura = HAL_GPIO_ReadPin(btn->port, btn->pin);
    //guarda lo que lee 

    if (lectura != btn->last_read) {
        btn->last_change_time = now;
        btn->last_read = lectura;
    }
    //si cambio respecto de ultima lectura actualiza 

    if ((now - btn->last_change_time) > DEBOUNCE_TIME_MS) {
        switch (btn->state) {
            case BUTTON_RELEASED:
                if (lectura == GPIO_PIN_SET) {
                    btn->state = BUTTON_PRESSED;
                    return EVENT_PRESS;
                    //si estaba 'released' ahora cambia a 'pressed'
                }
                break;
            case BUTTON_PRESSED:
                if (lectura == GPIO_PIN_RESET) {
                    btn->state = BUTTON_RELEASED;
                    return EVENT_RELEASE;
                    //si estaba 'pressed' ahora cambia a 'released'
                }
                break;
        }
    }

    return EVENTO_NONE;
}

/********************** API pública para FSM externas ******************/
bool boton1_presionado(void) { return botones[0].state == BUTTON_PRESSED; } //consulta si boton 1 esta siendo presionado
bool boton2_presionado(void) { return botones[1].state == BUTTON_PRESSED; } //consulta si boton 2 esta siendo presionado
bool boton3_presionado(void) { return botones[2].state == BUTTON_PRESSED; } //consulta si boton 3 esta siendo presionado

bool boton1_falling_edge(void) //consulta si se detecto flanco descendente (press-->release)
{
    static bool last = false;
    bool actual = HAL_GPIO_ReadPin(botones[0].port, botones[0].pin);
    bool flanco = (last == true && actual == false);
    last = actual;
    return flanco;
}

/********************** función principal de actualización ******************/
void task_botones_update() //actualiza info botones
{
    bool b_time_update_required = false; 

    __asm("CPSID i"); //deshabilita interrupciones para codigo bloqueante
    if (G_TASK_BOTONES_CNT_INI < g_task_botones_tick_cnt) //compara
    {
        g_task_botones_tick_cnt--; //si hay ticks pendientes lo decrementa
        b_time_update_required = true; 
    }
    __asm("CPSIE i");//habilita interrupciones
    //mientras haya ticks pendientes ejecutar tareas
    if (b_time_update_required)
    {
        g_task_botones_cnt++; //incrementar contador de ciclo de tareas

        // FSM para cada botón
        for (int i = 0; i < NUM_BOTONES; i++) {
            button_event_t evt = button_process(&botones[i]);

            switch (evt) {
                case EVENT_PRESS:
                    LOGGER_INFO("Botón %d PRESIONADO", i + 1);
                    break;
                case EVENT_RELEASE:
                    LOGGER_INFO("Botón %d LIBERADO", i + 1);
                    break;
                default:
                    break;
            }
        }
        __asm("CPSID i"); //deshabilita interrupciones
        if (G_TASK_BOTONES_CNT_INI < g_task_botones_tick_cnt) //compara
        {
            g_task_botones_tick_cnt--; //si hay ticks pendientes lo decrementa 
            b_time_update_required = true; 
        }
        else
        {
            b_time_update_required = false; //si no quedan ticks pendientes no es necesario actualizar aun
        }
        __asm("CPSIE i");//rehabilita interrupciones
    }
}

/********************** inicialización **************************/
void task_botones_init() //inicializa botones
{
    LOGGER_INFO(" ");
    LOGGER_INFO("  %s is running - %s", GET_NAME(task_botones_init), p_task_botones);
    LOGGER_INFO("  %s is a %s", GET_NAME(task_botones_update), p_task_botones_);
    g_task_botones_cnt = G_TASK_BOTONES_CNT_INI;
    LOGGER_INFO("   %s = %lu", GET_NAME(g_task_botones_cnt), g_task_botones_cnt);
}
