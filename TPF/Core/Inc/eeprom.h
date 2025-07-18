#ifndef INC_EEPROM_H_
#define INC_EEPROM_H_

#include <stdint.h>
#include <stdbool.h>  // para el tipo bool

// Dirección I2C del AT24C256 (A0, A1, A2 = GND)
#define EEPROM_I2C_ADDR 0xA0  // 0x50 << 1

// === Funciones públicas ===

// Guarda: temperatura, tipo de café, y stocks
// Retorna true si se escribió algo en la EEPROM
bool eeprom_save_config(void);

// Carga: temperatura, tipo de café, y stocks desde EEPROM
void eeprom_load_config(void);

// Lectura y escritura directa (1 byte)
uint8_t eeprom_read_byte(uint16_t mem_address);

#endif /* INC_EEPROM_H_ */
