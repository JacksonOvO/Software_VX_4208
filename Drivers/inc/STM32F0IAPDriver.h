/******************************************************************
*
*文件名称:  STM32F0IAPDriver.h
*文件标识:
*内容摘要:
*其它说明:
*当前版本:
*
*修改记录1:
*    修改日期:
*    版 本 号:
*    修 改 人:
*    修改内容:
******************************************************************/

#ifndef __STM32F0_IAP_H
#define __STM32F0_IAP_H

/******************************************************************
*                             头文件                              *
******************************************************************/

/******************************************************************
*                            常量                                 *
******************************************************************/

/******************************************************************
*                           宏定义                                *
******************************************************************/
#define __IAP_IO    volatile     
#define __IAP_I     volatile             /*!< defines 'read only' permissions                 */

/******************************************************************
*                           数据类型                              *
******************************************************************/
typedef unsigned char IAP_uint8_t;
typedef unsigned short IAP_uint16_t;
typedef unsigned int IAP_uint32_t;

/*IAP返回值定义*/    
typedef enum
{
    E_IAP_OK = 0,
    E_IAP_CONTENT__FRAME_CHECK_ERROR,
    E_IAP_CONTENT__FRAME_CHECK_UPGRADE_PACKAGE_SN_ERROR,  /*升级内容,升级包顺序号错误*/
    E_IAP_CONTENT__FRAME_CHECK_UPGRADE_PACKAGE_SIZE_ERROR, /*升级包大小错误*/
    E_IAP_CONTENT__FRAME_CHECK_UPGRADE_PACKAGE_NUM_ERROR,  /*升级包序列号错误*/
    E_IAP_CONTENT__FRAME_CHECK_UPGRADE_PACKAGE_RETRANSMISSION_ERROR,
    E_IAP_RAM_TO_FLASH_ERROR,          /*从RAM向FLASH拷贝错误*/
    E_IAP_BUTT    
}IAP_Result_e;

typedef struct
{
  __IAP_IO IAP_uint32_t MODER;        /*!< GPIO port mode register,                                  Address offset: 0x00 */
  __IAP_IO IAP_uint16_t OTYPER;       /*!< GPIO port output type register,                           Address offset: 0x04 */
  IAP_uint16_t RESERVED0;         /*!< Reserved,                                                                 0x06 */
  __IAP_IO IAP_uint32_t OSPEEDR;      /*!< GPIO port output speed register,                          Address offset: 0x08 */
  __IAP_IO IAP_uint32_t PUPDR;        /*!< GPIO port pull-up/pull-down register,                     Address offset: 0x0C */
  __IAP_IO IAP_uint16_t IDR;          /*!< GPIO port input data register,                            Address offset: 0x10 */
  IAP_uint16_t RESERVED1;         /*!< Reserved,                                                                 0x12 */
  __IAP_IO IAP_uint16_t ODR;          /*!< GPIO port output data register,                           Address offset: 0x14 */
  IAP_uint16_t RESERVED2;         /*!< Reserved,                                                                 0x16 */
  __IAP_IO IAP_uint32_t BSRR;         /*!< GPIO port bit set/reset registerBSRR,                     Address offset: 0x18 */
  __IAP_IO IAP_uint32_t LCKR;         /*!< GPIO port configuration lock register,                    Address offset: 0x1C */
  __IAP_IO IAP_uint32_t AFR[2];       /*!< GPIO alternate function low register,                Address offset: 0x20-0x24 */
  __IAP_IO IAP_uint16_t BRR;          /*!< GPIO bit reset register,                                  Address offset: 0x28 */
  IAP_uint16_t RESERVED3;         /*!< Reserved,                                                                 0x2A */
}IAP_GPIO_TypeDef;

typedef struct
{
  __IAP_IO IAP_uint32_t CCR;          /*!< DMA channel x configuration register                                           */
  __IAP_IO IAP_uint32_t CNDTR;        /*!< DMA channel x number of data register                                          */
  __IAP_IO IAP_uint32_t CPAR;         /*!< DMA channel x peripheral address register                                      */
  __IAP_IO IAP_uint32_t CMAR;         /*!< DMA channel x memory address register                                          */
}IAP_DMA_Channel_TypeDef;

typedef struct
{
  __IAP_IO IAP_uint32_t CR1;    /*!< USART Control register 1,                 Address offset: 0x00 */ 
  __IAP_IO IAP_uint32_t CR2;    /*!< USART Control register 2,                 Address offset: 0x04 */ 
  __IAP_IO IAP_uint32_t CR3;    /*!< USART Control register 3,                 Address offset: 0x08 */
  __IAP_IO IAP_uint16_t BRR;    /*!< USART Baud rate register,                 Address offset: 0x0C */
  IAP_uint16_t  RESERVED1;  /*!< Reserved, 0x0E                                                 */  
  __IAP_IO IAP_uint16_t GTPR;   /*!< USART Guard time and prescaler register,  Address offset: 0x10 */
  IAP_uint16_t  RESERVED2;  /*!< Reserved, 0x12                                                 */
  __IAP_IO IAP_uint32_t RTOR;   /*!< USART Receiver Time Out register,         Address offset: 0x14 */  
  __IAP_IO IAP_uint16_t RQR;    /*!< USART Request register,                   Address offset: 0x18 */
  IAP_uint16_t  RESERVED3;  /*!< Reserved, 0x1A                                                 */
  __IAP_IO IAP_uint32_t ISR;    /*!< USART Interrupt and status register,      Address offset: 0x1C */
  __IAP_IO IAP_uint32_t ICR;    /*!< USART Interrupt flag Clear register,      Address offset: 0x20 */
  __IAP_IO IAP_uint16_t RDR;    /*!< USART Receive Data register,              Address offset: 0x24 */
  IAP_uint16_t  RESERVED4;  /*!< Reserved, 0x26                                                 */
  __IAP_IO IAP_uint16_t TDR;    /*!< USART Transmit Data register,             Address offset: 0x28 */
  IAP_uint16_t  RESERVED5;  /*!< Reserved, 0x2A                                                 */
} IAP_USART_TypeDef;

typedef enum {IAP_DISABLE = 0, IAP_ENABLE = !IAP_DISABLE} IAP_FunctionalState;

