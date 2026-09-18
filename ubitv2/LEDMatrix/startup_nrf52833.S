/* ============================================================
 * startup_nrf52833.S — Startup code for nRF52833 (BBC micro:bit v2)
 *
 * Cortex-M4 / ARMv7E-M, Thumb/Thumb-2, optional FPU (unused here —
 * built for soft-float; see compile notes at the bottom of this file)
 *
 * Contains:
 *   1. The ARM Cortex-M4 vector table (16 core + 48 nRF52833 IRQs)
 *   2. Reset_Handler: copy .data from flash → RAM, zero .bss, call main()
 *   3. Default_Handler: infinite loop for unhandled exceptions/IRQs
 *   4. Weak aliases so application code can override any handler
 *
 * All external symbols (_estack, _sidata, _sdata, _edata, _sbss,
 * _ebss, main) are provided by nrf52833.ld or your application.
 *
 * Differences from the nRF51822 (micro:bit v1) version:
 *   - Cortex-M0 → Cortex-M4: adds MemManage/BusFault/UsageFault/
 *     DebugMon core exceptions (slots 4,5,6,12 are no longer reserved)
 *   - 32 peripheral IRQs → 48 (IRQ0–IRQ47), several new peripherals
 *     (NFCT, SAADC, TIMER3/4, PWM0-3, MWU, SPIM2/3, RTC2, I2S, FPU, USBD,
 *     UARTE1), some renamed (e.g. UART0 → UARTE0_UART0), some slots
 *     genuinely unused on this part (IRQ30,31,41-44,46)
 * ============================================================ */

    .syntax unified
    .cpu    cortex-m4
    .thumb

/* ============================================================
 * 1.  VECTOR TABLE
 *     Must be placed at the very start of flash (0x00000000).
 *     The linker script section name ".vectors" is matched by
 *     KEEP(*(.vectors)) in nrf52833.ld.
 * ============================================================ */

    .section .vectors, "a", %progbits
    .align   2
    .globl   _vectors
