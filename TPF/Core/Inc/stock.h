#ifndef STOCK_H
#define STOCK_H

#include <stdbool.h>
#include <stdint.h>
#include "fsm_3.h"  // Para usar tipo_de_cafe

// Variables de stock
extern int stock_cafe;
extern int stock_leche;
extern int stock_agua;
extern int stock_chocolate;

// Funciones públicas
bool check_stock(tipo_de_cafe tipo);
void descontar_stock(tipo_de_cafe tipo);
void stock_reset(void);  // Opcional: reinicia a valores máximos

#endif // STOCK_H
