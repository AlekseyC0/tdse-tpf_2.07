
/********************** definición FSM LCD ***********************************/

#include <stdint.h>
#include <stdbool.h>

/**
 * @enum lcd_estado_t
 * @brief estados de la máquina de estados finita del LCD.
 */
typedef enum {
    LCD_IDLE = 0,       /* estado inactivo: sin datos pendientes. */
    LCD_SET_CURSOR,     /* estado que posiciona el cursor en la línea correspondiente. */
    LCD_PRINTING        /* estado que imprime carácter por carácter la línea actual. */
} lcd_estado_t;

/**
 * @struct lcd_line_t
 * @brief estructura para representar una línea de texto pendiente de impresión.
 */
typedef struct {
    char buffer[17];    /* texto alineado a 16 caracteres + '\0'. */
    uint8_t row;        /* fila del LCD (0 o 1). */
    bool activo;        /* indica si hay un mensaje pendiente para imprimir. */
} lcd_line_t;

/**
 * @brief encola una línea de texto si cambió respecto al contenido actual.
 * @param row fila del display (0 o 1).
 * @param str texto a imprimir. se trunca o rellena a 16 caracteres.
 */
void lcd_print_if_changed(int row, const char* str);

/**
 * @brief función que ejecuta la FSM del LCD. debe llamarse periódicamente.
 */
void lcd_fsm_update(void);

/**
 * @brief prueba simple del LCD: borra pantalla y escribe "OK!".
 */
void lcd_test(void);
