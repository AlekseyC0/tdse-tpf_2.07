
/**
 * @file task_cafe_fsm_interface.h
 * @brief interfaz pública para la máquina de estados del sistema de café.
 *
 * contiene los prototipos de funciones que inicializan y actualizan la
 * máquina de estados finita (FSM) que gobierna el comportamiento principal
 * del sistema SmartCoffee.
 */

#ifndef TASK_CAFE_FSM_INTERFACE_H_
#define TASK_CAFE_FSM_INTERFACE_H_

#include <stdint.h>

/**
 * @brief inicializa la máquina de estados principal del sistema de café.
 *
 * esta función debe ser llamada una sola vez al inicio del sistema. 
 * configura variables internas, resetea contadores y registra el estado inicial.
 *
 * @param parameters parámetro genérico no utilizado (compatibilidad con RTOS).
 */
void task_cafe_fsm_init(void *parameters);

/**
 * @brief actualiza periódicamente la FSM principal del sistema de café.
 *
 * esta función se llama desde el scheduler (super-loop o RTOS) de manera no bloqueante.
 * detecta eventos, actualiza subestados y permite la transición entre estados.
 *
 * @param parameters parámetro genérico no utilizado (compatibilidad con RTOS).
 */
void task_cafe_fsm_update(void *parameters);

#endif /* TASK_CAFE_FSM_INTERFACE_H_ */
