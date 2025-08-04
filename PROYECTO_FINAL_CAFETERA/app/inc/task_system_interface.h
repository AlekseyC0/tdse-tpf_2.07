/**
 * @file task_system_interface.h
 * @brief Interfaz pública del sistema embebido base.
 *
 * Define las funciones de inicialización y actualización periódica del sistema.
 * También expone contadores globales utilizados para sincronización por ticks.
 */

#ifndef TASK_SYSTEM_INTERFACE_H
#define TASK_SYSTEM_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief inicializa los recursos base del sistema embebido.
 *
 * debe ser llamada al comienzo de la ejecución. configura temporizadores,
 * periféricos base, contadores y cualquier estructura necesaria para
 * el funcionamiento global del sistema.
 */
void task_system_init(void *param);

/**
 * @brief función periódica que actualiza el estado general del sistema.
 *
 * se debe llamar regularmente desde el loop principal
 * permite actualizar timers, verificar flags o mantener la lógica global.
 *
 */
void task_system_update(void *param);

/**
 * @brief contador global de ciclos ejecutados por `task_system_update()`.
 */
extern uint32_t g_task_system_cnt;

/**
 * @brief contador de ticks pendientes para `task_system_update()`.
 *
 */
extern volatile uint32_t g_task_system_tick_cnt;

#endif /* TASK_SYSTEM_INTERFACE_H */
