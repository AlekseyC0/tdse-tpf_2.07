/*********************************************************************************
 * Este modulo implementa la maquina de estados finita principal de la cafetera, controlando todo el
 * flujo de interaccion del usuario, los estados del sistema y la coordinacion con perifericos. Esta
 * estructurada en multiples manejadores por estados (fsm_xxx_handle), cada uno dividido en subestados
 * que permiten una ejecucion no bloqueante (tipo "por pasos") compatible con un sistema embebido.
 *
 * Los estados cubren desde el encedido (estado_off), lectura de EEPROM, diagnostico de hardware,
 * modos de fabricante y usuario, hasta la preparacion de cafe(estado_brewing) y gestion de ahorro de
 * energia (estado_save_eenergy). Cada transicion de estado es diaparada por eventos como botones,
 * joystick, tiempo inactivo o condiciones internas ( como temperatura o stock).
 *
 * El corazon de la FSM esta en cafe_fsc_update_core(), que detecta el evento actual, reinicia el
 * contador de inactividad (ultima_interaccion_ms) si hubo actividad, y delega el control al handler
 * del estado actual. Tambien define la logica de timeout para entrar en ahorro de energia.
 *
 * Adicionalmente, se usa task_cafe_fsm_update() como funcion periodica llamada desde el scheduler
 * app. ( main loop), que verifica si corresponde actualizar la FSM, incrementa el contador de
 * inactividad y llama a la FSM.
 *
 * En resumen, se encapsula toda la logica de comportamiento de la cafetera, integrando botones, EEPROM
 * sensores, LEDs, LCD y actuadores, de forma modular, escalable y organizada. Permite extender facil-
 * mente la funcionalidad, manteniendo un control estricto del flujo y del estado del sistema.
 *********************************************************************************/


/********************** inclusiones del sistema *******************************/
#include "main.h"
#include "stm32f1xx_hal.h"

/********************** utilidades generales **********************************/
#include "logger.h"
#include "dwt.h"

/********************** aplicación & tareas ***********************************/
#include "board.h"
#include "app.h"

#include "task_adc_interface.h"
#include "task_botones_attribute.h"
#include "task_actuator_interface.h"
#include "task_sensor_ultrasonico_attribute.h"
#include "task_lcd_attribute.h"

#include "task_stock_interface.h"

#include "task_system_attribute.h"

#include "task_cafe_fsm_attribute.h"
#include "task_cafe_fsm_interface.h"

#include <string.h>
#include <stdio.h>
/********************** macros y definiciones *********************************/
#define G_TASK_FSM_CNT_INI			0ul
#define G_TASK_FSM_TICK_CNT_INI		0ul

#define DEL_FSM_MIN					0ul
#define DEL_FSM_MED					50ul
#define DEL_FSM_MAX					500ul

/********************** variables internas y globales *************************/
//static uint32_t fsm_timer_ms = 0;
static uint8_t fsm_subestado = 0;
//fsm_cafe_t fsm_cafe;

const char *p_task_fsm  = "Task FSM (Coffe Statechart)";
const char *p_task_fsm_ = "Non-Blocking & Update By Time Code";

/********************** variables externas ************************************/
uint32_t g_task_cafe_fsm_cnt;
volatile uint32_t g_task_cafe_fsm_tick_cnt;

extern volatile uint8_t eeprom_terminado;
extern uint8_t temp_deseada;
extern uint8_t temp_actual;
extern uint32_t inicio_brewing_ms;
extern const uint32_t DURACION_BREWING_MS;
extern uint32_t adc_value[3];
extern uint16_t distancia_actual;

const uint32_t IDLE_TIMEOUT_MS = 30000;//tiempo de inactividad que activara el modo de ahorro de energia
uint8_t receta_personal[4] = {0};  // café, leche, agua, chocolate
tipo_de_cafe cafe_seleccionado = CAFE_LATTE;
estados estado_actual = estado_off;
uint32_t ultima_interaccion_ms;

void task_cafe_fsm_init(void *parameters)//inicializa la FSM, reinicia contadores y timeout
{

    LOGGER_INFO(" ");
    LOGGER_INFO("  %s is running - %s", GET_NAME(task_cafe_fsm_init), p_task_fsm);
    LOGGER_INFO("  %s is a %s", GET_NAME(task_cafe_fsm), p_task_fsm_);

    g_task_cafe_fsm_cnt = G_TASK_FSM_CNT_INI;
    ultima_interaccion_ms = 0;
    LOGGER_INFO("   %s = %lu", GET_NAME(g_task_cafe_fsm_cnt), g_task_cafe_fsm_cnt);

}

