/*
 * Author: Ramesh Yerraballi
 * Date: Fall 2026
*/
#ifndef NEOPIXEL_H
#define NEOPIXEL_H

#include <stdint.h>
#include "gpio.h"

/* -----------------------------------------------------------------------
 * color_t — one LED's colour value
 *
 * Fields were "unsigned int" (4 bytes each = 12 bytes per colour).
 * Changed to uint8_t (1 byte each = 3 bytes per colour).
 * With unsigned int, a 255-LED strip consumed 3 KB instead of 765 B.
 * The union layout maps the struct fields directly onto grb[]:
 * grb[0] == g,  grb[1] == r,  grb[2] == b  (GRB order ✓)
 * ----------------------------------------------------------------------- */
typedef union {
    struct {
        uint8_t g;
        uint8_t r;
        uint8_t b;
    } simple;
    uint8_t grb[3];   /* WS2812B wire order: G first, then R, then B */
} color_t;

/* -----------------------------------------------------------------------
 * neopixel_strip_t — strip descriptor
 * ----------------------------------------------------------------------- */
typedef struct {
    unsigned int  pin_num;
    unsigned int  num_leds;
    color_t      *leds;
} neopixel_strip_t;

/* -----------------------------------------------------------------------
 * API
 * ----------------------------------------------------------------------- */

/** Initialise GPIO and LED buffer. */
void neopixel_init(neopixel_strip_t *strip, unsigned int pin_num,
                   unsigned int num_leds);

/** Turn all LEDs off and push to strip. */
void neopixel_clear(neopixel_strip_t *strip);

/** Transmit frame buffer to strip. */
void neopixel_show(neopixel_strip_t *strip);

/**
 * Write RGB value to LED buffer (does not update strip until neopixel_show).
 * @return 0 on success, 1 if index >= num_leds.
 */
unsigned int neopixel_set_color(neopixel_strip_t *strip, unsigned int index,
                                unsigned int red, unsigned int green,
                                unsigned int blue);

/**
 * Write RGB value to LED buffer and immediately update strip.
 * @return 0 on success, 1 if index >= num_leds.
 */
unsigned int neopixel_set_color_and_show(neopixel_strip_t *strip,
                                         unsigned int index,
                                         unsigned int red, unsigned int green,
                                         unsigned int blue);

/** Free strip resources (zeros descriptor; leds pointer becomes invalid). */
void neopixel_destroy(neopixel_strip_t *strip);

#endif /* NEOPIXEL_H */