_vectors:
    /* --- Cortex-M4 system vectors (indices 0–15) --- */
    .word   _estack                 /*  0: Initial stack pointer        */
    .word   Reset_Handler           /*  1: Reset                        */
    .word   NMI_Handler             /*  2: Non-Maskable Interrupt       */
    .word   HardFault_Handler       /*  3: Hard Fault                   */
    .word   MemManage_Handler       /*  4: Memory Management Fault      */
    .word   BusFault_Handler        /*  5: Bus Fault                    */
    .word   UsageFault_Handler      /*  6: Usage Fault                  */
    .word   0                       /*  7: (reserved)                   */
    .word   0                       /*  8: (reserved)                   */
    .word   0                       /*  9: (reserved)                   */
    .word   0                       /* 10: (reserved)                   */
    .word   SVC_Handler             /* 11: Supervisor Call              */
    .word   DebugMon_Handler        /* 12: Debug Monitor                */
    .word   0                       /* 13: (reserved)                   */
    .word   PendSV_Handler          /* 14: Pend Supervisor Call         */
    .word   SysTick_Handler         /* 15: System Tick                  */

    /* --- nRF52833 peripheral interrupts (IRQ0–IRQ47) --- */
    .word   POWER_CLOCK_IRQHandler                          /* IRQ0  */
    .word   RADIO_IRQHandler                                /* IRQ1  */
    .word   UARTE0_UART0_IRQHandler                         /* IRQ2  */
    .word   SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler    /* IRQ3  */
    .word   SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQHandler    /* IRQ4  */
    .word   NFCT_IRQHandler                                 /* IRQ5  */
    .word   GPIOTE_IRQHandler                               /* IRQ6  */
    .word   SAADC_IRQHandler                                /* IRQ7  */
    .word   TIMER0_IRQHandler                               /* IRQ8  */
    .word   TIMER1_IRQHandler                               /* IRQ9  */
    .word   TIMER2_IRQHandler                               /* IRQ10 */
    .word   RTC0_IRQHandler                                 /* IRQ11 */
    .word   TEMP_IRQHandler                                 /* IRQ12 */
    .word   RNG_IRQHandler                                  /* IRQ13 */
    .word   ECB_IRQHandler                                  /* IRQ14 */
    .word   CCM_AAR_IRQHandler                               /* IRQ15 */
    .word   WDT_IRQHandler                                  /* IRQ16 */
    .word   RTC1_IRQHandler                                 /* IRQ17 */
    .word   QDEC_IRQHandler                                 /* IRQ18 */
    .word   COMP_LPCOMP_IRQHandler                          /* IRQ19 */
    .word   SWI0_EGU0_IRQHandler                            /* IRQ20 */
    .word   SWI1_EGU1_IRQHandler                            /* IRQ21 */
    .word   SWI2_EGU2_IRQHandler                            /* IRQ22 */
    .word   SWI3_EGU3_IRQHandler                            /* IRQ23 */
    .word   SWI4_EGU4_IRQHandler                            /* IRQ24 */
    .word   SWI5_EGU5_IRQHandler                            /* IRQ25 */
    .word   TIMER3_IRQHandler                               /* IRQ26 */
    .word   TIMER4_IRQHandler                               /* IRQ27 */
    .word   PWM0_IRQHandler                                 /* IRQ28 */
    .word   PDM_IRQHandler                                  /* IRQ29 */
    .word   0                                               /* IRQ30: (reserved) */
    .word   0                                               /* IRQ31: (reserved) */
    .word   MWU_IRQHandler                                  /* IRQ32 */
    .word   PWM1_IRQHandler                                 /* IRQ33 */
    .word   PWM2_IRQHandler                                 /* IRQ34 */
    .word   SPIM2_SPIS2_SPI2_IRQHandler                     /* IRQ35 */
    .word   RTC2_IRQHandler                                 /* IRQ36 */
    .word   I2S_IRQHandler                                  /* IRQ37 */
    .word   FPU_IRQHandler                                  /* IRQ38 */
    .word   USBD_IRQHandler                                 /* IRQ39 */
    .word   UARTE1_IRQHandler                               /* IRQ40 */
    .word   0                                               /* IRQ41: (reserved) */
    .word   0                                               /* IRQ42: (reserved) */
    .word   0                                               /* IRQ43: (reserved) */
    .word   0                                               /* IRQ44: (reserved) */
    .word   PWM3_IRQHandler                                 /* IRQ45 */
    .word   0                                               /* IRQ46: (reserved) */
    .word   SPIM3_IRQHandler                                /* IRQ47 */

/* ============================================================
 * 2.  RESET HANDLER
 *     Entry point after every reset.  ARM hardware loads PC
 *     from vector table[1] and SP from vector table[0].
 *     We must:
 *       a) Copy initialised data from flash (.data LMA) to RAM (.data VMA)
 *       b) Zero-fill .bss
 *       c) Call main()
 *       d) Loop forever if main() returns
 * ============================================================ */

    .section .text.Reset_Handler, "ax", %progbits
    .align   2
    .globl   Reset_Handler
    .type    Reset_Handler, %function

Reset_Handler:

    /* --- a) Copy .data from flash to RAM --- */
    ldr     r0, =_sidata        /* r0 = flash load address (source)     */
    ldr     r1, =_sdata         /* r1 = RAM VMA start (destination)     */
    ldr     r2, =_edata         /* r2 = RAM VMA end                     */
    b       .Ldata_test
.Ldata_copy:
    ldr     r3, [r0]            /* load word from flash                 */
    str     r3, [r1]            /* store to RAM                         */
    adds    r0, r0, #4          /* advance source                       */
    adds    r1, r1, #4          /* advance destination                  */
