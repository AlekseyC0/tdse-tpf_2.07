#ifndef TASK_SYSTEM_ATTRIBUTE_H
#define TASK_SYSTEM_ATTRIBUTE_H
#include <stdint.h>

// Estructura de configuración almacenada
typedef struct {
    uint8_t temp_deseada;            /* temp objetivo en grados Celsius */
    uint8_t cafe_seleccionado;       /* tipo de café seleccionado (enum) */
    uint8_t stock[4];                /* stock actual: [café, leche, agua, chocolate] */
    uint8_t receta_personal[4];      /* receta personalizada: [café, leche, agua, chocolate] */
} ConfigEEPROM;

typedef enum {
    EEPROM_STATE_IDLE = 0,           /* estado inactivo: no se está leyendo ni escribiendo */
    EEPROM_STATE_START_WRITE,        /* se inicia el proceso de escritura */
    EEPROM_STATE_WRITING,            /* escribiendo byte por byte */
    EEPROM_STATE_WRITE_DONE,         /* escritura completada */
    EEPROM_STATE_START_READ,         /* se inicia el proceso de lectura */
    EEPROM_STATE_READING,            /* leyendo byte por byte */
    EEPROM_STATE_READ_DONE           /* lectura completada */
} eeprom_fsm_state_t;

typedef struct {
    eeprom_fsm_state_t state;                      /* estado actual de la FSM */
    uint8_t index;                                 /* indice del byte actual (0 a sizeof(ConfigEEPROM)) */
    uint8_t buffer[sizeof(ConfigEEPROM)];          /* buffer temporal para lectura o escritura */
} eeprom_fsm_t;


/**
 * @brief cnicia el proceso de guardado no bloqueante de la configuración actual en EEPROM.
 *
 *la escritura se realiza en múltiples pasos a través de llamadas a `eeprom_save_update()`.
 */
void eeprom_start_save(void);

/**
 * @brief inicia la lectura no bloqueante de la configuración almacenada en EEPROM.
 *
 * el contenido se reconstruye byte a byte llamando repetidamente a `eeprom_update()`.
 */
void eeprom_start_read(void);

/**
 * @brief avanza un paso en el proceso de lectura desde EEPROM.
 *
 * se debe llamar periódicamente hasta que `eeprom_terminado == 1`.
 */
void eeprom_update(void);

/**
 * @brief avanza un paso en el proceso de guardado a la EEPROM.
 *
 * se debe llamar periódicamente hasta que `eeprom_terminado == 1`.
 */
void eeprom_save_update(void);

/**
 * @brief carga la configuración directamente desde la EEPROM (lectura bloqueante).
 *
 * si la EEPROM no responde correctamente, se cargan valores por defecto.
 */
void eeprom_load_config(void);

/**
 * @brief realiza una prueba de lectura y escritura para verificar el correcto funcionamiento de la EEPROM.
 * @return `true` si la EEPROM respondió correctamente, `false` si hubo error.
 */
bool eeprom_test(void);

/**
 * @brief flag que indica si se completó una operación de lectura o escritura EEPROM.
 *
 * esta variable se pone en `1` cuando `eeprom_update()` o `eeprom_save_update()` finalizan con éxito.
 */
extern volatile uint8_t eeprom_terminado;

#endif

