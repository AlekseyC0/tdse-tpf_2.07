#include "gpio_control.h"

void led_on(uint8_t led_num) {
    switch (led_num) {
        case 1: HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET); break;
        case 2: HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET); break;
        case 3: HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET); break;
        case 4: HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_RESET); break;
    }
}

void led_off(uint8_t led_num) {
    switch (led_num) {
        case 1: HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET); break;
        case 2: HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET); break;
        case 3: HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET); break;
        case 4: HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_SET); break;
    }
}

void led_toggle(uint8_t led_num) {
    switch (led_num) {
        case 1: HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin); break;
        case 2: HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin); break;
        case 3: HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin); break;
        case 4: HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin); break;
    }
}

void led_wave_effect(void) {
    static uint32_t last_step = 0;
    static uint8_t phase = 0;

    if (HAL_GetTick() - last_step >= 100) {
        last_step = HAL_GetTick();

        switch (phase) {
            case 0: led_off(1); break;  // Enciende LED 1
            case 1: led_off(2); break;  // Enciende LED 2
            case 2: led_off(3); break;  // Enciende LED 3
            case 3: led_off(4); break;  // Enciende LED 4

            case 4: led_soft_off(1); break;  // Fade-out LED 1
            case 5: led_soft_off(2); break;
            case 6: led_soft_off(3); break;
            case 7: led_soft_off(4); break;

            default: break;
        }

        phase++;
        if (phase > 7) phase = 0;
    }
}

void led_soft_off(uint8_t led_num) {
    for (int duty = 10; duty > 0; duty--) {
        led_off(led_num);         // Enciende
        HAL_Delay(duty);          // ON time
        led_on(led_num);          // Apaga
        HAL_Delay(10 - duty);     // OFF time
    }

    // Asegura que termine apagado (en tu lógica, ON = apagado)
    led_on(led_num);
}



// Created by Jorge - versión optimizada

void gpio_pin_on(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
  HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
}

void gpio_pin_off(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
  HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
}

void gpio_pins_on(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pins) {
  GPIOx->BSRR = GPIO_Pins;  // set bits at once
}

void gpio_pins_off(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pins) {
  GPIOx->BRR = GPIO_Pins;   // reset bits at once
}