typedef struct
{
  __IAP_IO IAP_uint32_t CR;         /*!< RCC clock control register,                                  Address offset: 0x00 */
  __IAP_IO IAP_uint32_t CFGR;       /*!< RCC clock configuration register,                            Address offset: 0x04 */
  __IAP_IO IAP_uint32_t CIR;        /*!< RCC clock interrupt register,                                Address offset: 0x08 */
  __IAP_IO IAP_uint32_t APB2RSTR;   /*!< RCC APB2 peripheral reset register,                          Address offset: 0x0C */
  __IAP_IO IAP_uint32_t APB1RSTR;   /*!< RCC APB1 peripheral reset register,                          Address offset: 0x10 */
  __IAP_IO IAP_uint32_t AHBENR;     /*!< RCC AHB peripheral clock register,                           Address offset: 0x14 */
  __IAP_IO IAP_uint32_t APB2ENR;    /*!< RCC APB2 peripheral clock enable register,                   Address offset: 0x18 */
  __IAP_IO IAP_uint32_t APB1ENR;    /*!< RCC APB1 peripheral clock enable register,                   Address offset: 0x1C */
  __IAP_IO IAP_uint32_t BDCR;       /*!< RCC Backup domain control register,                          Address offset: 0x20 */ 
  __IAP_IO IAP_uint32_t CSR;        /*!< RCC clock control & status register,                         Address offset: 0x24 */
  __IAP_IO IAP_uint32_t AHBRSTR;    /*!< RCC AHB peripheral reset register,                           Address offset: 0x28 */
  __IAP_IO IAP_uint32_t CFGR2;      /*!< RCC clock configuration register 2,                          Address offset: 0x2C */
  __IAP_IO IAP_uint32_t CFGR3;      /*!< RCC clock configuration register 3,                          Address offset: 0x30 */
  __IAP_IO IAP_uint32_t CR2;        /*!< RCC clock control register 2,                                Address offset: 0x34 */
} IAP_RCC_TypeDef;

typedef struct
{
  IAP_uint8_t NVIC_IRQChannel;             /*!< Specifies the IRQ channel to be enabled or disabled.
                                            This parameter can be a value of @ref IRQn_Type 
                                            (For the complete STM32 Devices IRQ Channels list, 
                                            please refer to stm32f0xx.h file) */

  IAP_uint8_t NVIC_IRQChannelPriority;     /*!< Specifies the priority level for the IRQ channel specified
                                            in NVIC_IRQChannel. This parameter can be a value
                                            between 0 and 3.  */

  IAP_FunctionalState NVIC_IRQChannelCmd;  /*!< Specifies whether the IRQ channel defined in NVIC_IRQChannel
                                            will be enabled or disabled. 
                                            This parameter can be set either to ENABLE or DISABLE */   
} IAP_NVIC_InitTypeDef;


typedef enum
{
  IAP_GPIO_Mode_IN   = 0x00, /*!< GPIO Input Mode              */
  IAP_GPIO_Mode_OUT  = 0x01, /*!< GPIO Output Mode             */
  IAP_GPIO_Mode_AF   = 0x02, /*!< GPIO Alternate function Mode */
  IAP_GPIO_Mode_AN   = 0x03  /*!< GPIO Analog In/Out Mode      */
}IAP_GPIOMode_TypeDef;
typedef enum
{
  IAP_GPIO_Speed_Level_1  = 0x01, /*!< Medium Speed */
  IAP_GPIO_Speed_Level_2  = 0x02, /*!< Fast Speed   */
  IAP_GPIO_Speed_Level_3  = 0x03  /*!< High Speed   */
}IAP_GPIOSpeed_TypeDef;
typedef enum
{
  IAP_GPIO_OType_PP = 0x00,
  IAP_GPIO_OType_OD = 0x01
}IAP_GPIOOType_TypeDef;
typedef enum
{
  IAP_GPIO_PuPd_NOPULL = 0x00,
  IAP_GPIO_PuPd_UP     = 0x01,
  IAP_GPIO_PuPd_DOWN   = 0x02
}IAP_GPIOPuPd_TypeDef;

typedef struct
{
  IAP_uint32_t GPIO_Pin;              /*!< Specifies the GPIO pins to be configured.
                                       This parameter can be any value of @ref GPIO_pins_define */
                                       
  IAP_GPIOMode_TypeDef GPIO_Mode;     /*!< Specifies the operating mode for the selected pins.
                                       This parameter can be a value of @ref GPIOMode_TypeDef   */

  IAP_GPIOSpeed_TypeDef GPIO_Speed;   /*!< Specifies the speed for the selected pins.
                                       This parameter can be a value of @ref GPIOSpeed_TypeDef  */

  IAP_GPIOOType_TypeDef GPIO_OType;   /*!< Specifies the operating output type for the selected pins.
                                       This parameter can be a value of @ref GPIOOType_TypeDef  */

  IAP_GPIOPuPd_TypeDef GPIO_PuPd;     /*!< Specifies the operating Pull-up/Pull down for the selected pins.
                                       This parameter can be a value of @ref GPIOPuPd_TypeDef   */
}IAP_GPIO_InitTypeDef;

typedef struct
{
  __IAP_IO IAP_uint32_t ISR;          /*!< DMA interrupt status register,                            Address offset: 0x00 */
  __IAP_IO IAP_uint32_t IFCR;         /*!< DMA interrupt flag clear register,                        Address offset: 0x04 */
} IAP_DMA_TypeDef;

