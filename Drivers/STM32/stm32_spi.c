#include <stddef.h>

#include "stm32_spi.h"
#include "gp_timer.h"

#define SPI_DEFAULT_TIMEOUT 100U
#define HAL_MAX_DELAY      0xFFFFFFFFU
#define UNUSED(x) ((void)(x))

static HAL_StatusTypeDef SPI_WaitFlagStateUntilTimeout(SPI_HandleTypeDef *hspi, uint32_t Flag, uint32_t State, uint32_t Timeout, struct timer *Timer);
static HAL_StatusTypeDef SPI_WaitFifoStateUntilTimeout(SPI_HandleTypeDef *hspi, uint32_t Fifo, uint32_t State, uint32_t Timeout, struct timer *Timer);
static HAL_StatusTypeDef SPI_EndRxTxTransaction(SPI_HandleTypeDef *hspi, uint32_t Timeout, struct timer *Timer);
/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @defgroup SPI_Exported_Functions SPI Exported Functions
  * @{
  */

/** @defgroup SPI_Exported_Functions_Group1 Initialization and de-initialization functions
 *  @brief    Initialization and Configuration functions
 *
@verbatim
 ===============================================================================
              ##### Initialization and de-initialization functions #####
 ===============================================================================
    [..]  This subsection provides a set of functions allowing to initialize and
          de-initialize the SPIx peripheral:

      (+) User must implement HAL_SPI_MspInit() function in which he configures
          all related peripherals resources (CLOCK, GPIO, DMA, IT and NVIC ).

      (+) Call the function HAL_SPI_Init() to configure the selected device with
          the selected configuration:
        (++) Mode
        (++) Direction
        (++) Data Size
        (++) Clock Polarity and Phase
        (++) NSS Management
        (++) BaudRate Prescaler
        (++) FirstBit
        (++) TIMode
        (++) CRC Calculation
        (++) CRC Polynomial if CRC enabled
        (++) CRC Length, used only with Data8 and Data16
        (++) FIFO reception threshold

      (+) Call the function HAL_SPI_DeInit() to restore the default configuration
          of the selected SPIx peripheral.

@endverbatim
  * @{
  */

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
  /* TODO: implement this */
  UNUSED(hspi);
}

/**
  * @brief  Initialize the SPI according to the specified parameters
  *         in the SPI_InitTypeDef and initialize the associated handle.
  * @param  hspi pointer to a SPI_HandleTypeDef structure that contains
  *               the configuration information for SPI module.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SPI_Init(SPI_HandleTypeDef *hspi)
{
  uint32_t frxth;

  /* Check the SPI handle allocation */
  if (hspi == NULL)
  {
    return HAL_ERROR;
  }

  hspi->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;

  if (hspi->State == HAL_SPI_STATE_RESET)
  {
    /* Init the low level hardware : GPIO, CLOCK, NVIC... */
    HAL_SPI_MspInit(hspi);
  }

  hspi->State = HAL_SPI_STATE_BUSY;

  /* Disable the selected SPI peripheral */
  __HAL_SPI_DISABLE(hspi);

  /* Align by default the rs fifo threshold on the data size */
  if (hspi->Init.DataSize > SPI_DATASIZE_8BIT)
  {
    frxth = SPI_RXFIFO_THRESHOLD_HF;
  }
  else
  {
    frxth = SPI_RXFIFO_THRESHOLD_QF;
  }

  /* CRC calculation is valid only for 16Bit and 8 Bit */
  if ((hspi->Init.DataSize != SPI_DATASIZE_16BIT) && (hspi->Init.DataSize != SPI_DATASIZE_8BIT))
  {
    /* CRC must be disabled */
    hspi->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  }

  /* Align the CRC Length on the data size */
  if (hspi->Init.CRCLength == SPI_CRC_LENGTH_DATASIZE)
  {
    /* CRC Length aligned on the data size : value set by default */
    if (hspi->Init.DataSize > SPI_DATASIZE_8BIT)
    {
      hspi->Init.CRCLength = SPI_CRC_LENGTH_16BIT;
    }
    else
    {
      hspi->Init.CRCLength = SPI_CRC_LENGTH_8BIT;
    }
  }

  /*----------------------- SPIx CR1 & CR2 Configuration ---------------------*/
  /* Configure : SPI Mode, Communication Mode, Clock polarity and phase, NSS management,
  Communication speed, First bit and CRC calculation state */
  WRITE_REG(hspi->Instance->CR1, (hspi->Init.Mode | hspi->Init.Direction |
                                  hspi->Init.CLKPolarity | hspi->Init.CLKPhase | (hspi->Init.NSS & SPI_CR1_SSM) |
                                  hspi->Init.BaudRatePrescaler | hspi->Init.FirstBit  | hspi->Init.CRCCalculation));
  /* Configure : NSS management, TI Mode, NSS Pulse, Data size and Rx Fifo threshold */
  WRITE_REG(hspi->Instance->CR2, (((hspi->Init.NSS >> 16U) & SPI_CR2_SSOE) | hspi->Init.TIMode |
                                  hspi->Init.NSSPMode | hspi->Init.DataSize) | frxth);

  hspi->ErrorCode = HAL_SPI_ERROR_NONE;
  hspi->State     = HAL_SPI_STATE_READY;

  return HAL_OK;
}

