#include "ledmatrix.h"
#include "gpio.h"    /* Delay() */

/* -----------------------------------------------------------------------
 * Minimal random number generator (replaces libc rand — unavailable with
 * -nostdlib).  Linear congruential generator; returns 0–32767.
 * Call srand(seed) with any non-zero value to vary the sequence.
 * ----------------------------------------------------------------------- */
static unsigned int rand_state = 1u;

static int rand(void)
{
    rand_state = rand_state * 1664525u + 1013904223u;
    return (int)((rand_state >> 16) & 0x7FFF);
}

static void srand(unsigned int seed)
{
    rand_state = seed ? seed : 1u;
}

/* -----------------------------------------------------------------------
 * Module state
 * ----------------------------------------------------------------------- */
static neopixel_strip_t *g_strip;

/* -----------------------------------------------------------------------
 * 5×8 bitmap font
 * 5 bytes per glyph × 99 glyphs = 495 bytes (placed in flash as const).
 * Glyph index = (uint8_t)ch - 32  for printable ASCII 0x20–0x7E.
 * Glyph indices 95–98 are the built-in special glyphs (\b \t \n \v).
 * ----------------------------------------------------------------------- */
const uint8_t LedMatrix_Font[] = {
    0x00, 0x00, 0x00, 0x00, 0x00,  /* 0x20   SPACE */
    0x00, 0x00, 0xBE, 0x00, 0x00,  /* 0x21 ! */
    0x00, 0xE0, 0x00, 0xE0, 0x00,  /* 0x22 " */
    0x28, 0xFE, 0x28, 0xFE, 0x28,  /* 0x23 # */
    0x4C, 0x92, 0xFF, 0x92, 0x64,  /* 0x24 $ */
    0xC6, 0xC8, 0x10, 0x26, 0xC6,  /* 0x25 % */
    0x6C, 0x92, 0xAA, 0x44, 0x0A,  /* 0x26 & */
    0x00, 0x00, 0xE0, 0x00, 0x00,  /* 0x27 ' */
    0x00, 0x38, 0x44, 0x82, 0x00,  /* 0x28 ( */
    0x00, 0x82, 0x44, 0x38, 0x00,  /* 0x29 ) */
    0x28, 0x10, 0x7C, 0x10, 0x28,  /* 0x2A * */
    0x10, 0x10, 0x7C, 0x10, 0x10,  /* 0x2B + */
    0x00, 0x00, 0x06, 0x04, 0x00,  /* 0x2C , */
    0x10, 0x10, 0x10, 0x10, 0x10,  /* 0x2D - */
    0x00, 0x00, 0x06, 0x06, 0x00,  /* 0x2E . */
    0x02, 0x04, 0x08, 0x10, 0x20,  /* 0x2F / */

    0x7C, 0x8A, 0x92, 0xA2, 0x7C,  /* 0x30 0 */
    0x00, 0x42, 0xFE, 0x02, 0x00,  /* 0x31 1 */
    0x46, 0x8A, 0x92, 0x92, 0x62,  /* 0x32 2 */
    0x84, 0x82, 0x92, 0xB2, 0xCC,  /* 0x33 3 */
    0x18, 0x28, 0x48, 0xFE, 0x08,  /* 0x34 4 */
    0xE4, 0xA2, 0xA2, 0xA2, 0x9C,  /* 0x35 5 */
    0x3C, 0x52, 0x92, 0x92, 0x0C,  /* 0x36 6 */
    0x80, 0x8E, 0x90, 0xA0, 0xC0,  /* 0x37 7 */
    0x6C, 0x92, 0x92, 0x92, 0x6C,  /* 0x38 8 */
    0x60, 0x92, 0x92, 0x94, 0x78,  /* 0x39 9 */

    0x00, 0x00, 0x36, 0x36, 0x00,  /* 0x3A : */
    0x00, 0x02, 0x36, 0x34, 0x00,  /* 0x3B ; */
    0x00, 0x10, 0x28, 0x44, 0x82,  /* 0x3C < */
    0x28, 0x28, 0x28, 0x28, 0x28,  /* 0x3D = */
    0x82, 0x44, 0x28, 0x10, 0x00,  /* 0x3E > */
    0x40, 0x80, 0x9A, 0x90, 0x60,  /* 0x3F ? */
    0x4C, 0x92, 0xBE, 0x82, 0x7C,  /* 0x40 @ */

    0x3E, 0x48, 0x88, 0x48, 0x3E,  /* 0x41 A */
    0xFE, 0x92, 0x92, 0x92, 0x6C,  /* 0x42 B */
    0x7C, 0x82, 0x82, 0x82, 0x44,  /* 0x43 C */
    0xFE, 0x82, 0x82, 0x82, 0x7C,  /* 0x44 D */
    0xFE, 0x92, 0x92, 0x92, 0x82,  /* 0x45 E */
    0xFE, 0x90, 0x90, 0x90, 0x80,  /* 0x46 F */
    0x7C, 0x82, 0x82, 0x8A, 0xCE,  /* 0x47 G */
    0xFE, 0x10, 0x10, 0x10, 0xFE,  /* 0x48 H */
    0x00, 0x82, 0xFE, 0x82, 0x00,  /* 0x49 I */
    0x04, 0x02, 0x82, 0xFC, 0x80,  /* 0x4A J */
    0xFE, 0x10, 0x28, 0x44, 0x82,  /* 0x4B K */
    0xFE, 0x02, 0x02, 0x02, 0x02,  /* 0x4C L */
    0xFE, 0x40, 0x38, 0x40, 0xFE,  /* 0x4D M */
    0xFE, 0x20, 0x10, 0x08, 0xFE,  /* 0x4E N */
    0x7C, 0x82, 0x82, 0x82, 0x7C,  /* 0x4F O */
    0xFE, 0x90, 0x90, 0x90, 0x60,  /* 0x50 P */
    0x7C, 0x82, 0x8A, 0x84, 0x7A,  /* 0x51 Q */
    0xFE, 0x90, 0x98, 0x94, 0x62,  /* 0x52 R */
    0x64, 0x92, 0x92, 0x92, 0x4C,  /* 0x53 S */
    0xC0, 0x80, 0xFE, 0x80, 0xC0,  /* 0x54 T */
    0xFC, 0x02, 0x02, 0x02, 0xFC,  /* 0x55 U */
    0xF8, 0x04, 0x02, 0x04, 0xF8,  /* 0x56 V */
    0xFC, 0x02, 0x1C, 0x02, 0xFC,  /* 0x57 W */
    0xC6, 0x28, 0x10, 0x28, 0xC6,  /* 0x58 X */
    0xC0, 0x20, 0x1E, 0x20, 0xC0,  /* 0x59 Y */
    0x86, 0x9A, 0x92, 0xB2, 0xC2,  /* 0x5A Z */

    0x00, 0xFE, 0x82, 0x82, 0x00,  /* 0x5B [ */
    0x20, 0x10, 0x08, 0x04, 0x02,  /* 0x5C \ */
    0x00, 0x82, 0x82, 0xFE, 0x00,  /* 0x5D ] */
    0x20, 0x40, 0x80, 0x40, 0x20,  /* 0x5E ^ */
    0x01, 0x01, 0x01, 0x01, 0x01,  /* 0x5F _ */
    0x00, 0x80, 0x40, 0x20, 0x00,  /* 0x60 ` */

    0x04, 0x2A, 0x2A, 0x2A, 0x1E,  /* 0x61 a */
    0xFE, 0x12, 0x22, 0x22, 0x1C,  /* 0x62 b */
    0x1C, 0x22, 0x22, 0x22, 0x14,  /* 0x63 c */
    0x1C, 0x22, 0x22, 0x12, 0xFE,  /* 0x64 d */
    0x1C, 0x2A, 0x2A, 0x2A, 0x18,  /* 0x65 e */
    0x10, 0x7E, 0x90, 0x80, 0x40,  /* 0x66 f */
    0x18, 0x25, 0x25, 0x25, 0x3E,  /* 0x67 g */
    0xFE, 0x10, 0x10, 0x10, 0x0E,  /* 0x68 h */
    0x00, 0x12, 0x5E, 0x02, 0x00,  /* 0x69 i */
    0x04, 0x02, 0x22, 0xBC, 0x00,  /* 0x6A j */
    0xFE, 0x08, 0x14, 0x22, 0x00,  /* 0x6B k */
    0x00, 0x82, 0xFE, 0x02, 0x00,  /* 0x6C l */
    0x3E, 0x20, 0x18, 0x20, 0x3E,  /* 0x6D m */
    0x3E, 0x20, 0x20, 0x20, 0x1E,  /* 0x6E n */
    0x1C, 0x22, 0x22, 0x22, 0x1C,  /* 0x6F o */
    0x3F, 0x28, 0x28, 0x28, 0x10,  /* 0x70 p */
    0x10, 0x28, 0x28, 0x28, 0x3F,  /* 0x71 q */
    0x3E, 0x10, 0x20, 0x20, 0x10,  /* 0x72 r */
    0x12, 0x2A, 0x2A, 0x2A, 0x04,  /* 0x73 s */
    0x20, 0xFC, 0x22, 0x02, 0x04,  /* 0x74 t */
    0x3C, 0x02, 0x02, 0x04, 0x3E,  /* 0x75 u */
    0x38, 0x04, 0x02, 0x04, 0x38,  /* 0x76 v */
    0x3C, 0x02, 0x0C, 0x02, 0x3C,  /* 0x77 w */
    0x22, 0x14, 0x08, 0x14, 0x22,  /* 0x78 x */
    0x39, 0x05, 0x05, 0x05, 0x3E,  /* 0x79 y */
    0x22, 0x26, 0x2A, 0x32, 0x22,  /* 0x7A z */

    0x00, 0x10, 0x6C, 0x82, 0x00,  /* 0x7B { */
    0x00, 0x00, 0xFE, 0x00, 0x00,  /* 0x7C | */
    0x00, 0x82, 0x6C, 0x10, 0x00,  /* 0x7D } */
    0x20, 0x40, 0x30, 0x10, 0x20,  /* 0x7E ~ */

    /* Built-in glyphs (glyph indices 95–98, accessed via special chars) */
    0x18, 0x44, 0x04, 0x44, 0x18,  /* index 95  \b (0x08): smile */
    0x38, 0x44, 0x22, 0x44, 0x38,  /* index 96  \t (0x09): heart */
    0x18, 0x18, 0x00, 0x18, 0x18,  /* index 97  \n (0x0A): eyes  */
    0x18, 0x18, 0x00, 0x18, 0x18,  /* index 98  \v (0x0B): eyes  */
};

