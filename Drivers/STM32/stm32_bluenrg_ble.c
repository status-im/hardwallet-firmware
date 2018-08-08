/**
 * This file is part of the Status project, https://status.im/
 *
 * Copyright (c) 2018 Status Research & Development GmbH
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "stm32_bluenrg_ble.h"
#include "gp_timer.h"

#define HEADER_SIZE 5
#define MAX_BUFFER_SIZE 255
#define TIMEOUT_DURATION 15
#define CLOCK_SECOND 1000

static void HAL_Delay(uint32_t ms) {

}

static void set_irq_as_output(void) {
 /* GPIO_InitTypeDef  GPIO_InitStructure;*/

  /* Pull IRQ high */
  /*GPIO_InitStructure.Pin = BNRG_SPI_IRQ_PIN;
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Speed = BNRG_SPI_IRQ_SPEED;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BNRG_SPI_IRQ_PORT, &GPIO_InitStructure);
  HAL_GPIO_WritePin(BNRG_SPI_IRQ_PORT, BNRG_SPI_IRQ_PIN, GPIO_PIN_SET);*/
}

static void set_irq_as_input(void) {
  /*GPIO_InitTypeDef  GPIO_InitStructure;*/

  /* IRQ input */
  /*GPIO_InitStructure.Pin = BNRG_SPI_IRQ_PIN;
  GPIO_InitStructure.Mode = BNRG_SPI_IRQ_MODE;
  GPIO_InitStructure.Pull = GPIO_PULLDOWN;
  GPIO_InitStructure.Speed = BNRG_SPI_IRQ_SPEED;
  GPIO_InitStructure.Alternate = BNRG_SPI_IRQ_ALTERNATE;
  HAL_GPIO_Init(BNRG_SPI_IRQ_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.Pull = BNRG_SPI_IRQ_PULL;
  HAL_GPIO_Init(BNRG_SPI_IRQ_PORT, &GPIO_InitStructure);*/
}


void Hal_Write_Serial(const void* data1, const void* data2, int32_t n_bytes1, int32_t n_bytes2) {
  struct timer t;

  Timer_Set(&t, CLOCK_SECOND/10);

  while(1){
    if(BlueNRG_SPI_Write((uint8_t *)data1,(uint8_t *)data2, n_bytes1, n_bytes2)==0) break;
    if(Timer_Expired(&t)){
      break;
    }
  }
}

void BNRG_SPI_Init(void) {
/*  SpiHandle.Instance = BNRG_SPI_INSTANCE;
  SpiHandle.Init.Mode = BNRG_SPI_MODE;
  SpiHandle.Init.Direction = BNRG_SPI_DIRECTION;
  SpiHandle.Init.DataSize = BNRG_SPI_DATASIZE;
  SpiHandle.Init.CLKPolarity = BNRG_SPI_CLKPOLARITY;
  SpiHandle.Init.CLKPhase = BNRG_SPI_CLKPHASE;
  SpiHandle.Init.NSS = BNRG_SPI_NSS;
  SpiHandle.Init.FirstBit = BNRG_SPI_FIRSTBIT;
  SpiHandle.Init.TIMode = BNRG_SPI_TIMODE;
  SpiHandle.Init.CRCPolynomial = BNRG_SPI_CRCPOLYNOMIAL;
  SpiHandle.Init.BaudRatePrescaler = BNRG_SPI_BAUDRATEPRESCALER;
  SpiHandle.Init.CRCCalculation = BNRG_SPI_CRCCALCULATION;

  HAL_SPI_Init(&SpiHandle);*/
}


void BlueNRG_RST(void) {
  HAL_GPIO_WritePin(BNRG_SPI_RESET_PORT, BNRG_SPI_RESET_PIN, GPIO_PIN_RESET);
  HAL_Delay(5);
  HAL_GPIO_WritePin(BNRG_SPI_RESET_PORT, BNRG_SPI_RESET_PIN, GPIO_PIN_SET);
  HAL_Delay(5);
}

uint8_t BlueNRG_DataPresent(void) {
  return HAL_GPIO_ReadPin(BNRG_SPI_EXTI_PORT, BNRG_SPI_EXTI_PIN) == GPIO_PIN_SET;
}