/**
  * @}
  */

/** @defgroup SPI_Exported_Functions_Group2 IO operation functions
 *  @brief   Data transfers functions
 *
@verbatim
  ==============================================================================
                      ##### IO operation functions #####
 ===============================================================================
 [..]
    This subsection provides a set of functions allowing to manage the SPI
    data transfers.

    [..] The SPI supports master and slave mode :

    (#) There are two modes of transfer:
       (++) Blocking mode: The communication is performed in polling mode.
            The HAL status of all data processing is returned by the same function
            after finishing transfer.
       (++) No-Blocking mode: The communication is performed using Interrupts
            or DMA, These APIs return the HAL status.
            The end of the data processing will be indicated through the
            dedicated SPI IRQ when using Interrupt mode or the DMA IRQ when
            using DMA mode.
            The HAL_SPI_TxCpltCallback(), HAL_SPI_RxCpltCallback() and HAL_SPI_TxRxCpltCallback() user callbacks
            will be executed respectively at the end of the transmit or Receive process
            The HAL_SPI_ErrorCallback()user callback will be executed when a communication error is detected

    (#) APIs provided for these 2 transfer modes (Blocking mode or Non blocking mode using either Interrupt or DMA)
        exist for 1Line (simplex) and 2Lines (full duplex) modes.

@endverbatim
  * @{
  */

