#include "eeprom.h"
#include "stm32f1xx_hal.h"
#include "stock.h"
#include "fsm_3.h"

extern I2C_HandleTypeDef hi2c2;
#define EEPROM_I2C &hi2c2


#define ADDR_TEMP       0x0000
#define ADDR_CAFE       0x0001
#define ADDR_STOCK_BASE 0x0010  // Café, leche, agua, chocolate

// === Función auxiliar ===
static void eeprom_write_if_changed(uint16_t addr, uint8_t value) {
    uint8_t current = eeprom_read_byte(addr);
    if (current != value) {
        HAL_I2C_Mem_Write(EEPROM_I2C, EEPROM_I2C_ADDR, addr, I2C_MEMADD_SIZE_16BIT, &value, 1, HAL_MAX_DELAY);
        HAL_Delay(5);
    }
}

uint8_t eeprom_read_byte(uint16_t mem_address) {
    uint8_t data = 0;
    HAL_I2C_Mem_Read(EEPROM_I2C, EEPROM_I2C_ADDR, mem_address, I2C_MEMADD_SIZE_16BIT, &data, 1, HAL_MAX_DELAY);
    return data;
}

// === GUARDAR configuración solo si hubo cambios ===
bool eeprom_save_config(void) {
    bool guardo = false;

    // Temperatura
    uint8_t raw_temp = eeprom_read_byte(ADDR_TEMP);
    if (raw_temp != temp_deseada) {
        eeprom_write_if_changed(ADDR_TEMP, temp_deseada);
        guardo = true;
    }

    // Tipo de café
    uint8_t raw_cafe = eeprom_read_byte(ADDR_CAFE);
    if (raw_cafe != (uint8_t)cafe_seleccionado) {
        eeprom_write_if_changed(ADDR_CAFE, (uint8_t)cafe_seleccionado);
        guardo = true;
    }

    // Stocks
    const uint8_t stock_actual[4] = { stock_cafe, stock_leche, stock_agua, stock_chocolate };
    for (uint8_t i = 0; i < 4; i++) {
        uint8_t eeprom_stock = eeprom_read_byte(ADDR_STOCK_BASE + i);
        if (eeprom_stock != stock_actual[i]) {
            eeprom_write_if_changed(ADDR_STOCK_BASE + i, stock_actual[i]);
            guardo = true;
        }
    }

    return guardo;
}

// === CARGAR configuración desde EEPROM ===
void eeprom_load_config(void) {
    // Temperatura
	uint8_t raw_temp = eeprom_read_byte(ADDR_TEMP);
	if (raw_temp < 20 || raw_temp > 90) {
	    temp_deseada = 60;
	} else {
	    temp_deseada = raw_temp;
	}

    // Café seleccionado
    uint8_t raw_cafe = eeprom_read_byte(ADDR_CAFE);
    cafe_seleccionado = (raw_cafe <= CAFE_AMERICANO) ? (tipo_de_cafe)raw_cafe : CAFE_LATTE;

    // Stocks
    uint8_t raw_stock;
    raw_stock = eeprom_read_byte(ADDR_STOCK_BASE + 0);
    stock_cafe = (raw_stock <= 20) ? raw_stock : 10;

    raw_stock = eeprom_read_byte(ADDR_STOCK_BASE + 1);
    stock_leche = (raw_stock <= 20) ? raw_stock : 10;

    raw_stock = eeprom_read_byte(ADDR_STOCK_BASE + 2);
    stock_agua = (raw_stock <= 20) ? raw_stock : 10;

    raw_stock = eeprom_read_byte(ADDR_STOCK_BASE + 3);
    stock_chocolate = (raw_stock <= 20) ? raw_stock : 10;
}