static bool sistema_autotest_ok(void)//devuelve true si todos los test funcionen correctamente
{
    lcd_test();
    return  sensor_ultrasonico_test()
         && eeprom_test()
        && potenciometro_test()
         && test_leds();
}

evento_t detectar_evento(void) //evalua posibles inputs (botones+joystick)
{
    if (boton1_falling_edge()) return EVENT_BOTON1_EDGE;
    if (boton1_presionado())   return EVENT_BOTON1_PRESS;
    if (boton2_presionado())   return EVENT_BOTON2_PRESS;
    if (boton3_presionado())   return EVENT_BOTON3_PRESS;
    if (joystick_arriba())     return EVENT_JOY_UP;
    if (joystick_abajo())      return EVENT_JOY_DOWN;
    if (joystick_izquierda())  return EVENT_JOY_LEFT;
    if (joystick_derecha())    return EVENT_JOY_RIGHT;
    if (ultima_interaccion_ms > IDLE_TIMEOUT_MS) return EVENT_TIMEOUT; //si sistema inactivo por mas de 30s devuelve timeout
    return EVENT_NONE; //si no ocurre nada event_none
}

static void fsm_off_handle(evento_t e)//cuando sist en off
{
    switch (fsm_subestado) {
        case 0: //estamos en estado idle
            mostrar_estado_leds(estado_off); 
            lcd_print_if_changed(0, "Presione Start  ");
            lcd_print_if_changed(1, "Mod Fab: BotAzul");
            fsm_subestado = 1;
            break;

        case 1: //se aprotó boton
            if (e == EVENT_BOTON1_EDGE) {
                lcd_print_if_changed(0, "Cargando...");
                eeprom_start_read();
                fsm_subestado = 2;
				//se carga eeprom 
            }
            else if (e == EVENT_BOTON3_PRESS) {
                lcd_print_if_changed(0, "Modo FAB...");
                ultima_interaccion_ms = 0;
                fsm_subestado = 3;
				//se entra a modo fabrica
            }
            break;

        case 2: //termino de cargar la eeprom
            if (eeprom_terminado) {
            	eeprom_terminado = 0;
                lcd_print_if_changed(0, "Konnichiwa!!");
                lcd_print_if_changed(1, "");
                ultima_interaccion_ms = 0;
                fsm_subestado = 4;
				//saludamos :)
            }
            break;

        case 3: //sigue cargando para edicion
            if (ultima_interaccion_ms >= 1000) {
                lcd_print_if_changed(0, "Cargando EEPROM");
                eeprom_start_read();
                fsm_subestado = 5;
            }
            break;

        case 4: //salimos del estado idle a estado menu principal
            if (ultima_interaccion_ms >= 1000) { 
                estado_actual = estado_menu_principal;
                fsm_subestado = 0;
            }
            break;

        case 5: //terminas de cargar la eeprom y te vas a modo fabrica 
            eeprom_update();
            if (eeprom_terminado) {
            	eeprom_terminado = 0;
                estado_actual = estado_fabricante;
                fsm_subestado = 0;
            }
            break;
    }
}

static void fsm_diagnostico_handle(evento_t e) //autodiagnostico, si algo falla entra a estado error
{
    static bool test_ejecutado = false;

    if (!test_ejecutado) //si todavia no se ejecuto test entra al bloque 
	{
        if (sistema_autotest_ok()) {
            // reinicio micro si todos tests pasan
            NVIC_SystemReset();
        } else {
            lcd_print_if_changed(0, "Falla detectada");
            estado_actual = estado_error;
			//si algo falla en la prueba, se muestra mensaje de error y cambia a estado 'error'para evitar continuar con flujo normal 
        }

        test_ejecutado = true; //test ya se ejecuto para no repetir
        ultima_interaccion_ms = 0; //resetea contador inactividad a 0
    }

}

