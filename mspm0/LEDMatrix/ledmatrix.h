#ifndef LEDMATRIX_H
#define LEDMATRIX_H

#include <stdint.h>
#include "neopixel.h"

/* -----------------------------------------------------------------------
 * 16×16 WS2812B LED Matrix library
 * C translation of ledmatrix.py (microbit-module: ledmatrix@1.0.0)
 *
 * Assumed wiring: serpentine raster, GPIO 3 = micro:bit edge pad P0,
 * 256 LEDs total.
 *
 * Call order:
 *   neopixel_init(&strip, 3, 256);
 *   ledmatrix_init(&strip);
 *   ... draw calls ...
 *   ledmatrix_show();
 * ----------------------------------------------------------------------- */

#define MATRIX_WIDTH   16
#define MATRIX_HEIGHT  16
#define MATRIX_SIZE    (MATRIX_WIDTH * MATRIX_HEIGHT)   /* 256 */

/* Approximate millisecond delay built on gpio.c's Delay().
 * Delay() runs ~4–7 cycles/iteration at 16 MHz; 4000 gives ~1 ms. */
#define Delay_ms(n)    Delay((int)((n) * 4000))

/* -----------------------------------------------------------------------
 * 5×8 bitmap font
 * 5 bytes per glyph, column-major (byte 0 = leftmost column).
 * Within each byte: bit 0 = first drawn row (j=0), bit 7 = last (j=7).
 * Covers printable ASCII 0x20–0x7E (95 glyphs) plus 4 built-in glyphs:
 *   glyph index 95 (\b = 0x08): smile
 *   glyph index 96 (\t = 0x09): heart
 *   glyph index 97 (\n = 0x0A): eyes
 *   glyph index 98 (\v = 0x0B): eyes
 * ----------------------------------------------------------------------- */
extern const uint8_t LedMatrix_Font[];

/* -----------------------------------------------------------------------
 * Initialisation
 * ----------------------------------------------------------------------- */

/* Bind the module to an already-configured neopixel strip. */
void ledmatrix_init(neopixel_strip_t *strip);

/* Seed the internal RNG used by animated effects (optional; default seed = 1). */
void ledmatrix_srand(unsigned int seed);

/* Push the current frame buffer to the physical strip. */
void ledmatrix_show(void);

/* -----------------------------------------------------------------------
 * Coordinate mapping
 * ----------------------------------------------------------------------- */

/* Map matrix (x, y) → strip LED index for a 16×16 serpentine layout.
 * Even columns count y upward; odd columns count y downward. */
int spos(int x, int y);

/* -----------------------------------------------------------------------
 * Drawing primitives
 * ----------------------------------------------------------------------- */

/* Set one pixel.  Call ledmatrix_show() to push to the strip. */
void DrawPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);

/* Vertical line of length `len` starting at (x, y), going in +y direction. */
void DrawVLine(int x, int y, int len, uint8_t r, uint8_t g, uint8_t b);

/* Horizontal line of length `len` starting at (x, y), going in +x direction. */
void DrawHLine(int x, int y, int len, uint8_t r, uint8_t g, uint8_t b);

/* Filled w×h rectangle with top-left corner at (x, y). */
void DrawRectangle(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b);

/* Unfilled w×h rectangle outline. */
void DrawUnfilledRectangle(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b);

/* Draw a 16-bit RGB555 image.
 * Pixel format: R[4:0] | G[9:5] | B[14:10]  (same as MicroPython source).
 * image must contain at least w×h entries; the image stride is always 16. */
void DrawBitmap(const uint16_t *image, int x, int y, int w, int h);

/* Draw a 5×8 character glyph at (x, y) in the given colour.
 * ch: printable ASCII, or \b \t \n \v for built-in glyphs (drawn in
 * random colour; the r/g/b arguments are ignored for those). */
void DrawChar(int x, int y, char ch, uint8_t r, uint8_t g, uint8_t b);

/* Set all 256 LEDs to black (does not call ledmatrix_show). */
void ClearPixels(void);

/* Fill the matrix with a rainbow (16 rows, 2–3 rows per colour band). */
void DrawRainbow(void);

/* Fill the matrix with random coloured pixels.
 * pixpercent : 0–100, fraction of LEDs that are lit
 * brightness : 0–100, maximum brightness scale */
void RandomPixels(int pixpercent, int brightness);

/* Display two rows of three characters each.
 * top[0..2]: upper row; bot[0..2]: lower row.
 * If both strings begin with "===", DrawRainbow() is called instead.
 * Default palette from the Python original: top = purple, bot = green. */
void Say(const char *top, const char *bot,
         uint8_t hr, uint8_t hg, uint8_t hb,
         uint8_t lr, uint8_t lg, uint8_t lb);

/* -----------------------------------------------------------------------
 * Animated effects
 * These call ledmatrix_show() and Delay_ms() internally.
 * DrawPlot and DrawSquares run forever (infinite loop).
 * DrawSpiral runs one cycle (spiral out, then unwind) and returns.
 * ----------------------------------------------------------------------- */
void DrawPlot(void);
void DrawSpiral(void);
void DrawSquares(void);

#endif /* LEDMATRIX_H */
