#ifndef INC_TASK_BOTONES_INTERFACE_H_
#define INC_TASK_BOTONES_INTERFACE_H_

/**
 * @file botones_interface.h
 * @brief declaración de funciones y variables públicas para la tarea de manejo de botones.
 */

/**
 * @brief inicializa la tarea de manejo de botones.
 *
 * configura el estado inicial de cada botón, imprime información de diagnóstico
 * y reinicia el contador de eventos de la tarea.
 */
void task_botones_init(void *params);

/**
 * @brief actualiza el estado de los botones en forma no bloqueante.
 *
 * esta función debe ser llamada periódicamente, se encarga de:
 * - leer el estado de cada botón físico,
 * - aplicar lógica de antirrebote por software,
 * - detectar eventos (presión/liberación),
 * - registrar eventos con LOGGER.
 */
void task_botones_update(void *params);

/**
 * @brief contador global de ejecuciones de la tarea de botones.
 */
extern uint32_t g_task_botones_cnt;

/**
 * @brief ticks pendientes para activar la tarea de botones.
 */
extern volatile uint32_t g_task_botones_tick_cnt;

#endif /* INC_TASK_BOTONES_INTERFACE_H_ */