/* -----------------------------------------------------------------------
 * Internal helpers
 * ----------------------------------------------------------------------- */

/* Write directly to the LED buffer by strip index (bypasses spos). */
static void set_led(int idx, uint8_t r, uint8_t g, uint8_t b)
{
    if ((unsigned int)idx >= MATRIX_SIZE) return;
    g_strip->leds[idx].simple.r = r;
    g_strip->leds[idx].simple.g = g;
    g_strip->leds[idx].simple.b = b;
}

/* -----------------------------------------------------------------------
 * Initialisation
 * ----------------------------------------------------------------------- */

void ledmatrix_init(neopixel_strip_t *strip)
{
    g_strip = strip;
}

void ledmatrix_srand(unsigned int seed)
{
    srand(seed);
}

void ledmatrix_show(void)
{
    neopixel_show(g_strip);
}

/* -----------------------------------------------------------------------
 * Coordinate mapping
 * ----------------------------------------------------------------------- */

int spos(int x, int y)
{
    int pos;
    if (x & 1)              /* odd column: count y downward */
        pos = x * 16 + (15 - y);
    else                    /* even column: count y upward */
        pos = x * 16 + y;
    return 255 - pos;
}

/* -----------------------------------------------------------------------
 * Drawing primitives
 * ----------------------------------------------------------------------- */

void DrawPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
    set_led(spos(x, y), r, g, b);
}

void DrawVLine(int x, int y, int len, uint8_t r, uint8_t g, uint8_t b)
{
    for (int i = y; i < y + len; i++)
        DrawPixel(x, i, r, g, b);
}

void DrawHLine(int x, int y, int len, uint8_t r, uint8_t g, uint8_t b)
{
    for (int i = x; i < x + len; i++)
        DrawPixel(i, y, r, g, b);
}

void DrawRectangle(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b)
{
    for (int i = 0; i < w; i++)
        for (int j = 0; j < h; j++)
            DrawPixel(x + i, y + j, r, g, b);
}

void DrawUnfilledRectangle(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b)
{
    DrawVLine(x,     y,     h,     r, g, b);
    DrawHLine(x,     y,     w,     r, g, b);
    DrawVLine(x + w, y,     h,     r, g, b);
    DrawHLine(x,     y + h, w + 1, r, g, b);
}

void DrawBitmap(const uint16_t *image, int x, int y, int w, int h)
{
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            uint16_t px = image[j * 16 + i];
            uint8_t r =  px        & 0x1Fu;
            uint8_t g = (px >>  5) & 0x1Fu;
            uint8_t b = (px >> 11) & 0x1Fu;
            DrawPixel(x + i, y + j, r, g, b);
        }
    }
}

