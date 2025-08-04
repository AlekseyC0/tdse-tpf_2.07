#define JOY_UMBRAL 1000  // margen para evitar sensibilidad excesiva
#define JOY_CENTRO 2048



#include "fsm_3.h"
#include "i2c_lcd.h"
#include "stock.h"
#include "gpio_control.h"
#include "eeprom.h"
#include <stdio.h>
#include <stdlib.h>
#include "non_blocking_delay.h"
#include "non_blocking_msg.h"

// === Variables de estado ===
estados estado_actual = estado_off;
tipo_de_cafe cafe_seleccionado = CAFE_LATTE;

uint8_t temp_deseada = 60;
uint8_t temp_actual = 25;
uint8_t nueva_temp = 0;

uint32_t ultima_interaccion_ms = 0;
const uint32_t IDLE_TIMEOUT_MS = 30000;

uint32_t inicio_brewing_ms = 0;
const uint32_t DURACION_BREWING_MS = 5000;

// === ADC lectura externa ===
extern uint32_t adc_value[3];  // valor de potenciómetro por DMA
extern volatile uint32_t timer_ms;


// === Filtro botones con rebote ===



bool joystick_derecha(void) {
    static bool prev_estado = false;
    bool estado_actual = (adc_value[2] > (JOY_CENTRO + JOY_UMBRAL));  // X
    if (estado_actual && !prev_estado) {
        prev_estado = true;
        return true;
    } else if (!estado_actual) {
        prev_estado = false;
    }
    return false;
}


bool joystick_arriba(void) {
    static bool prev_estado = false;
    bool estado_actual = (adc_value[1] > (JOY_CENTRO + JOY_UMBRAL));  // Y
    if (estado_actual && !prev_estado) {
        prev_estado = true;
        return true;
    } else if (!estado_actual) {
        prev_estado = false;
    }
    return false;
}

bool debounce(GPIO_TypeDef* port, uint16_t pin, uint32_t delay_ms) {
    static uint32_t last_time[10] = {0};  // índice por botón
    static bool last_state[10] = {0};
    uint8_t idx = pin % 10;

    bool current = (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_RESET);
    if (current && !last_state[idx] && (timer_ms - last_time[idx] > delay_ms)) {
        last_state[idx] = true;
        last_time[idx] = timer_ms;
        return true;
    } else if (!current) {
        last_state[idx] = false;
    }
    return false;
}


bool cafe_listo(void) {
    return (timer_ms - inicio_brewing_ms) >= DURACION_BREWING_MS;
}

bool boton_azul_presionado(void) {
    return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET;
}

bool boton_azul_sostenido(uint16_t ms) {
    static uint32_t t_ini = 0;
    if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET) {
        if (t_ini == 0) t_ini = timer_ms;
        if (timer_ms - t_ini >= ms) {
            t_ini = 0;
            return true;
        }
    } else {
        t_ini = 0;
    }
    return false;
}

// === Botones ===
bool boton1_presionado(void) { return debounce(BOTON1_GPIO_Port, BOTON1_Pin, 50); }
/*bool boton2_presionado(void) {
    static uint32_t last_press = 0;
    if (HAL_GPIO_ReadPin(BOTON2_GPIO_Port, BOTON2_Pin) == GPIO_PIN_RESET) {
        if (HAL_GetTick() - last_press > 200) {
            last_press = HAL_GetTick();
            return true;
        }
    }
    return false;
}
*/
	//bool boton2_presionado(void) { return debounce(BOTON2_GPIO_Port, BOTON2_Pin, 50); }
//bool boton3_presionado(void) { return debounce(BOTON3_GPIO_Port, BOTON3_Pin, 50); }




void mostrar_estado_leds(estados est) {
    led_on(1); led_on(2); led_on(3); led_on(4);
    switch (est) {
        case estado_off:             led_off(1); break;
        case estado_menu_principal:  led_off(2); break;
        case estado_seleccion_cafe:  led_off(3); break;
        case estado_fabricante:
            led_off(1); led_off(2); led_off(3); led_off(4); break;
        default: break;
    }
}

