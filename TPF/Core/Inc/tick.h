/*
 * tick.h
 *
 *  Created on: Jun 16, 2025
 *      Author: alekseychahuillco
 */

#ifndef TICK_H
#define TICK_H

#include <stdint.h>

// Inicialización (opcional si usás HAL_Init)
void tick_init(void);

// Devuelve el tiempo desde el arranque en milisegundos
uint32_t millis(void);

// Delay no bloqueante por software
void delay_ms(uint32_t ms);

// Verifica si pasó cierto tiempo (tipo timeout)
uint8_t timeout_elapsed(uint32_t start_time, uint32_t interval);

#endif // TICK_H