void BlueNRG_HW_Bootloader(void) {
  Disable_SPI_IRQ();
  set_irq_as_output();
  BlueNRG_RST();
  set_irq_as_input();
  Enable_SPI_IRQ();
}

int32_t BlueNRG_SPI_Read_All(uint8_t *buffer, uint8_t buff_size) {
#if 0
  uint16_t byte_count;
  uint8_t len = 0;
  uint8_t char_ff = 0xff;
  volatile uint8_t read_char;

  uint8_t header_master[HEADER_SIZE] = {0x0b, 0x00, 0x00, 0x00, 0x00};
  uint8_t header_slave[HEADER_SIZE];

  // CS reset
  HAL_GPIO_WritePin(BNRG_SPI_CS_PORT, BNRG_SPI_CS_PIN, GPIO_PIN_RESET);

  // Read the header
  HAL_SPI_TransmitReceive(&SpiHandle, header_master, header_slave, HEADER_SIZE, TIMEOUT_DURATION);

  if (header_slave[0] == 0x02) {
    // device is ready
    byte_count = (header_slave[4]<<8)|header_slave[3];

    if (byte_count > 0) {

      // avoid to read more data that size of the buffer
      if (byte_count > buff_size){
        byte_count = buff_size;
      }

      for (len = 0; len < byte_count; len++){
        HAL_SPI_TransmitReceive(&SpiHandle, &char_ff, (uint8_t*)&read_char, 1, TIMEOUT_DURATION);
        buffer[len] = read_char;
      }

    }
  }
  // Release CS line
  HAL_GPIO_WritePin(BNRG_SPI_CS_PORT, BNRG_SPI_CS_PIN, GPIO_PIN_SET);

  // Add a small delay to give time to the BlueNRG to set the IRQ pin low
  // to avoid a useless SPI read at the end of the transaction
  for(volatile int i = 0; i < 2; i++)__NOP();

  return len;
#endif
}

int32_t BlueNRG_SPI_Write(uint8_t* data1, uint8_t* data2, uint8_t Nb_bytes1, uint8_t Nb_bytes2) {
#if 0
  int32_t result = 0;

  unsigned char header_master[HEADER_SIZE] = {0x0a, 0x00, 0x00, 0x00, 0x00};
  unsigned char header_slave[HEADER_SIZE]  = {0xaa, 0x00, 0x00, 0x00, 0x00};

  unsigned char read_char_buf[MAX_BUFFER_SIZE];

  Disable_SPI_IRQ();

  /* CS reset */
  HAL_GPIO_WritePin(BNRG_SPI_CS_PORT, BNRG_SPI_CS_PIN, GPIO_PIN_RESET);

  /* Exchange header */
  HAL_SPI_TransmitReceive(&SpiHandle, header_master, header_slave, HEADER_SIZE, TIMEOUT_DURATION);

  if (header_slave[0] == 0x02) {
    /* SPI is ready */
    if (header_slave[1] >= (Nb_bytes1+Nb_bytes2)) {

      /*  Buffer is big enough */
      if (Nb_bytes1 > 0) {
        HAL_SPI_TransmitReceive(&SpiHandle, data1, read_char_buf, Nb_bytes1, TIMEOUT_DURATION);
      }
      if (Nb_bytes2 > 0) {
        HAL_SPI_TransmitReceive(&SpiHandle, data2, read_char_buf, Nb_bytes2, TIMEOUT_DURATION);
      }

    } else {
      /* Buffer is too small */
      result = -2;
    }
  } else {
    /* SPI is not ready */
    result = -1;
  }

  /* Release CS line */
  HAL_GPIO_WritePin(BNRG_SPI_CS_PORT, BNRG_SPI_CS_PIN, GPIO_PIN_SET);

  Enable_SPI_IRQ();

  return result;
#endif
}

void Enable_SPI_IRQ(void) {
  NVIC_EnableIRQ(BNRG_SPI_EXTI_IRQn);
}

void Disable_SPI_IRQ(void) {
  NVIC_DisableIRQ(BNRG_SPI_EXTI_IRQn);
}

void Clear_SPI_IRQ(void) {
  NVIC_ClearPendingIRQ(BNRG_SPI_EXTI_IRQn);
}

void Clear_SPI_EXTI_Flag(void) {
  __HAL_GPIO_EXTI_CLEAR_IT(BNRG_SPI_EXTI_PIN);
}
