#ifndef INC_TASK_BOTONES_ATTRIBUTE_H_
#define INC_TASK_BOTONES_ATTRIBUTE_H_

#include <stdbool.h>

/**
 * @file botones_attribute.h
 * @brief definiciones de estructuras, enums y funciones asociadas al manejo de botones físicos.
 */

/** 
 * @brief consulta si el botón 1 está presionado.
 * @return true si el botón está presionado, false en caso contrario.
 */
bool boton1_presionado(void);

/** 
 * @brief consulta si el botón 2 está presionado.
 * @return true si el botón está presionado, false en caso contrario.
 */
bool boton2_presionado(void);

/** 
 * @brief consulta si el botón 3 está presionado.
 * @return true si el botón está presionado, false en caso contrario.
 */
bool boton3_presionado(void);

/** 
 * @brief detecta un flanco descendente en el botón 1.
 * @return true si se detecta una transición de no presionado a presionado (flanco).
 */
bool boton1_falling_edge(void);

/**
 * @enum button_state_t
 * @brief estados posibles de un botón.
 */
typedef enum {
    BUTTON_RELEASED, /* el botón está liberado. */
    BUTTON_PRESSED   /* el botón está presionado. */
} button_state_t;

/**
 * @enum button_event_t
 * @brief eventos que puede generar un botón.
 */
typedef enum {
    EVENTO_NONE,   /* no hubo cambio de estado. */
    EVENT_PRESS,   /* se detectó una presión del botón. */
    EVENT_RELEASE  /* se detectó una liberación del botón. */
} button_event_t;

/**
 * @struct Button
 * @brief representa un botón físico con su estado y datos de antirrebote.
 */
typedef struct {
    GPIO_TypeDef* port;           /* puerto GPIO al que está conectado el botón. */
    uint16_t pin;                 /* pin específico del GPIO. */
    button_state_t state;         /* estado actual del botón. */
    bool last_read;               /* ultima lectura lógica del pin. */
    uint32_t last_change_time;    /* timestamp del último cambio de lectura. */
} Button;

#endif