#ifndef GPIO_H
#define GPIO_H

void PB0Init();
void Delay(uint32_t cycles);

/* FIX: added include guard (was missing, would cause duplicate-definition
        errors if gpio.h was included more than once in the same translation unit) */

#define __I  volatile const  /*!< Defines 'read only' permissions */
#define __O  volatile          /*!< Defines 'write only' permissions */
#define __IO  volatile      /*!< Defines 'read / write' permissions */
  /* following defines should be used for structure members */
#define __IM  volatile const   /*! Defines 'read only' structure member permissions */
#define __OM  volatile         /*! Defines 'write only' structure member permissions */
#define __IOM  volatile        /*! Defines 'read / write' structure member permissions */

typedef struct {
  __I  uint32_t IIDX;                              /* !< (@ 0x00001080) Interrupt index */
       uint32_t RESERVED0;
  __IO uint32_t IMASK;                             /* !< (@ 0x00001088) Interrupt mask */
       uint32_t RESERVED1;
  __I  uint32_t RIS;                               /* !< (@ 0x00001090) Raw interrupt status */
       uint32_t RESERVED2;
  __I  uint32_t MIS;                               /* !< (@ 0x00001098) Masked interrupt status */
       uint32_t RESERVED3;
  __O  uint32_t ISET;                              /* !< (@ 0x000010A0) Interrupt set */
       uint32_t RESERVED4;
  __O  uint32_t ICLR;                              /* !< (@ 0x000010A8) Interrupt clear */
} GPIO_GEN_EVENT1_Regs;

typedef struct {
  __I  uint32_t IIDX;                              /* !< (@ 0x00001050) Interrupt index */
       uint32_t RESERVED0;
  __IO uint32_t IMASK;                             /* !< (@ 0x00001058) Interrupt mask */
       uint32_t RESERVED1;
  __I  uint32_t RIS;                               /* !< (@ 0x00001060) Raw interrupt status */
       uint32_t RESERVED2;
  __I  uint32_t MIS;                               /* !< (@ 0x00001068) Masked interrupt status */
       uint32_t RESERVED3;
  __O  uint32_t ISET;                              /* !< (@ 0x00001070) Interrupt set */
       uint32_t RESERVED4;
  __O  uint32_t ICLR;                              /* !< (@ 0x00001078) Interrupt clear */
} GPIO_GEN_EVENT0_Regs;


typedef struct {
  __I  uint32_t IIDX;                              /* !< (@ 0x00001020) Interrupt index */
       uint32_t RESERVED0;
  __IO uint32_t IMASK;                             /* !< (@ 0x00001028) Interrupt mask */
       uint32_t RESERVED1;
  __I  uint32_t RIS;                               /* !< (@ 0x00001030) Raw interrupt status */
       uint32_t RESERVED2;
  __I  uint32_t MIS;                               /* !< (@ 0x00001038) Masked interrupt status */
       uint32_t RESERVED3;
  __O  uint32_t ISET;                              /* !< (@ 0x00001040) Interrupt set */
       uint32_t RESERVED4;
  __O  uint32_t ICLR;                              /* !< (@ 0x00001048) Interrupt clear */
} GPIO_CPU_INT_Regs;


typedef struct {
  __IO uint32_t PWREN;                             /* !< (@ 0x00000800) Power enable */
  __O  uint32_t RSTCTL;                            /* !< (@ 0x00000804) Reset Control */
       uint32_t RESERVED0[3];
  __I  uint32_t STAT;                              /* !< (@ 0x00000814) Status Register */
} GPIO_GPRCM_Regs;

/* -----------------------------------------------------------------------
 * mspm0 GPIO register map (Port B, base 0x400A2000)
 * Offsets verified against nRF51822 Product Specification v3.3, §13.
 * ----------------------------------------------------------------------- */
