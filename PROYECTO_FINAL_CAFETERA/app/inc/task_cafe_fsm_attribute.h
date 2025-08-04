/**
 * @file task_cafe_fsm_attribute.h
 * @brief Declaraciones para la máquina de estados finita principal del sistema de café.
 *
 * Este archivo define las estructuras, enumeraciones y variables necesarias para
 * el manejo del flujo de estados del sistema SmartCoffee, incluyendo el seguimiento
 * del estado actual, subestados y eventos externos. 
 */

#ifndef TASK_CAFE_FSM_H_
#define TASK_CAFE_FSM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief estados principales del sistema SmartCoffee.
 */
typedef enum {
    estado_off = 0,                   /* sistema apagado */
    estado_diagnostico,               /* modo de diagnóstico de hardware */
    estado_fabricante,                /* modo de configuración para fabricante */
    estado_submenu_stock,             /* submenú para edición de stock */
    estado_submenu_temp_deseada,      /* submenú para editar temperatura deseada */
    estado_menu_principal,            /* pantalla principal, muestra estado térmico */
    estado_seleccion_cafe,            /* selección del tipo de café */
    estado_cafe_personalizado,        /* edición de receta personalizada */
    estado_brewing,                   /* preparación del café */
    estado_save_energy,               /* modo de bajo consumo */
    estado_error                      /* estado de error del sistema */
} estados;

/**
 * @brief tipos de café disponibles en el sistema.
 */
typedef enum {
    CAFE_LATTE = 0,           /* café con leche */
    CAFE_MOCHA,               /* café con leche y chocolate */
    CAFE_DOBLE,               /* café doble carga */
    CAFE_AMERICANO,           /* café con agua extra */
    CAFE_PERSONALIZADO,       /* receta configurada por el usuario */
    CAFE_ULTIMO,              /* ultima receta personalizada guardada */
    TOTAL_CAFES               /* total de cafés definidos (para control) */
} tipo_de_cafe;

/**
 * @brief contexto opcional para representar el estado completo de la FSM.
 */
typedef struct {
    estados estado;                  /* estado actual */
    uint8_t subestado;               /* subestado dentro del estado */
    uint32_t timer_ms;               /* timer para delays no bloqueantes */
    uint8_t ingrediente_actual;      /* indice del ingrediente activo */
    bool primera_carga;              /* indica si se debe hacer carga inicial */
    bool taza_detectada;             /* flag de detección ultrasónica de taza */
    uint8_t menu_idx;                /* indice del menú activo */
    int32_t ultima_interaccion_ms;   /* tiempo sin interacción */
    uint8_t stock_idx;               /* indice de ingrediente en edición de stock */
    bool editando_stock;             /* bandera si se está editando stock */
    bool boton1_espera_liberacion;   /* evita múltiples flancos del botón */
    bool test_ejecutado;             /* flag para evitar repetir diagnósticos */
    uint8_t ingrediente_idx;         /* ingrediente en edición en modo personalizado */
    uint8_t shots_restantes[4];      /* shots disponibles por ingrediente */
    uint8_t receta_tmp[4];           /* receta temporal en modo personalizado */
} fsm_cafe_t;

/**
 * @brief lista de eventos que puede procesar la FSM.
 */
typedef enum {
    EVENT_NONE = 0,             /* sin evento */
    EVENT_BOTON1_PRESS,         /* botón azul presionado */
    EVENT_BOTON1_EDGE,          /* flanco descendente del botón azul */
    EVENT_BOTON2_PRESS,         /* botón 2 presionado */
    EVENT_BOTON3_PRESS,         /* botón 3 presionado */
    EVENT_JOY_UP,               /* joystick arriba */
    EVENT_JOY_DOWN,             /* joystick abajo */
    EVENT_JOY_LEFT,             /* joystick izquierda */
    EVENT_JOY_RIGHT,            /* joystick derecha */
    EVENT_TEMP_OK,              /* temperatura alcanzada */
    EVENT_TIMEOUT,              /* timeout por inactividad */
    EVENT_TAZA_DETECTADA,       /* taza detectada por sensor ultrasónico */
    EVENT_TAZA_RETIRADA,        /* taza retirada */
    EVENT_EEPROM_OK,            /* operación EEPROM completada */
    EVENT_STOCK_OK,             /* stock disponible */
    EVENT_STOCK_ERROR,          /* stock insuficiente */
    EVENT_RECETA_OK,            /* receta válida */
    EVENT_RECETA_ERROR          /* error en receta */
} evento_t;

/** @brief contador de ejecuciones de la tarea FSM. */
extern uint32_t g_task_cafe_fsm_cnt;

/** @brief tick no bloqueante usado por el scheduler para la FSM. */
extern volatile uint32_t g_task_cafe_fsm_tick_cnt;

#ifdef __cplusplus
}
#endif

#endif /* TASK_CAFE_FSM_H_ */
