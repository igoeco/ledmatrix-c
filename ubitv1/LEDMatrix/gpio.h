#ifndef GPIO_H
#define GPIO_H

/* FIX: added include guard (was missing, would cause duplicate-definition
        errors if gpio.h was included more than once in the same translation unit) */

#define GPIO   ((GPIO_REGS *)0x50000000)
#define CLOCK  ((CLOCK_REGS *)0x40000000)

void Delay(int count);
void gpio_cfg_output(unsigned int pin_num);

/* -----------------------------------------------------------------------
 * nRF51822 GPIO register map (Port 0, base 0x50000000)
 *
 * Offsets verified against nRF51822 Product Specification v3.3, §13.
 * ----------------------------------------------------------------------- */
typedef struct {
    volatile unsigned int aDummy0[321]; /* pad to 0x504 (321 × 4 = 1284 = 0x504) */
    volatile unsigned int OUT;          /* 0x504  Write GPIO port                 */
    volatile unsigned int OUTSET;       /* 0x508  Set individual bits              */
    volatile unsigned int OUTCLR;       /* 0x50C  Clear individual bits            */
    volatile unsigned int IN;           /* 0x510  Read GPIO port                   */
    volatile unsigned int DIR;          /* 0x514  Direction of GPIO pins           */
    volatile unsigned int DIRSET;       /* 0x518  DIR set register                 */
    volatile unsigned int DIRCLR;       /* 0x51C  DIR clear register               */
    volatile unsigned int aDummy1[120]; /* pad to 0x700 (0x520 + 120×4 = 0x700)  */
    /* FIX: was PIN_CNF[31] — nRF51822 has 32 GPIO pins (P0.00–P0.31) */
    volatile unsigned int PIN_CNF[32];  /* 0x700–0x77C  Pin configuration          */
} GPIO_REGS;

/* -----------------------------------------------------------------------
 * nRF51822 CLOCK register map (base 0x40000000)
 *
 * Offsets verified against nRF51822 Product Specification v3.3, §14.
 * ----------------------------------------------------------------------- */
typedef struct {
    volatile unsigned int HFCLKSTART;   /* 0x000  Task: start HFCLK crystal osc   */
    volatile unsigned int HFCLKSTOP;    /* 0x004  Task: stop  HFCLK crystal osc   */
    volatile unsigned int LFCLKSTART;   /* 0x008  Task: start LFCLK source        */
    volatile unsigned int LFCLKSTOP;    /* 0x00C  Task: stop  LFCLK source        */
    volatile unsigned int CAL;          /* 0x010  Task: calibrate LFCLK RC osc    */
    volatile unsigned int CTSTART;      /* 0x014  Task: start calibration timer   */
    volatile unsigned int CTSTOP;       /* 0x018  Task: stop  calibration timer   */
    volatile unsigned int aDummy[57];   /* pad to 0x100 (0x01C + 57×4 = 0x100)   */
    volatile unsigned int HFCLKSTARTED; /* 0x100  Event: 16 MHz oscillator started */
    volatile unsigned int LFCLKSTARTED; /* 0x104  Event: 32 kHz oscillator started */
    volatile unsigned int Dummy1;
    volatile unsigned int DONE;         /* 0x10C  Event: LFCLK RC calibration done */
    volatile unsigned int CTTO;         /* 0x110  Event: calibration timer timeout */
} CLOCK_REGS;

#endif /* GPIO_H */