typedef struct
{
  IAP_uint32_t DMA_PeripheralBaseAddr; /*!< Specifies the peripheral base address for DMAy Channelx.              */

  IAP_uint32_t DMA_MemoryBaseAddr;     /*!< Specifies the memory base address for DMAy Channelx.                  */

  IAP_uint32_t DMA_DIR;                /*!< Specifies if the peripheral is the source or destination.
                                        This parameter can be a value of @ref DMA_data_transfer_direction     */

  IAP_uint32_t DMA_BufferSize;         /*!< Specifies the buffer size, in data unit, of the specified Channel. 
                                        The data unit is equal to the configuration set in DMA_PeripheralDataSize
                                        or DMA_MemoryDataSize members depending in the transfer direction     */

  IAP_uint32_t DMA_PeripheralInc;      /*!< Specifies whether the Peripheral address register is incremented or not.
                                        This parameter can be a value of @ref DMA_peripheral_incremented_mode */

  IAP_uint32_t DMA_MemoryInc;          /*!< Specifies whether the memory address register is incremented or not.
                                        This parameter can be a value of @ref DMA_memory_incremented_mode     */

  IAP_uint32_t DMA_PeripheralDataSize; /*!< Specifies the Peripheral data width.
                                        This parameter can be a value of @ref DMA_peripheral_data_size        */

  IAP_uint32_t DMA_MemoryDataSize;     /*!< Specifies the Memory data width.
                                        This parameter can be a value of @ref DMA_memory_data_size            */

  IAP_uint32_t DMA_Mode;               /*!< Specifies the operation mode of the DMAy Channelx.
                                        This parameter can be a value of @ref DMA_circular_normal_mode
                                        @note: The circular buffer mode cannot be used if the memory-to-memory
                                              data transfer is configured on the selected Channel */

  IAP_uint32_t DMA_Priority;           /*!< Specifies the software priority for the DMAy Channelx.
                                        This parameter can be a value of @ref DMA_priority_level              */

  IAP_uint32_t DMA_M2M;                /*!< Specifies if the DMAy Channelx will be used in memory-to-memory transfer.
                                        This parameter can be a value of @ref DMA_memory_to_memory            */
}IAP_DMA_InitTypeDef;
typedef struct
{
  IAP_uint32_t USART_BaudRate;            /*!< This member configures the USART communication baud rate.
                                           The baud rate is computed using the following formula:
                                            - IntegerDivider = ((PCLKx) / (16 * (USART_InitStruct->USART_BaudRate)))
                                            - FractionalDivider = ((IntegerDivider - ((IAP_uint32_t) IntegerDivider)) * 16) + 0.5 */

  IAP_uint32_t USART_WordLength;          /*!< Specifies the number of data bits transmitted or received in a frame.
                                           This parameter can be a value of @ref USART_Word_Length */

  IAP_uint32_t USART_StopBits;            /*!< Specifies the number of stop bits transmitted.
                                           This parameter can be a value of @ref USART_Stop_Bits */

  IAP_uint32_t USART_Parity;              /*!< Specifies the parity mode.
                                           This parameter can be a value of @ref USART_Parity
                                           @note When parity is enabled, the computed parity is inserted
                                                 at the MSB position of the transmitted data (9th bit when
                                                 the word length is set to 9 data bits; 8th bit when the
                                                 word length is set to 8 data bits). */
 
  IAP_uint32_t USART_Mode;                /*!< Specifies wether the Receive or Transmit mode is enabled or disabled.
                                           This parameter can be a value of @ref USART_Mode */

  IAP_uint32_t USART_HardwareFlowControl; /*!< Specifies wether the hardware flow control mode is enabled
                                           or disabled.
                                           This parameter can be a value of @ref USART_Hardware_Flow_Control*/
} IAP_USART_InitTypeDef;
typedef struct
{
  IAP_uint32_t SYSCLK_Frequency;
  IAP_uint32_t HCLK_Frequency;
  IAP_uint32_t PCLK_Frequency;
  IAP_uint32_t ADCCLK_Frequency;
  IAP_uint32_t CECCLK_Frequency;
  IAP_uint32_t I2C1CLK_Frequency;
  IAP_uint32_t USART1CLK_Frequency;
}IAP_RCC_ClocksTypeDef;
typedef struct
{
  __IAP_IO IAP_uint32_t ACR;          /*!<FLASH access control register,                 Address offset: 0x00 */
  __IAP_IO IAP_uint32_t KEYR;         /*!<FLASH key register,                            Address offset: 0x04 */
  __IAP_IO IAP_uint32_t OPTKEYR;      /*!<FLASH OPT key register,                        Address offset: 0x08 */
  __IAP_IO IAP_uint32_t SR;           /*!<FLASH status register,                         Address offset: 0x0C */
  __IAP_IO IAP_uint32_t CR;           /*!<FLASH control register,                        Address offset: 0x10 */
  __IAP_IO IAP_uint32_t AR;           /*!<FLASH address register,                        Address offset: 0x14 */
  __IAP_IO IAP_uint32_t RESERVED;     /*!< Reserved,                                                     0x18 */
  __IAP_IO IAP_uint32_t OBR;          /*!<FLASH option bytes register,                   Address offset: 0x1C */
  __IAP_IO IAP_uint32_t WRPR;         /*!<FLASH option bytes register,                   Address offset: 0x20 */
} IAP_FLASH_TypeDef;
typedef enum
{
  IAP_FLASH_OK,    
  IAP_FLASH_BUSY = 1,
  IAP_FLASH_ERROR_WRP,
  IAP_FLASH_ERROR_PROGRAM,
  IAP_FLASH_COMPLETE,
  IAP_FLASH_TIMEOUT
}IAP_FLASH_Status;
typedef struct
{
  __IAP_IO IAP_uint32_t ISER[1];                 /*!< Offset: 0x000 (R/W)  Interrupt Set Enable Register           */
       IAP_uint32_t RESERVED0[31];
  __IAP_IO IAP_uint32_t ICER[1];                 /*!< Offset: 0x080 (R/W)  Interrupt Clear Enable Register          */
       IAP_uint32_t RSERVED1[31];
  __IAP_IO IAP_uint32_t ISPR[1];                 /*!< Offset: 0x100 (R/W)  Interrupt Set Pending Register           */
       IAP_uint32_t RESERVED2[31];
  __IAP_IO IAP_uint32_t ICPR[1];                 /*!< Offset: 0x180 (R/W)  Interrupt Clear Pending Register         */
       IAP_uint32_t RESERVED3[31];
       IAP_uint32_t RESERVED4[64];
  __IAP_IO IAP_uint32_t IP[8];                   /*!< Offset: 0x300 (R/W)  Interrupt Priority Register              */
} IAP_NVIC_Type;