void DrawChar(int x, int y, char ch, uint8_t r, uint8_t g, uint8_t b)
{
    int c;
    int special = 0;

    /* \b \t \n \v select the built-in glyphs starting at font index 95.
     * Formula matches the Python original: ('~'-' ') + ord(ch) - 7 */
    if (ch == '\b' || ch == '\t' || ch == '\n' || ch == '\v') {
        c = ('~' - ' ') + (int)(unsigned char)ch - 7;
        special = 1;
    } else {
        c = (int)(unsigned char)ch - 32;
    }

    /* Each glyph is 5 columns wide + 1 blank spacing column, 8 rows tall.
     * The 6th column (i==5) is always blank. */
    for (int i = 0; i < 6; i++) {
        uint8_t line = (i == 5) ? 0x00u : LedMatrix_Font[c * 5 + i];
        for (int j = 0; j < 8; j++) {
            if (line & 0x01u) {
                if (special) {
                    /* Built-in glyphs are rendered in random colour */
                    DrawPixel(x + i, y + j,
                              (uint8_t)(rand() % 256),
                              (uint8_t)(rand() % 256),
                              (uint8_t)(rand() % 256));
                } else {
                    DrawPixel(x + i, y + j, r, g, b);
                }
            } else {
                DrawPixel(x + i, y + j, 0, 0, 0);
            }
            line >>= 1;
        }
    }
}

void ClearPixels(void)
{
    for (int i = 0; i < MATRIX_SIZE; i++)
        set_led(i, 0, 0, 0);
}

