/********************** EXPLICACION BREVE *****************************/
/*
 * Este modulo gestiona la lectura de los sensores analogicos mediante ADC con DMA, permitiendo obtener
 * datos en segundo plano sin bloquear la CPU. Captura tres canales: un potenciometro para controlar la
 * temperatura y dos para el joystick (ejes x e y). La funcion task_adc_update(); se ejecuta de forma
 * no bloqueante segun un contador de ticks, y se encarga de actualizar la temperatura (temp_aactual)
 * solo si cambia significativamente (+2 °C). Ademas, el modulo detecta eventos de joystick con umbrales
 * y debounce por software.
*/
/**************************************************************************/

#include "main.h"
#include "logger.h"
#include "dwt.h"
#include "board.h"
#include "task_adc_interface.h"

#include "task_actuator_interface.h"
#include "task_sensor_ultrasonico_interface.h"
#include "task_lcd_interface.h"
#include "task_system_interface.h"
#include "task_system_attribute.h"
#include "task_cafe_fsm_attribute.h"
#include "stm32f1xx_hal.h"
#include <stdlib.h>

#define JOY_UMBRAL 1000 //sensibilidad de joystick
#define JOY_CENTRO 2048 //centro esperado del joystick 

#define G_TASK_ADC_CNT_INI			0ul
#define G_TASK_ADC_TICK_CNT_INI		0ul

extern I2C_HandleTypeDef hi2c2;
extern ADC_HandleTypeDef hadc1;

/********************** variables globales ***********************************/
uint32_t g_task_adc_cnt;
volatile uint32_t g_task_adc_tick_cnt;
uint32_t adc_value[3]; //0: valor potenciometro ; 1: joystickx ; 2: joysticky

uint8_t temp_deseada = 50; 
uint8_t temp_actual = 0; 
uint32_t inicio_brewing_ms = 0; 

/********************** etiquetas debug **************************************/
const char *p_task_adc  = "Task ADC";
const char *p_task_adc_ = "Non-Blocking & Update By Time Code";

/********************** funciones privadas ***********************************/
static void actualizar_temperatura_actual(void) //convierte adc_value[0] del pote a tempo de 30 a 90 grados 
{
    static uint8_t temp_ant = 0;
    uint8_t temp = (adc_value[0] * 60) / 4095 + 30;

    //se actualiza sii temp_actual cambia +- 2 grados celsius de ultima medicion 
    if (abs((int)temp - temp_ant) >= 2) {
        temp_actual = temp;
        temp_ant = temp;
    }
}

static bool joystick_event(uint8_t canal, int signo)//detecta mov joystick en una direccion (+arriba/der; -abajo/izq)
{
    static bool prev[2] = {false, false};
    static uint32_t last_event_ms[2] = {0, 0};  // nuevo
    const uint32_t DEBOUNCE_MS = 300;           // ajustá según necesites distancia temporal min entre dos eventos 
    bool now = false;
    //canal 1: eje y 
    //canal 2: eje x
    uint8_t idx = canal - 1;
    if (signo > 0)
        now = (adc_value[canal] > JOY_CENTRO + JOY_UMBRAL);
    else
        now = (adc_value[canal] < JOY_CENTRO - JOY_UMBRAL);

    uint32_t now_ms = HAL_GetTick();

    if (now && !prev[idx]) {
        prev[idx] = true;
        if (now_ms - last_event_ms[idx] > DEBOUNCE_MS) {
            last_event_ms[idx] = now_ms;
            return true;
        }
    }
    if (!now) prev[idx] = false;
    return false;
}

/********************** API pública para eventos *****************************/
bool joystick_arriba(void)    { return joystick_event(1, +1); } //mov arriba
bool joystick_abajo(void)     { return joystick_event(1, -1); } //mov abajo
bool joystick_derecha(void)   { return joystick_event(2, +1); } //mov der
bool joystick_izquierda(void) { return joystick_event(2, -1); } //mov izq

bool potenciometro_test(void)//retorna false si pote da valores externos invalidos
{
    return !(adc_value[0] < 10 || adc_value[0] > 4090);
}

/********************** lógica principal: estado -> evento *******************/
void task_adc_update(void *params) //determina si es momento de realizar tarea y actualiza
{
    bool b_time_update_required = false;

    __asm("CPSID i");//deshabilita interrupciones para codigo bloqueante
    if (G_TASK_ADC_CNT_INI < g_task_adc_tick_cnt) //compara
    {
        g_task_adc_tick_cnt--; //si hay ticks pendientes lo decrementa
        b_time_update_required = true; 
    }
    __asm("CPSIE i");//// rehabilita interrupciones
     /*mientras haya ticks pendientes ejecutar tareas*/
    if (b_time_update_required) 
    {
        g_task_adc_cnt++;  //incremento contador de ciclo de tarea 
        actualizar_temperatura_actual();

        // rechequeo si quedó tick pendiente
        __asm("CPSID i"); //dehabilito interrupciones
        if (G_TASK_ADC_CNT_INI < g_task_adc_tick_cnt) //comparo
        {
            g_task_adc_tick_cnt--; //si hay ticks pendientes lo decrementa 
            b_time_update_required = true;
        }
        else
        {
            b_time_update_required = false; //si no quedan ticks pendientes no es necesario actualizar aun 
        }
        __asm("CPSIE i");// rehabilito interrupciones 
    }
}

/********************** inicialización ***************************************/
void task_adc_init(void *params) //inicializar tarea del adc
{
    HAL_ADC_Start_DMA(&hadc1, adc_value, 3); //activa adc en modo dma tal que resultados de conversion de los 3 canales se alamcenaran en array adc_value[]
    //DMA permite lectura continua sin intervencion del CPU, ideal para sistemas embebods con mutiples tareas
    LOGGER_INFO(" ");
    LOGGER_INFO("  %s is running - %s", GET_NAME(task_adc_init), p_task_adc);
    LOGGER_INFO("  %s is a %s", GET_NAME(task_adc_update), p_task_adc_);

    g_task_adc_cnt = G_TASK_ADC_CNT_INI; //reinicia contador global de ejecuciones de tareas adc en 0 
    LOGGER_INFO("   %s = %lu", GET_NAME(g_task_adc_cnt), g_task_adc_cnt);
}
