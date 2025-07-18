/*
 * sensores.h
 *
 *  Created on: Jun 16, 2025
 *      Author: alekseychahuillco
 */

#ifndef SENSORS_H
#define SENSORS_H

#include <stdint.h>
#include <stdbool.h>

// Inicialización del sensor (si fuera necesaria)
void sensors_init(void);

// Leer temperatura actual (valor crudo ADC o en °C)
float get_temp_celsius(void);

// Setear temperatura deseada manualmente (SETUP)
void set_temp_deseada(uint8_t grados);
uint8_t get_temp_deseada(void);

// ¿Ya se alcanzó la temperatura objetivo?
bool temp_objetivo_alcanzado(void);

#endif // SENSORS_H

