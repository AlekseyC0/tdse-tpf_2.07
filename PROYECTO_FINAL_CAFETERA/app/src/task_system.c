/*
 * Este modulo gestiona la logica de sistema relacionada con la condfiguracion perisistente en la EEPROM.
 * Define una estructura FSM (eeprom_fsm) para realizar operaciones de lectura y escritura no bloqueantes
 * sobre la memoria EEPROM via I2C. Incluye funciones para cargar configuracion de inicio (eeprom_load_config),
 * iniciar un guardado (eeprom_start_save), y manejar un progreso (eeprom_save_update). Tambien permite
 * iniciar una lectura (eeprom_start_read) y completarla (eeprom_update), aplicando la configuracion leida
 * con aplicar_config. Esta disenado para ejecutarse periodicamente en task_system_update, verificando si
 * hay operaciones pendientes. Usa flags (eeprom_terminado, eeprom_guardado, eeprom_leyendo) para
 * sincronizar con otras tareas, asegurando persistencia de parametros como temperatura deseada,
 * receta personizada y stock.
 *
*/
/********************* includes **********************************************/
#include "stm32f1xx_hal.h"          // HAL de STM32
#include "task_system_interface.h"  
#include "task_system_attribute.h"  
#include "task_stock_interface.h"    // manejo stock (café, leche, etc.)
#include "task_cafe_fsm_attribute.h" //FSM principal
#include "task_lcd_interface.h"      // manejo LCD por I2C (si usás I2C)
#include "task_actuator_interface.h" //manejo LEDS
#include "logger.h"
#include <string.h>

/********************** macros y definiciones *********************************/
#define TASK_SYSTEM_DELAY_MS        2
#define EEPROM_I2C_ADDR             0xA0
#define EEPROM_CONFIG_ADDR          0x0000
#define EEPROM_TIMEOUT              5  // ms

#define G_TASK_SYSTEM_CNT_INI       0ul
#define G_TASK_SYSTEM_TICK_CNT_INI  0ul

#define DEL_SYSTEM_MIN              0ul
#define DEL_SYSTEM_MED              50ul
#define DEL_SYSTEM_MAX              500ul

/********************** constantes ********************************************/
const char *p_task_system  = "Task System";
const char *p_task_system_ = "Non-Blocking & Update By Time Code";

/********************** variables externas ************************************/
extern I2C_HandleTypeDef hi2c2;    // handler del bus I2C2
extern uint8_t receta_personal[4]; //arreglo global 
extern uint8_t temp_deseada;
extern tipo_de_cafe cafe_seleccionado;
extern int stock_cafe, stock_leche, stock_agua, stock_chocolate;

/********************** variables globales ************************************/
uint32_t g_task_system_cnt;
volatile uint32_t g_task_system_tick_cnt;

volatile uint8_t eeprom_terminado = 0; //flag: termino operacion
volatile uint8_t eeprom_leyendo = 0;   //flag: se esta leyendo
volatile uint8_t eeprom_guardando = 0; //flag: se esta escribiendo
static eeprom_fsm_t eeprom_fsm;

/********************** funciones privadas ************************************/
bool eeprom_test(void) //prueba basica de lectura/escritura 
{
    uint8_t test_val = 0xAB, read_val = 0;

    if (HAL_I2C_Mem_Write(&hi2c2, EEPROM_I2C_ADDR, 0x01FF, I2C_MEMADD_SIZE_16BIT, &test_val, 1, 50) != HAL_OK)
        return false;

    HAL_Delay(5); // tiempo de espera de escritura

    if (HAL_I2C_Mem_Read(&hi2c2, EEPROM_I2C_ADDR, 0x01FF, I2C_MEMADD_SIZE_16BIT, &read_val, 1, 50) != HAL_OK)
        return false;

    return read_val == test_val; // solo retorna true si se escribio y leyó bien
}

static void aplicar_config(ConfigEEPROM *cfg) //copia datos del buffer (temp,selec cafe,stock,personaliz) al sistema global 
{
    temp_deseada = cfg->temp_deseada;
    cafe_seleccionado = cfg->cafe_seleccionado;

    stock_cafe      = cfg->stock[0];
    stock_leche     = cfg->stock[1];
    stock_agua      = cfg->stock[2];
    stock_chocolate = cfg->stock[3];

    memcpy(receta_personal, cfg->receta_personal, 4);
     //te guarda el último cafe seleccionado, si coincide con ser el personalizado te lo guarda, si no es así se pierde
}

