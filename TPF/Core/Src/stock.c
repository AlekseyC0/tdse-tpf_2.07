#include "stock.h"
#include <stdio.h>

// Inicialización de stock
int stock_cafe = 10;
int stock_leche = 10;
int stock_agua = 10;
int stock_chocolate = 10;

// Configuración: cuántos shots usa cada tipo de café
// Lo podés personalizar según tu lógica real
static const struct {
    uint8_t cafe;
    uint8_t leche;
    uint8_t agua;
    uint8_t chocolate;
} recetas[] = {
    [CAFE_LATTE]      = {1, 2, 1, 0},
    [CAFE_MOCHA]      = {1, 1, 1, 1},
    [CAFE_DOBLE]      = {2, 0, 2, 0},
    [CAFE_AMERICANO]  = {1, 0, 2, 0}
};

bool check_stock(tipo_de_cafe tipo) {
    if (tipo < CAFE_LATTE || tipo > CAFE_AMERICANO) return false;

    return  stock_cafe     >= recetas[tipo].cafe &&
            stock_leche    >= recetas[tipo].leche &&
            stock_agua     >= recetas[tipo].agua &&
            stock_chocolate>= recetas[tipo].chocolate;
}

void descontar_stock(tipo_de_cafe tipo) {
    if (!check_stock(tipo)) return;

    stock_cafe     -= recetas[tipo].cafe;
    stock_leche    -= recetas[tipo].leche;
    stock_agua     -= recetas[tipo].agua;
    stock_chocolate-= recetas[tipo].chocolate;
}

// Útil para pruebas: resetea todo
void stock_reset(void) {
    stock_cafe = 10;
    stock_leche = 10;
    stock_agua = 10;
    stock_chocolate = 10;
}