typedef struct {
       uint32_t RESERVED0[256];
  __IO uint32_t FSUB_0;                            /* !< (@ 0x00000400) Subsciber Port 0 */
  __IO uint32_t FSUB_1;                            /* !< (@ 0x00000404) Subscriber Port 1 */
       uint32_t RESERVED1[15];
  __IO uint32_t FPUB_0;                            /* !< (@ 0x00000444) Publisher Port 0 */
  __IO uint32_t FPUB_1;                            /* !< (@ 0x00000448) Publisher Port 1 */
       uint32_t RESERVED2[237];
  GPIO_GPRCM_Regs  GPRCM;                             /* !< (@ 0x00000800) */
       uint32_t RESERVED3[510];
  __IO uint32_t CLKOVR;                            /* !< (@ 0x00001010) Clock Override */
       uint32_t RESERVED4;
  __IO uint32_t PDBGCTL;                           /* !< (@ 0x00001018) Peripheral Debug Control */
       uint32_t RESERVED5;
  GPIO_CPU_INT_Regs  CPU_INT;                           /* !< (@ 0x00001020) */
       uint32_t RESERVED6;
  GPIO_GEN_EVENT0_Regs  GEN_EVENT0;                        /* !< (@ 0x00001050) */
       uint32_t RESERVED7;
  GPIO_GEN_EVENT1_Regs  GEN_EVENT1;                        /* !< (@ 0x00001080) */
       uint32_t RESERVED8[13];
  __IO uint32_t EVT_MODE;                          /* !< (@ 0x000010E0) Event Mode */
       uint32_t RESERVED9[6];
  __I  uint32_t DESC;                              /* !< (@ 0x000010FC) Module Description */
       uint32_t RESERVED10[64];
  __O  uint32_t DOUT3_0;                           /* !< (@ 0x00001200) Data output 3 to 0 */
  __O  uint32_t DOUT7_4;                           /* !< (@ 0x00001204) Data output 7 to 4 */
  __O  uint32_t DOUT11_8;                          /* !< (@ 0x00001208) Data output 11 to 8 */
  __O  uint32_t DOUT15_12;                         /* !< (@ 0x0000120C) Data output 15 to 12 */
  __O  uint32_t DOUT19_16;                         /* !< (@ 0x00001210) Data output 19 to 16 */
  __O  uint32_t DOUT23_20;                         /* !< (@ 0x00001214) Data output 23 to 20 */
  __O  uint32_t DOUT27_24;                         /* !< (@ 0x00001218) Data output 27 to 24 */
  __O  uint32_t DOUT31_28;                         /* !< (@ 0x0000121C) Data output 31 to 28 */
       uint32_t RESERVED11[24];
  __IO uint32_t DOUT31_0;                          /* !< (@ 0x00001280) Data output 31 to 0 */
       uint32_t RESERVED12[3];
  __O  uint32_t DOUTSET31_0;                       /* !< (@ 0x00001290) Data output set 31 to 0 */
       uint32_t RESERVED13[3];
  __O  uint32_t DOUTCLR31_0;                       /* !< (@ 0x000012A0) Data output clear 31 to 0 */
       uint32_t RESERVED14[3];
  __O  uint32_t DOUTTGL31_0;                       /* !< (@ 0x000012B0) Data output toggle 31 to 0 */
       uint32_t RESERVED15[3];
  __IO uint32_t DOE31_0;                           /* !< (@ 0x000012C0) Data output enable 31 to 0 */
       uint32_t RESERVED16[3];
  __O  uint32_t DOESET31_0;                        /* !< (@ 0x000012D0) Data output enable set 31 to 0 */
       uint32_t RESERVED17[3];
  __O  uint32_t DOECLR31_0;                        /* !< (@ 0x000012E0) Data output enable clear 31 to 0 */
       uint32_t RESERVED18[7];
  __I  uint32_t DIN3_0;                            /* !< (@ 0x00001300) Data input 3 to 0 */
  __I  uint32_t DIN7_4;                            /* !< (@ 0x00001304) Data input 7 to 4 */
  __I  uint32_t DIN11_8;                           /* !< (@ 0x00001308) Data input 11 to 8 */
  __I  uint32_t DIN15_12;                          /* !< (@ 0x0000130C) Data input 15 to 12 */
  __I  uint32_t DIN19_16;                          /* !< (@ 0x00001310) Data input 19 to 16 */
  __I  uint32_t DIN23_20;                          /* !< (@ 0x00001314) Data input 23 to 20 */
  __I  uint32_t DIN27_24;                          /* !< (@ 0x00001318) Data input 27 to 24 */
  __I  uint32_t DIN31_28;                          /* !< (@ 0x0000131C) Data input 31 to 28 */
       uint32_t RESERVED19[24];
  __I  uint32_t DIN31_0;                           /* !< (@ 0x00001380) Data input 31 to 0 */
       uint32_t RESERVED20[3];
  __IO uint32_t POLARITY15_0;                      /* !< (@ 0x00001390) Polarity 15 to 0 */
       uint32_t RESERVED21[3];
  __IO uint32_t POLARITY31_16;                     /* !< (@ 0x000013A0) Polarity 31 to 16 */
       uint32_t RESERVED22[23];
  __IO uint32_t CTL;                               /* !< (@ 0x00001400) FAST WAKE GLOBAL EN */
  __IO uint32_t FASTWAKE;                          /* !< (@ 0x00001404) FAST WAKE ENABLE */
       uint32_t RESERVED23[62];
  __IO uint32_t SUB0CFG;                           /* !< (@ 0x00001500) Subscriber 0 configuration */
       uint32_t RESERVED24;
  __IO uint32_t FILTEREN15_0;                      /* !< (@ 0x00001508) Filter Enable 15 to 0 */
  __IO uint32_t FILTEREN31_16;                     /* !< (@ 0x0000150C) Filter Enable 31 to 16 */
  __IO uint32_t DMAMASK;                           /* !< (@ 0x00001510) DMA Write MASK */
       uint32_t RESERVED25[3];
  __IO uint32_t SUB1CFG;                           /* !< (@ 0x00001520) Subscriber 1 configuration */
} GPIO_Regs;

