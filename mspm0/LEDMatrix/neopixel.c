/*
 * Author: Ramesh Yerraballi
 * Date: Fall 2026
*/
#include "neopixel.h"

#define GPIOB_OUTSET31_0 (*(volatile uint32_t *)0x400A3290UL)
#define GPIOB_OUTCLR31_0 (*(volatile uint32_t *)0x400A32A0UL)

/* Single-NOP building blocks.  Each __asm__ volatile block is a separate
 * compiler barrier; this prevents the optimizer from merging or reordering
 * them with surrounding volatile memory accesses. */
#define NOP1()   __asm__ volatile ("nop")
#define NOP2()   NOP1(); NOP1()
#define NOP3()   NOP2(); NOP1()
#define NOP4()   NOP2(); NOP2()
#define NOP6()   NOP3(); NOP3()
#define NOP9()   NOP3(); NOP3(); NOP3()
#define NOP18()   NOP9(); NOP9()
#define NOP10()  NOP9(); NOP1()
#define NOP20()   NOP10(); NOP10()

/* -----------------------------------------------------------------------
 * send_byte — timing-critical inner loop
 * NOP counts proven on mspm0g3507 @ 32 MHz (1 cycle = 31.25 ns):
 *   T0H : 4+4 NOPs  (250ns)   T0L : 4+20 NOPs (750ns)
 *   T1H : 4+18 NOPs (687.5ns) T1L : 4+6 NOPs (312.5ns)
 * The 4+ is for clearing/setting the bit using the mask
 * LDR R4, =addr [1]
 * LDR R5, =mask [1]
 * STR R4, [R5]  [2]
 * T0H+T0L == T1H+T1L == 28NOPs = 875ns.
 * The Datasheet says this has to > 1250ns +/- 600ns, so 875ns should work. 
 Also from the Datasheet (Adafruit Datasheets gives slight;y different numbers):
 * T0H 0 code, high voltage time 220ns~380ns (Adafruit: 0.4µs ±150ns)
 * T1H 1 code, high voltage time 750ns~1.6µs (Adafruit: 0.8µs ±150ns)
 * T0L 0 code, low voltage time 750ns~1.6µs  (Adafruit: 0.85µs ±150ns)
 * T1L 1 code, low voltage time 220ns~420ns  (Adafruit: 0.45µs ±150ns)
 * RES low voltage time >300µs (Another Datasheet from Adafruit puts this as >50µs)
 * The NOP count chosen fall within the specs of the AdaFruit Datasheet
 * ----------------------------------------------------------------------- */

static void send_byte(uint32_t mask, uint8_t byte)
{
    for (int8_t bit = 7; bit >= 0; bit--) {
        if (byte & (1u << bit)) {
            GPIOB_OUTSET31_0 = mask; NOP18();
            GPIOB_OUTCLR31_0 = mask; NOP6();
        } else {
            GPIOB_OUTSET31_0 = mask; NOP4();
            GPIOB_OUTCLR31_0 = mask; NOP20();
        }
    }
}

/* -----------------------------------------------------------------------
 * LED buffer — static allocation avoids malloc on a bare-metal target.
 * 256 × sizeof(color_t) = 256 × 3 = 768 bytes.
 * ----------------------------------------------------------------------- */
static color_t leds[256];

void neopixel_init(neopixel_strip_t *strip, unsigned int pin_num,
                   unsigned int num_leds)
{
    strip->leds     = leds;
    strip->pin_num  = pin_num;
    strip->num_leds = num_leds;

    PB0Init();
    GPIOB_OUTCLR31_0 = (1UL << pin_num);   /* ensure line starts LOW */

    for (unsigned int i = 0; i < num_leds; i++) {
        strip->leds[i].simple.g = 0;
        strip->leds[i].simple.r = 0;
        strip->leds[i].simple.b = 0;
    }
}

void neopixel_clear(neopixel_strip_t *strip)
{
    for (unsigned int i = 0; i < strip->num_leds; i++) {
        strip->leds[i].simple.g = 0;
        strip->leds[i].simple.r = 0;
        strip->leds[i].simple.b = 0;
    }
    neopixel_show(strip);
}

void neopixel_show(neopixel_strip_t *strip)
{
    /* Pre-compute mask once; pass by value so send_byte keeps it in a
     * register rather than reloading it through strip->pin_num each call. */
    uint32_t mask = 1UL << strip->pin_num;

    /* Disable interrupts for the duration of the frame.  */
    unsigned int primask;
    __asm volatile ("mrs %0, primask\n\t"
                    "cpsid i\n\t"
                    : "=r"(primask) :: "memory");

    for (unsigned int i = 0; i < strip->num_leds; i++) {
        send_byte(mask, strip->leds[i].grb[0]);   /* Green  (WS2812B GRB order) */
        send_byte(mask, strip->leds[i].grb[1]);   /* Red                         */
        send_byte(mask, strip->leds[i].grb[2]);   /* Blue                        */
    }

    /* Re-enable interrupts before the reset delay */
    __asm volatile ("msr primask, %0\n\t" :: "r"(primask) : "memory");

    /* Reset pulse: hold data line LOW for 300 µs (Adafruit: >50 µs).
     * Delay(9600) ≈ 9600 cycles = 300 µs @ 32 MHz. 
     * Delay(2400) ≈ 2400 cycles = 75 µs @ 32 MHz.
     * 75 µs is what I used on the BBC Microbit */
    GPIOB_OUTCLR31_0 = mask;
    //Delay(9600);
    Delay(2400);
}

unsigned int neopixel_set_color(neopixel_strip_t *strip, unsigned int index,
                                unsigned int red, unsigned int green,
                                unsigned int blue)
{
    if (index >= strip->num_leds)
        return 1;

    strip->leds[index].simple.r = (uint8_t)red;
    strip->leds[index].simple.g = (uint8_t)green;
    strip->leds[index].simple.b = (uint8_t)blue;
    return 0;
}

unsigned int neopixel_set_color_and_show(neopixel_strip_t *strip,
                                         unsigned int index,
                                         unsigned int red, unsigned int green,
                                         unsigned int blue)
{
    if (index >= strip->num_leds)
        return 1;

    strip->leds[index].simple.r = (uint8_t)red;
    strip->leds[index].simple.g = (uint8_t)green;
    strip->leds[index].simple.b = (uint8_t)blue;
    neopixel_show(strip);
    return 0;
}

void neopixel_destroy(neopixel_strip_t *strip)
{
    strip->num_leds = 0;
    strip->pin_num  = 0;
    strip->leds     = 0;
}