// === ADC suavizado ===
void actualizar_temperatura_actual(void) {
    static uint8_t temp_anterior = 0;
    uint8_t temp_leida = (adc_value[0] * 60) / 4095 + 30;
    if (abs((int)temp_leida - temp_anterior) >= 2) {
        temp_actual = temp_leida;
        temp_anterior = temp_leida;
    }
}

void fsm_update(void) {
    static tipo_de_cafe last_cafe = -1;
    static uint8_t last_temp = 0xFF;
    static uint8_t subestado_off = 0;
    static nb_msg_t mensaje_boot;
    static uint8_t subestado_brewing = 0;
    static uint8_t subestado_save = 0;
    static nb_msg_t mensaje_confirmacion;
    static uint8_t subestado_temp = 0;
    static uint8_t subestado_stock = 0;
    static uint8_t subestado_cafe = 0;




    // Mapear el valor del potenciómetro a temperatura deseada (por ejemplo 30–90°C)
    actualizar_temperatura_actual();
    mostrar_estado_leds(estado_actual);
    if (boton1_presionado() || joystick_arriba() || joystick_derecha())
        ultima_interaccion_ms = timer_ms;

    switch (estado_actual) {

    case estado_off: {
        switch (subestado_off) {
            case 0:
                lcd_put_cur(0, 0);
                lcd_send_string("Presione Start");
                subestado_off = 1;
                break;

            case 1:
                if (boton1_presionado()) {
                    static const char* textos[] = { "Cargando EEPROM...", "Konnichiwa !!", "Espere..." };
                    static const uint16_t tiempos[] = { 1000, 1500, 1000 };
                    nb_msg_init(&mensaje_boot, textos, tiempos, 3);
                    subestado_off = 2;
                }

                if (boton_azul_presionado()) {
                    static const char* textos[] = { "Modo FAB...", "Cargando EEPROM..." };
                    static const uint16_t tiempos[] = { 1000, 1000 };
                    nb_msg_init(&mensaje_boot, textos, tiempos, 2);
                    subestado_off = 3;
                }
                break;

            case 2:  // luego de boton1
                nb_msg_update(&mensaje_boot);
                if (nb_msg_finalizada(&mensaje_boot)) {
                    eeprom_load_config();
                    estado_actual = estado_menu_principal;
                    subestado_off = 0;
                }
                break;

            case 3:  // luego de boton azul
                nb_msg_update(&mensaje_boot);
                if (nb_msg_finalizada(&mensaje_boot)) {
                    eeprom_load_config();
                    estado_actual = estado_fabricante;
                    subestado_off = 0;
                }
                break;
        }
        break;
    }



		case estado_fabricante:
		    static uint8_t menu_idx = 0;
		    const char* menus[] = { "Stock actual", "Temp deseada" };

		    lcd_put_cur(0, 0);
		    lcd_send_string("FAB: Seleccione ");
		    lcd_put_cur(1, 0);
		    lcd_send_string(menus[menu_idx]);

		    if (joystick_arriba()) {
		        menu_idx = (menu_idx + 1) % 2;
		        HAL_Delay(150);
		    }

		    if (boton1_presionado()) {
		    	lcd_clear();
		        estado_actual = (menu_idx == 0) ? estado_submenu_stock : estado_submenu_temp_deseada;
		    }

		    if (boton_azul_sostenido(2000)) {
		        lcd_clear();
			    lcd_put_cur(0, 0);
		        lcd_send_string("Saliendo...");
		        HAL_Delay(500);
		        estado_actual = estado_off;
		    }

		    break;

		case estado_submenu_stock: {
		    static uint8_t idx = 0;
		    static int nuevo = 0;
		    static bool edit = false;
		    const char* nombres[] = { "Cafe", "Leche", "Agua", "Choc" };
		    int* stocks[] = { &stock_cafe, &stock_leche, &stock_agua, &stock_chocolate };

		    switch (subestado_stock) {
		        case 0:
		            lcd_put_cur(0, 0);
		            if (!edit) {
		                char linea[16];
		                sprintf(linea, "%s: Stock=%d", nombres[idx], *stocks[idx]);
		                lcd_send_string(linea);

		                if (joystick_arriba()) { idx = (idx + 1) % 4; lcd_clear(); }
		                if (boton1_presionado()) { edit = true; nuevo = *stocks[idx]; lcd_clear(); }
		            } else {
		                char linea[17];
		                sprintf(linea, "Nuevo %s: %2d", nombres[idx], nuevo);
		                lcd_send_string(linea);
		                if (joystick_arriba()) { nuevo = (nuevo + 1) % 21; lcd_clear(); }
		                if (boton1_presionado()) {
		                    *stocks[idx] = nuevo;
		                    edit = false;
		                    const char* msg[] = {
		                        eeprom_save_config() ? "Stock Guardada!" : "Sin cambios."
		                    };
		                    uint16_t dur[] = { 600 };
		                    nb_msg_init(&mensaje_confirmacion, msg, dur, 1);
		                    subestado_stock = 1;
		                }
		            }

		            if (boton_azul_sostenido(2000)) {
		                const char* msg[] = { "Saliendo..." };
		                uint16_t dur[] = { 1000 };
		                nb_msg_init(&mensaje_confirmacion, msg, dur, 1);
		                subestado_stock = 2;
		            }
		            break;

		        case 1:
		            nb_msg_update(&mensaje_confirmacion);
		            if (nb_msg_finalizada(&mensaje_confirmacion)) {
		                lcd_clear();
		                subestado_stock = 0;
		            }
		            break;

		        case 2:
		            nb_msg_update(&mensaje_confirmacion);
		            if (nb_msg_finalizada(&mensaje_confirmacion)) {
		                estado_actual = estado_fabricante;
		                subestado_stock = 0;
		            }
		            break;
		    }
		    break;
		}


		case estado_submenu_temp_deseada:
		    switch (subestado_temp) {
		        case 0:
		            lcd_put_cur(0, 0);
		            lcd_send_string("Edit Temp");
		            lcd_put_cur(1, 0);
		            nueva_temp = (adc_value[0] * 60) / 4095 + 30;
		            char linea[16];
		            sprintf(linea, "T: %d/%d", nueva_temp, temp_deseada);
		            lcd_send_string(linea);

		            if (boton1_presionado()) {
		                temp_deseada = nueva_temp;

		                const char* msg[] = {
		                    eeprom_save_config() ? "Temp Guardada!" : "Sin cambios."
		                };
		                uint16_t dur[] = { 1000 };
		                nb_msg_init(&mensaje_confirmacion, msg, dur, 1);
		                subestado_temp = 1;
		            }

		            if (boton_azul_sostenido(2000)) {
		                static const char* msg[] = { "Saliendo..." };
		                static const uint16_t dur[] = { 500 };
		                nb_msg_init(&mensaje_confirmacion, msg, dur, 1);
		                subestado_temp = 2;
		            }
		            break;

		        case 1:
		            nb_msg_update(&mensaje_confirmacion);
		            if (nb_msg_finalizada(&mensaje_confirmacion)) {
		                lcd_clear();
		                subestado_temp = 0;
		            }
		            break;

		        case 2:
		            nb_msg_update(&mensaje_confirmacion);
		            if (nb_msg_finalizada(&mensaje_confirmacion)) {
		                estado_actual = estado_fabricante;
		                subestado_temp = 0;
		            }
		            break;
		    }
		    break;

		case estado_menu_principal:
			lcd_put_cur(0, 0);
			lcd_send_string("Calentando...");
			lcd_put_cur(1, 0);
			char tmsg[16];
			sprintf(tmsg, "T:%d/%d", temp_actual, temp_deseada);
			lcd_send_string(tmsg);

			if (temp_actual >= temp_deseada) {
				lcd_clear();
				lcd_put_cur(0, 0);
				lcd_send_string("Selecciona cafe");
				HAL_Delay(1000);
				lcd_clear();
				estado_actual = estado_seleccion_cafe;
				ultima_interaccion_ms = timer_ms;
			}
			break;

		case estado_seleccion_cafe:
		    switch (subestado_cafe) {
		    	case 0:
		    		ultima_interaccion_ms = timer_ms;
		    		subestado_cafe = 1;
		    		break;
		        case 1:
		            if (joystick_arriba()) {
		                cafe_seleccionado++;
		                if (cafe_seleccionado > CAFE_AMERICANO)
		                    cafe_seleccionado = CAFE_LATTE;
		            }

		            if (joystick_derecha()) {
		                if (check_stock(cafe_seleccionado)) {
		                    descontar_stock(cafe_seleccionado);
		                    eeprom_save_config();

		                    lcd_clear();
		                    lcd_put_cur(0, 0);
		                    lcd_send_string("Preparando...");

		                    const char* msg[] = { "Preparando..." };
		                    uint16_t dur[] = { 1000 };
		                    nb_msg_init(&mensaje_confirmacion, msg, dur, 1);

		                    inicio_brewing_ms = timer_ms;
		                    subestado_cafe = 3;
		                } else {
		                    const char* msg[] = { "Sin stock!" };
		                    uint16_t dur[] = { 1500 };
		                    nb_msg_init(&mensaje_confirmacion, msg, dur, 1);
		                    subestado_cafe = 2;
		                }
		            }

		            if (boton_azul_sostenido(2000)) {
		                const char* msg[] = { "Saliendo..." };
		                uint16_t dur[] = { 500 };
		                nb_msg_init(&mensaje_confirmacion, msg, dur, 1);
		                subestado_cafe = 4;
		            }

		            if (cafe_seleccionado != last_cafe || temp_actual != last_temp) {
		                lcd_clear();
		                lcd_put_cur(0, 0);
		                switch (cafe_seleccionado) {
		                    case CAFE_LATTE:     lcd_send_string("Cafe: Latte"); break;
		                    case CAFE_MOCHA:     lcd_send_string("Cafe: Mocha"); break;
		                    case CAFE_DOBLE:     lcd_send_string("Cafe: Doble"); break;
		                    case CAFE_AMERICANO: lcd_send_string("Cafe: Americano"); break;
		                }
		                lcd_put_cur(1, 0);
		                char buf[16];
		                sprintf(buf, "Temp=%dC", temp_actual);
		                lcd_send_string(buf);

		                last_cafe = cafe_seleccionado;
		                last_temp = temp_actual;
		            }

		            if (temp_actual <= temp_deseada) {
		                estado_actual = estado_menu_principal;
		            }

		            if (timer_ms - ultima_interaccion_ms > IDLE_TIMEOUT_MS) {
		                estado_actual = estado_save_energy;
		            }
		            break;

		        case 2:  // Mensaje "Sin stock!"
		            nb_msg_update(&mensaje_confirmacion);
		            if (nb_msg_finalizada(&mensaje_confirmacion)) {
		                lcd_clear();
		                subestado_cafe = 1;
		                estado_actual = estado_menu_principal;
		            }
		            break;

		        case 3:  // Mensaje "Preparando..."
		            nb_msg_update(&mensaje_confirmacion);
		            if (nb_msg_finalizada(&mensaje_confirmacion)) {
		                lcd_clear();
		                estado_actual = estado_brewing;
		                ultima_interaccion_ms = timer_ms;
		                subestado_cafe = 1;
		            }
		            break;

		        case 4:  // Mensaje "Saliendo..."
		            nb_msg_update(&mensaje_confirmacion);
		            if (nb_msg_finalizada(&mensaje_confirmacion)) {
		                estado_actual = estado_off;
		                subestado_cafe = 1;
		            }
		            break;
		    }
		    break;


		    case estado_brewing:
		        switch (subestado_brewing) {
		            case 0:
		                led_wave_effect();
		                if (cafe_listo()) {
		                    lcd_clear();
		                    lcd_put_cur(0, 0);
		                    lcd_send_string("Cafe listo!");
		                    const char* msg[] = { "Cafe listo!" };
		                    uint16_t dur[] = { 1000 };
		                    nb_msg_init(&mensaje_confirmacion, msg, dur, 1);
		                    subestado_brewing = 1;
		                }
		                if (timer_ms - ultima_interaccion_ms > IDLE_TIMEOUT_MS) {
		                    estado_actual = estado_save_energy;
		                    subestado_brewing = 0;
		                }
		                break;

		            case 1:
		                nb_msg_update(&mensaje_confirmacion);
		                if (nb_msg_finalizada(&mensaje_confirmacion)) {
		                    lcd_clear();
		                    estado_actual = estado_menu_principal;
		                    ultima_interaccion_ms = timer_ms;
		                    subestado_brewing = 0;
		                }
		                break;
		        }
		        break;


		    case estado_save_energy:
		        switch (subestado_save) {
		            case 0:

		                lcd_send_string("Modo ahorro...");
		                const char* msg[] = { "Modo ahorro..." };
		                uint16_t dur[] = { 1000 };
		                nb_msg_init(&mensaje_confirmacion, msg, dur, 1);
		                subestado_save = 1;
		                break;

		            case 1:
		                nb_msg_update(&mensaje_confirmacion);
		                if (nb_msg_finalizada(&mensaje_confirmacion)) {
		                    lcd_clear();
		                    subestado_save = 2;
		                }
		                break;

		            case 2:
		                if (joystick_derecha() || boton1_presionado() || joystick_arriba()) {
		                    lcd_clear();
		                    lcd_put_cur(0, 0);
		                    lcd_send_string("Reactivando...");
		                    const char* msg[] = { "Reactivando..." };
		                    uint16_t dur[] = { 1000 };
		                    nb_msg_init(&mensaje_confirmacion, msg, dur, 1);
		                    subestado_save = 3;
		                }
		                break;

		            case 3:
		                nb_msg_update(&mensaje_confirmacion);
		                if (nb_msg_finalizada(&mensaje_confirmacion)) {
		                    lcd_clear();
		                    led_off(4);
		                    estado_actual = estado_menu_principal;
		                    ultima_interaccion_ms = timer_ms;
		                    subestado_save = 0;
		                }
		                break;
		        }
		        break;


		}
}


