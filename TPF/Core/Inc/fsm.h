/*
 * fsm.h
 *
 *  Created on: Jun 16, 2025
 *      Author: alekseychahuillco
 */

 typedef enum {
	 estado_off,
	 estado_menu_principal,
	 estado_seleccion_cafe,
	 estado_brewing,
	 estado_save_energy
 } estados;

 extern estados estado_actual;
 void fsm_update(void); //para llamar a la maquina de estados en el main loop


 typedef enum {
     CAFE_LATTE,
     CAFE_MOCHA,
     CAFE_DOBLE,
     CAFE_AMERICANO
 } tipo_de_cafe;

 extern tipo_de_cafe cafe_seleccionado;





