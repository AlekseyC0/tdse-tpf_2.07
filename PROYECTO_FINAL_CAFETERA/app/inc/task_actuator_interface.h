#ifndef TASK_ACTUATOR_INTERFACE_H
#define TASK_ACTUATOR_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>
#include "task_cafe_fsm_attribute.h"

void task_actuator_init(void *params);
void task_actuator_update(void *params);

// LED control functions
void led_on(uint8_t led_num);
void led_off(uint8_t led_num);
void led_toggle(uint8_t led_num);
void mostrar_estado_leds(estados est);
bool test_leds(void);

extern uint32_t g_task_actuator_cnt;
extern volatile uint32_t g_task_actuator_tick_cnt;

#endif

/**
 * @brief inicializa la tarea del actuador.
 *
 * enciende/apaga los LEDs para verificar su funcionamiento
 * y muestra mensajes por consola.
 */
void task_actuator_init(void *params);

/**
 * @brief actualiza la tarea del actuador en cada ciclo no bloqueante.
 *
 * verifica si hay un tick disponible y ejecuta las acciones correspondientes.
 */
void task_actuator_update(void *params);

/**
 * @brief apaga el LED especificado.
 *
 * @param led_num número del LED a apagar (1 a 4).
 */
void led_on(uint8_t led_num);

/**
 * @brief enciende el LED especificado.
 *
 * @param led_num número del LED a prender (1 a 4).
 */
void led_off(uint8_t led_num);

/**
 * @brief cambia el estado del LED (toggle).
 *
 * @param led_num número del LED a alternar (1 a 4).
 */
void led_toggle(uint8_t led_num);

/**
 * @brief muestra el estado actual del sistema en los LEDs.
 *
 * cada estado de la máquina de café se representa visualmente
 * con un patrón específico de LEDs encendidos/apagados.
 *
 * @param est estado actual del sistema (`enum estados`).
 */
void mostrar_estado_leds(estados est);

/**
 * @brief prueba el funcionamiento de los 4 LEDs.
 *
 * enciende, verifica y apaga cada LED secuencialmente.
 * @return true si todos los LEDs funcionan correctamente, false si alguno falla.
 */
bool test_leds(void);

/**
 * @brief contador de ejecuciones de la tarea del actuador.
 */
extern uint32_t g_task_actuator_cnt;

/**
 * @brief contador de ticks de la tarea del actuador.
 *
 * Controla la temporización no bloqueante de la función `task_actuator_update()`.
 */
extern volatile uint32_t g_task_actuator_tick_cnt;