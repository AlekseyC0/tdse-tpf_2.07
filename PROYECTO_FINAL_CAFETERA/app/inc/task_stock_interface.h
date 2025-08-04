#ifndef TASK_STOCK_INTERFACE_H_
#define TASK_STOCK_INTERFACE_H_

#include <stdint.h>
#include <stdbool.h>
#include "task_cafe_fsm_attribute.h"

/**
 * @brief estructura que representa una receta de café con cantidades por ingrediente.
 */
typedef struct {
    uint8_t cafe;       /**< Cantidad de café requerida */
    uint8_t leche;      /**< Cantidad de leche requerida */
    uint8_t agua;       /**< Cantidad de agua requerida */
    uint8_t chocolate;  /**< Cantidad de chocolate requerida */
} RecetaCafe;

/**
 * @brief stock actual de café.
 */
extern int stock_cafe;

/**
 * @brief stock actual de leche.
 */
extern int stock_leche;

/**
 * @brief stock actual de agua.
 */
extern int stock_agua;

/**
 * @brief stock actual de chocolate.
 */
extern int stock_chocolate;

/**
 * @brief arreglo constante con las recetas predefinidas del sistema.
 */
extern const RecetaCafe recetas[];

/**
 * @brief verifica si hay suficiente stock para una receta predefinida.
 * 
 * @param tipo tipo de café (enum tipo_de_cafe).
 * @return true si hay suficiente de todos los ingredientes, false en caso contrario.
 */
bool check_stock(tipo_de_cafe tipo);

/**
 * @brief descuenta del stock los ingredientes usados por una receta predefinida.
 * 
 * @param tipo tipo de café (enum tipo_de_cafe).
 */
void descontar_stock(tipo_de_cafe tipo);

/**
 * @brief verifica si hay stock suficiente para una receta personalizada.
 * 
 * @param receta arreglo de 4 elementos: café, leche, agua, chocolate.
 * @return true si alcanza el stock para todos los ingredientes, false si falta alguno.
 */
bool check_stock_receta(uint8_t receta[4]);

/**
 * @brief descuenta del stock según los valores de una receta personalizada.
 * 
 * @param receta arreglo de 4 elementos: café, leche, agua, chocolate.
 */
void descontar_stock_receta(uint8_t receta[4]);

/**
 * @brief devuelve el valor actual de un ingrediente según índice.
 * 
 * @param idx indice del ingrediente (0: café, 1: leche, 2: agua, 3: chocolate).
 * @return cantidad disponible del ingrediente indicado, o 0 si el índice es inválido.
 */
uint8_t get_stock(uint8_t idx);

/**
 * @brief reinicia el stock de todos los ingredientes a 10 unidades.
 */
void stock_reset(void);

#endif /* TASK_STOCK_INTERFACE_H_ */


