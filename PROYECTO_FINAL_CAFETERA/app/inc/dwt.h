/*
 * Copyright (c) 2023 Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @file   : dwt.h
 * @date   : Set 26, 2023
 * @author : Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>
 * @version	v1.0.0
 */

#ifndef DWT_INC_DWT_H_
#define DWT_INC_DWT_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusiones *******************************************/

/********************** macros ***********************************************/

/* init cycle counter */
/* DWT (Data Watchpoint and Trace) registers, only exists on ARM Cortex with a DWT unit */
/*!< DEMCR: Debug Exception and Monitor Control Register */
/*!< TRCENA: Enable trace and debug block DEMCR (Debug Exception and Monitor Control Register) */
/*!< DWT Cycle Counter register */
/*!< CYCCNTENA bit in DWT_CONTROL register */
static inline void cycle_counter_init(void) __attribute__((always_inline));
static inline void cycle_counter_init(void)
{
	 CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;/* enable DWT hardware */
	 DWT->CYCCNT = 0;								/* reset cycle counter */
	 DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;			/* start counting */
}

/* reset cycle counter */
/*!< DWT Cycle Counter register */
static inline void cycle_counter_reset(void) __attribute__((always_inline));
static inline void cycle_counter_reset(void)
{
	DWT->CYCCNT = 0;
}

/* enable counting */
/*!< CYCCNTENA bit in DWT_CONTROL register */
static inline void cycle_counter_enable(void) __attribute__((always_inline));
static inline void cycle_counter_enable(void)
{
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/* disable counting */
/*!< CYCCNTENA bit in DWT_CONTROL register */
static inline void cycle_counter_disable(void) __attribute__((always_inline));
static inline void cycle_counter_disable(void)
{
	DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;
}

/* read cycle counter */
/*!< DWT Cycle Counter register */
static inline uint32_t cycle_counter_get(void) __attribute__((always_inline));
static inline uint32_t cycle_counter_get(void)
{
	return (DWT->CYCCNT);
}

static inline uint32_t cycle_counter_get_time_us(void) __attribute__((always_inline));
static inline uint32_t cycle_counter_get_time_us(void)
{
	return (DWT->CYCCNT / (SystemCoreClock / 1000000));
}

/*  uint32_t cycle_counter = 0;
 *  uint32_t cycle_counter_time_us = 0;
 *															// PC8 (GPIO)
 *  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);	// => ______
 *  cycle_counter_init();
 *
 *  ...
 *  ...														// => ______
 *															//		 ___
 *  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET); 	// => __/
 *  // or => HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_8);
 *  cycle_counter_reset();
 *															//	  ______
 *  ...														// =>
 *
 *  cycle_counter = cycle_counter_get();
 *  cycle_counter_time_us = cycle_counter_get_time_us();	//	  __
 *  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);	// =>   \___
 *  // or => HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_8);
 *
 *  														// => ______
 *
 *  LOGGER_LOG("Cycles: %lu - Time %lu uS\r\n", cycle_counter, cycle_counter_time_us);
 */

/********************** fin de CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* DWT_INC_DWT_H_ */

/********************** end of file ******************************************/