typedef struct
{
    IAP_uint32_t uiFlashDestination;
    IAP_uint8_t PageNumber;
}IapSector_t;
/******************************************************************
*                           宏定义                                *
******************************************************************/
/*C语言实现整数除法和模除*/
#define IAP_LOOP_CONTROL      0x80000000
#define IAP_CALCULATE         1

/*标志帧*/
#define IAP_DATA_SECTOR        0
#define IAP_DEVICE_SECTOR      1
#define IAP_VERSION_SECTOR     2
/*DMA通道复用*/
typedef struct
{
  __IAP_IO IAP_uint32_t CFGR1;       /*!< SYSCFG configuration register 1,                           Address offset: 0x00 */
       IAP_uint32_t RESERVED;    /*!< Reserved,                                                                  0x04 */
  __IAP_IO IAP_uint32_t EXTICR[4];   /*!< SYSCFG external interrupt configuration register,     Address offset: 0x14-0x08 */
  __IAP_IO IAP_uint32_t CFGR2;       /*!< SYSCFG configuration register 2,                           Address offset: 0x18 */
} IAP_SYSCFG_TypeDef;

#define IAP_PERIPH_BASE           ((IAP_uint32_t)0x40000000) /*!< Peripheral base address in the alias region */

/*!< Peripheral memory map */
#define IAP_APBPERIPH_BASE        IAP_PERIPH_BASE

#define IAP_SYSCFG_BASE           (IAP_APBPERIPH_BASE + 0x00010000)

#define IAP_SYSCFG              ((IAP_SYSCFG_TypeDef *) IAP_SYSCFG_BASE)

#define IAP_RCC_APB2ENR_SYSCFGEN                ((IAP_uint32_t)0x00000001)        /*!< SYSCFG clock enable */
#define IAP_RCC_APB2Periph_SYSCFG            IAP_RCC_APB2ENR_SYSCFGEN

#define IAP_SYSCFG_CFGR1_USART1RX_DMA_RMP       ((IAP_uint32_t)0x00000400) /*!< USART1 RX DMA remap */
#define IAP_SYSCFG_CFGR1_USART1TX_DMA_RMP       ((IAP_uint32_t)0x00000200) /*!< USART1 TX DMA remap */

/*DMA通道复用*/

#define  IAP_RCC_AHBENR_GPIOAEN                  ((IAP_uint32_t)0x00020000)        /*!< GPIOA clock enable */
#define  IAP_RCC_AHBENR_GPIOBEN                  ((IAP_uint32_t)0x00040000)        /*!< GPIOB clock enable */
#define  IAP_RCC_APB2ENR_USART1EN                ((IAP_uint32_t)0x00004000)        /*!< USART1 clock enable */
#define  IAP_RCC_AHBENR_DMA1EN                   ((IAP_uint32_t)0x00000001)        /*!< DMA1 clock enable */


#define IAP_AHB2PERIPH_BASE       (IAP_PERIPH_BASE + 0x08000000)
#define IAP_GPIOA_BASE            (IAP_AHB2PERIPH_BASE + 0x00000000)
#define IAP_GPIOA                  ((IAP_GPIO_TypeDef *) IAP_GPIOA_BASE)

#define IAP_GPIOB_BASE            (IAP_AHB2PERIPH_BASE + 0x00000400)
#define IAP_GPIOB                   ((IAP_GPIO_TypeDef *) IAP_GPIOB_BASE)

#define IAP_GPIO_PinSource12           ((IAP_uint8_t)0x0C)
#define IAP_GPIO_PinSource9            ((IAP_uint8_t)0x09)
#define IAP_GPIO_PinSource10           ((IAP_uint8_t)0x0A)

#define IAP_GPIO_AF_1            ((IAP_uint8_t)0x01) 

#define IAP_GPIO_Pin_0                 ((IAP_uint16_t)0x0001)  /*!< Pin 0 selected    */
#define IAP_GPIO_Pin_1                 ((IAP_uint16_t)0x0002)  /*!< Pin 1 selected    */
#define IAP_GPIO_Pin_2                 ((IAP_uint16_t)0x0004)  /*!< Pin 2 selected    */
#define IAP_GPIO_Pin_3                 ((IAP_uint16_t)0x0008)  /*!< Pin 3 selected    */
#define IAP_GPIO_Pin_4                 ((IAP_uint16_t)0x0010)  /*!< Pin 4 selected    */
#define IAP_GPIO_Pin_5                 ((IAP_uint16_t)0x0020)  /*!< Pin 5 selected    */
#define IAP_GPIO_Pin_6                 ((IAP_uint16_t)0x0040)  /*!< Pin 6 selected    */
#define IAP_GPIO_Pin_7                 ((IAP_uint16_t)0x0080)  /*!< Pin 7 selected    */
#define IAP_GPIO_Pin_8                 ((IAP_uint16_t)0x0100)  /*!< Pin 8 selected    */
#define IAP_GPIO_Pin_9                 ((IAP_uint16_t)0x0200)  /*!< Pin 9 selected    */
#define IAP_GPIO_Pin_10                ((IAP_uint16_t)0x0400)  /*!< Pin 10 selected   */
#define IAP_GPIO_Pin_11                ((IAP_uint16_t)0x0800)  /*!< Pin 11 selected   */
#define IAP_GPIO_Pin_12                ((IAP_uint16_t)0x1000)  /*!< Pin 12 selected   */
#define IAP_GPIO_Pin_13                ((IAP_uint16_t)0x2000)  /*!< Pin 13 selected   */
#define IAP_GPIO_Pin_14                ((IAP_uint16_t)0x4000)  /*!< Pin 14 selected   */
#define IAP_GPIO_Pin_15                ((IAP_uint16_t)0x8000)  /*!< Pin 15 selected   */
#define IAP_GPIO_Pin_All               ((IAP_uint16_t)0xFFFF)  /*!< All pins selected */



#define IAP_GPIO_Speed_50MHz           ((IAP_uint8_t)0x03)    /*!< High Speed:50MHz   */

#define IAP_USART1_TDR_ADDRESS           (0x40013828)
#define IAP_DMA_DIR_PeripheralDST       ((IAP_uint32_t)0x00000010)
#define IAP_DMA_SEND_BUFSIZE             (256)
#define IAP_DMA_MemoryInc_Enable         ((IAP_uint32_t)0x00000080) 
#define IAP_DMA_PeripheralDataSize_Byte  ((IAP_uint32_t)0x00000000)
#define IAP_DMA_PeripheralDataSize_HalfWord    ((IAP_uint32_t)0x00000100)        /*!< Bit 0                               */