static void fsm_fabricante_handle(evento_t e) //maneja modo fabrica
{
	static uint8_t menu_idx = 0;
	const char* menus[] = { "Stock actual", "Temp deseada", "Prueba de HW" };
	//guarda el menu actual seleccionado 
	switch (fsm_subestado) {
		case 0:
			mostrar_estado_leds(estado_actual);
			lcd_print_if_changed(0, "FAB: Seleccione ");
			lcd_print_if_changed(1, menus[menu_idx]);

			if (e == EVENT_JOY_DOWN) //se mueve joystick hacia abajo  
			{
				menu_idx = (menu_idx + 1) % 3;
				ultima_interaccion_ms = 0;
				fsm_subestado = 1;
				//se incrementa indice de menu
			}
			else if (e == EVENT_JOY_UP) //se mueve joystick hacia arriba 
			{
				menu_idx = (menu_idx + 2) % 3;
				ultima_interaccion_ms = 0;
				fsm_subestado = 1;
				//se decrementa indice de menu 
			}
			else if (e == EVENT_BOTON1_EDGE) //se presiona boton azul
			{
				switch (menu_idx) {
					case 0: estado_actual = estado_submenu_stock; break;
					case 1: estado_actual = estado_submenu_temp_deseada; break;
					case 2: estado_actual = estado_diagnostico; break;
					//se entra al submenu correspondiente 
				}
				fsm_subestado = 0;
			}
			else if (e == EVENT_JOY_LEFT) //se mueve joystick izq 
			{
				lcd_print_if_changed(0, "Saliendo...");
				lcd_print_if_changed(1, " ");
				ultima_interaccion_ms = 0;
				fsm_subestado = 2;
				//se sale del modo fabricante
			}
			break;

		case 1:
			if (ultima_interaccion_ms >= 150) {
				fsm_subestado = 0;
				//espera 150ms antes de permitir volver al estado 0 para prevenir que joystick se saltee mutiples opciones al mantenerlo 
			}
			break;

		case 2:
			if (ultima_interaccion_ms >= 500) {
				estado_actual = estado_off;
				fsm_subestado = 0;
				//espera 500ms antes de salir definitivamente del modo fabrica
			}
			break;
	}
}

static void fsm_stock_handle(evento_t e) //maneja submenu_stock
{
	static uint8_t idx = 0; //indica cual ingrediente esta seleccionando del 0 al 3 
	static int nuevo = 0;   //valor temporal del stock que se esta editando
	static bool edit = false; //indica si estamos en modo edicion o solo navegando 


	const char* nombres[] = { "Cafe", "Leche", "Agua", "Choc" };
	int* stocks[] = { &stock_cafe, &stock_leche, &stock_agua, &stock_chocolate };
	//arreglo de punteros a las variables de stock globales, lo que permite acceder dinámicamente al stock seleccionado por idx
	
	switch (fsm_subestado)
	{
		case 0:  // mostrar pantalla
			if (!edit) {
				char linea[17];
				snprintf(linea, sizeof(linea), "%s: Stock=%d", nombres[idx], *stocks[idx]);
				lcd_print_if_changed(0, "Indique stock...");
				lcd_print_if_changed(1, linea);
				//si no estamos editando muestra stock actual del ingrediente seleccionado
			} else {
				char linea[17];
				snprintf(linea, sizeof(linea), "Nuevo %s: %2d", nombres[idx], nuevo);
				lcd_print_if_changed(0, linea);
				lcd_print_if_changed(1, "Presione Azul");
				//si estamos editando muestra valor temporal nuevo y mensaje para confirmar con boton azul
			}
			fsm_subestado = 1; //para esperar entrada del usuario
			break;

		case 1: // espera evento
			if (!edit) {
				//si no estamos editando
				if (e == EVENT_JOY_UP) {
					idx = (idx + 1) % 4;
					fsm_subestado = 0;
					//si joystick hacia arriba avanza al sig estado
				}
				else if (e == EVENT_JOY_DOWN) {
					idx = (idx == 0) ? 3 : idx - 1;
					fsm_subestado = 0;
					//si joystick hacia abajo retrocede 
				}
				else if (e == EVENT_BOTON1_EDGE) {
					edit = true;
					nuevo = *stocks[idx];
					fsm_subestado = 0;
					//si boton azul entra en modo edicion copiando el valor actual a nuevo
				}
			} else {
				//si estamos editando 
				if (e == EVENT_JOY_UP) {
					nuevo = (nuevo + 1) % 21;
					fsm_subestado = 0;
					//si joystick hacia arriba incrementa nuevo (hasta 20)
				}
				else if (e == EVENT_JOY_DOWN) {
					nuevo = (nuevo == 0) ? 20 : nuevo - 1;
					fsm_subestado = 0;
					//si joystick hacia abajo decrementa (hasta 0)
				}
				else if (e == EVENT_BOTON1_EDGE) {
					*stocks[idx] = nuevo;
					edit = false;
					lcd_print_if_changed(0, "Guardando...");
					ultima_interaccion_ms = 0;
					fsm_subestado = 2;
					//si boton azul guarda nuevo en stock real y pasa a subestado 2
				}
			}

			//no importa si estoy editando o no 
			if (e == EVENT_JOY_LEFT) {
				fsm_subestado = 10;
				//salida
			}
			break;

		case 2:  // iniciar guardado EEPROM
			eeprom_start_save();
			fsm_subestado = 12;
			break;

		case 12:  // esperar EEPROM
			if (eeprom_terminado) {
				eeprom_terminado = 0;
				lcd_print_if_changed(0, "Stock Guardado!");
				ultima_interaccion_ms = 0;
				fsm_subestado = 3;
			}
			break;

		case 3: //retardo luego del guardado
			if (ultima_interaccion_ms >= 600) {
				fsm_subestado = 0;
				//vuelve al subestado 0
			}
			break;

		case 10:  //mensaje de salida
			lcd_print_if_changed(0, "Saliendo...");
			lcd_print_if_changed(1, "");
			ultima_interaccion_ms = 0;
			fsm_subestado = 11;
			break;

		case 11: //sale de modo fabricante
			if (ultima_interaccion_ms >= 1000) {
				estado_actual = estado_fabricante;
				fsm_subestado = 0;
			}
			break;
	}
}

