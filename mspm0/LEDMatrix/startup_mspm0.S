/* ============================================================
 * startup_mspm0.S — Startup code for TI's MSPM0G3507
 *
 * Cortex-M0 / ARMv6-M / Thumb-16 only
 *
 * Contains:
 *   1. The ARM Cortex-M0 vector table (16 core + 32 IRQs)
 *   2. Reset_Handler: copy .data from flash → RAM, zero .bss, call main()
 *   3. Default_Handler: infinite loop for unhandled exceptions/IRQs
 *   4. Weak aliases so application code can override any handler
 *
 * All external symbols (_estack, _sidata, _sdata, _edata, _sbss,
 * _ebss, main) are provided by nrf51.ld or your application.
 * ============================================================ */

    .syntax unified
    .cpu    cortex-m0
    .thumb

/* ============================================================
 * 1.  VECTOR TABLE
 *     Must be placed at the very start of flash (0x00000000).
 *     The linker script section name ".vectors" is matched by
 *     KEEP(*(.vectors)) in nrf51.ld.
 * ============================================================ */

    .section .vectors, "a", %progbits
    .align   2
    .globl   _vectors
_vectors:
    /* --- Cortex-M0 system vectors (indices 0–15) --- */
    .word   _estack                 /*  0: Initial stack pointer        */
    .word   Reset_Handler           /*  1: Reset                        */
    .word   NMI_Handler             /*  2: Non-Maskable Interrupt       */
    .word   HardFault_Handler       /*  3: Hard Fault                   */
    .word   0                       /*  4: (reserved on M0)             */
    .word   0                       /*  5: (reserved)                   */
    .word   0                       /*  6: (reserved)                   */
    .word   0                       /*  7: (reserved)                   */
    .word   0                       /*  8: (reserved)                   */
    .word   0                       /*  9: (reserved)                   */
    .word   0                       /* 10: (reserved)                   */
    .word   SVC_Handler             /* 11: Supervisor Call              */
    .word   0                       /* 12: (reserved on M0)             */
    .word   0                       /* 13: (reserved)                   */
    .word   PendSV_Handler          /* 14: Pend Supervisor Call         */
    .word   SysTick_Handler         /* 15: System Tick                  */

    /* --- nRF51822 peripheral interrupts (IRQ0–IRQ31) --- */
    .word   POWER_CLOCK_IRQHandler  /* IRQ0:  POWER / CLOCK             */
    .word   RADIO_IRQHandler        /* IRQ1:  RADIO                     */
    .word   UART0_IRQHandler        /* IRQ2:  UART0                     */
    .word   SPI0_TWI0_IRQHandler    /* IRQ3:  SPI0 / TWI0               */
    .word   SPI1_TWI1_IRQHandler    /* IRQ4:  SPI1 / TWI1               */
    .word   0                       /* IRQ5:  (reserved)                */
    .word   GPIOTE_IRQHandler       /* IRQ6:  GPIOTE                    */
    .word   ADC_IRQHandler          /* IRQ7:  ADC                       */
    .word   TIMER0_IRQHandler       /* IRQ8:  TIMER0                    */
    .word   TIMER1_IRQHandler       /* IRQ9:  TIMER1                    */
    .word   TIMER2_IRQHandler       /* IRQ10: TIMER2                    */
    .word   RTC0_IRQHandler         /* IRQ11: RTC0                      */
    .word   TEMP_IRQHandler         /* IRQ12: TEMP (temperature sensor) */
    .word   RNG_IRQHandler          /* IRQ13: RNG                       */
    .word   ECB_IRQHandler          /* IRQ14: ECB (AES encryption)      */
    .word   CCM_AAR_IRQHandler      /* IRQ15: CCM / AAR                 */
    .word   WDT_IRQHandler          /* IRQ16: WDT (watchdog)            */
    .word   RTC1_IRQHandler         /* IRQ17: RTC1                      */
    .word   QDEC_IRQHandler         /* IRQ18: QDEC (quadrature decoder) */
    .word   LPCOMP_IRQHandler       /* IRQ19: LPCOMP                    */
    .word   SWI0_IRQHandler         /* IRQ20: SWI0 (software interrupt) */
    .word   SWI1_IRQHandler         /* IRQ21: SWI1                      */
    .word   SWI2_IRQHandler         /* IRQ22: SWI2                      */
    .word   SWI3_IRQHandler         /* IRQ23: SWI3                      */
    .word   SWI4_IRQHandler         /* IRQ24: SWI4                      */
    .word   SWI5_IRQHandler         /* IRQ25: SWI5                      */
    .word   0                       /* IRQ26: (reserved)                */
    .word   0                       /* IRQ27: (reserved)                */
    .word   0                       /* IRQ28: (reserved)                */
    .word   0                       /* IRQ29: (reserved)                */
    .word   0                       /* IRQ30: (reserved)                */
    .word   0                       /* IRQ31: (reserved)                */

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

    /* Cortex-M0 core exceptions */
    .weak   NMI_Handler
    .thumb_set NMI_Handler, Default_Handler

    .weak   HardFault_Handler
    .thumb_set HardFault_Handler, Default_Handler

    .weak   SVC_Handler
    .thumb_set SVC_Handler, Default_Handler

    .weak   PendSV_Handler
    .thumb_set PendSV_Handler, Default_Handler

    .weak   SysTick_Handler
    .thumb_set SysTick_Handler, Default_Handler

    /* nRF51822 peripheral IRQs */
    .weak   POWER_CLOCK_IRQHandler
    .thumb_set POWER_CLOCK_IRQHandler, Default_Handler

    .weak   RADIO_IRQHandler
    .thumb_set RADIO_IRQHandler, Default_Handler

    .weak   UART0_IRQHandler
    .thumb_set UART0_IRQHandler, Default_Handler

    .weak   SPI0_TWI0_IRQHandler
    .thumb_set SPI0_TWI0_IRQHandler, Default_Handler

    .weak   SPI1_TWI1_IRQHandler
    .thumb_set SPI1_TWI1_IRQHandler, Default_Handler

    .weak   GPIOTE_IRQHandler
    .thumb_set GPIOTE_IRQHandler, Default_Handler

    .weak   ADC_IRQHandler
    .thumb_set ADC_IRQHandler, Default_Handler

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

    .weak   LPCOMP_IRQHandler
    .thumb_set LPCOMP_IRQHandler, Default_Handler

    .weak   SWI0_IRQHandler
    .thumb_set SWI0_IRQHandler, Default_Handler

    .weak   SWI1_IRQHandler
    .thumb_set SWI1_IRQHandler, Default_Handler

    .weak   SWI2_IRQHandler
    .thumb_set SWI2_IRQHandler, Default_Handler

    .weak   SWI3_IRQHandler
    .thumb_set SWI3_IRQHandler, Default_Handler

    .weak   SWI4_IRQHandler
    .thumb_set SWI4_IRQHandler, Default_Handler

    .weak   SWI5_IRQHandler
    .thumb_set SWI5_IRQHandler, Default_Handler
