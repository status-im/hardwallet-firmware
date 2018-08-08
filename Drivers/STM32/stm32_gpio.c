#include "stm32_gpio.h"

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