static void fsm_temp_deseada_handle(evento_t e) //maneja submenu_temp_deseada
{
	static uint8_t nueva_temp; //nueva temp medida por pote antes de confirmar cambio

	switch (fsm_subestado) 
	{
		case 0: //se lee valor ADC del canal 0 (potenciometro) 
		{ 
			nueva_temp = (adc_value[0] * 60) / 4095 + 30;

			lcd_print_if_changed(0, "Edit Temp");
			char linea[16];
			snprintf(linea, sizeof(linea), "T: %d/%d", nueva_temp, temp_deseada);
			// se imprime 'T: nueva / actual'
			lcd_print_if_changed(1, linea);

			if (e == EVENT_BOTON1_EDGE) //se presiona boton azul
			{
				temp_deseada = nueva_temp;
				lcd_print_if_changed(0, "Guardando...");
				eeprom_start_save();
				fsm_subestado = 10;
				//se actualiza temp_deseada, se lanza guardado a eeprom
			}

			if (e == EVENT_JOY_LEFT) //se mueve joystick hacia izq
			{
				lcd_print_if_changed(0, "Saliendo...");
				lcd_print_if_changed(1, "");
				ultima_interaccion_ms = 0;
				fsm_subestado = 2;
				//se cancela edicion
			}
			break;
		}

		case 10:  // esperar EEPROM
			if (eeprom_terminado) 
			{
				eeprom_terminado = 0;
				lcd_print_if_changed(0, "Temp Guardada!");
				ultima_interaccion_ms = 0;
				fsm_subestado = 1;
				//espera confirmacion que se guardo con exito eeprom
			}
			break;

		case 1:  // delay luego del guardado
			if (ultima_interaccion_ms >= 1000) 
			{
				fsm_subestado = 0;
				//espera un segundo y vuelve al estado 0
			}
			break;

		case 2:  // delay de salida
			if (ultima_interaccion_ms >= 500) 
			{
				estado_actual = estado_fabricante;
				fsm_subestado = 0;
				// desp de cancelar espera 500ms y vuelve a 'modo fabrica'
			}
			break;
	}
}

static void fsm_menu_principal_handle(evento_t e) //maneja pantalla principal del usuario
{
	switch (fsm_subestado)
	{
		case 0:  // evaluar temperatura
			mostrar_estado_leds(estado_actual);
			//se encienden leds correspondientes al estado actual 
			if (temp_actual <= temp_deseada) //si temp actual no alcanzo la deseada
			{
				lcd_print_if_changed(0, "Calentando...   ");
				char linea[16];
				snprintf(linea, sizeof(linea), "T:%d/%d", temp_actual, temp_deseada);
				lcd_print_if_changed(1, linea);
				//se mantiene en subestado (espera)
			}
			else
			{
				fsm_subestado = 1;
				//si temp fue alcanzada o superado se avanza al siguiente
			}
			break;

		case 1:  //por entrar a seleccion de cafe 
			lcd_print_if_changed(0, "Selecciona cafe ");
			lcd_print_if_changed(1, "mueva el cursor");
			fsm_subestado = 2;
			break;

		case 2: //entrando a seleccion de cafe
			if(e != EVENT_NONE){
				estado_actual = estado_seleccion_cafe;
				fsm_subestado = 0;
				//espera cualquier evento (joystick o boton)
				//entra a seleccion de cafe 
			}
			break;

	}
}