///------------------------------

#define JOY_UMBRAL 1000  // margen para evitar sensibilidad excesiva
#define JOY_CENTRO 2048



#include "fsm_3.h"
#include "i2c_lcd.h"
#include "stock.h"
#include "gpio_control.h"
#include "eeprom.h"
#include <stdio.h>
#include <stdlib.h>

// === Variables de estado ===
estados estado_actual = estado_off;
tipo_de_cafe cafe_seleccionado = CAFE_LATTE;

uint8_t temp_deseada = 60;
uint8_t temp_actual = 25;
uint8_t nueva_temp = 0;

uint32_t ultima_interaccion_ms = 0;
const uint32_t IDLE_TIMEOUT_MS = 30000;

uint32_t inicio_brewing_ms = 0;
const uint32_t DURACION_BREWING_MS = 5000;

// === ADC lectura externa ===
extern uint32_t adc_value[3];  // valor de potenciómetro por DMA

// === Filtro botones con rebote ===



bool joystick_derecha(void) {
    static bool prev_estado = false;
    bool estado_actual = (adc_value[2] > (JOY_CENTRO + JOY_UMBRAL));  // X
    if (estado_actual && !prev_estado) {
        prev_estado = true;
        return true;
    } else if (!estado_actual) {
        prev_estado = false;
    }
    return false;
}


