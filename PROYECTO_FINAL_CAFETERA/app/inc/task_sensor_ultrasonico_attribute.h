#include <stdint.h>
#include <stdbool.h>

/**
 * @enum sensor_estado_t
 * @brief estados de la máquina de estados del sensor ultrasónico.
 */
typedef enum {
    SENSOR_IDLE = 0,              /* sensor inactivo, listo para un nuevo trigger */
    SENSOR_WAIT_ECHO_START,       /* esperando flanco de subida del eco */
    SENSOR_WAIT_ECHO_END          /* esperando flanco de bajada del eco */
} sensor_estado_t;

/**
 * @brief dispara el sensor ultrasónico generando el pulso TRIG.
 *
 * debe llamarse solo si el sensor está en estado IDLE.
 */
void HCSR04_Trigger(void);

/**
 * @brief indica si hay una nueva medición lista para ser leída.
 *
 * @return true si hay una distancia nueva disponible, false si aún no se completó la medición.
 */
bool HCSR04_Ready(void);

/**
 * @brief devuelve la última distancia medida por el sensor en centímetros.
 *
 * @return distancia en cm. marca internamente el dato como leído.
 */
uint16_t HCSR04_GetLast(void);

/**
 * @brief realiza una prueba rápida del sensor ultrasónico.
 *
 * dispara el sensor, espera un eco, y evalúa si la distancia está dentro de un rango válido.
 *
 * @return true si el sensor funciona correctamente, false si hay un fallo.
 */
bool sensor_ultrasonico_test(void);