#define IAP_DMA_MemoryDataSize_Byte       ((IAP_uint32_t)0x00000000)
#define IAP_DMA_Mode_Normal                ((IAP_uint32_t)0x00000000)
#define IAP_DMA_Priority_Low                ((IAP_uint32_t)0x00000000)
#define IAP_DMA_M2M_Disable                 ((IAP_uint32_t)0x00000000)

#define IAP_USART1_RDR_ADDRESS               (0x40013824)
#define IAP_DMA_DIR_PeripheralSRC           ((IAP_uint32_t)0x00000000)
#define IAP_DMA_RECEIVE_BUFSIZE               (1024u)
#define IAP_DMA_PeripheralInc_Disable        ((IAP_uint32_t)0x00000000)
#define IAP_USART1_IRQn                        (27)
#define IAP_DMA1_Channel2_3_IRQn              (10)

#define IAP_USART_WordLength_8b               ((IAP_uint32_t)0x00000000)
#define IAP_USART_StopBits_1                  ((IAP_uint32_t)0x00000000)
#define IAP_USART_Parity_No                   ((IAP_uint32_t)0x00000000)
#define IAP_USART_HardwareFlowControl_RTS     ((IAP_uint32_t)0x00000100)
#define IAP_USART_HardwareFlowControl_None       ((IAP_uint32_t)0x00000000)

#define IAP_USART_Mode_Rx                      ((IAP_uint32_t)0x00000004)
#define IAP_USART_Mode_Tx                      ((IAP_uint32_t)0x00000008)

#define IAP_DMA1_Channel2       ((IAP_DMA_Channel_TypeDef *) IAP_DMA1_Channel2_BASE)
#define IAP_DMA1_Channel2_BASE    (IAP_DMA1_BASE + 0x0000001C)
#define IAP_DMA1_BASE             (IAP_AHBPERIPH_BASE + 0x00000000)
#define IAP_AHBPERIPH_BASE        (IAP_PERIPH_BASE + 0x00020000)
#define IAP_DMA1_Channel3_BASE    (IAP_DMA1_BASE + 0x00000030)
#define IAP_DMA1_Channel3       ((IAP_DMA_Channel_TypeDef *) IAP_DMA1_Channel3_BASE)
#define IAP_USART_DMAReq_Rx       ((IAP_uint32_t)0x00000040)            /*!< DMA Enable Receiver */

#define IAP_USART_DMAReq_Tx                      IAP_USART_CR3_DMAT
#define IAP_USART_CR3_DMAT                      ((IAP_uint32_t)0x00000080)            /*!< DMA Enable Transmitter */


#define IAP_USART1_BASE           (IAP_APBPERIPH_BASE + 0x00013800)
#define IAP_USART1              ((IAP_USART_TypeDef *) IAP_USART1_BASE)

#define IAP_RCC_BASE              (IAP_AHBPERIPH_BASE + 0x00001000)
#define IAP_RCC                 ((IAP_RCC_TypeDef *) IAP_RCC_BASE)

#define IAP_IS_RCC_AHB_PERIPH(PERIPH) ((((PERIPH) & 0xFEA1FFAA) == 0x00) && ((PERIPH) != 0x00))
#define IAP_IS_FUNCTIONAL_STATE(STATE) (((STATE) == 0) || ((STATE) == 1))
#define IAP_IS_RCC_APB2_PERIPH(PERIPH) ((((PERIPH) & 0xFFB8A5FE) == 0x00) && ((PERIPH) != 0x00))

#define IAP_SCS_BASE            (0xE000E000UL) 

#define IAP_NVIC_BASE           (IAP_SCS_BASE +  0x0100UL)                    /*!< NVIC Base Address                 */
#define IAP_NVIC                ((IAP_NVIC_Type      *)     IAP_NVIC_BASE     )   /*!< NVIC configuration struct          */

#define IAP_GPIO_OSPEEDER_OSPEEDR0     ((IAP_uint32_t)0x00000003)
#define IAP_GPIO_OTYPER_OT_0           ((IAP_uint32_t)0x00000001)
#define IAP_GPIO_MODER_MODER0          ((IAP_uint32_t)0x00000003)
#define IAP_GPIO_PUPDR_PUPDR0          ((IAP_uint32_t)0x00000003)

#define IAP_DMA_CCR_EN                  ((IAP_uint32_t)0x00000001)        /*!< Channel enable                      */
//#define IAP_DMA1_BASE             (IAP_AHBPERIPH_BASE + 0x00000000)
#define IAP_DMA1                ((IAP_DMA_TypeDef *) IAP_DMA1_BASE)
#define IAP_DMA_ISR_GIF2                        ((IAP_uint32_t)0x00000010)        /*!< Channel 2 Global interrupt flag    */
#define IAP_DMA_ISR_TCIF2                       ((IAP_uint32_t)0x00000020)        /*!< Channel 2 Transfer Complete flag   */
#define IAP_DMA_ISR_HTIF2                       ((IAP_uint32_t)0x00000040)        /*!< Channel 2 Half Transfer flag       */
#define IAP_DMA_ISR_TEIF2                       ((IAP_uint32_t)0x00000080)        /*!< Channel 2 Transfer Error flag      */
#define IAP_DMA_ISR_GIF3                        ((IAP_uint32_t)0x00000100)        /*!< Channel 3 Global interrupt flag    */
#define IAP_DMA_ISR_TCIF3                       ((IAP_uint32_t)0x00000200)        /*!< Channel 3 Transfer Complete flag   */
#define IAP_DMA_ISR_HTIF3                       ((IAP_uint32_t)0x00000400)        /*!< Channel 3 Half Transfer flag       */
#define IAP_DMA_ISR_TEIF3                       ((IAP_uint32_t)0x00000800)        /*!< Channel 3 Transfer Error flag      */
#define IAP_DMA1_CHANNEL2_IT_MASK    ((IAP_uint32_t)(IAP_DMA_ISR_GIF2 | IAP_DMA_ISR_TCIF2 | IAP_DMA_ISR_HTIF2 | IAP_DMA_ISR_TEIF2))
#define IAP_DMA1_CHANNEL3_IT_MASK    ((IAP_uint32_t)(IAP_DMA_ISR_GIF3 | IAP_DMA_ISR_TCIF3 | IAP_DMA_ISR_HTIF3 | IAP_DMA_ISR_TEIF3))
#define IAP_CCR_CLEAR_MASK                ((IAP_uint32_t)0xFFFF800F) /* DMA Channel config registers Masks */
#define IAP_USART_CR1_UE                        ((IAP_uint32_t)0x00000001)            /*!< USART Enable */
#define IAP_USART_CR2_STOP                      ((IAP_uint32_t)0x00003000)            /*!< STOP[1:0] bits (STOP bits) */
#define IAP_CR1_CLEAR_MASK          ((IAP_uint32_t)0x00CFE0FF)  /*<! I2C CR1 clear register Mask */
#define IAP_USART_CR3_RTSE                      ((IAP_uint32_t)0x00000100)            /*!< RTS Enable */
#define IAP_USART_CR3_CTSE                      ((IAP_uint32_t)0x00000200)            /*!< CTS Enable */
#define IAP_CR3_CLEAR_MASK            ((IAP_uint32_t)(IAP_USART_CR3_RTSE | IAP_USART_CR3_CTSE))
#define IAP_USART_CR1_OVER8                     ((IAP_uint32_t)0x00008000)            /*!< Oversampling by 8-bit or 16-bit mode */

