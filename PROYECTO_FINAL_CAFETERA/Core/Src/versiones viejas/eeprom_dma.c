#include "eeprom_dma.h"
#include "stm32f1xx_hal.h"
#include "stock.h"
#include "fsm_3.h"

extern I2C_HandleTypeDef hi2c2;

#define EEPROM_I2C_ADDR     0xA0
#define EEPROM_MEM_ADDR     0x0000
#define EEPROM_TIMEOUT_MS   10

typedef struct {
    uint8_t temp_deseada;
    uint8_t cafe_seleccionado;
    uint8_t stock[4];
} __attribute__((packed)) ConfigEEPROM;

static ConfigEEPROM config_rx;
static ConfigEEPROM config_tx;
static volatile bool transfer_done = false;
static volatile bool config_loaded = false;

void eeprom_dma_init(void) {
    // No se requiere inicialización especial
    transfer_done = false;
    config_loaded = false;
}

// === Callback de DMA completado ===
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
    if (hi2c == &hi2c2) {
        transfer_done = true;
        config_loaded = true;
    }
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c) {
    if (hi2c == &hi2c2) {
        transfer_done = true;
    }
}

// === Leer EEPROM (modo no bloqueante) ===
bool eeprom_read_config_async(void) {
    transfer_done = false;
    config_loaded = false;
    return HAL_I2C_Mem_Read_DMA(&hi2c2, EEPROM_I2C_ADDR, EEPROM_MEM_ADDR,
                                I2C_MEMADD_SIZE_16BIT, (uint8_t*)&config_rx,
                                sizeof(config_rx)) == HAL_OK;
}

// === Escribir EEPROM (modo no bloqueante) ===
bool eeprom_write_config_async(void) {
    transfer_done = false;

    config_tx.temp_deseada = temp_deseada;
    config_tx.cafe_seleccionado = cafe_seleccionado;
    config_tx.stock[0] = stock_cafe;
    config_tx.stock[1] = stock_leche;
    config_tx.stock[2] = stock_agua;
    config_tx.stock[3] = stock_chocolate;

    return HAL_I2C_Mem_Write_DMA(&hi2c2, EEPROM_I2C_ADDR, EEPROM_MEM_ADDR,
                                 I2C_MEMADD_SIZE_16BIT, (uint8_t*)&config_tx,
                                 sizeof(config_tx)) == HAL_OK;
}

// === Consultar si la última transferencia terminó ===
bool eeprom_transfer_complete(void) {
    return transfer_done;
}

// === Transferir datos leídos a variables globales ===
bool eeprom_get_loaded_config(void) {
    if (!config_loaded) return false;

    temp_deseada = (config_rx.temp_deseada >= 20 && config_rx.temp_deseada <= 90)
                   ? config_rx.temp_deseada : 60;

    cafe_seleccionado = (config_rx.cafe_seleccionado <= CAFE_AMERICANO)
                        ? config_rx.cafe_seleccionado : CAFE_LATTE;

    stock_cafe      = (config_rx.stock[0] <= 20) ? config_rx.stock[0] : 10;
    stock_leche     = (config_rx.stock[1] <= 20) ? config_rx.stock[1] : 10;
    stock_agua      = (config_rx.stock[2] <= 20) ? config_rx.stock[2] : 10;
    stock_chocolate = (config_rx.stock[3] <= 20) ? config_rx.stock[3] : 10;

    config_loaded = false;
    return true;
}
