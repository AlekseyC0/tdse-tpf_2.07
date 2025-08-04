/********************** EXPLICACION BREVE *****************************/
/*
 * Este modulo gestiona el stock de ingredientes (cafe, leche, agua, chocolate) para preparar distintos
 * tipos de cafe. Define recetas fijas (recetas[]) asociadas a cada tipo, y permite verificar
 * (check_stock) si hay suficiente cantidad para preparar una bebida. Tambien soporta recetas
 * personlizadas con check_stock_receta. Al preparar, se descuenta el stock mediante
 * descontar_stock_receta. La funcion get_stock(idx) permite consultar el nivel de un ingrediente, util
 * para mostrarlo en pantalla. Ademas, stock_reset() reinicio los valores a 10 unidades. Es un modulo
 * simple y eficiente, esencial para controlar la logica de disponibilidad del mismo al preparar un cafe.
*/
/**************************************************************************/

#include "task_stock_interface.h"

int stock_cafe = 10;
int stock_leche = 10;
int stock_agua = 10;
int stock_chocolate = 10;
//inicializa stocks en 10 

extern uint8_t receta_personal[4];

const RecetaCafe recetas[] = {
    [CAFE_LATTE]     = {1, 2, 1, 0},
    [CAFE_MOCHA]     = {1, 1, 1, 1},
    [CAFE_DOBLE]     = {2, 0, 2, 0},
    [CAFE_AMERICANO] = {1, 0, 2, 0}
}; //define arreglo constante con receta de cada tipo de cafe, cada entrada especifica cuantas cantidades de ingredientes necesita {cafe,leche,agua,choco}

bool check_stock(tipo_de_cafe tipo)//chequea si suficiente stock para un tipo de cafe dado 
{
    if (tipo < CAFE_LATTE || tipo > CAFE_AMERICANO) return false;

    return  stock_cafe     >= recetas[tipo].cafe &&
            stock_leche    >= recetas[tipo].leche &&
            stock_agua     >= recetas[tipo].agua &&
            stock_chocolate>= recetas[tipo].chocolate;

    //retorna true sii hay suficiente de todos los ingredientes
}

void descontar_stock(tipo_de_cafe tipo)//descuenta del stock ingredientes usados por tipo de cafe indicado
{
    if (!check_stock(tipo)) return;
    //verifica haya stock suficiente, si no hay sale sin hacer nada

    stock_cafe      -= recetas[tipo].cafe;
    stock_leche     -= recetas[tipo].leche;
    stock_agua      -= recetas[tipo].agua;
    stock_chocolate -= recetas[tipo].chocolate;
    //resta cantidades correspondientes al stock actual
}

bool check_stock_receta(uint8_t receta[4]) //verifica si hay stock suficiente para personalizado
{
    return receta[0] <= stock_cafe &&
           receta[1] <= stock_leche &&
           receta[2] <= stock_agua &&
           receta[3] <= stock_chocolate;
    //verifica que haya suficiente de cada ingrediente, devuelve true si alcanza
}

void descontar_stock_receta(uint8_t receta[4]) //descuenta stock para personalizado
{
    stock_cafe      -= receta[0];
    stock_leche     -= receta[1];
    stock_agua      -= receta[2];
    stock_chocolate -= receta[3];
    //resta valores corresponientes de stock
}

uint8_t get_stock(uint8_t idx)//devuelve valor actual de stock para ingrediente dado segun su indice 
{
    switch (idx) {
        case 0: return stock_cafe;  //cafe
        case 1: return stock_leche; //leche
        case 2: return stock_agua;  //agua 
        case 3: return stock_chocolate; //choco
        default: return 0; //indice no valido
    }
}

void stock_reset(void) //reinicia stock de todos ingredientes a valor por defecto
{
    stock_cafe = 10;
    stock_leche = 10;
    stock_agua = 10;
    stock_chocolate = 10;
}