#define IAP_RCC_CFGR_SWS                        ((IAP_uint32_t)0x0000000C)        /*!< SWS[1:0] bits (System Clock Switch Status) */
#define IAP_HSI_VALUE                            ((IAP_uint32_t)8000000)
#define IAP_HSE_VALUE                            ((IAP_uint32_t)12000000)
#define IAP_RCC_CFGR_PLLMULL                    ((IAP_uint32_t)0x003C0000)        /*!< PLLMUL[3:0] bits (PLL multiplication factor) */
#define IAP_RCC_CFGR_PLLSRC                     ((IAP_uint32_t)0x00010000)        /*!< PLL entry clock source */
#define IAP_RCC_CFGR2_PREDIV1                   ((IAP_uint32_t)0x0000000F)        /*!< PREDIV1[3:0] bits */
#define IAP_RCC_CFGR_HPRE                       ((IAP_uint32_t)0x000000F0)        /*!< HPRE[3:0] bits (AHB prescaler) */
#define IAP_RCC_CFGR_PPRE                       ((IAP_uint32_t)0x00000700)        /*!< PRE[2:0] bits (APB prescaler) */
#define IAP_RCC_CFGR3_ADCSW                     ((IAP_uint32_t)0x00000100)        /*!< ADCSW bits */ 
#define IAP_HSI14_VALUE                          ((IAP_uint32_t)14000000)
#define IAP_RCC_CFGR_ADCPRE                     ((IAP_uint32_t)0x00004000)        /*!< ADCPRE bit (ADC prescaler) */
#define IAP_RCC_CFGR3_CECSW                     ((IAP_uint32_t)0x00000040)        /*!< CECSW bits */ 
#define IAP_LSE_VALUE                             ((IAP_uint32_t)32768)  
#define IAP_RCC_CFGR3_I2C1SW                    ((IAP_uint32_t)0x00000010)        /*!< I2C1SW bits */ 
#define IAP_RCC_CFGR3_USART1SW                  ((IAP_uint32_t)0x00000003)        /*!< USART1SW[1:0] bits */
#define IAP_RCC_CFGR3_USART1SW_0                ((IAP_uint32_t)0x00000001)   
#define IAP_RCC_CFGR3_USART1SW_1                ((IAP_uint32_t)0x00000002)   
#define IAP_USART_DE_ENABLE                      ((IAP_uint32_t)0x4000)

#define IAP_FLASH_R_BASE          (IAP_AHBPERIPH_BASE + 0x00002000) /*!< FLASH registers base address */
#define IAP_FLASH               ((IAP_FLASH_TypeDef *) IAP_FLASH_R_BASE)
#define IAP_FLASH_CR_LOCK                       ((IAP_uint32_t)0x00000080)        /*!< Lock */
#define IAP_FLASH_FKEY1                          ((IAP_uint32_t)0x45670123)        /*!< Flash program erase key1 */
#define IAP_FLASH_FKEY2                          ((IAP_uint32_t)0xCDEF89AB)        /*!< Flash program erase key2: used with FLASH_PEKEY1*/
#define IAP_FLASH_ER_PRG_TIMEOUT         ((IAP_uint32_t)0x000B0000)
#define IAP_FLASH_CR_PER                        ((IAP_uint32_t)0x00000002)        /*!< Page Erase */
#define IAP_FLASH_CR_STRT                       ((IAP_uint32_t)0x00000040)        /*!< Start */
#define IAP_FLASH_FLAG_BSY                 ((IAP_uint32_t)0x00000001)     /*!< FLASH Busy flag */
#define IAP_FLASH_FLAG_PGERR               ((IAP_uint32_t)0x00000004)    /*!< FLASH Programming error flag */
#define IAP_FLASH_FLAG_WRPERR              ((IAP_uint32_t)0x00000010)  /*!< FLASH Write protected error flag */
#define IAP_FLASH_FLAG_EOP                 ((IAP_uint32_t)0x00000020)        /*!< FLASH End of Programming flag */
        /*!< defines 'read / write' permissions              */
#define IAP_FLASH_CR_PG                         ((IAP_uint32_t)0x00000001)        /*!< Programming */

#define CLI()                           __set_PRIMASK(1)  
#define IAP_NVIC_BASE           (IAP_SCS_BASE +  0x0100UL)                    /*!< NVIC Base Address                 */
#define IAP_NVIC                ((IAP_NVIC_Type      *)     IAP_NVIC_BASE     )   /*!< NVIC configuration struct          */
#define IAP_UPGRADE_CODE        (0x08000000)
#define IAP_UPGRADE_DEVICE      (0x0800C000)
#define IAP_UPGRADE_VERSION     (0x0800D000)

