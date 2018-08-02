#ifndef __STM32_BLUENRG_BLE_H
#define __STM32_BLUENRG_BLE_H

#include "stm32l4xx_bluenrg.h"

void Hal_Write_Serial(const void* data1, const void* data2, int32_t n_bytes1, int32_t n_bytes2);
void BNRG_SPI_Init(void);
void BlueNRG_RST(void);
uint8_t BlueNRG_DataPresent(void);
void BlueNRG_HW_Bootloader(void);
int32_t BlueNRG_SPI_Read_All(uint8_t *buffer, uint8_t buff_size);
int32_t BlueNRG_SPI_Write(uint8_t* data1, uint8_t* data2, uint8_t Nb_bytes1, uint8_t Nb_bytes2);


#endif
