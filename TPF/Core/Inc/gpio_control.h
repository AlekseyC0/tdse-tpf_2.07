#ifndef GPIO_CONTROL_H
#define GPIO_CONTROL_H

#include "stm32f1xx_hal.h"
#include "main.h"

// LEDs definidos
#define LED1_GPIO_Port GPIOB
#define LED1_Pin       GPIO_PIN_1

#define LED2_GPIO_Port GPIOB
#define LED2_Pin       GPIO_PIN_2

#define LED3_GPIO_Port GPIOB

#define LED4_GPIO_Port GPIOB

// Encender/apagar
void led_on(uint8_t led_num);
void led_off(uint8_t led_num);
void led_toggle(uint8_t led_num);
void led_wave_effect(void);
void led_soft_off(uint8_t led_num);


//created by jorge
void gpio_pin_on(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void gpio_pin_off(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void gpio_pins_on(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pins);
void gpio_pins_off(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pins);

#endif /* GPIO_CONTROL_H */
