#include "stm32_gpio.h"

#define GPIO_MODE             ((uint32_t)0x00000003)
#define ANALOG_MODE           ((uint32_t)0x00000008)
#define EXTI_MODE             ((uint32_t)0x10000000)
#define GPIO_MODE_IT          ((uint32_t)0x00010000)
#define GPIO_MODE_EVT         ((uint32_t)0x00020000)
#define RISING_EDGE           ((uint32_t)0x00100000)
#define FALLING_EDGE          ((uint32_t)0x00200000)
#define GPIO_OUTPUT_TYPE      ((uint32_t)0x00000010)

#define GPIO_NUMBER           ((uint32_t)16)

void HAL_GPIO_Init(GPIO_TypeDef* GPIOx, uint32_t Pin, uint32_t Mode, uint32_t Pull, uint32_t Speed, uint32_t Alternate) {
  uint32_t temp = 0x00;

  /*--------------------- GPIO Mode Configuration ------------------------*/
  /* In case of Alternate function mode selection */
  if ((Mode == GPIO_MODE_AF_PP) || (Mode == GPIO_MODE_AF_OD)) {
    /* Configure Alternate function mapped with the current IO */
    temp = GPIOx->AFR[Pin >> 3];
    temp &= ~((uint32_t)0xF << ((uint32_t)(Pin & (uint32_t)0x07) * 4)) ;
    temp |= ((uint32_t)(Alternate) << (((uint32_t)Pin & (uint32_t)0x07) * 4));
    GPIOx->AFR[Pin >> 3] = temp;
  }

  /* Configure IO Direction mode (Input, Output, Alternate or Analog) */
  temp = GPIOx->MODER;
  temp &= ~(GPIO_MODER_MODE0 << (Pin * 2));
  temp |= ((Mode & GPIO_MODE) << (Pin * 2));
  GPIOx->MODER = temp;

  /* In case of Output or Alternate function mode selection */
  if ((Mode == GPIO_MODE_OUTPUT_PP) || (Mode == GPIO_MODE_AF_PP) || (Mode == GPIO_MODE_OUTPUT_OD) || (Mode == GPIO_MODE_AF_OD)) {
    /* Configure the IO Speed */
    temp = GPIOx->OSPEEDR;
    temp &= ~(GPIO_OSPEEDR_OSPEED0 << (Pin * 2));
    temp |= (Speed << (Pin * 2));
    GPIOx->OSPEEDR = temp;

    /* Configure the IO Output Type */
    temp = GPIOx->OTYPER;
    temp &= ~(GPIO_OTYPER_OT0 << Pin) ;
    temp |= (((Mode & GPIO_OUTPUT_TYPE) >> 4) << Pin);
    GPIOx->OTYPER = temp;
  }

  /* In case of Analog mode, check if ADC control mode is selected */
  if ((Mode & GPIO_MODE_ANALOG) == GPIO_MODE_ANALOG) {
    /* Configure the IO Output Type */
    temp = GPIOx->ASCR;
    temp &= ~(GPIO_ASCR_ASC0 << Pin) ;
    temp |= (((Mode & ANALOG_MODE) >> 3) << Pin);
    GPIOx->ASCR = temp;
  }

  /* Activate the Pull-up or Pull down resistor for the current IO */
  temp = GPIOx->PUPDR;
  temp &= ~(GPIO_PUPDR_PUPD0 << (Pin * 2));
  temp |= ((Pull) << (Pin * 2));
  GPIOx->PUPDR = temp;
}

void HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint32_t PinState) {
  if(PinState != GPIO_PIN_RESET) {
    GPIOx->BSRR = (uint32_t)GPIO_Pin;
  } else{
    GPIOx->BRR = (uint32_t)GPIO_Pin;
  }
}

uint32_t HAL_GPIO_ReadPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
  uint32_t bitstatus;

  if((GPIOx->IDR & GPIO_Pin) != GPIO_PIN_RESET) {
    bitstatus = GPIO_PIN_SET;
  } else {
    bitstatus = GPIO_PIN_RESET;
  }

  return bitstatus;
}
