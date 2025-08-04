#ifndef TASK_SENSOR_ULTRASONICO_INTERFACE_H
#define TASK_SENSOR_ULTRASONICO_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief inicializa el sensor ultrasónico HCSR04.
 *
 * configura el pin TRIG como salida, inicia el canal de captura por interrupción
 * y deja todo listo para mediciones. debe llamarse una sola vez al comenzar el sistema.
 */
void task_sensor_ultrasonico_init(void *params);

/**
 * @brief función periódica.
 *
 * se llama desde el loop principal, y permite controlar el tiempo
 * y lógica de actualización del sensor. no bloquea.
*/
void task_sensor_ultrasonico_update(void *params);

/**
 * @brief contador de ejecuciones de la tarea.
 */
extern uint32_t g_task_sensor_cnt;

/**
 * @brief contador de ticks para la tarea.
 */
extern volatile uint32_t g_task_sensor_tick_cnt;

#endif
