/*
 * ui.h
 *
 *  Created on: Jun 16, 2025
 *      Author: alekseychahuillco
 */

#ifndef UI_H
#define UI_H

#include <fsm.h>

// Inicialización del LCD
void ui_init(void);

// Mostrar el café actualmente seleccionado
void ui_show_cafe(tipo_de_cafe cafe);

// Muestra el mensaje en el LCD (línea 1 y 2)
void lcd_print(const char* linea1, const char* linea2);

// Mostrar stock actual (opcional)
void ui_show_stock(void);

// Leer y actualizar la selección de café con el joystick
void joystick_update(void);

#endif // UI_H