#define PACKET_LENGTH            (128 & 0xFFFF)
#define RECEIVE_SIZE              (256)
#define RESPOND_LENGTH            (16 & 0xFFFF)
#define MESSGAE_SIZE               (16)

#define ALL_SECTOR               60
#define CYCLE_TIMES              240
#define RECEIVE_COMPLETE          (0)
#define PAGE_SIZE                  (0x400)
#define CODE_PAGE_NUMBER          (52)
#define DEVICE_PAGE_NUMBER        (4)
#define VERSION_PAGE_NUMBER       (4)
#define PROGRAMME_LENGTH        (0x04)
#define IAP_SECTOR_ERASE_TIMES     4


/*添加CRC*/
//#define IAP_CRC16_USETABLE          ( 1 )
//#define IAP_STATIC                         static

typedef unsigned char IAPUBYTE;
typedef unsigned short IAPUSHORT;


/*从网关接收请求帧长度*/
#define FROM_GW_REQUEST_FRAME_LEN          (160)

/*升级类型指示*/
#define IAP_TYPE_INDICATE_REQ          1

/*升级内容指示*/
#define IAP_CONTENT_REQ                2

/*升级结束指示*/
#define IAP_COMPLETE_REQ               3

#define BIT_16        (16)

#define IAP_HARDWARE_VERSION1_LOCATION  (2) /* 产品批次1位于诊断信息的第四位*/
#define IAP_HARDWARE_VERSION2_LOCATION  (3) /* 产品批次1位于诊断信息的第四位*/
#define IAP_PRODUCT_BATCH1_LOCATION  (4) /* 产品批次1位于诊断信息的第四位*/
#define IAP_PRODUCT_BATCH2_LOCATION  (5) /* 产品批次2位于诊断信息的第五位*/
#define IAP_PRODUCT_SERIALNUM1_LOCATION  (6)  /*  产品序列号1位于诊断信息的第六位*/
#define IAP_PRODUCT_SERIALNUM2_LOCATION  (7)  /*  产品序列号2位于诊断信息的第七位*/
#define IAP_PRODUCT_BATCH_LOCATION_OFFSET  (8) /*产品批次偏移*/

/*产品批次错误*/
#define IAP_PRODUCT_BATCHES_ERROR         0x1
/*序列号错误*/
#define IAP_SERIAL_NUM_ERROR              0x2
/*升级类型错误*/
#define IAP_UPGRADE_TYPE_ERROR            0x4
/*系统升级成功*/
#define SYSTEM_UPGRADE_REQ_HANDLE_SUCCESS  0x0
/*升级包长度错误*/
#define IAP_UPGRADE_PACKAGE_LEN_ERROR     0x06
/*升级包序号错误*/
#define IAP_UPGRADE_PACKAGE_NUM_ERROR      0x5
#define IAP_UPGRADE_PACKAGE_CONTENT_LOCATION  10
/*未知错误*/
#define IAP_UNKNOW_ERROR                  0x99

/*FLASH擦除失败*/
#define IAP_FLASH_ERASER_ERROR           0x10

/*FLASH写入失败*/
#define IAP_FLASH_WRITE_ERROR             0x7

/*程序升级*/
#define IAP_PROGRAM_UPGRADE                0
/*设备描述文件升级*/
#define IAP_DEVICE_FILE                   1
/*杂项数据改写*/
#define IAP_VERSION_DATA                  2

/*代码扇区*/
#define IAP_CODE_SECTOR_NUM                   12
/*13*1024 /128 = 104*/
#define IAP_MAX_PROGRAM_UPGRADE_PACKAGE_COUNT (104*4)  
#define PAGE_PROGRAMME_TIME       (32)/*128 / 4 = 32*/
#define IAP_FLASH_SECTOR_SIZE     128
/*设备描述文件扇区*/
#define IAP_DEVICE_SECTOR_NUM                 1
#define IAP_MAX_DEVICE_UPGRADE_PACKAGE_COUNT  8
/*杂项数据扇区*/
#define IAP_VERSION_SECTOR_NUM                1
#define IAP_MAX_VERSION_UPGRADE_PACKAGE_COUNT  8


/* 除产品信息外的其他诊断数 */
#define IAP_COMMON_DIAGNOSE_NUM (2)
/* 产品信息长度 */
#define IAP_PRODUCT_INFO_NUMBER (6)
/* 诊断数据定义 */
#define IAP_MAX_DIAGNOSE_DATA_NUM   (IAP_COMMON_DIAGNOSE_NUM + IAP_PRODUCT_INFO_NUMBER)     /* 最大诊断数 */

#define IAP_PRODUCT_BATCH1_SIZE               2
#define IAP_PRODUCT_BATCH2_SIZE               2
#define IAP_PRODUCT_BATCH_SIZE               4
#define IAP_PRODUCT_BATCH_LOCATION           2 

#define IAP_PRODUCT_SERIAL_NUM1_SIZE          2 
#define IAP_PRODUCT_SERIAL_NUM2_SIZE         2 
#define IAP_PRODUCT_SERIAL_NUM_SIZE          4  
#define IAP_PRODUCT_SERIAL_NUM_LOCATION      6 

#define IAP_UPGRADE_TYPE_SIZE               1
#define IAP_UPGRADE_TYPE_LOCATION           2 
#define IAP_UPGRADE_PACKAGE_SIZE            2

#define IAP_UPGRADE_PACKAGE_SN_SIZE        (4) /*升级包序列号的长度为4*/
#define IAP_END_MARK_LENGTH                  1
#define IAP_UPGRADE_CONTENT_REQ_FRAME_SIZE  (9) /*升级内容请求帧的长度为9*/
#define END_MASK                            1
#define IAP_GW_STATION_NUM_LOCATION         0      /*网关站号处于响应帧第0位*/
#define IAP_GW_STATION_NUM_LEN              1
#define IAP_CONTENT_RESPONSE_SIZE           7
  
#define IAP_SLAVE_UPGRADE_PACKAGE_SIZE      128
#define IAP_TO_NEXT_SN                      1