static void fsm_seleccion_cafe_handle(evento_t e) //maneja estado seleccion_cafe
{
	const char* nombres[] = {
		"Latte", "Mocha", "Doble", "Americano",
		"Pers. nuevo", "Pers. reciente"
	};
	static uint8_t TOTAL_CAFES = 6; //cantidad total de opciones de café (4 estándar + 2 personalizadas)
	char buf[17]; //Buffer para construir mensajes a mostrar en la segunda línea del LCD

	switch (fsm_subestado)
	{
	case 0:  // mostrar selección
		mostrar_estado_leds(estado_actual);
		//activa los LEDs según el estado actual (selección de café)

		lcd_print_if_changed(0, nombres[cafe_seleccionado]);
		//muestra el nombre del café actualmente seleccionado en la primera línea del LCD

		if (cafe_seleccionado < CAFE_PERSONALIZADO) {
			snprintf(buf, sizeof(buf), "Temp=%dC", temp_actual);
			//si es un café estándar, muestra la temp actual

		} else if (cafe_seleccionado == CAFE_PERSONALIZADO) {
			snprintf(buf, sizeof(buf), "Configura nuevo");
			//si es personalizado nueo indica que se debe configurar
		} else {
			snprintf(buf, sizeof(buf), "Usar ultimo");
			//si es personalizado usado indica 'usar ultimo'
		}
		
		lcd_print_if_changed(1, buf);
		fsm_subestado = 1;
		break;

	case 1: //esperar interacción

		if (e == EVENT_JOY_UP) //si joystick hacia arriba pasa a sig opcion cafe
		{
			cafe_seleccionado = (cafe_seleccionado + 1) % TOTAL_CAFES;
			fsm_subestado = 0;
		}

		if (e == EVENT_JOY_DOWN) //si joystick hacia abajo va hacia opcion anterior
		{
			cafe_seleccionado = (cafe_seleccionado == 0) ? TOTAL_CAFES - 1 : cafe_seleccionado - 1;
			fsm_subestado = 0;
		}

		if (e == EVENT_BOTON1_EDGE)//si boton azul confirma seleccion
		{
			if (cafe_seleccionado == CAFE_PERSONALIZADO) {
				estado_actual = estado_cafe_personalizado;
				fsm_subestado = 0;
				//si selecciono personalizado nuevo, se pasa a estado_cafe_personalizado
			} else if (cafe_seleccionado == CAFE_ULTIMO) {
				//si se eligio personalizado anterior
				if (check_stock_receta(receta_personal)) {
					descontar_stock_receta(receta_personal);
					lcd_print_if_changed(0, "Preparando...");
					ultima_interaccion_ms = 0;
					fsm_subestado = 2;
					//si hay stock, se descuenta y pasa a subestado 2
				} else {
					lcd_print_if_changed(0, "Sin stock!");
					ultima_interaccion_ms = 0;
					fsm_subestado = 3;
					//si no hay stock, se muestra sin stock y pasa a subestado 3
				}
			} else {
				if (check_stock(cafe_seleccionado)) {
					descontar_stock(cafe_seleccionado);
					lcd_print_if_changed(0, "Preparando...");
					ultima_interaccion_ms = 0;
					fsm_subestado = 2;
					//si hay stock, se descuenta y pasa a subestado 2
				} else {
					lcd_print_if_changed(0, "Sin stock!");
					ultima_interaccion_ms = 0;
					fsm_subestado = 3;
					//si no hay stock, se muestra sin stock y pasa a subestado 3 
				}
			}
		}

		if (e == EVENT_JOY_LEFT) //si joystick hacia izq se cancela seleccion
		{
			lcd_print_if_changed(0, "Saliendo...");
			lcd_print_if_changed(1, "");
			ultima_interaccion_ms = 0;
			fsm_subestado = 4;
			//salida
		}

		if (temp_actual <= temp_deseada) 
		{
			estado_actual = estado_menu_principal;
			fsm_subestado = 0;
			//si la temp baja de la deseada mientras el usuario elige, se vuelve al estado anterior
		}
		break;

	case 2:  //inicia guardado de config cafe en eeprom
		eeprom_start_save();
		fsm_subestado = 5;
		break;

	case 5:  //espero fin de guardado de eeprom
		if (eeprom_terminado) {
			eeprom_terminado = 0;
			estado_actual = estado_brewing;
			fsm_subestado = 0;
		}
		break;

	case 3:  //mensaje de error por timeout
		if (ultima_interaccion_ms >= 1000) {
			fsm_subestado = 0;
		}
		break;

	case 4:  //salida al estado OFF
		if (ultima_interaccion_ms >= 1000) {
			estado_actual = estado_off;
			fsm_subestado = 0;
		}
		break;
	}
}