bool joystick_arriba(void) {
    static bool prev_estado = false;
    bool estado_actual = (adc_value[1] > (JOY_CENTRO + JOY_UMBRAL));  // Y
    if (estado_actual && !prev_estado) {
        prev_estado = true;
        return true;
    } else if (!estado_actual) {
        prev_estado = false;
    }
    return false;
}

bool debounce(GPIO_TypeDef* port, uint16_t pin, uint32_t delay_ms) {
    static uint32_t last_time[10] = {0};  // índice por botón
    static bool last_state[10] = {0};
    uint8_t idx = pin % 10;

    bool current = (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_RESET);
    if (current && !last_state[idx] && (HAL_GetTick() - last_time[idx] > delay_ms)) {
        last_state[idx] = true;
        last_time[idx] = HAL_GetTick();
        return true;
    } else if (!current) {
        last_state[idx] = false;
    }
    return false;
}


bool cafe_listo(void) {
    return (HAL_GetTick() - inicio_brewing_ms) >= DURACION_BREWING_MS;
}

bool boton_azul_presionado(void) {
    return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET;
}

bool boton_azul_sostenido(uint16_t ms) {
    static uint32_t t_ini = 0;
    if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET) {
        if (t_ini == 0) t_ini = HAL_GetTick();
        if (HAL_GetTick() - t_ini >= ms) {
            t_ini = 0;
            return true;
        }
    } else {
        t_ini = 0;
    }
    return false;
}