.Ldata_test:
    cmp     r1, r2              /* finished?                            */
    bcc     .Ldata_copy         /* branch if r1 < r2                    */

    /* --- b) Zero .bss --- */
    ldr     r0, =_sbss          /* r0 = bss start                       */
    ldr     r1, =_ebss          /* r1 = bss end                         */
    movs    r2, #0              /* r2 = 0                               */
    b       .Lbss_test
.Lbss_zero:
    str     r2, [r0]            /* write zero                           */
    adds    r0, r0, #4          /* advance pointer                      */
.Lbss_test:
    cmp     r0, r1              /* finished?                            */
    bcc     .Lbss_zero          /* branch if r0 < r1                    */

    /* --- c) Call main --- */
    bl      main

    /* --- d) main() returned — hang --- */
.Lhang:
    b       .Lhang

    .size   Reset_Handler, . - Reset_Handler

/* ============================================================
 * 3.  DEFAULT HANDLER
 *     Weak, overridable infinite loop for any unhandled exception.
 *     To add a real handler in C:
 *         void HardFault_Handler(void) { ... }
 *     That symbol overrides the weak alias below.
 * ============================================================ */

    .section .text.Default_Handler, "ax", %progbits
    .align   2
    .globl   Default_Handler
    .type    Default_Handler, %function

Default_Handler:
    b       Default_Handler

    .size   Default_Handler, . - Default_Handler