void DrawRainbow(void)
{
    DrawHLine(0,  0, 16, 60, 0,   0 );  /* Red    */
    DrawHLine(0,  1, 16, 60, 0,   0 );
    DrawHLine(0,  2, 16, 60, 0,   0 );
    DrawHLine(0,  3, 16, 60, 30,  0 );  /* Orange */
    DrawHLine(0,  4, 16, 60, 30,  0 );
    DrawHLine(0,  5, 16, 60, 60,  0 );  /* Yellow */
    DrawHLine(0,  6, 16, 60, 60,  0 );
    DrawHLine(0,  7, 16, 30, 60,  0 );  /* Green  */
    DrawHLine(0,  8, 16, 30, 60,  0 );
    DrawHLine(0,  9, 16, 0,  0,   60);  /* Blue   */
    DrawHLine(0, 10, 16, 0,  0,   60);
    DrawHLine(0, 11, 16, 5,  0,   30);  /* Indigo */
    DrawHLine(0, 12, 16, 5,  0,   30);
    DrawHLine(0, 13, 16, 35, 0,   52);  /* Violet */
    DrawHLine(0, 14, 16, 35, 0,   52);
    DrawHLine(0, 15, 16, 35, 0,   52);
}

void RandomPixels(int pixpercent, int brightness)
{
    int scale = brightness * 255 / 100;
    for (int i = 0; i < MATRIX_SIZE; i++) {
        if ((rand() % 101) >= pixpercent) {
            set_led(i, 0, 0, 0);
        } else {
            set_led(i,
                    (uint8_t)(rand() % (scale + 1)),
                    (uint8_t)(rand() % (scale + 1)),
                    (uint8_t)(rand() % (scale + 1)));
        }
    }
}

void Say(const char *top, const char *bot,
         uint8_t hr, uint8_t hg, uint8_t hb,
         uint8_t lr, uint8_t lg, uint8_t lb)
{
    /* Rainbow Easter egg: Say("===", "===", ...) */
    if (top[0] == '=' && top[1] == '=' && top[2] == '=' &&
        bot[0] == '=' && bot[1] == '=' && bot[2] == '=') {
        DrawRainbow();
        return;
    }

    DrawChar(0,  8, top[0], hr, hg, hb);
    DrawChar(5,  8, top[1], hr, hg, hb);
    DrawChar(10, 8, top[2], hr, hg, hb);
    DrawChar(0,  0, bot[0], lr, lg, lb);
    DrawChar(5,  0, bot[1], lr, lg, lb);
    DrawChar(10, 0, bot[2], lr, lg, lb);
}

/* -----------------------------------------------------------------------
 * Animated effects
 * ----------------------------------------------------------------------- */

void DrawPlot(void)
{
    /* static: zero-initialised in .bss by startup_nrf51.S — no memset call */
    static int heights[MATRIX_WIDTH];

    while (1) {
        for (int col = 0; col < MATRIX_WIDTH; col++) {
            int change = (rand() % 3) - 1;  /* -1, 0, or +1 */
            int new_h  = heights[col] + change;
            if (new_h < 0)             new_h = 0;
            if (new_h > MATRIX_HEIGHT) new_h = MATRIX_HEIGHT;

            uint8_t r = (uint8_t)(50 + rand() % 206);
            uint8_t g = (uint8_t)(50 + rand() % 206);
            uint8_t b = (uint8_t)(50 + rand() % 206);

            if (new_h > heights[col]) {
                for (int row = heights[col]; row < new_h; row++) {
                    DrawPixel(col, row, r, g, b);
                    ledmatrix_show();
                    Delay_ms(50);
                }
            } else if (new_h < heights[col]) {
                for (int row = new_h; row < heights[col]; row++) {
                    DrawPixel(col, row, 0, 0, 0);
                    ledmatrix_show();
                    Delay_ms(50);
                }
            }
            heights[col] = new_h;
        }
        Delay_ms(50);
    }
}