// === Botones ===
bool boton1_presionado(void) { return debounce(BOTON1_GPIO_Port, BOTON1_Pin, 50); }
bool boton2_presionado(void) {
    static uint32_t last_press = 0;
    if (HAL_GPIO_ReadPin(BOTON2_GPIO_Port, BOTON2_Pin) == GPIO_PIN_RESET) {
        if (HAL_GetTick() - last_press > 200) {
            last_press = HAL_GetTick();
            return true;
        }
    }
    return false;
}
	//bool boton2_presionado(void) { return debounce(BOTON2_GPIO_Port, BOTON2_Pin, 50); }
	bool boton3_presionado(void) { return debounce(BOTON3_GPIO_Port, BOTON3_Pin, 50); }




void mostrar_estado_leds(estados est) {
    led_on(1); led_on(2); led_on(3); led_on(4);
    switch (est) {
        case estado_off:             led_off(1); break;
        case estado_menu_principal:  led_off(2); break;
        case estado_seleccion_cafe:  led_off(3); break;
        case estado_fabricante:
            led_off(1); led_off(2); led_off(3); led_off(4); break;
        default: break;
    }
}

// === ADC suavizado ===
void actualizar_temperatura_actual(void) {
    static uint8_t temp_anterior = 0;
    uint8_t temp_leida = (adc_value[0] * 60) / 4095 + 30;
    if (abs((int)temp_leida - temp_anterior) >= 2) {
        temp_actual = temp_leida;
        temp_anterior = temp_leida;
    }
}

