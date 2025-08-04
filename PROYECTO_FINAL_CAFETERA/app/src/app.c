/********************** inclusiones *******************************************/
#include <task_adc_interface.h>
#include "main.h"
#include "logger.h"
#include "dwt.h"
#include "board.h"
#include "app.h"

/*tareas del sistema */
#include "task_sensor_ultrasonico_interface.h"
#include "task_actuator_interface.h"
#include "task_system_interface.h"
#include "task_lcd_interface.h" 
#include "task_cafe_fsm_interface.h"
#include "task_botones_interface.h"
/********************** macros y definiciones *******************************/
#define G_APP_CNT_INI        0ul
#define G_APP_TICK_CNT_INI   0ul
#define TASK_X_WCET_INI      0ul

/* delays de cada tarea */
#define DELAY_SENSOR_MS      10
#define DELAY_ACTUATOR_MS     5
#define DELAY_SYSTEM_MS       2
#define DELAY_LCD_MS         50

/* ticks globales */;
/********************** config de tareas *************************************/
typedef struct {
    void (*task_init)(void *);
    void (*task_update)(void *);
    void *parameters;
} task_cfg_t;

typedef struct {
    uint32_t WCET;
} task_dta_t;

/*struct guarda el WCET (worst case execution time) estimado de cada tarea queremos que sea <1ms el total del proyecto*/ 
const task_cfg_t task_cfg_list[] = {
{ task_sensor_ultrasonico_init,   task_sensor_ultrasonico_update,   NULL },  // sensor distancia
{ task_system_init,               task_system_update,   NULL },  // EEPROM, ciclo base
{ task_actuator_init,             task_actuator_update, NULL },  // LEDs, relés, etc.
{ task_lcd_init,                  task_lcd_update,      NULL },  // buffer LCD
{ task_adc_init,				  task_adc_update,		NULL },   // conversor
{ task_botones_init,			  task_botones_update,	NULL },
{ task_cafe_fsm_init,             task_cafe_fsm_update, NULL }   // FSM principal
};

#define TASK_QTY    (sizeof(task_cfg_list)/sizeof(task_cfg_t))
/********************** definicion data *****************************/
const char *p_sys	= " Bare Metal - Event-Triggered Systems (ETS)";
const char *p_app	= " App - Model Integration - C codig";
uint32_t g_app_cnt;
uint32_t g_app_runtime_us;
volatile uint32_t g_app_tick_cnt;
task_dta_t task_dta_list[TASK_QTY];

/********************** funciones ***********************************/

void app_init(void) //define la arquitectura de inicialización de una app modular con múltiples tareas periódicas o event-driven
{
    uint32_t index;

    LOGGER_INFO(" ");
    LOGGER_INFO("%s is running - Tick = %lu", GET_NAME(app_init), HAL_GetTick());
	
    LOGGER_INFO(p_sys);
    LOGGER_INFO(p_app);

    g_app_cnt = G_APP_CNT_INI;
	LOGGER_INFO(" %s = %lu", GET_NAME(g_app_cnt), g_app_cnt);
	
	/*inicializacion del contador de ciclos del CPU*/
    cycle_counter_init();

	/*inicia cada tarea con su WCET*/
    for (index = 0; index < TASK_QTY; index++) {
        task_cfg_list[index].task_init(task_cfg_list[index].parameters);
        task_dta_list[index].WCET = TASK_X_WCET_INI;
    }

    /*proteccion recursos compartidos al inicializar tick global*/
    	__asm("CPSID i");	//desactiva interrupciones
    	g_app_tick_cnt = G_APP_TICK_CNT_INI;
        __asm("CPSIE i");//vuelve a activarlas 
}

void app_update(void)
{
	/* declaracion variables */
	uint32_t index;
	bool b_time_update_required = false;
	uint32_t cycle_counter_time_us;

	/* verifica si hay tick disponible */
	__asm("CPSID i"); //desactiva interrupciones para evitar codigo bloqueante  
    if (G_APP_TICK_CNT_INI < g_app_tick_cnt) //compara
    {
		//si hay ticks pendientes lo decrementa
    	g_app_tick_cnt--;
    	b_time_update_required = true;
    }
    __asm("CPSIE i");//activa interrupciones de nuevo

	/*mientras haya ticks pendientes ejecutar tareas*/
    while (b_time_update_required)
    {
    	/*contador de ciclos y tiempo total*/
    	g_app_cnt++;
    	g_app_runtime_us = 0;

		/*iteracion sobre cada tarea*/
		for (index = 0; TASK_QTY > index; index++)
		{
			cycle_counter_reset(); //reinicia contador de tiempo
			(*task_cfg_list[index].task_update)(task_cfg_list[index].parameters); //ejecuta tarea
			cycle_counter_time_us = cycle_counter_get_time_us(); //mide tiempo que tardó
			g_app_runtime_us += cycle_counter_time_us; //suma tiempo de ejecucion al total del ciclo 
			
			if (task_dta_list[index].WCET < cycle_counter_time_us) //si el tiempo actual supera el WCET anterior lo actualiza 
			{	
				task_dta_list[index].WCET = cycle_counter_time_us;
			}
		}

	   /*mostrar WCET de cada tarea*/
		//LOGGER_INFO("---- WCET por tarea ----");
		//for (index = 0; index < TASK_QTY; index++) {
		//	LOGGER_INFO("Tarea %lu: %lu us", index, task_dta_list[index].WCET);
		//}

		/*verifica si hay más ticks pendientes*/
		__asm("CPSID i");//desactiva interrupciones para evitar codigo bloqueante  
		if (G_APP_TICK_CNT_INI < g_app_tick_cnt) //compara
		{
			g_app_tick_cnt--; //si hay ticks pendientes lo decrementa
			b_time_update_required = true; 
		}
		else
		{
			b_time_update_required = false;
		}
		__asm("CPSIE i");//activa interrupciones de nuevo
	}
}

void HAL_SYSTICK_Callback(void)
{
    g_app_tick_cnt++;
	g_task_lcd_tick_cnt++;
	g_task_cafe_fsm_tick_cnt++;
	g_task_adc_tick_cnt++;
	g_task_actuator_tick_cnt++;
	g_task_system_tick_cnt++;
	g_task_botones_tick_cnt++;

}
