/********************** EXPLICACION BREVE *****************************/
/*
 * Este modulo implementa el control no bloqueante del display LCD via i2c usando una maquina de
 * estados finitos. El display se actualiza por lineas mediante una cola (lcd_queue) que almacena los
 * mensajes pendientes. La funcon lcd_print_if_changed() compara el nuevo mensaje con el anterior (lcd_
 * shadow) para evitar escrituras redundantes. Si el texto cambio, se encola para su actualizacion. La
 * FSM (lcd_fsm_update()) recorre 3 esteados, IDLE, SET_CURSOR (posicion del cursor), y PRINTING (caracter
 * por caracter). Todo esto se ejecuta periodicamente desde task_lcd_update(), sincronizado con ticks
 * del sistema. Esto permite que el sistema imprima en pantalla sin bloquear otras tareas.
*/
/**************************************************************************/


#include "task_lcd_interface.h"
#include "task_lcd_attribute.h"
#include "main.h"
#include "logger.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

extern I2C_HandleTypeDef hi2c1; //I2C para communcarse con display
#define SLAVE_ADDRESS_LCD 0x4E //direccion esclavo LCD

/********************** macros y etiquetas debug *****************************/
#define G_TASK_LCD_CNT_INI        0ul
#define G_TASK_LCD_TICK_CNT_INI   0ul

#define DEL_LCD_MIN               0ul
#define DEL_LCD_MED               50ul
#define DEL_LCD_MAX               500ul

const char *p_task_lcd   = "Task LCD";
const char *p_task_lcd_  = "Non-Blocking & Update By Time Code";

/********************** variables globales públicas **************************/
uint32_t g_task_lcd_cnt;
volatile uint32_t g_task_lcd_tick_cnt;

/********************** funciones internas bajo nivel ************************/
static void lcd_send_cmd(char cmd) //envia comando al LCD en formato 4bit (upper y lower nibble)
{
    char data_u = (cmd & 0xF0);
    char data_l = ((cmd << 4) & 0xF0);
    uint8_t data_t[4] = {
        data_u | 0x0C, data_u | 0x08,
        data_l | 0x0C, data_l | 0x08
    };
    HAL_I2C_Master_Transmit(&hi2c1, SLAVE_ADDRESS_LCD, data_t, 4, 100);
}

static void lcd_send_data(char data) //envia datos al LCD en formato 4bit (upper y lower nibble)
{
    char data_u = (data & 0xF0);
    char data_l = ((data << 4) & 0xF0);
    uint8_t data_t[4] = {
        data_u | 0x0D, data_u | 0x09,
        data_l | 0x0D, data_l | 0x09
    };
    HAL_I2C_Master_Transmit(&hi2c1, SLAVE_ADDRESS_LCD, data_t, 4, 100);
}

static void lcd_put_cur(int row, int col) //calcula direccion del cursor y envia comando correspondiente
{
    uint8_t pos = (row == 0) ? (0x80 + col) : (0xC0 + col);
    lcd_send_cmd(pos);
}

static void lcd_init(void) //inicializa lcd paso a paso
{
    HAL_Delay(50);
    lcd_send_cmd(0x30); HAL_Delay(5);
    lcd_send_cmd(0x30); HAL_Delay(1);
    lcd_send_cmd(0x30); HAL_Delay(10);
    lcd_send_cmd(0x20); HAL_Delay(10);

    lcd_send_cmd(0x28); // 2 líneas
    lcd_send_cmd(0x08); // display OFF
    lcd_send_cmd(0x01); HAL_Delay(2); // clear
    lcd_send_cmd(0x06); // entry mode
    lcd_send_cmd(0x0C); // display ON
}

#define LCD_MAX_QUEUE 2
static lcd_line_t lcd_queue[LCD_MAX_QUEUE]; 
static char lcd_shadow[2][17] = {0}; //guarda la última línea escrita para evitar escribir lo mismo

static lcd_estado_t lcd_estado = LCD_IDLE; //estado actual de maquina
static const char* lcd_str = NULL; //puntero al string en proceso
static uint8_t lcd_row = 0; //fila actual
static uint8_t lcd_index = 0; //indice del caracter a imprimir