/******************************************************************************
* IOMUX Registers
******************************************************************************/
#define IOMUX_SECCFG_OFS                         ((uint32_t)0x00000000U)

typedef struct {
       uint32_t RESERVED0;
  __IO uint32_t PINCM[251];                        /* !< (@ 0x00000004) Pin Control Management Register in SECCFG region */
} IOMUX_SECCFG_Regs;


typedef struct {
  IOMUX_SECCFG_Regs  SECCFG;                            /* !< (@ 0x00000000) SECCFG register region */
} IOMUX_Regs;

/**
 * \brief The following constants are used to index into the PINCM table
 */
//                      Mode2     Mode3     Mode4     Mode5     Mode6     Mode7     Mode8     Mode9
#define PA0INDEX   0 // UART0_TX  I2C0_SDA  TIMA0_C0  TIMA_FAL1 TIMG8_C1  FCC_IN
#define PA1INDEX   1 // UART0_RX  I2C0_SCL  TIMA0_C1  TIMA_FAL2 TIMG8_IDX TIMG8_C0
#define PA2INDEX   6 // TIMG8_C1  SPI0_CS0  TIMG7_C1  SPI1_CS0
#define PA3INDEX   7 // TIMG8_C0  SPI0_CS1  UART2_CTS TIMA0_C2  COMP1_OUT TIMG7_C0  TIMA0_C1  I2C1_SDA
#define PA4INDEX   8 // TIMG8_C1  SPI0_POCI UART2_RTS TIMA0_C3  LFCLK_IN  TIMG7_C1  TIMA0_C1N I2C1_SCL
#define PA5INDEX   9 // TIMG8_C0  SPI0_PICO TIMA_FAL1 TIMG0_C0  TIMG6_C0  FCC_IN
#define PA6INDEX  10 // TTIMG8_C1 SPI0_SCK  TIMA_FAL0 TIMG0_C1  HFCLK_IN  TIMG6_C1  TIMA0_C2N
#define PA7INDEX  13 // COMP0_OUT CLK_OUT   TIMG8_C0  TIMA0_C2  TIMG8_IDX TIMG7_C1  TIMA0_C1
#define PA8INDEX  18 // UART1_TX  SPI0_CS0  UART0_RTS TIMA0_C0  TIMA1_C0N
#define PA9INDEX  19 // UART1_RX  SPI0_PICO UART0_CTS TIMA0_C1  RTC_OUT   TIMA0_C0N TIMA1_C1N CLK_OUT
#define PA10INDEX 20 // UART0_TX  SPI0_POCI I2C0_SDA  TIMA1_C0  TIMG12_C0 TIMA0_C2  I2C1_SDA  CLK_OUT
#define PA11INDEX 21 // UART0_RX  SPI0_SCK  I2C0_SCL  TIMA1_C1  COMP0_OUT TIMA0_C2N I2C1_SCL
#define PA12INDEX 33 // UART3_CTS SPI0_SCK  TIMG0_C0  CAN_TX    TIMA0_C3  FCC_IN
#define PA13INDEX 34 // UART3_RTS SPI0_POCI UART3_RX  TIMG0_C1  CAN_RX    TIMA0_C3N
#define PA14INDEX 35 // UART0_CTS SPI0_PICO UART3_TX  TIMG12_C0 CLK_OUT
#define PA15INDEX 36 // UART0_RTS SPI1_CS2  I2C1_SCL  TIMA1_C0  TIMG8_IDX TIMA1_C0N TIMA0_C2
#define PA16INDEX 37 // COMP2_OUT SPI1_POCI I2C1_SDA  TIMA1_C1  TIMA1_C1N TIMA0_C2N FCC_IN
#define PA17INDEX 38 // UART1_TX  SPI1_SCK  I2C1_SCL  TIMA0_C3  TIMG7_C0  TIMA1_C0
#define PA18INDEX 39 // UART1_RX  SPI1_PICO I2C1_SDA  TIMA0_C3N TIMG7_C1  TIMA1_C1
#define PA19INDEX 40 // SWDIO
#define PA20INDEX 41 // SWCLK
#define PA21INDEX 45 // UART2_TX  TIMG8_C0  UART1_CTS TIMA0_C0  TIMG6_C0
#define PA22INDEX 46 // UART2_RX  TIMG8_C1  UART1_RTS TIMA0_C1  CLK_OUT   TIMA0_C0N TIMG6_C1
#define PA23INDEX 52 // UART2_TX  SPI0_CS3  TIMA0_C3  TIMG0_C0  UART3_CTS TIMG7_C0  TIMG8_C0
#define PA24INDEX 53 // UART2_RX  SPI0_CS2  TIMA0_C3N TIMG0_C1  UART3_RTS TIMG7_C1  TIMA1_C1
#define PA25INDEX 54 // UART3_RX  SPI1_CS3  TIMG12_C1 TIMA0_C3  TIMA0_C1N
#define PA26INDEX 58 // UART3_TX  SPI1_CS0  TIMG8_C0  TIMA_FAL0 CAN_TX    TIMG7_C0
#define PA27INDEX 59 // RTC_OUT   SPI1_CS1  TIMG8_C1  TIMA_FAL2 CAN_RX    TIMG7_C1
#define PA28INDEX  2 // UART0_TX  I2C0_SDA  TIMA0_C3  TIMA_FAL0 TIMG7_C0  TIMA1_C0
#define PA29INDEX  3 // I2C1_SCL  UART2_RTS TIMG8_C0  TIMG6_C0
#define PA30INDEX  4 // I2C1_SDA  UART2_CTS TIMG8_C1  TIMG6_C1
#define PA31INDEX  5 // UART0_RX  I2C0_SCL  TIMA0_C3N TIMG12_C1 CLK_OUT   TIMG7_C1  TIMA1_C1
#define PB0INDEX  11 // UART0_TX  SPI1_CS2  TIMA1_C0  TIMA0_C2
#define PB1INDEX  12 // UART0_RX  SPI1_CS3  TIMA1_C1  TIMA0_C2N
#define PB2INDEX  14 // UART3_TX  UART2_CTS I2C1_SCL  TIMA0_C3  UART1_CTS TIMG6_C0  TIMA1_C0
#define PB3INDEX  15 // UART3_RX  UART2_RTS I2C1_SDA  TIMA0_C3N UART1_RTS TIMG6_C1  TIMA1_C1
#define PB4INDEX  16 // UART1_TX  UART3_CTS TIMA1_C0  TIMA0_C2  TIMA1_C0N
#define PB5INDEX  17 // UART1_RX  UART3_RTS TIMA1_C1  TIMA0_C2N TIMA1_C1N
#define PB6INDEX  22 // UART1_TX  SPI1_CS0  SPI0_CS1  TIMG8_C0  UART2_CTS TIMG6_C0  TIMA1_C0N
#define PB7INDEX  23 // UART1_RX  SPI1_POCI SPI0_CS2  TIMG8_C1  UART2_RTS TIMG6_C1  TIMA1_C1N
#define PB8INDEX  24 // UART1_CTS SPI1_PICO TIMA0_C0  COMP1_OUT
#define PB9INDEX  25 // UART1_RTS SPI1_SCK  TIMA0_C1  TIMA0_C0N
#define PB10INDEX 26 // TIMG0_C0  TIMG8_C0  COMP1_OUT TIMG6_C0
#define PB11INDEX 27 // TIMG0_C1  TIMG8_C1  CLK_OUT   TIMG6_C1
#define PB12INDEX 28 // UART3_TX  TIMA0_C2  TIMA_FAL1 TIMA0_C1
#define PB13INDEX 29 // UART3_RX  TIMA0_C3  TIMG12_C0 TIMA0_C1N
#define PB14INDEX 30 // SPI1_CS3  SPI1_POCI SPI0_CS3  TIMG12_C1 TIMG8_IDX TIMA0_C0
#define PB15INDEX 31 // UART2_TX  SPI1_PICO UART3_CTS TIMG8_C0  TIMG7_C0
#define PB16INDEX 32 // UART2_RX  SPI1_SCK  UART3_RTS TIMG8_C1  TIMG7_C1
#define PB17INDEX 42 // UART2_TX  SPI0_PICO SPI1_CS1  TIMA1_C0  TIMA0_C2
#define PB18INDEX 43 // UART2_RX  SPI0_SCK  SPI1_CS2  TIMA1_C1  TIMA0_C2N
#define PB19INDEX 44 // COMP2_OUT SPI0_POCI TIMG8_C1  UART0_CTS TIMG7_C1
#define PB20INDEX 47 // SPI0_CS2  SPI1_CS0  TIMA0_C2  TIMG12_C0 TIMA_FAL1 TIMA0_C1  TIMA1_C1N
#define PB21INDEX 48 // SPI1_POCI TIMG8_C0
#define PB22INDEX 49 // SPI1_PICO TIMG8_C1
#define PB23INDEX 50 // SPI1_SCK  COMP0_OUT TIMA_FAL0
#define PB24INDEX 51 // SPI0_CS3  SPI0_CS1  TIMA0_C3  TIMG12_C1 TIMA0_C1N TIMA1_C0N
#define PB25INDEX 55 // UART0_CTS SPI0_CS0  TIMA_FAL2
#define PB26INDEX 56 // UART0_RTS SPI0_CS1  TIMA0_C3  TIMG6_C0  TIMA1_C0
#define PB27INDEX 57 // COMP2_OUT SPI1_CS1  TIMA0_C3N TIMG6_C1  TIMA1_C1



#define GPIOA_BASE   ((GPIO_Regs *)0x400A0000)
#define GPIOB_BASE   ((GPIO_Regs *)0x400A2000)
#define IOMUX_BASE   ((IOMUX_Regs*)0x40428000)

#endif /* GPIO_H */