/*响应帧*/
#define GW_STATION_NUM                      0xFE
#define GW_STATION_LOCATION                 0
#define SLAVE_STATION_LOCATION              1
#define FRAME_TYPE_LOCATION                 2
#define IAP_COMPLETE_HANDLE_RESULT_LOCATION 3
#define UPGRADE_TYPE_LOCATION               3
#define IAP_TYPE__HANDLE_RESULT_LOCATION    4
#define IAP_TYPE_RESPONSE_FRAME_LEN         5
#define IAP_COMPLETE_RESPONSE_FRAME_LEN     4
#define IAP_UPGRADE_TYPE_LOCATE             10
#define IAP_UPGRADE_PACKAGE_TYPE_ERROR      0x08
#define IAP_UPGRADE_PACKAGE_TYPE_LOCATE     5
#define IAP_UPGRADE_PACKAGE_TYPE_SIZE        1
#define IAP_PACKAGE_RETRANSMISSION_SIZE          1
#define IAP_NOT_PACKAGE_RETRANSMISSION      0
#define IAP_PACKAGE_RETRANSMISSION      1
#define IAP_PACKAGE_RETRANSMISSION_ERROR      0x0A


#define IAP_HARDWARE_VERSION_ANZAHL_LOCATE  1
#define IAP_HARDWARE_VERSION_ANZAHL_SIZE    1
#define IAP_SLAVE_UPGRADE_PACKAGE_TYPE_LOCATE    0



#define IAP_HARDWARE_VERSION_NUM_ERROR      0x09
#define IAP_HARDWARE_VERSION_NUM_LOCATE     13
#define IAP_HARDWARE_VERSION_NUM1_SIZE       2
#define IAP_HARDWARE_VERSION_NUM2_SIZE       2
#define IAP_HARDWARE_VERSION_NUM_SIZE       4
/*升级信息最大长度 */
#define IAP_MAX_IO_UPGRADE_INFO_LENGTH  (256)

/*请求帧*/
#define FROM_GW_SLAVE_STATION_LOCATION      0
#define FROM_GW_FRAME_TYPE_LOCATION         1

#define IAP_USART_DEAT_VALUE        (16)
#define IAP_USART_CR1_DEAT          ((IAP_uint32_t)0x03E00000)           

#define IAP_PRODUCTBATCH_SERIAL_NUM_LEN     4
#ifndef IAP_NULL
#define IAP_NULL    ((void *)0)
#endif

/*软复位*/
/* IO definitions (access restrictions to peripheral registers) */
#ifdef __cplusplus
  #define   _IAP__I     volatile             /*!< defines 'read only' permissions                 */
#else
  #define   _IAP__I     volatile const       /*!< defines 'read only' permissions                 */
#endif

typedef struct
{
  _IAP__I  IAP_uint32_t CPUID;                   /*!< Offset: 0x000 (R/ )  CPUID Base Register                                   */
  __IAP_IO IAP_uint32_t ICSR;                    /*!< Offset: 0x004 (R/W)  Interrupt Control and State Register                  */
       IAP_uint32_t RESERVED0;
  __IAP_IO IAP_uint32_t AIRCR;                   /*!< Offset: 0x00C (R/W)  Application Interrupt and Reset Control Register      */
  __IAP_IO IAP_uint32_t SCR;                     /*!< Offset: 0x010 (R/W)  System Control Register                               */
  __IAP_IO IAP_uint32_t CCR;                     /*!< Offset: 0x014 (R/W)  Configuration Control Register                        */
       IAP_uint32_t RESERVED1;
  __IAP_IO IAP_uint32_t SHP[2];                  /*!< Offset: 0x01C (R/W)  System Handlers Priority Registers. [0] is RESERVED   */
  __IAP_IO IAP_uint32_t SHCSR;                   /*!< Offset: 0x024 (R/W)  System Handler Control and State Register             */
} IAP_SCB_Type;

#define IAP_SCB_BASE            (IAP_SCS_BASE +  0x0D00UL)                    /*!< System Control Block Base Address */
#define IAP_SCB                 ((IAP_SCB_Type       *)     IAP_SCB_BASE      )   /*!< SCB configuration struct           */    

#define IAP_SCB_AIRCR_SYSRESETREQ_Pos           2                                             /*!< SCB AIRCR: SYSRESETREQ Position */
#define IAP_SCB_AIRCR_SYSRESETREQ_Msk          (1UL << IAP_SCB_AIRCR_SYSRESETREQ_Pos)             /*!< SCB AIRCR: SYSRESETREQ Mask */


#if   defined ( __CC_ARM )
#define IAP__DSB()                           __dsb(0xF)

  #define __ASM            __asm                                      /*!< asm keyword for ARM Compiler          */
  #define __INLINE         __inline                                   /*!< inline keyword for ARM Compiler       */

#elif defined ( __ICCARM__ )
  #define __ASM           __asm                                       /*!< asm keyword for IAR Compiler          */
  #define __INLINE        inline                                      /*!< inline keyword for IAR Compiler. Only available in High optimization mode! */

#elif defined ( __GNUC__ )
  #define __ASM            __asm                                      /*!< asm keyword for GNU Compiler          */
  #define __INLINE         inline                                     /*!< inline keyword for GNU Compiler       */

#elif defined ( __TASKING__ )
  #define __ASM            __asm                                      /*!< asm keyword for TASKING Compiler      */
  #define __INLINE         inline                                     /*!< inline keyword for TASKING Compiler   */
#endif  //__CC_ARM


/*DMA4，5通道*/
#define IAP_DMA1_Channel4_BASE    (IAP_DMA1_BASE + 0x00000044)
#define IAP_DMA1_Channel4       ((IAP_DMA_Channel_TypeDef *) IAP_DMA1_Channel4_BASE)
#define IAP_DMA1_Channel5_BASE    (IAP_DMA1_BASE + 0x00000058)
#define IAP_DMA1_Channel5       ((IAP_DMA_Channel_TypeDef *) IAP_DMA1_Channel5_BASE)

/*软复位*/
#define IAP_RESET               (0x05FA0000| 0x04)
#define IAP_CRC16_VALUE_LENGTH     (2)
#define IAP_CRC16_VALUE_NOERR     (0)

/******************************************************************
*                           全局变量声明                          *
******************************************************************/

/******************************************************************
*                         全局函数声明                            *
******************************************************************/
#endif