/********************** API pública FSM LCD **********************************/
void lcd_print_if_changed(int row, const char* str)//imprime linea solo si algo cambio respecto al buffer
{
    if (row < 0 || row > 1 || str == NULL) return;
    char temp[17];
    snprintf(temp, sizeof(temp), "%-16s", str);  // alineado a 16 chars
    //formetea string a 16 caracteres 

    if (strncmp(lcd_shadow[row], temp, 16) != 0)
    {
        strncpy(lcd_shadow[row], temp, 17);

        for (int i = 0; i < LCD_MAX_QUEUE; i++) {
            if (!lcd_queue[i].activo) {
                //si es diferente lo guarda dentro de la cola
                strncpy(lcd_queue[i].buffer, temp, 17);
                lcd_queue[i].row = row;
                lcd_queue[i].activo = true;
                break;
            }
        }
    }
}

void lcd_fsm_update(void)//implementacion central
{
    switch (lcd_estado)
    {
        case LCD_IDLE: //busca mensajes pendientes en la cola si encunetra la prepara para imprimir 
            for (int i = 0; i < LCD_MAX_QUEUE; i++) {
                if (lcd_queue[i].activo) {
                    lcd_row = lcd_queue[i].row;
                    lcd_str = lcd_queue[i].buffer;
                    lcd_index = 0;
                    lcd_estado = LCD_SET_CURSOR;
                    lcd_queue[i].activo = false;
                    break;
                }
            }
            break;

        case LCD_SET_CURSOR: //mueve el cursor a la posicion incial de la linea correspondiente 
            lcd_put_cur(lcd_row, 0);
            lcd_estado = LCD_PRINTING;
            break;

        case LCD_PRINTING: //imprime caracter por caracter (es NO BLOQUEANTE; PARA EVITAR PICOS EN TIEMPO DE EJECUCION)
            if (lcd_index < 16) {
                lcd_send_data(lcd_str[lcd_index++]);
            } else {
                lcd_estado = LCD_IDLE;
            }
            break;
    }
}

void lcd_test(void) //limpia pantalla y muestra ok
{
    lcd_send_cmd(0x01);
    HAL_Delay(5);
    lcd_send_data('O');
    lcd_send_data('K');
    lcd_send_data('!');
    //es bloqueante
}

/********************** tareas de sistema (init/update) **********************/

void task_lcd_init(void *params)//inicializa el lcd
{
    lcd_init();
    LOGGER_INFO(" ");
    LOGGER_INFO("  %s is running - %s", GET_NAME(task_lcd_init), p_task_lcd);
    LOGGER_INFO("  %s is a %s", GET_NAME(task_lcd_update), p_task_lcd_);
    g_task_lcd_cnt = G_TASK_LCD_CNT_INI;
    LOGGER_INFO("   %s = %lu", GET_NAME(g_task_lcd_cnt), g_task_lcd_cnt);
}

void task_lcd_update(void *params)//actualizar lcd 
{
    bool b_time_update_required = false; //flag se activa si hay ticks pendientes

    __asm("CPSID i"); //deshabilita interrupciones para codigo bloqueante 
    if (G_TASK_LCD_CNT_INI < g_task_lcd_tick_cnt) //compara
    {
        g_task_lcd_tick_cnt--; //si hay ticks pendientes decrementa
        b_time_update_required = true;
    }
    __asm("CPSIE i"); //habilita interrupciones

    if (b_time_update_required) 
    {
        g_task_lcd_cnt++;//incrementar contador de ciclo de tareas 

        lcd_fsm_update(); //rechequear si hay tick pendiente

        __asm("CPSID i"); //dehabilito interrupciones
        if (G_TASK_LCD_CNT_INI < g_task_lcd_tick_cnt) //comparo
        {
            g_task_lcd_tick_cnt--;//si hay ticks pendientes decrementa 
            b_time_update_required = true;
        }
        else
        {
            b_time_update_required = false; //si no hay ticks pendientes no es necesario actualizar aun
        }
        __asm("CPSIE i");//rehabilitar interrupciones
    }
}