/**
  * @brief  Transmit and Receive an amount of data in blocking mode.
  * @param  hspi pointer to a SPI_HandleTypeDef structure that contains
  *               the configuration information for SPI module.
  * @param  pTxData pointer to transmission data buffer
  * @param  pRxData pointer to reception data buffer
  * @param  Size amount of data to be sent and received
  * @param  Timeout Timeout duration
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SPI_TransmitReceive(SPI_HandleTypeDef *hspi, uint8_t *pTxData, uint8_t *pRxData, uint16_t Size,
                                          uint32_t Timeout)
{
  uint32_t tmp = 0U, tmp1 = 0U;
  struct timer timeoutTimer;
  /* Variable used to alternate Rx and Tx during transfer */
  uint32_t txallowed = 1U;
  HAL_StatusTypeDef errorcode = HAL_OK;

  /* Init tickstart for timeout management*/
  Timer_Set(&timeoutTimer, Timeout);

  tmp  = hspi->State;
  tmp1 = hspi->Init.Mode;

  if (!((tmp == HAL_SPI_STATE_READY) || \
        ((tmp1 == SPI_MODE_MASTER) && (hspi->Init.Direction == SPI_DIRECTION_2LINES) && (tmp == HAL_SPI_STATE_BUSY_RX))))
  {
    errorcode = HAL_BUSY;
    goto error;
  }

  if ((pTxData == NULL) || (pRxData == NULL) || (Size == 0U))
  {
    errorcode = HAL_ERROR;
    goto error;
  }

  /* Don't overwrite in case of HAL_SPI_STATE_BUSY_RX */
  if (hspi->State != HAL_SPI_STATE_BUSY_RX)
  {
    hspi->State = HAL_SPI_STATE_BUSY_TX_RX;
  }

  /* Set the transaction information */
  hspi->ErrorCode   = HAL_SPI_ERROR_NONE;
  hspi->pRxBuffPtr  = (uint8_t *)pRxData;
  hspi->RxXferCount = Size;
  hspi->RxXferSize  = Size;
  hspi->pTxBuffPtr  = (uint8_t *)pTxData;
  hspi->TxXferCount = Size;
  hspi->TxXferSize  = Size;

  /*Init field not used in handle to zero */
  hspi->RxISR       = NULL;
  hspi->TxISR       = NULL;

  /* Set the Rx Fifo threshold */
  if ((hspi->Init.DataSize > SPI_DATASIZE_8BIT) || (hspi->RxXferCount > 1U))
  {
    /* Set fiforxthreshold according the reception data length: 16bit */
    CLEAR_BIT(hspi->Instance->CR2, SPI_RXFIFO_THRESHOLD);
  }
  else
  {
    /* Set fiforxthreshold according the reception data length: 8bit */
    SET_BIT(hspi->Instance->CR2, SPI_RXFIFO_THRESHOLD);
  }

  /* Check if the SPI is already enabled */
  if ((hspi->Instance->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE)
  {
    /* Enable SPI peripheral */
    __HAL_SPI_ENABLE(hspi);
  }

  if ((hspi->Init.Mode == SPI_MODE_SLAVE) || (hspi->TxXferCount == 0x01U))
    {
      if (hspi->TxXferCount > 1U)
      {
        hspi->Instance->DR = *((uint16_t *)pTxData);
        pTxData += sizeof(uint16_t);
        hspi->TxXferCount -= 2U;
      }
      else
      {
        *(__IO uint8_t *)&hspi->Instance->DR = (*pTxData++);
        hspi->TxXferCount--;
      }
    }
    while ((hspi->TxXferCount > 0U) || (hspi->RxXferCount > 0U))
    {
      /* Check TXE flag */
      if (txallowed && (hspi->TxXferCount > 0U) && (__HAL_SPI_GET_FLAG(hspi, SPI_FLAG_TXE)))
      {
        if (hspi->TxXferCount > 1U)
        {
          hspi->Instance->DR = *((uint16_t *)pTxData);
          pTxData += sizeof(uint16_t);
          hspi->TxXferCount -= 2U;
        }
        else
        {
          *(__IO uint8_t *)&hspi->Instance->DR = (*pTxData++);
          hspi->TxXferCount--;
        }
        /* Next Data is a reception (Rx). Tx not allowed */
        txallowed = 0U;
      }

      /* Wait until RXNE flag is reset */
      if ((hspi->RxXferCount > 0U) && (__HAL_SPI_GET_FLAG(hspi, SPI_FLAG_RXNE)))
      {
        if (hspi->RxXferCount > 1U)
        {
          *((uint16_t *)pRxData) = hspi->Instance->DR;
          pRxData += sizeof(uint16_t);
          hspi->RxXferCount -= 2U;
          if (hspi->RxXferCount <= 1U)
          {
            /* Set RX Fifo threshold before to switch on 8 bit data size */
            SET_BIT(hspi->Instance->CR2, SPI_RXFIFO_THRESHOLD);
          }
        }
        else
        {
          (*(uint8_t *)pRxData++) = *(__IO uint8_t *)&hspi->Instance->DR;
          hspi->RxXferCount--;
        }
        /* Next Data is a Transmission (Tx). Tx is allowed */
        txallowed = 1U;
      }
      if ((Timeout != HAL_MAX_DELAY) && Timer_Expired(&timeoutTimer))
      {
        errorcode = HAL_TIMEOUT;
        goto error;
      }
  }

  /* Check the end of the transaction */
  if (SPI_EndRxTxTransaction(hspi, Timeout, &timeoutTimer) != HAL_OK)
  {
    errorcode = HAL_ERROR;
    hspi->ErrorCode = HAL_SPI_ERROR_FLAG;
  }

error :
  hspi->State = HAL_SPI_STATE_READY;
  return errorcode;
}

/**
  * @brief  Handle SPI Communication Timeout.
  * @param  hspi pointer to a SPI_HandleTypeDef structure that contains
  *              the configuration information for SPI module.
  * @param  Flag SPI flag to check
  * @param  State flag state to check
  * @param  Timeout Timeout duration
  * @param  Tickstart tick start value
  * @retval HAL status
  */
static HAL_StatusTypeDef SPI_WaitFlagStateUntilTimeout(SPI_HandleTypeDef *hspi, uint32_t Flag, uint32_t State,
                                                       uint32_t Timeout, struct timer *Timer)
{
  while ((__HAL_SPI_GET_FLAG(hspi, Flag) ? SET : RESET) != State)
  {
    if (Timeout != HAL_MAX_DELAY)
    {
      if ((Timeout == 0U) || Timer_Expired(Timer))
      {
        /* Disable the SPI and reset the CRC: the CRC value should be cleared
        on both master and slave sides in order to resynchronize the master
        and slave for their respective CRC calculation */

        /* Disable TXE, RXNE and ERR interrupts for the interrupt process */
        __HAL_SPI_DISABLE_IT(hspi, (SPI_IT_TXE | SPI_IT_RXNE | SPI_IT_ERR));

        if ((hspi->Init.Mode == SPI_MODE_MASTER) && ((hspi->Init.Direction == SPI_DIRECTION_1LINE)
                                                     || (hspi->Init.Direction == SPI_DIRECTION_2LINES_RXONLY)))
        {
          /* Disable SPI peripheral */
          __HAL_SPI_DISABLE(hspi);
        }

        /* Reset CRC Calculation */
        if (hspi->Init.CRCCalculation == SPI_CRCCALCULATION_ENABLE)
        {
          SPI_RESET_CRC(hspi);
        }

        hspi->State = HAL_SPI_STATE_READY;

        return HAL_TIMEOUT;
      }
    }
  }

  return HAL_OK;
}