static void fsm_personal_handle(evento_t e) //manejo estado_cafe_person
{
	static uint8_t receta_tmp[4] = {0}; //guarda receta que el usuario está armando temporalmente: [café, leche, agua, chocolate]
	static uint8_t ingrediente_idx = 0; //indice del ingrediente actualmente seleccionado (0 a 3)
	static const char* nombres[] = { "Cafe", "Leche", "Agua", "Choc" };

	char buf[17]; //buffer para construir mensajes a mostrar en el LCD (líneas de 16 caracteres + '\0')

	switch (fsm_subestado){
	
	case 0:  // mostrar ingrediente actual y cantidad
		snprintf(buf, sizeof(buf), "%s: %2d", nombres[ingrediente_idx], receta_tmp[ingrediente_idx]);
		lcd_print_if_changed(0, buf);
		//imprime en la primera línea qué ingrediente se está configurando y su cantidad

		uint8_t total_shots = receta_tmp[0] + receta_tmp[1] + receta_tmp[2] + receta_tmp[3];
		snprintf(buf, sizeof(buf), "Shots: %d", total_shots);
		lcd_print_if_changed(1, buf);
		//imprime en la segunda línea el total de shots ya asignados a la receta
		fsm_subestado = 1;
		break;

	case 1:  //esperar interacción
		if (e == EVENT_JOY_UP)//si joystick hacia arriba intenta aumentar shot del ingrediente actual 
		{
			uint8_t total = receta_tmp[0] + receta_tmp[1] + receta_tmp[2] + receta_tmp[3];
			if (receta_tmp[ingrediente_idx] < 20 &&
				total < 4 &&
				receta_tmp[ingrediente_idx] < get_stock(ingrediente_idx)) {
				receta_tmp[ingrediente_idx]++;
				//condiciones: máximo 20 por ingrediente, máximo 4 en total para la receta, no mas que stock disponible 
			}
			fsm_subestado = 0;
		}

		if (e == EVENT_JOY_DOWN)//si joystick hacia abajo, decrementa valor del ingrediente 
		{
			if (receta_tmp[ingrediente_idx] > 0) {
				receta_tmp[ingrediente_idx]--;
			}
			fsm_subestado = 0;
		}

		if (e == EVENT_JOY_RIGHT) //si joystick hacia der, cambia de ingrediente
		{
			ingrediente_idx = (ingrediente_idx + 1) % 4;
			fsm_subestado = 0;
		}

		if (e == EVENT_BOTON1_EDGE)//si boton azul 
		{
			uint8_t total = receta_tmp[0] + receta_tmp[1] + receta_tmp[2] + receta_tmp[3];
			//se calcula la suma total de shots
			if (total == 4 &&
				receta_tmp[0] <= stock_cafe &&
				receta_tmp[1] <= stock_leche &&
				receta_tmp[2] <= stock_agua &&
				receta_tmp[3] <= stock_chocolate) {
				//si total 4 y suf stock
				//guardar receta
				memcpy(receta_personal, receta_tmp, 4);
				stock_cafe      -= receta_tmp[0];
				stock_leche     -= receta_tmp[1];
				stock_agua      -= receta_tmp[2];
				stock_chocolate -= receta_tmp[3];
				//descuenta stock actual
				//actualiza seleccion seleccion
				cafe_seleccionado = CAFE_PERSONALIZADO;

				lcd_print_if_changed(0, "Guardando...");
				lcd_print_if_changed(1, "");
				eeprom_start_save();
				fsm_subestado = 4;
			} else {
				//caso invalido (excede stock o no suma 4)
				lcd_print_if_changed(0, "Error!");
				lcd_print_if_changed(1, "Stock/Max=4!");
				ultima_interaccion_ms = 0;
				fsm_subestado = 3;
			}
		}
		break;

	case 3:  
		//desp de 1s de mostrar el error, vuelve al menú de edición
		if (ultima_interaccion_ms >= 1000) 
		{
			fsm_subestado = 0;
		}
		break;

	case 4:  
		//espera confirmacion de guardado EEPROM
		if (eeprom_terminado) 
		{
			eeprom_terminado = 0;
			lcd_print_if_changed(0, "Guardado!");
			lcd_print_if_changed(1, "Preparando...");
			ultima_interaccion_ms = 0;
			fsm_subestado = 2;
		}
		break;

	case 2:  // delay previo a brewing
		if (ultima_interaccion_ms >= 800) 
		{
			estado_actual = estado_brewing;
			fsm_subestado = 0;
		}
		break;
	}
}

