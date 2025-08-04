

#ifndef APP_INC_APP_H_
#define APP_INC_APP_H_

/********************** CPP guard********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusiones *******************************************/

/********************** macros ***********************************************/
#define TEST_0 (0)
#define TEST_1 (1)
#define TEST_2 (2)

#define TEST_X (TEST_0)

/********************** declaraciones data extrerna ****************************/
extern uint32_t g_app_cnt;
extern uint32_t g_app_runtime_us;

extern volatile uint32_t g_app_tick_cnt;

/********************** declaraciones funciones externas ***********************/
/**
 * @brief inicializa la aplicación embebida.
 *
 * esta función configura e inicializa todos los módulos/tareas del sistema.
 * también inicializa los contadores globales, el ciclo de reloj (DWT)
 * y protege el recurso compartido de ticks usando instrucciones críticas.
 *
 * debe llamarse una sola vez al comienzo del programa.
 *
 * @return void
 */
void app_init(void);

/**
 * @brief Actualiza el estado de la aplicación embebida.
 *
 * esta función se ejecuta dentro del super-loop. revisa si hay ticks pendientes
 * y en tal caso ejecuta todas las tareas registradas, midiendo el tiempo
 * de ejecución de cada una con el contador de ciclos. 
 * también actualiza estadísticas de tiempo total (`g_app_runtime_us`) y WCET por tarea.
 *
 * se asegura de proteger el recurso `g_app_tick_cnt` con instrucciones críticas.
 *
 * @return void
 */
void app_update(void);

/********************** fin de CPP guard*************************************/
#ifdef __cplusplus
}
#endif

#endif /* APP_INC_APP_H_ */

/********************** end of file ******************************************/
