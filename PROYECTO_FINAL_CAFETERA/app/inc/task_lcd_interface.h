#ifndef TASK_LCD_INTERFACE_H_
#define TASK_LCD_INTERFACE_H_

#include <stdint.h>
/**
 * @brief inicializa la tarea del LCD.
 *
 * configura el display LCD y reinicia los contadores asociados. se llama una sola vez al inicio del sistema.
 *
 * @param params parámetro no utilizado (puede ser NULL).
 */
void task_lcd_init(void *params);

/**
 * @brief actualiza la tarea del LCD.
 *
 * función periódica no bloqueante que actualiza el estado del LCD según la FSM interna.
 * debe ser llamada (`app_update`).
 *
 * @param params parámetro no utilizado (puede ser NULL).
 */
void task_lcd_update(void *params);

/**
 * @brief contador de ejecuciones de la tarea del LCD.
 */
extern uint32_t g_task_lcd_cnt;

/**
 * @brief ticks restantes hasta la próxima ejecución de la tarea del LCD.
 */
extern volatile uint32_t g_task_lcd_tick_cnt;

#endif /* TASK_LCD_INTERFACE_H_ */