void fsm_update(void) {
    static tipo_de_cafe last_cafe = -1;
    static uint8_t last_temp = 0xFF;
    // Mapear el valor del potenciómetro a temperatura deseada (por ejemplo 30–90°C)
    actualizar_temperatura_actual();
    mostrar_estado_leds(estado_actual);
    if (boton1_presionado() || boton2_presionado() || boton3_presionado())
        ultima_interaccion_ms = HAL_GetTick();

    switch (estado_actual) {

		case estado_off:
		    lcd_put_cur(0, 0);
		  	lcd_send_string("Presione Start");

			if (boton1_presionado()) {
			    lcd_clear();
			    lcd_put_cur(0, 0);
			    lcd_send_string("Cargando EEPROM...");
			    delay_ms(1000);

			    eeprom_load_config();  // <--- CARGA valores desde EEPROM aquí


				lcd_clear();
				lcd_put_cur(0, 0);
				lcd_send_string("Konnichiwa !!");
				delay_ms(1500);
				lcd_clear();
				lcd_put_cur(0, 0);
				lcd_send_string("Espere...");
				delay_ms(1000);
				estado_actual = estado_menu_principal;

			}

			if (boton_azul_presionado()) {
			    lcd_clear();
				lcd_put_cur(0, 0);
			    lcd_send_string("Modo FAB...");
			    delay_ms(1000);
			    lcd_clear();
			    lcd_put_cur(0, 0);
			    lcd_send_string("Cargando EEPROM...");
			    delay_ms(1000);

			    eeprom_load_config();  // <--- CARGA valores desde EEPROM aquí
			    estado_actual = estado_fabricante;
			}


			break;

		case estado_fabricante:
		    static uint8_t menu_idx = 0;
		    const char* menus[] = { "Stock actual", "Temp deseada" };

		    lcd_put_cur(0, 0);
		    lcd_send_string("FAB: Seleccione ");
		    lcd_put_cur(1, 0);
		    lcd_send_string(menus[menu_idx]);

		    if (joystick_arriba()) {
		        menu_idx = (menu_idx + 1) % 2;
		        delay_ms(150);
		    }

		    if (boton1_presionado()) {
		    	lcd_clear();
		        estado_actual = (menu_idx == 0) ? estado_submenu_stock : estado_submenu_temp_deseada;
		    }

		    if (boton_azul_sostenido(2000)) {
		        lcd_clear();
			    lcd_put_cur(0, 0);
		        lcd_send_string("Saliendo...");
		        delay_ms(500);
		        estado_actual = estado_off;
		    }

		    break;

		case estado_submenu_stock: {
		        static uint8_t idx = 0;
		        static int nuevo = 0;
		        static bool edit = false;
		        const char* nombres[] = { "Cafe", "Leche", "Agua", "Choc" };
		        int* stocks[] = { &stock_cafe, &stock_leche, &stock_agua, &stock_chocolate };

		        lcd_put_cur(0, 0);

		        if (!edit) {
		            char linea[16];
		            sprintf(linea, "%s: Stock=%d", nombres[idx], *stocks[idx]);
		            lcd_send_string(linea);

		            if (joystick_arriba()) { idx = (idx + 1) % 4; lcd_clear(); }
		            if (boton1_presionado()) { edit = true; nuevo = *stocks[idx]; lcd_clear(); }
		        } else {
		            char linea[17];
		            sprintf(linea, "Nuevo %s: %2d", nombres[idx], nuevo);
		            lcd_send_string(linea);
		            if (joystick_arriba()) { nuevo = (nuevo + 1) % 21; lcd_clear(); }
		            if (boton1_presionado()) {
		                *stocks[idx] = nuevo;
		                edit = false;
		                lcd_clear();
		    		    lcd_put_cur(0, 0);
		    		    if (eeprom_save_config()) {
		    		    	lcd_clear();
		    		    	lcd_put_cur(0,0);
		    		    	lcd_send_string("Stock Guardada!");
		    		    	} else {
		    		    	lcd_clear();
		    		    	lcd_put_cur(0,0);
		    		    	lcd_send_string("Sin cambios.");
		    		    	}
		    		    delay_ms(600);
		                lcd_clear();
		            }
		        }

		        if (boton_azul_sostenido(2000)) {
		            lcd_clear();
				    lcd_put_cur(0, 0);
		            lcd_send_string("Saliendo...");
		            delay_ms(1000);
		            estado_actual = estado_fabricante;
		        }
		        break;
		    }


		case estado_submenu_temp_deseada:{

		    lcd_put_cur(0, 0);
		    lcd_send_string("Edit Temp");
		    lcd_put_cur(1, 0);
		    nueva_temp = (adc_value[0] * 60) / 4095 + 30;
		    char linea[16];
		    sprintf(linea, "T: %d/%d", nueva_temp,temp_deseada);
		    lcd_send_string(linea);

		    if (boton1_presionado()) {
		    	temp_deseada = nueva_temp;
		    	if (eeprom_save_config()) {
		    	        lcd_clear();
				    	lcd_put_cur(0,0);
		    	        lcd_send_string("Temp Guardada!");
		    	    } else {
		    	        lcd_clear();
				    	lcd_put_cur(0,0);
		    	        lcd_send_string("Sin cambios.");
		    	    }
		    	delay_ms(1000);  // confirmación visual
		    	lcd_clear();
		    }

		    if (boton_azul_sostenido(2000)) {
		        lcd_clear();
			    lcd_put_cur(0, 0);
		        lcd_send_string("Saliendo...");
		        delay_ms(500);
		        estado_actual = estado_fabricante;
		    }

		    break;
		}
		case estado_menu_principal:
			lcd_put_cur(0, 0);
			lcd_send_string("Calentando...");
			lcd_put_cur(1, 0);
			char tmsg[16];
			sprintf(tmsg, "T:%d/%d", temp_actual, temp_deseada);
			lcd_send_string(tmsg);

			if (temp_actual >= temp_deseada) {
				lcd_clear();
				lcd_put_cur(0, 0);
				lcd_send_string("Selecciona cafe");
				delay_ms(1000);
				lcd_clear();
				estado_actual = estado_seleccion_cafe;
				ultima_interaccion_ms = HAL_GetTick();
			}
			break;

		case estado_seleccion_cafe:

			if (joystick_arriba()) {
					       cafe_seleccionado++;
					        if (cafe_seleccionado > CAFE_AMERICANO)
					       cafe_seleccionado = CAFE_LATTE;
			}

			if (joystick_derecha()) {
						if (check_stock(cafe_seleccionado)) {
							descontar_stock(cafe_seleccionado);
							eeprom_save_config();  // ← guarda tipo de café, stock, temperatura
							lcd_clear();
							lcd_put_cur(0, 0);
							lcd_send_string("Preparando...");
							inicio_brewing_ms = HAL_GetTick(); //  esto es clave
							estado_actual = estado_brewing;
						} else {
							lcd_clear();
							lcd_put_cur(0, 0);
							lcd_send_string("Sin stock!");
							delay_ms(1500);
							estado_actual = estado_menu_principal;

						}
					}

			if (boton_azul_sostenido(2000)) {
				lcd_clear();
					lcd_put_cur(0, 0);
					lcd_send_string("Saliendo...");
					delay_ms(500);
					estado_actual = estado_off;
				}

			if (cafe_seleccionado != last_cafe || temp_actual != last_temp) {
							lcd_clear();
							lcd_put_cur(0, 0);
							switch (cafe_seleccionado) {
								case CAFE_LATTE: lcd_send_string("Cafe: Latte"); break;
								case CAFE_MOCHA: lcd_send_string("Cafe: Mocha"); break;
								case CAFE_DOBLE: lcd_send_string("Cafe: Doble"); break;
								case CAFE_AMERICANO: lcd_send_string("Cafe: Americano"); break;
							}
							lcd_put_cur(1, 0);
							char buf[16];
							sprintf(buf, "Temp=%dC", temp_actual);
							lcd_send_string(buf);

							last_cafe = cafe_seleccionado;
							last_temp = temp_actual;
						}


			if (temp_actual <= temp_deseada) {
						estado_actual = estado_menu_principal;
					}

			if (HAL_GetTick() - ultima_interaccion_ms > IDLE_TIMEOUT_MS) {
				estado_actual = estado_save_energy;
		}

			break;

		case estado_brewing:
		    led_wave_effect();  // animación estilo serpiente/onda

			if (cafe_listo()) {
				lcd_clear();
				lcd_put_cur(0, 0);
				lcd_send_string("Cafe listo!");
				delay_ms(1000);
				estado_actual = estado_menu_principal;
				ultima_interaccion_ms = HAL_GetTick();
			}

			if (HAL_GetTick() - ultima_interaccion_ms > IDLE_TIMEOUT_MS) {
				estado_actual = estado_save_energy;
			}
			break;

		case estado_save_energy:
			lcd_clear();
			lcd_put_cur(0, 0);
			lcd_send_string("Modo ahorro...");
			delay_ms(1000);
			lcd_clear();
			  // Esperar botón para salir
				 if (joystick_derecha() || boton1_presionado()||joystick_arriba()) {
					   lcd_clear();
					   lcd_put_cur(0, 0);
					   lcd_send_string("Reactivando...");
					   delay_ms(1000);
					   led_off(4);
					   estado_actual = estado_menu_principal;
					   ultima_interaccion_ms = HAL_GetTick();
					 }
				 break;
		}
}

