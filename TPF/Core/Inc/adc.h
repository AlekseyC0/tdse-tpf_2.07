/*
 * adc.h
 *
 *  Created on: Jun 16, 2025
 *      Author: alekseychahuillco
 */

#ifndef ADC_H
#define ADC_H

#include <stdint.h>

// Inicializa el ADC (opcional si usás CubeMX)
void adc_init(void);

// Leer un canal específico del ADC
uint16_t adc_read_channel(uint32_t channel);

// Funciones específicas para sensores
uint16_t adc_read_joystick(void);
uint16_t adc_read_temp(void);

#endif // ADC_H

