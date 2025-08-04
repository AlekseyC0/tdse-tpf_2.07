
#ifndef TASK_INPUT_INTERFACE_H_
#define TASK_INPUT_INTERFACE_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief inicializa el ADC en modo DMA y arranca la adquisición continua de 3 canales.
 *
 * debe llamarse una sola vez al inicio del sistema. El resultado de las conversiones se almacena
 * automáticamente en el arreglo global `adc_value[]`.
 */
void task_adc_init(void *params);

/**
 * @brief lógica de actualización de la tarea ADC. verifica si hay tick pendiente y actualiza temperatura.
 *
 * aplica actualización no bloqueante.
 */
void task_adc_update(void *params);

/**
 * @brief detecta si el joystick fue movido hacia arriba (eje Y positivo).
 * @return true si se detecta un evento de subida, false en caso contrario.
 */
bool joystick_arriba(void);

/**
 * @brief detecta si el joystick fue movido hacia abajo (eje Y negativo).
 * @return true si se detecta un evento de bajada, false en caso contrario.
 */
bool joystick_abajo(void);

/**
 * @brief detecta si el joystick fue movido hacia la izquierda (eje X negativo).
 * @return true si se detecta un evento de izquierda, false en caso contrario.
 */
bool joystick_izquierda(void);

/**
 * @brief detecta si el joystick fue movido hacia la derecha (eje X positivo).
 * @return true si se detecta un evento de derecha, false en caso contrario.
 */
bool joystick_derecha(void);

/**
 * @brief obtiene la temperatura actual convertida desde el valor del potenciómetro (ADC).
 * @return temperatura actual en grados Celsius (entre 30 °C y 90 °C aprox.).
 */
uint8_t get_temp_actual(void);  // ⚠️ Asumí que esta función existe. Si no, podés quitarla.

/**
 * @brief verifica si el potenciómetro está conectado y funcionando correctamente.
 * @return true si el valor ADC está dentro del rango válido (no saturado).
 */
bool potenciometro_test(void);

/**
 * @brief contador de ejecuciones de la tarea ADC.
 */
extern uint32_t g_task_adc_cnt;

/**
 * @brief contador de ticks pendientes para ejecutar `task_adc_update`.
 */
extern volatile uint32_t g_task_adc_tick_cnt;

#ifdef __cplusplus
}
#endif

#endif  // TASK_INPUT_INTERFACE_H_
