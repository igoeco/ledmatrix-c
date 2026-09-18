#include "neopixel.h"

/* -----------------------------------------------------------------------
 * Direct fixed-address GPIO access — same technique as the proven working
 * implementation.  Using struct-pointer access (GPIO->OUTSET) causes the
 * compiler to emit extra load instructions between the NOP delay and the
 * OUTCLR write, which invisibly extends T0H beyond this strip's '0'/'1'
 * threshold and causes every bit to be read as '1' (all-white output).
 *
 * These addresses are unchanged on the nRF52833: P0's GPIO peripheral is
 * still at 0x50000000 (OUTSET/OUTCLR at the same +0x508/+0x50C offsets),
 * so this only needs to move if your strip's data pin is one of the
 * micro:bit v2 pins wired to P1 (P1.00-P1.15) instead of P0 — in that
 * case use 0x50000300/0x50000304 (GPIO1 in gpio.h) instead.
 * ----------------------------------------------------------------------- */
#define NRF_GPIO_OUTSET (*(volatile uint32_t *)0x50000508UL)
#define NRF_GPIO_OUTCLR (*(volatile uint32_t *)0x5000050CUL)

/* nRF52833 instruction cache (NVMC.ICACHECNF, base 0x4001E000 + 0x540).
 * Bare-metal builds without a SoftDevice do NOT enable this by default.
 * At 64 MHz a flash read that misses the cache costs extra wait states
 * (the nRF51822 @ 16 MHz didn't need this — flash kept up with the core
 * clock with no wait states). Without the cache enabled, the very first
 * pass through send_byte's two branches (or any code layout change) can
 * silently stretch a NOP delay by one or more flash-wait cycles and blow
 * the WS2812 timing budget. Enabling it makes flash-resident code run at
 * a deterministic 0-wait-state, 1-cycle-per-instruction rate once warm,
 * same as the M0 always did. */
#define NRF_NVMC_ICACHECNF (*(volatile uint32_t *)0x4001E540UL)

/* Single-NOP building blocks.  Each __asm__ volatile block is a separate
 * compiler barrier; this prevents the optimizer from merging or reordering
 * them with surrounding volatile memory accesses. */
#define NOP1()   __asm__ volatile ("nop")
#define NOP2()   NOP1(); NOP1()
#define NOP4()   NOP2(); NOP2()
#define NOP8()   NOP4(); NOP4()
#define NOP16()  NOP8(); NOP8()
#define NOP24()  NOP16(); NOP8()
#define NOP27()  NOP16(); NOP8(); NOP2(); NOP1()
#define NOP48()  NOP24(); NOP24()
#define NOP49()  NOP48(); NOP1()
#define NOP52()  NOP48(); NOP4()

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
 * NOP counts recalculated for nRF52833 @ 64 MHz (1 cycle = 15.625 ns),
 * targeting the same WS2812B timing this strip was tuned against on the
 * nRF51822 (T0H~400ns/T0L~850ns, T1H~800ns/T1L~450ns -> 1250ns/bit,
 * i.e. 800 kHz):
 *   T0H : 24 NOPs (~375ns + overhead)   T0L : 52 NOPs (~813ns + overhead)
 *   T1H : 49 NOPs (~766ns + overhead)   T1L : 27 NOPs (~422ns + overhead)
 * The nRF52833's GPIO is on a fast bus (unlike the nRF51822's APB, which
 * added ~4 cycles of synchronization latency per OUTSET/OUTCLR write —
 * that's most of what the old 2/10/9/3 NOP counts were compensating
 * for). These counts assume only ~2 cycles of write/branch overhead
 * instead, which is a reasonable estimate but NOT hardware-verified —
 * this sandbox has no way to probe a real strip with a scope or logic
 * analyzer. Treat these as a starting point: flash them, check the
 * colors are correct (not all-white/all-off), and if not, nudge the
 * NOP counts up or down the same way the original nRF51822 values were
 * tuned.
 * ----------------------------------------------------------------------- */
__attribute__((noinline, optimize("O2")))
static void send_byte(uint32_t mask, uint8_t byte)
{
    for (int8_t bit = 7; bit >= 0; bit--) {
        if (byte & (1u << bit)) {
            NRF_GPIO_OUTSET = mask; NOP49();
            NRF_GPIO_OUTCLR = mask; NOP27();
        } else {
            NRF_GPIO_OUTSET = mask; NOP24();
            NRF_GPIO_OUTCLR = mask; NOP52();
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

    /* Enable flash I-cache so send_byte's NOP timing is deterministic
     * (0-wait-state fetch) at 64 MHz -- see comment above NRF_NVMC_ICACHECNF. */
    NRF_NVMC_ICACHECNF = 1;

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
     * Delay(count) is ~4 cycles/iteration (see gpio.c) regardless of CPU
     * clock, so the same *count* now buys fewer microseconds at 64 MHz
     * than it did at 16 MHz: Delay(300) would only be ~18.75 µs here,
     * under the WS2812 reset threshold. Scaled by 4x to match:
     * Delay(1200) ≈ 1200 × 4 cycles = 4800 cycles = 75 µs @ 64 MHz. */
    NRF_GPIO_OUTCLR = mask;
    Delay(1200);
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
