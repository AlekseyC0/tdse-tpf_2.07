/********************** EXPLICACION BREVE *****************************/
/*
 * Este modulo controla un sensor ultrasonico (HC-SR04) usando interrupciones de captura de tiempo
 * (input capture) del temporizador TIM1. El pin TRIG se configura como salida para disparar el pulso
 * ultrasonico, y el pin ECHO se mide con precision mediante eventos de flanco del timer. La funcion
 * HCSR04_Trigger() lanza una medicion, y la FSM (sensor_estado)gestiona la secuencia de captura: primero
 * detecta el flanco de subida (inicio del eco) y luego el flanco de bajada (fin del eco). El tiempo
 * entre ambos se convierte en distancia en centrimetros. El resultado se almacena en dist y puede
 * consultarse con HCSR04_GetLast(). La funcion HCSR04_Ready() indica cuando una medicion esta lista
 * Todo el procesamiento de tiempo ocurre en la ISR HAL_TIM_IC_CaptureCallback(), permitiendo medir
 * sin bloquear y con alta precision. El diseno es modular, no bloqueante y adecuado el sistema.
*/
/**************************************************************************/

#include "task_sensor_ultrasonico_interface.h"
#include "task_sensor_ultrasonico_attribute.h"
#include "stm32f1xx_hal.h"
#include "logger.h"

/********************** configuración HW ******************************/
#define TRIG_GPIO GPIOC      
#define TRIG_PIN  GPIO_PIN_7    //define pin del trigger
extern TIM_HandleTypeDef htim1; // TIM1_CH1

/********************** etiquetas debug *******************************/
#define G_TASK_SENSOR_CNT_INI      0ul
#define G_TASK_SENSOR_TICK_CNT_INI 0ul

#define DEL_SENSOR_MIN             0ul
#define DEL_SENSOR_MED             50ul
#define DEL_SENSOR_MAX             500ul

const char *p_task_sensor   = "Task SENSOR";
const char *p_task_sensor_  = "Non-Blocking & Update By Time Code";

/********************** variables públicas ****************************/
uint32_t g_task_sensor_cnt;
volatile uint32_t g_task_sensor_tick_cnt;

/********************** variables internas ****************************/
static volatile uint32_t t_ini = 0;
static volatile uint32_t t_end = 0;
static volatile uint16_t dist = 0;
static volatile bool flag_ready = false;

/********************** FSM ultrasónico ******************************/
static volatile sensor_estado_t sensor_estado = SENSOR_IDLE;

/********************** inicialización del sensor *********************/
void task_sensor_ultrasonico_init(void *params) //inicializa sensor
{
    // GPIO TRIG 
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitStruct.Pin = TRIG_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(TRIG_GPIO, &GPIO_InitStruct);
    HAL_GPIO_WritePin(TRIG_GPIO, TRIG_PIN, GPIO_PIN_RESET);

    // timer en modo input capture
    HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_1);
    __HAL_TIM_SET_CAPTUREPOLARITY(&htim1, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);

    // log
    LOGGER_INFO(" ");
    LOGGER_INFO("  %s is running - %s", GET_NAME(task_sensor_ultrasonico_init), p_task_sensor);
    LOGGER_INFO("  %s is a %s", GET_NAME(task_sensor_update), p_task_sensor_);
    g_task_sensor_cnt = G_TASK_SENSOR_CNT_INI;
    LOGGER_INFO("   %s = %lu", GET_NAME(g_task_sensor_cnt), g_task_sensor_cnt);
}

/********************** función periódica del sistema *****************/
void task_sensor_ultrasonico_update(void *params) //actualiza sensor
{
    bool b_time_update_required = false; 

    __asm("CPSID i"); //deshabilito interrupciones para codigo bloqueante
    if (G_TASK_SENSOR_CNT_INI < g_task_sensor_tick_cnt) //compara
    {
        g_task_sensor_tick_cnt--; //si hay ticks pendientes decrementa
        b_time_update_required = true;
    }
    __asm("CPSIE i");// rehabilito interrupciones

    if (b_time_update_required)
    {
        g_task_sensor_cnt++; // incremento contador de ciclos de tarea 

        __asm("CPSID i");// deshabilito interrupciones 

        if (G_TASK_SENSOR_CNT_INI < g_task_sensor_tick_cnt) //comparar
        {
            g_task_sensor_tick_cnt--; //si hay ticks pendientes decrementa
            b_time_update_required = true;
        }
        else
        {
            b_time_update_required = false; //si no hay ticks pendientes no hace falta actualizar aun
        }
        __asm("CPSIE i"); //rehabilito interrupciones
    }
}

/********************** API pública de medición ************************/
void HCSR04_Trigger(void) //lanza pulso ultrasonico
{
    if (sensor_estado != SENSOR_IDLE) return;
    //verifica que el sensor este en idle

    flag_ready = false;
    sensor_estado = SENSOR_WAIT_ECHO_START;
    //cambia estado a esperando eco
    HAL_GPIO_WritePin(TRIG_GPIO, TRIG_PIN, GPIO_PIN_SET);
    uint32_t start = __HAL_TIM_GET_COUNTER(&htim1);
    //activa TRIG 10 us
    while ((__HAL_TIM_GET_COUNTER(&htim1) - start) < 10);  // delay_us(10)
    HAL_GPIO_WritePin(TRIG_GPIO, TRIG_PIN, GPIO_PIN_RESET);
    //lo vuelve a apagar para iniciar la medicion
}

bool HCSR04_Ready(void) //retorna true si hay medicion lista
{
    return flag_ready;
}

uint16_t HCSR04_GetLast(void) //retorna ultima distancia medida
{
    flag_ready = false;
    return dist;
}

/********************** interrupción de captura TIM ********************/
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) //llamada por interrupcion cuando se detecta flanco (rise o fall) en canal del timer
{
    if (htim->Channel != HAL_TIM_ACTIVE_CHANNEL_1) return;

    switch (sensor_estado) {
        case SENSOR_WAIT_ECHO_START:
            t_ini = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
            __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_FALLING);
            sensor_estado = SENSOR_WAIT_ECHO_END;
            break;
            //si 'esperando flanco de subida', guarda t_ini y cambia a 'esperar flanco bajada'

        case SENSOR_WAIT_ECHO_END:
            t_end = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
            __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);

            uint32_t delta = (t_end >= t_ini) ? (t_end - t_ini) : (0xFFFF - t_ini + t_end);
            dist = (uint16_t)((delta * 0.0343f) / 2.0f);  // distancia en cm
            flag_ready = true;
            sensor_estado = SENSOR_IDLE;
            break;
            //si esta esperando 'flanco de bajada', guarda t_end, calcula delta (duracion el pulso), y la convierte a distancia,marca 'ready', vuelve a 'idle'

        default:
            break;
    }
}

/********************** autotest del sensor ****************************/
bool sensor_ultrasonico_test(void) //lanza disparo
{
    HCSR04_Trigger();
    HAL_Delay(100); //espera 100ms 
    if (!HCSR04_Ready()) return false;
    uint16_t d = HCSR04_GetLast();
    return (d > 0 && d < 200); //chequea si recibio valor valido entre 1 y 200 cm 
}
