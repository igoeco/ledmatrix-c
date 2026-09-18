#include "neopixel.h"

/* -----------------------------------------------------------------------
 * Direct fixed-address GPIO access — same technique as the proven working
 * implementation.  Using struct-pointer access (GPIO->OUTSET) causes the
 * compiler to emit extra load instructions between the NOP delay and the
 * OUTCLR write, which invisibly extends T0H beyond this strip's '0'/'1'
 * threshold and causes every bit to be read as '1' (all-white output).
 * ----------------------------------------------------------------------- */
#define NRF_GPIO_OUTSET (*(volatile uint32_t *)0x50000508UL)
#define NRF_GPIO_OUTCLR (*(volatile uint32_t *)0x5000050CUL)

/* Single-NOP building blocks.  Each __asm__ volatile block is a separate
 * compiler barrier; this prevents the optimizer from merging or reordering
 * them with surrounding volatile memory accesses. */
#define NOP1()   __asm__ volatile ("nop")
#define NOP2()   NOP1(); NOP1()
#define NOP3()   NOP2(); NOP1()
#define NOP9()   NOP3(); NOP3(); NOP3()
#define NOP10()  NOP9(); NOP1()

/* -----------------------------------------------------------------------
 * send_byte — timing-critical inner loop
 *
 * Two attributes are essential:
 *
 *   __attribute__((noinline))
 *     Keeps this in its own function so the compiler optimises it in
 *     isolation.  With 256×3 = 768 calls the surrounding loop in
 *     neopixel_show would otherwise suffer extreme register pressure,
 *     causing spills between OUTSET and OUTCLR.
 *
 *   __attribute__((optimize("O2")))
 *     Forces -O2 code generation for this function even when the rest
 *     of the translation unit is compiled at -O0 (debug build).
 *     At -O0, the compiler stores every variable to the stack and
 *     reloads it for each use.  The reloads between the T0H NOPs and
 *     the OUTCLR write add 4-6 cycles ≈ 250–375 ns, pushing T0H above
 *     the strip's threshold and making every bit read as '1'.
 *     With -O2, mask and both addresses stay in registers; the only
 *     code between OUTSET and OUTCLR is the requested NOPs.
 *
 * NOP counts proven on nRF51822 @ 16 MHz (1 cycle = 62.5 ns):
 *   T0H : 2 NOPs   T0L : 10 NOPs
 *   T1H : 9 NOPs   T1L :  3 NOPs
 * The AHB-APB bridge adds latency, so effective durations are longer
 * than raw NOP counts suggest; these values were calibrated empirically.
 * ----------------------------------------------------------------------- */
__attribute__((noinline, optimize("O2")))
static void send_byte(uint32_t mask, uint8_t byte)
{
    for (int8_t bit = 7; bit >= 0; bit--) {
        if (byte & (1u << bit)) {
            NRF_GPIO_OUTSET = mask; NOP9();
            NRF_GPIO_OUTCLR = mask; NOP3();
        } else {
            NRF_GPIO_OUTSET = mask; NOP2();
            NRF_GPIO_OUTCLR = mask; NOP10();
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

    gpio_cfg_output(pin_num);
    NRF_GPIO_OUTCLR = (1UL << pin_num);   /* ensure line starts LOW */

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

    /* Disable interrupts for the duration of the frame.  Any interrupt
     * that fires mid-frame can stretch a LOW period past 50 µs, which the
     * WS2812B interprets as a reset and discards the remaining data.
     * PRIMASK is saved and restored so the caller's masking state is
     * preserved and any already-pending IRQ is not lost. */
    unsigned int primask;
    __asm volatile ("mrs %0, primask\n\t"
                    "cpsid i\n\t"
                    : "=r"(primask) :: "memory");

    for (unsigned int i = 0; i < strip->num_leds; i++) {
        send_byte(mask, strip->leds[i].grb[0]);   /* Green  (WS2812B GRB order) */
        send_byte(mask, strip->leds[i].grb[1]);   /* Red                         */
        send_byte(mask, strip->leds[i].grb[2]);   /* Blue                        */
    }

    /* Re-enable interrupts before the reset delay; the reset only requires
     * the line to stay LOW, which is timing-tolerant. */
    __asm volatile ("msr primask, %0\n\t" :: "r"(primask) : "memory");

    /* Reset pulse: hold data line LOW for > 50 µs.
     * Delay(300) ≈ 300 × 4 cycles = 1200 cycles = 75 µs @ 16 MHz. */
    NRF_GPIO_OUTCLR = mask;
    Delay(300);
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