void DrawSpiral(void)
{
    typedef struct { int x, y; } Point;
    static Point pixel_order[MATRIX_SIZE];
    int pixel_count = 0;

    int x  = 8, y  = 8;
    int dx = 1, dy = 0;
    int steps_in_dir  = 1;
    int steps_taken   = 0;
    int dir_changes   = 0;

    while (pixel_count < MATRIX_SIZE) {
        if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT) {
            DrawPixel(x, y,
                      (uint8_t)(rand() % 256),
                      (uint8_t)(rand() % 256),
                      (uint8_t)(rand() % 256));
            pixel_order[pixel_count].x = x;
            pixel_order[pixel_count].y = y;
            pixel_count++;
            ledmatrix_show();
            Delay_ms(50);
        }

        x += dx;
        y += dy;
        steps_taken++;

        if (steps_taken == steps_in_dir) {
            steps_taken = 0;
            dir_changes++;

            /* Turn 90° clockwise: right → down → left → up → right */
            if      (dx ==  1 && dy ==  0) { dx =  0; dy =  1; }
            else if (dx ==  0 && dy ==  1) { dx = -1; dy =  0; }
            else if (dx == -1 && dy ==  0) { dx =  0; dy = -1; }
            else                           { dx =  1; dy =  0; }

            if (dir_changes % 2 == 0)
                steps_in_dir++;
        }
    }

    Delay_ms(50);

    /* Unwind: clear pixels in reverse order */
    for (int i = pixel_count - 1; i >= 0; i--) {
        DrawPixel(pixel_order[i].x, pixel_order[i].y, 0, 0, 0);
        ledmatrix_show();
        Delay_ms(50);
    }
}

void DrawSquares(void)
{
    /* Each ring (size 0–8) has at most 60 pixels (full 16×16 perimeter).
     * Use 64 slots per ring for alignment. */
    typedef struct { int x, y; } Point;
    static Point ring_px[9][64];
    static int   ring_n[9];

    while (1) {
        uint8_t r = (uint8_t)(rand() % 256);
        uint8_t g = (uint8_t)(rand() % 256);
        uint8_t b = (uint8_t)(rand() % 256);

        /* Draw concentric squares outward (size 0 = centre, size 8 = edge) */
        for (int size = 0; size < 9; size++) {
            ring_n[size] = 0;

            int x1 = 7 - size,  y1 = 7 - size;
            int x2 = 7 + size + 1, y2 = 7 + size + 1;

            if (x1 < 0)  x1 = 0;
            if (y1 < 0)  y1 = 0;
            if (x2 > 16) x2 = 16;
            if (y2 > 16) y2 = 16;

            /* Helper: draw one pixel and record it */
#define DRAW_AND_RECORD(px, py) \
            DrawPixel((px), (py), r, g, b); \
            ring_px[size][ring_n[size]].x = (px); \
            ring_px[size][ring_n[size]].y = (py); \
            ring_n[size]++; \
            ledmatrix_show()

            /* Top edge */
            if (y1 < MATRIX_HEIGHT)
                for (int px = x1; px < x2; px++) { DRAW_AND_RECORD(px, y1); }

            /* Bottom edge (skip if same row as top) */
            if (y2 - 1 >= 0 && y2 - 1 != y1)
                for (int px = x1; px < x2; px++) { DRAW_AND_RECORD(px, y2 - 1); }

            /* Left edge (excluding corners) */
            if (x1 < MATRIX_WIDTH)
                for (int py = y1 + 1; py < y2 - 1; py++) { DRAW_AND_RECORD(x1, py); }

            /* Right edge (excluding corners, skip if same col as left) */
            if (x2 - 1 >= 0 && x2 - 1 != x1)
                for (int py = y1 + 1; py < y2 - 1; py++) { DRAW_AND_RECORD(x2 - 1, py); }

#undef DRAW_AND_RECORD

            Delay_ms(50);
        }

        Delay_ms(50);

        /* Clear outermost ring first, inward */
        for (int size = 8; size >= 0; size--) {
            for (int k = 0; k < ring_n[size]; k++) {
                DrawPixel(ring_px[size][k].x, ring_px[size][k].y, 0, 0, 0);
                ledmatrix_show();
                Delay_ms(50);
            }
        }

        Delay_ms(50);
    }
}