/* ============================================================
 * 4.  WEAK ALIASES
 *     Each handler defaults to Default_Handler unless the
 *     application defines its own symbol with the same name.
 * ============================================================ */

    /* Cortex-M4 core exceptions */
    .weak   NMI_Handler
    .thumb_set NMI_Handler, Default_Handler

    .weak   HardFault_Handler
    .thumb_set HardFault_Handler, Default_Handler

    .weak   MemManage_Handler
    .thumb_set MemManage_Handler, Default_Handler

    .weak   BusFault_Handler
    .thumb_set BusFault_Handler, Default_Handler

    .weak   UsageFault_Handler
    .thumb_set UsageFault_Handler, Default_Handler

    .weak   SVC_Handler
    .thumb_set SVC_Handler, Default_Handler

    .weak   DebugMon_Handler
    .thumb_set DebugMon_Handler, Default_Handler

    .weak   PendSV_Handler
    .thumb_set PendSV_Handler, Default_Handler

    .weak   SysTick_Handler
    .thumb_set SysTick_Handler, Default_Handler

    /* nRF52833 peripheral IRQs */
    .weak   POWER_CLOCK_IRQHandler
    .thumb_set POWER_CLOCK_IRQHandler, Default_Handler

    .weak   RADIO_IRQHandler
    .thumb_set RADIO_IRQHandler, Default_Handler

    .weak   UARTE0_UART0_IRQHandler
    .thumb_set UARTE0_UART0_IRQHandler, Default_Handler

    .weak   SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler
    .thumb_set SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler, Default_Handler

    .weak   SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQHandler
    .thumb_set SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQHandler, Default_Handler

    .weak   NFCT_IRQHandler
    .thumb_set NFCT_IRQHandler, Default_Handler

    .weak   GPIOTE_IRQHandler
    .thumb_set GPIOTE_IRQHandler, Default_Handler

    .weak   SAADC_IRQHandler
    .thumb_set SAADC_IRQHandler, Default_Handler

    .weak   TIMER0_IRQHandler
    .thumb_set TIMER0_IRQHandler, Default_Handler

    .weak   TIMER1_IRQHandler
    .thumb_set TIMER1_IRQHandler, Default_Handler

    .weak   TIMER2_IRQHandler
    .thumb_set TIMER2_IRQHandler, Default_Handler

    .weak   RTC0_IRQHandler
    .thumb_set RTC0_IRQHandler, Default_Handler

    .weak   TEMP_IRQHandler
    .thumb_set TEMP_IRQHandler, Default_Handler

    .weak   RNG_IRQHandler
    .thumb_set RNG_IRQHandler, Default_Handler

    .weak   ECB_IRQHandler
    .thumb_set ECB_IRQHandler, Default_Handler

    .weak   CCM_AAR_IRQHandler
    .thumb_set CCM_AAR_IRQHandler, Default_Handler

    .weak   WDT_IRQHandler
    .thumb_set WDT_IRQHandler, Default_Handler

    .weak   RTC1_IRQHandler
    .thumb_set RTC1_IRQHandler, Default_Handler

    .weak   QDEC_IRQHandler
    .thumb_set QDEC_IRQHandler, Default_Handler

    .weak   COMP_LPCOMP_IRQHandler
    .thumb_set COMP_LPCOMP_IRQHandler, Default_Handler

    .weak   SWI0_EGU0_IRQHandler
    .thumb_set SWI0_EGU0_IRQHandler, Default_Handler

    .weak   SWI1_EGU1_IRQHandler
    .thumb_set SWI1_EGU1_IRQHandler, Default_Handler

    .weak   SWI2_EGU2_IRQHandler
    .thumb_set SWI2_EGU2_IRQHandler, Default_Handler

    .weak   SWI3_EGU3_IRQHandler
    .thumb_set SWI3_EGU3_IRQHandler, Default_Handler

    .weak   SWI4_EGU4_IRQHandler
    .thumb_set SWI4_EGU4_IRQHandler, Default_Handler

    .weak   SWI5_EGU5_IRQHandler
    .thumb_set SWI5_EGU5_IRQHandler, Default_Handler

    .weak   TIMER3_IRQHandler
    .thumb_set TIMER3_IRQHandler, Default_Handler

    .weak   TIMER4_IRQHandler
    .thumb_set TIMER4_IRQHandler, Default_Handler

    .weak   PWM0_IRQHandler
    .thumb_set PWM0_IRQHandler, Default_Handler

    .weak   PDM_IRQHandler
    .thumb_set PDM_IRQHandler, Default_Handler

    .weak   MWU_IRQHandler
    .thumb_set MWU_IRQHandler, Default_Handler

    .weak   PWM1_IRQHandler
    .thumb_set PWM1_IRQHandler, Default_Handler

    .weak   PWM2_IRQHandler
    .thumb_set PWM2_IRQHandler, Default_Handler

    .weak   SPIM2_SPIS2_SPI2_IRQHandler
    .thumb_set SPIM2_SPIS2_SPI2_IRQHandler, Default_Handler

    .weak   RTC2_IRQHandler
    .thumb_set RTC2_IRQHandler, Default_Handler

    .weak   I2S_IRQHandler
    .thumb_set I2S_IRQHandler, Default_Handler

    .weak   FPU_IRQHandler
    .thumb_set FPU_IRQHandler, Default_Handler

    .weak   USBD_IRQHandler
    .thumb_set USBD_IRQHandler, Default_Handler

    .weak   UARTE1_IRQHandler
    .thumb_set UARTE1_IRQHandler, Default_Handler

    .weak   PWM3_IRQHandler
    .thumb_set PWM3_IRQHandler, Default_Handler

    .weak   SPIM3_IRQHandler
    .thumb_set SPIM3_IRQHandler, Default_Handler

/* ============================================================
 * NOTE ON FPU:
 *   The nRF52833 is a Cortex-M4F (has a single-precision FPU), but
 *   this startup file does not enable it (no CPACR write). That's
 *   correct if you compile with -mfloat-abi=soft (no VFP registers
 *   touched, matches the drop-in nRF51/Cortex-M0 build style).
 *   If you switch to -mfloat-abi=hard -mfpu=fpv4-sp-d16, you must
 *   add code at the top of Reset_Handler to set CPACR (0xE000ED88)
 *   bits 20-23 before any floating-point instruction executes.
 * ============================================================ */
