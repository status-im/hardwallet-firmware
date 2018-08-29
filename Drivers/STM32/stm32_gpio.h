#ifndef STM32_STM32_GPIO_H_
#define STM32_STM32_GPIO_H_

#include "stm32l4xx.h"

#define GPIO_PIN_0 ((uint16_t)0x0001)
#define GPIO_PIN_1 ((uint16_t)0x0002)
#define GPIO_PIN_2 ((uint16_t)0x0004)
#define GPIO_PIN_3 ((uint16_t)0x0008)
#define GPIO_PIN_4 ((uint16_t)0x0010)
#define GPIO_PIN_5 ((uint16_t)0x0020)
#define GPIO_PIN_6 ((uint16_t)0x0040)
#define GPIO_PIN_7 ((uint16_t)0x0080)
#define GPIO_PIN_8 ((uint16_t)0x0100)
#define GPIO_PIN_9 ((uint16_t)0x0200)
#define GPIO_PIN_10 ((uint16_t)0x0400)
#define GPIO_PIN_11 ((uint16_t)0x0800)
#define GPIO_PIN_12 ((uint16_t)0x1000)
#define GPIO_PIN_13 ((uint16_t)0x2000)
#define GPIO_PIN_14 ((uint16_t)0x4000)
#define GPIO_PIN_15 ((uint16_t)0x8000)

#define GPIO_PIN_RESET 0
#define GPIO_PIN_SET 1

#define  GPIO_MODE_INPUT ((uint32_t)0x00000000)
#define  GPIO_MODE_OUTPUT_PP ((uint32_t)0x00000001)
#define  GPIO_MODE_OUTPUT_OD ((uint32_t)0x00000011)
#define  GPIO_MODE_AF_PP ((uint32_t)0x00000002)
#define  GPIO_MODE_AF_OD ((uint32_t)0x00000012)
#define  GPIO_MODE_ANALOG ((uint32_t)0x00000003)
#define  GPIO_MODE_ANALOG_ADC_CONTROL ((uint32_t)0x0000000B)
#define  GPIO_MODE_IT_RISING ((uint32_t)0x10110000)
#define  GPIO_MODE_IT_FALLING ((uint32_t)0x10210000)
#define  GPIO_MODE_IT_RISING_FALLING ((uint32_t)0x10310000)
#define  GPIO_MODE_EVT_RISING ((uint32_t)0x10120000)
#define  GPIO_MODE_EVT_FALLING ((uint32_t)0x10220000)
#define  GPIO_MODE_EVT_RISING_FALLING ((uint32_t)0x10320000)

#define  GPIO_SPEED_FREQ_LOW ((uint32_t)0x00000000)
#define  GPIO_SPEED_FREQ_MEDIUM ((uint32_t)0x00000001)
#define  GPIO_SPEED_FREQ_HIGH ((uint32_t)0x00000002)
#define  GPIO_SPEED_FREQ_VERY_HIGH ((uint32_t)0x00000003)

#define  GPIO_NOPULL ((uint32_t)0x00000000)
#define  GPIO_PULLUP ((uint32_t)0x00000001)
#define  GPIO_PULLDOWN ((uint32_t)0x00000002)

#define __HAL_GPIO_EXTI_CLEAR_IT(__EXTI_LINE__) (EXTI->PR1 = (__EXTI_LINE__))

void HAL_GPIO_Init(GPIO_TypeDef* GPIOx, uint32_t Pin, uint32_t Mode, uint32_t Pull, uint32_t Speed, uint32_t Alternate);
void HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint32_t PinState);
uint32_t HAL_GPIO_ReadPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

#endif /* STM32_STM32_GPIO_H_ */