static void fsm_brewing_handle(evento_t e)//maneja el estado brewing
{
	static uint8_t shots[4] = {0}; //inicializo cuantas dosis de shots de cada ingretiente a servir
	static uint8_t ingrediente = 0; //indice del ingretinete
	static bool primera_carga = true; //se usa para evitar repetir carga de configuración del ingrediente
	static uint32_t shot_start_time = 0;//marca el inicio (en ms) del inicio del shot actual
	static bool taza_detectada = false;

	switch (fsm_subestado)
	{
	case 0:  //esperar a que el usario coloque la taza
		mostrar_estado_leds(estado_actual);
		lcd_print_if_changed(0, "Coloque la taza"); //imprime el mensaje en pantalla
		lcd_print_if_changed(1, "                ");
		HCSR04_Trigger(); //activa el sensor ultrasonido
		ultima_interaccion_ms = 0; //reinicia la ultima interracción con el usuario
		taza_detectada = false; //setea la detección en cero hasta pasarla el subestado 1
		fsm_subestado = 1;
		break;

	case 1:
		if (HCSR04_Ready()) {
			uint16_t d = HCSR04_GetLast();
			if (d > 0 && d <= 3) {
				taza_detectada = true; //si detecto taza (a menos de 3cm ) comienza a preparar
				lcd_print_if_changed(0, "Preparando...");
				fsm_subestado = 2;
			} else {
				ultima_interaccion_ms = 0; //si no detecto,reinicia el contador de interacción y va al subestado 3
				fsm_subestado = 3;
			}
		}
		break;

	case 2:
		if (taza_detectada) {
			if (cafe_seleccionado < CAFE_PERSONALIZADO) //usa receta personalizadqa si se seleccionó
				{
				shots[0] = recetas[cafe_seleccionado].cafe;
				shots[1] = recetas[cafe_seleccionado].leche;
				shots[2] = recetas[cafe_seleccionado].agua;
				shots[3] = recetas[cafe_seleccionado].chocolate;
			} else //usa la receta predefinida que se eligió
				{
				memcpy(shots, receta_personal, sizeof(shots));
			}
			ingrediente = 0;
			primera_carga = true; //indica que se la va a empezar a cargar a la taza
			fsm_subestado = 10;
		}
		break;

	case 3:  // Reintento
		if (ultima_interaccion_ms >= 1000) {
			HCSR04_Trigger(); //manda otro trigger del sensor para ver si detecta algo
			fsm_subestado = 1; //vuelve al subestado 1 para checkear lo que detecto
		}
		break;

	// === Secuencia de shots ===
	case 10:
		if (primera_carga) //carga los shots de cada ingrediente en la maquina
			{
			while (ingrediente < 4 && shots[ingrediente] == 0) {
				ingrediente++; //salta los ingredientes que no tengan l
			}
			primera_carga = false; //indica que ya arranco a cargar si termino de iterar
		}

		if (ingrediente >= 4) //si ya termino de iterar indica que el cafe esta listo
			{
			lcd_print_if_changed(0, "Cafe listo!");
			lcd_print_if_changed(1, "Retire la taza");
			fsm_subestado = 20;
		} else {
			led_off(ingrediente + 1); //prende el led que corresponde al ingrediente
			shot_start_time = HAL_GetTick(); //inicia el tiempo del shot
			fsm_subestado = 11;
		}
		break;

	case 11:
		if (HAL_GetTick() - shot_start_time >= 1000) {
			led_on(ingrediente + 1); //apaga el led del ingrediente correspondiente
			shots[ingrediente]--; //disminuye el stock del ingrediente
			fsm_subestado = 12;
		}
		break;

	case 12:
		if (HAL_GetTick() - shot_start_time >= 1200) {
			if (shots[ingrediente] > 0) {
				fsm_subestado = 10; //vuelve a cargar mas dosis del ingrediente si es necesario
			} else {
				ingrediente++;
				primera_carga = true; //indica de que sigue cargando ingredientes
				fsm_subestado = 10; //sigue al proximo ingrediente
			}
		}
		break;

	//espera retirar taza
	case 20:
		HCSR04_Trigger(); //activa el trigger para ver si el usuario retira la taza
		fsm_subestado = 21;
		break;

	case 21:
		if (HCSR04_Ready()) {
			uint16_t d = HCSR04_GetLast();
			if (d > 4 || d == 0xFFFF) {
				estado_actual = estado_menu_principal; //si retiro la taza vuelve a estado menu principal
				fsm_subestado = 0;
			} else {
				ultima_interaccion_ms = 0; //si no detecto,reinicia el temporizador de ultima interacción
				fsm_subestado = 22;
			}
		}
		break;

	case 22:
		if (ultima_interaccion_ms >= 1000) {
			fsm_subestado = 20; //si no detecta nada despues de 1000ms vuelve a activa el sensor
		}
		break;
	}
}