/**
  * @brief  Handle SPI FIFO Communication Timeout.
  * @param  hspi pointer to a SPI_HandleTypeDef structure that contains
  *              the configuration information for SPI module.
  * @param  Fifo Fifo to check
  * @param  State Fifo state to check
  * @param  Timeout Timeout duration
  * @param  Tickstart tick start value
  * @retval HAL status
  */
static HAL_StatusTypeDef SPI_WaitFifoStateUntilTimeout(SPI_HandleTypeDef *hspi, uint32_t Fifo, uint32_t State,
                                                       uint32_t Timeout, struct timer *Timer)
{
  __IO uint8_t tmpreg;

  while ((hspi->Instance->SR & Fifo) != State)
  {
    if ((Fifo == SPI_SR_FRLVL) && (State == SPI_FRLVL_EMPTY))
    {
      tmpreg = *((__IO uint8_t *)&hspi->Instance->DR);
      /* To avoid GCC warning */
      UNUSED(tmpreg);
    }

    if (Timeout != HAL_MAX_DELAY)
    {
      if ((Timeout == 0U) || Timer_Expired(Timer))
      {
        /* Disable the SPI and reset the CRC: the CRC value should be cleared
           on both master and slave sides in order to resynchronize the master
           and slave for their respective CRC calculation */

        /* Disable TXE, RXNE and ERR interrupts for the interrupt process */
        __HAL_SPI_DISABLE_IT(hspi, (SPI_IT_TXE | SPI_IT_RXNE | SPI_IT_ERR));

        if ((hspi->Init.Mode == SPI_MODE_MASTER) && ((hspi->Init.Direction == SPI_DIRECTION_1LINE)
                                                     || (hspi->Init.Direction == SPI_DIRECTION_2LINES_RXONLY)))
        {
          /* Disable SPI peripheral */
          __HAL_SPI_DISABLE(hspi);
        }

        /* Reset CRC Calculation */
        if (hspi->Init.CRCCalculation == SPI_CRCCALCULATION_ENABLE)
        {
          SPI_RESET_CRC(hspi);
        }

        hspi->State = HAL_SPI_STATE_READY;

        return HAL_TIMEOUT;
      }
    }
  }

  return HAL_OK;
}

/**
  * @brief  Handle the check of the RXTX or TX transaction complete.
  * @param  hspi SPI handle
  * @param  Timeout Timeout duration
  * @param  Tickstart tick start value
  * @retval HAL status
  */
static HAL_StatusTypeDef SPI_EndRxTxTransaction(SPI_HandleTypeDef *hspi, uint32_t Timeout, struct timer *Timer)
{
  /* Control if the TX fifo is empty */
  if (SPI_WaitFifoStateUntilTimeout(hspi, SPI_FLAG_FTLVL, SPI_FTLVL_EMPTY, Timeout, Timer) != HAL_OK)
  {
    SET_BIT(hspi->ErrorCode, HAL_SPI_ERROR_FLAG);
    return HAL_TIMEOUT;
  }

  /* Control the BSY flag */
  if (SPI_WaitFlagStateUntilTimeout(hspi, SPI_FLAG_BSY, RESET, Timeout, Timer) != HAL_OK)
  {
    SET_BIT(hspi->ErrorCode, HAL_SPI_ERROR_FLAG);
    return HAL_TIMEOUT;
  }

  /* Control if the RX fifo is empty */
  if (SPI_WaitFifoStateUntilTimeout(hspi, SPI_FLAG_FRLVL, SPI_FRLVL_EMPTY, Timeout, Timer) != HAL_OK)
  {
    SET_BIT(hspi->ErrorCode, HAL_SPI_ERROR_FLAG);
    return HAL_TIMEOUT;
  }

  return HAL_OK;
}