/********************** funciones públicas ************************************/
void eeprom_load_config(void) //cargar configuración desde EEPROM 
{
    ConfigEEPROM leida;
    //intenta leer toda la estructura desde EEPROM
    if (HAL_I2C_Mem_Read(&hi2c2, EEPROM_I2C_ADDR, EEPROM_CONFIG_ADDR,
                         I2C_MEMADD_SIZE_16BIT, (uint8_t*)&leida,
                         sizeof(leida), EEPROM_TIMEOUT) != HAL_OK) {
        // si hay falla/error: carga valores por defecto
        temp_deseada = 60;
        cafe_seleccionado = CAFE_LATTE;
        stock_cafe = stock_leche = stock_agua = stock_chocolate = 10;
        return;
    }
    //validaciones básicas: temp en rango valido;tipo cafe dentro de num valida; stocks<20;
    temp_deseada       = (leida.temp_deseada >= 20 && leida.temp_deseada <= 90) ? leida.temp_deseada : 60;
    cafe_seleccionado  = (leida.cafe_seleccionado <= CAFE_AMERICANO) ? leida.cafe_seleccionado : CAFE_LATTE;

    stock_cafe         = (leida.stock[0] <= 20) ? leida.stock[0] : 10;
    stock_leche        = (leida.stock[1] <= 20) ? leida.stock[1] : 10;
    stock_agua         = (leida.stock[2] <= 20) ? leida.stock[2] : 10;
    stock_chocolate    = (leida.stock[3] <= 20) ? leida.stock[3] : 10;

    memcpy(receta_personal, leida.receta_personal, 4);
}

void eeprom_start_save(void) //guardar configuración (solo si cambia) 
{
    if (eeprom_fsm.state != EEPROM_STATE_IDLE) return;
   //arma una nueva estructura con los valores actuales
    ConfigEEPROM nueva = {
        .temp_deseada = temp_deseada,
        .cafe_seleccionado = cafe_seleccionado,
        .stock = { stock_cafe, stock_leche, stock_agua, stock_chocolate },
        .receta_personal = {
            receta_personal[0],
            receta_personal[1],
            receta_personal[2],
            receta_personal[3]
        }
    };
    //copia esa estructura al buffer e inicia proceso de guardado
    memcpy(eeprom_fsm.buffer, &nueva, sizeof(ConfigEEPROM));
    eeprom_fsm.index = 0;
    eeprom_guardando = 1;
    eeprom_terminado = 0;

    eeprom_fsm.state = EEPROM_STATE_START_WRITE;
}

void eeprom_save_update(void) //escribe un byte del buffer a la eeprom, avanza al indice y cuando termina marca eeprom terminado
{
    switch (eeprom_fsm.state) {
        case EEPROM_STATE_START_WRITE:
            eeprom_fsm.state = EEPROM_STATE_WRITING;
            break;
            //si estaba en 'start write' simplemente cambia estado a 'writing'

        case EEPROM_STATE_WRITING:
            if (HAL_I2C_Mem_Write(&hi2c2, EEPROM_I2C_ADDR,
                                  EEPROM_CONFIG_ADDR + eeprom_fsm.index,
                                  I2C_MEMADD_SIZE_16BIT,
                                  &eeprom_fsm.buffer[eeprom_fsm.index], 1,
                                  EEPROM_TIMEOUT) == HAL_OK) {
                //intenta escribir byte desde buffer en eeprom
                eeprom_fsm.index++; //si se pudo incremente indice
                if (eeprom_fsm.index >= sizeof(ConfigEEPROM)) {
                    eeprom_fsm.state = EEPROM_STATE_WRITE_DONE;
                    //si ya escribio toda estructura pasa a 'write done'
                }
            }
            break;

        case EEPROM_STATE_WRITE_DONE: 
            //si termino de guardar
            eeprom_guardando = 0; //deja de estar guardando
            eeprom_terminado = 1; //marca que termino
            eeprom_fsm.state = EEPROM_STATE_IDLE; //estado pasivo
            break;

        default:
            break;
    }
}

