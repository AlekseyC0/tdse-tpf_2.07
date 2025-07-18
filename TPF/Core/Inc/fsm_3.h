#ifndef FSM_3_H
#define FSM_3_H

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

// Estados del sistema
typedef enum {
    estado_off,
    estado_menu_principal,
    estado_seleccion_cafe,
    estado_brewing,
    estado_save_energy,
    estado_fabricante,           // 🆕 estado principal técnico
    estado_submenu_stock,        // 🆕 submenú de stock
    estado_submenu_temp_deseada  // 🆕 submenú de temperatura
} estados;


typedef enum {
    CAFE_LATTE,
    CAFE_MOCHA,
    CAFE_DOBLE,
    CAFE_AMERICANO
} tipo_de_cafe;

extern estados estado_actual;
extern tipo_de_cafe cafe_seleccionado;

// Función principal de la FSM
void fsm_update(void);

// Temperatura deseada y actual
extern uint8_t temp_deseada;
extern uint8_t temp_actual;

#endif // FSM_3_H
