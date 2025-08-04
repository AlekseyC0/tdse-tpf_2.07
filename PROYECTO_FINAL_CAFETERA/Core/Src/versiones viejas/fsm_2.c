#include "fsm_3.h"
#include "teclado.h"
#include "i2c_lcd.h"
#include "stock.h"
#include <stdio.h>


// === Variables de estado ===
estados estado_actual = estado_off;
tipo_de_cafe cafe_seleccionado = CAFE_LATTE;

uint8_t temp_actual = 25;
uint8_t temp_deseada = 80;

uint32_t ultima_interaccion_ms = 0;
const uint32_t IDLE_TIMEOUT_MS = 60000;

uint32_t inicio_brewing_ms = 0;
const uint32_t DURACION_BREWING_MS = 5000;

bool cafe_listo(void) {
    return (HAL_GetTick() - inicio_brewing_ms) >= DURACION_BREWING_MS;
}

void mostrar_cafe_y_temp(void) {
    lcd_clear();
    lcd_put_cur(0, 0);
    switch (cafe_seleccionado) {
        case CAFE_LATTE:      lcd_send_string("Cafe: Latte"); break;
        case CAFE_MOCHA:      lcd_send_string("Cafe: Mocha"); break;
        case CAFE_DOBLE:      lcd_send_string("Cafe: Doble"); break;
        case CAFE_AMERICANO:  lcd_send_string("Cafe: Americano"); break;
    }
    lcd_put_cur(1, 0);
    char buf[16];
    sprintf(buf, "Temp=%dC", temp_deseada);
    lcd_send_string(buf);
}

void fsm_update(void) {
    static tipo_de_cafe last_cafe = -1;
    static uint8_t last_temp = 0xFF;

    char tecla = Keypad_Get_Char();

    switch (estado_actual) {

    case estado_off:
        if (tecla == 'D') {
            lcd_clear();
            lcd_put_cur(0, 0);
            lcd_send_string("Konnichiwa !!");
            HAL_Delay(1500);
            lcd_clear();
            lcd_put_cur(0, 0);
            lcd_send_string("Espere...");
            HAL_Delay(1000);
            ultima_interaccion_ms = HAL_GetTick();
            estado_actual = estado_menu_principal;
        }
        break;

    case estado_menu_principal:
        if (tecla == 'D') {
            temp_actual++;
        }

        lcd_put_cur(0, 0);
        lcd_send_string("Calentando...");
        lcd_put_cur(1, 0);
        char tmsg[16];
        sprintf(tmsg, "T:%d/%d", temp_actual, temp_deseada);
        lcd_send_string(tmsg);

        if (temp_actual >= temp_deseada) {
            lcd_clear();
            lcd_send_string("Selecciona cafe");
            HAL_Delay(1000);
            estado_actual = estado_seleccion_cafe;
            ultima_interaccion_ms = HAL_GetTick();
        }
        break;

    case estado_seleccion_cafe:
        if (tecla == 'A') {
            cafe_seleccionado--;
            if (cafe_seleccionado < CAFE_LATTE)
                cafe_seleccionado = CAFE_AMERICANO;
        } else if (tecla == 'B') {
            cafe_seleccionado++;
            if (cafe_seleccionado > CAFE_AMERICANO)
                cafe_seleccionado = CAFE_LATTE;
        } else if (tecla == 'C') {
            temp_deseada++;
        } else if (tecla == '#') {
            if (check_stock(cafe_seleccionado)) {
                descontar_stock(cafe_seleccionado);
                lcd_clear();
                lcd_send_string("Preparando...");
                inicio_brewing_ms = HAL_GetTick(); // ✅ esto es clave
                estado_actual = estado_brewing;
                return;
            } else {
                lcd_clear();
                lcd_send_string("Sin stock!");
                HAL_Delay(1500);
            }
        }

        // Solo actualizamos pantalla si cambió algo
        if (cafe_seleccionado != last_cafe || temp_deseada != last_temp) {
            mostrar_cafe_y_temp();
            last_cafe = cafe_seleccionado;
            last_temp = temp_deseada;
        }

        if (HAL_GetTick() - ultima_interaccion_ms > IDLE_TIMEOUT_MS) {
            estado_actual = estado_save_energy;
        }

        ultima_interaccion_ms = HAL_GetTick();
        break;

    case estado_brewing:
        if (cafe_listo()) {
            lcd_clear();
            lcd_send_string("Cafe listo!");
            HAL_Delay(2000);
            estado_actual = estado_seleccion_cafe;
            ultima_interaccion_ms = HAL_GetTick();
        }

        if (HAL_GetTick() - ultima_interaccion_ms > IDLE_TIMEOUT_MS) {
            estado_actual = estado_save_energy;
        }

        break;

    case estado_save_energy:
        lcd_clear();
        lcd_send_string("Modo ahorro...");
        HAL_Delay(1000);
        lcd_clear();  // simula apagado

        // Esperar entrada
        while (1) {
            char t = Keypad_Get_Char();
            if (t) {
                lcd_clear();
                lcd_send_string("Reactivando...");
                HAL_Delay(1000);
                estado_actual = estado_seleccion_cafe;
                ultima_interaccion_ms = HAL_GetTick();
                break;
            }
            HAL_Delay(50);
        }
        break;
    }
}