static void fsm_save_energy_handle(evento_t e) //maneja el estado de save energy
{
	switch (fsm_subestado)
	{
	case 0: //inicia el modo save emergy
		mostrar_estado_leds(estado_actual);
		lcd_print_if_changed(1, "");
		lcd_print_if_changed(0, "Modo ahorro...  ");
		fsm_subestado = 1;
		break;

	case 1:
		if (e != EVENT_NONE) //si detecta algo sale del modo save energy al modo off
			{
			lcd_print_if_changed(0, "Reactivando...  ");
			lcd_print_if_changed(1, "");
			estado_actual = estado_off;
			fsm_subestado = 0;
		} else {
			__WFI(); //se mantiene en el modo save energy
		}
		break;
	}
}

static void fsm_error_handle(evento_t e) //maneja el estado de error
{
	switch (fsm_subestado)
	{
	case 0: //se detecto un error de hardware
		lcd_print_if_changed(0, "ERROR DE HARDWARE");
		lcd_print_if_changed(1, "Presione Azul");
		fsm_subestado = 1;
		break;

	case 1: //vuelve a reiniciar el sistema
		if (e == EVENT_BOTON3_PRESS) {
			lcd_print_if_changed(0, "Reiniciando...");
			lcd_print_if_changed(1, " ");
			ultima_interaccion_ms = 0;
			fsm_subestado = 2;
		}
		break;

	case 2: //entra en modo diagnostico para que el usuario vea que falló
		if (ultima_interaccion_ms >= 1000) {
			estado_actual = estado_diagnostico;
			fsm_subestado = 0;
		}
		break;
	}
}

void cafe_fsm_update_core(void)
{
    evento_t evento = detectar_evento(); //lee el evento actual

    if (evento != EVENT_NONE) //si se detecta un evento reinicia el contador
    	{
        ultima_interaccion_ms = 0;
    }
    if (evento == EVENT_TIMEOUT && estado_actual != estado_save_energy) //si no detecto nada pasa al estado save energy (si es que ya no esta)
    {
    	estado_actual = estado_save_energy;
    	fsm_subestado =0;
    }
    if (evento == EVENT_BOTON2_PRESS) //si se presiona el boton 2 reinicia el sistema
    {
        lcd_print_if_changed(0, "Reiniciando...");
        HAL_Delay(1000);
        NVIC_SystemReset();
    }

    switch (estado_actual) //usa la función de cada estado dependiendo del estado actual
    {
        case estado_off:                  fsm_off_handle(evento); break;
        case estado_diagnostico:          fsm_diagnostico_handle(evento); break;
        case estado_fabricante:           fsm_fabricante_handle(evento); break;
        case estado_submenu_stock:        fsm_stock_handle(evento); break;
        case estado_submenu_temp_deseada: fsm_temp_deseada_handle(evento); break;
        case estado_menu_principal:       fsm_menu_principal_handle(evento); break;
        case estado_seleccion_cafe:       fsm_seleccion_cafe_handle(evento); break;
        case estado_cafe_personalizado:   fsm_personal_handle(evento); break;
        case estado_brewing:              fsm_brewing_handle(evento); break;
        case estado_save_energy:          fsm_save_energy_handle(evento); break;
        case estado_error:                fsm_error_handle(evento); break;
        default:                          fsm_error_handle(evento); break;
    }
}

void task_cafe_fsm_update(void *parameters)
{
    bool b_time_update_required = false;

    __asm("CPSID i"); //deshabilita las interrupciones para codigo bloqueante
    if (G_TASK_FSM_TICK_CNT_INI < g_task_cafe_fsm_tick_cnt) //comparo
    {
        g_task_cafe_fsm_tick_cnt--; //si hay ticks pendiente los decrementa
        b_time_update_required = true;
    }
    __asm("CPSIE i"); //rehabilita interrupciones
    //mientras hay ticks pendiente ejecuta tareas
    if (b_time_update_required)
    {
        g_task_cafe_fsm_cnt++; //incremento contador de cilco de tareas
        ultima_interaccion_ms++;
        cafe_fsm_update_core();  //activa la lógica de la FSM

       __asm("CPSID i"); //deshabilita las interrupciones para codigo bloqueante
        if (G_TASK_FSM_TICK_CNT_INI < g_task_cafe_fsm_tick_cnt)
        {
            g_task_cafe_fsm_tick_cnt--;//si hay ticks pendiente los decrementa
            b_time_update_required = true;
        }
        else
        {
            b_time_update_required = false;//si no quedan ticks pendientes no es necesario actualizar aún
        }
        __asm("CPSIE i");//rehabilita interrupciones
    }
}