void eeprom_start_read(void) //inicia lectura no bloqueante del buffer(resetea indice y estados) 
{
    if (eeprom_fsm.state != EEPROM_STATE_IDLE) return;
    //si esta leyendo o terminado evitar reinicio
    eeprom_fsm.index = 0;
    eeprom_leyendo = 1;
    eeprom_terminado = 0;

    eeprom_fsm.state = EEPROM_STATE_START_READ;
}

void eeprom_update(void) //update info 
{
    switch (eeprom_fsm.state) {
        case EEPROM_STATE_START_READ:
            eeprom_fsm.state = EEPROM_STATE_READING;
            break;
            //cambia estado a estado de lectura activa (no hace nada)

        case EEPROM_STATE_READING:
            if (HAL_I2C_Mem_Read(&hi2c2, EEPROM_I2C_ADDR,
                                 EEPROM_CONFIG_ADDR + eeprom_fsm.index,
                                 I2C_MEMADD_SIZE_16BIT,
                                 &eeprom_fsm.buffer[eeprom_fsm.index], 1,
                                 EEPROM_TIMEOUT) == HAL_OK) {
                //lee byte desde eeprom a partir de indice actual y se guarda en buffer
                eeprom_fsm.index++;
                if (eeprom_fsm.index >= sizeof(ConfigEEPROM)) {
                    aplicar_config((ConfigEEPROM*)eeprom_fsm.buffer);
                    eeprom_fsm.state = EEPROM_STATE_READ_DONE;
                }
                //avanza indice una vez que lectura exitosa
            }
            break;

        case EEPROM_STATE_READ_DONE: //si se leyo todo 
            eeprom_leyendo = 0; //ya no esta leyendo
            eeprom_terminado = 1; //ya termino
            eeprom_fsm.state = EEPROM_STATE_IDLE; //cambia a estado pasivo
            break;

        default:
            break;
    }
}

/********************** tareas ************************************************/
void task_system_init(void *param)//inicializa tarea el sist 
{
    //inicializa el estado de la fsm desde eeprom
    eeprom_fsm.state = EEPROM_STATE_IDLE;
    eeprom_fsm.index = 0;
    //verifica si eeprom responde bien
    if (!eeprom_test()) {
        LOGGER_ERROR("EEPROM no detectada");
    }
    //carga la config guardada desde eeprom en bloque
    eeprom_load_config();

    LOGGER_INFO(" ");
    LOGGER_INFO("  %s is running - %s", GET_NAME(task_system_init), p_task_system);
    LOGGER_INFO("  %s is a %s", GET_NAME(task_system_update), p_task_system_);
    //inicializa contador de ejecuciones de la tarea
    g_task_system_cnt = G_TASK_SYSTEM_CNT_INI;
    LOGGER_INFO("   %s = %lu", GET_NAME(g_task_system_cnt), g_task_system_cnt);
}

void task_system_update(void *param)//ejecuta periodicamente tarea del sist 
{
    bool b_time_update_required = false;

    __asm("CPSID i");//desactiva interrupciones para evitar codigo bloqueante
    if (G_TASK_SYSTEM_TICK_CNT_INI < g_task_system_tick_cnt) //compara
    {
        g_task_system_tick_cnt--; //si hay ticks pendientes lo decrementa
        b_time_update_required = true;
    }
    __asm("CPSIE i"); //reactiva interrupciones
    
    /*mientras haya ticks pendientes ejecutar tareas*/
    if (b_time_update_required) {
        /*contador de ciclos y tiempo total*/
        g_task_system_cnt++;

        eeprom_update();
        eeprom_save_update();

        __asm("CPSID i"); //desactiva interrupciopciones
        if (G_TASK_SYSTEM_TICK_CNT_INI < g_task_system_tick_cnt) //compara
        {
            g_task_system_tick_cnt--; //si hay ticks pendientes lo decrementa
            b_time_update_required = true;
        } else {
            b_time_update_required = false;
        }
        __asm("CPSIE i");//reactiva interrupciones
    }
}

