/* ============================================================
 * nrf52833.ld — Linker script for nRF52833 (BBC micro:bit v2)
 *
 * Memory map:
 *   FLASH : 512 KB @ 0x00000000  (code, rodata, vector table)
 *   RAM   : 128 KB @ 0x20000000  (data, bss, stack)
 *
 * Stack pointer is set by the first word of the vector table.
 * _estack = top of RAM = 0x20020000.
 *
 * Section layout in RAM:
 *   0x20000000  .data  (initialised globals — copied from flash at startup)
 *               .bss   (zero-initialised globals)
 *               heap   (grows up — not used in this bare-metal demo)
 *               ...
 *   0x20020000  <- _estack  (stack grows DOWN from here)
 *
 * Differences from nrf51.ld:
 *   - FLASH grows 256K -> 512K, RAM grows 16K -> 128K
 *   - _estack moves from 0x20004000 to 0x20020000
 *   - Everything else (section layout, symbol names consumed by
 *     startup_nrf52833.S) is unchanged, since Reset_Handler's
 *     data/bss copy loop doesn't care about the CPU core.
 * ============================================================ */

ENTRY(Reset_Handler)

/* Top of 128 KB RAM */
_estack = 0x20020000;

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 512K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

SECTIONS
{
    /* --------------------------------------------------------
     * .text — code and read-only data, placed in FLASH
     * ------------------------------------------------------ */
    .text :
    {
        /* Vector table MUST be first (Cortex-M reads it at 0x0) */
        KEEP(*(.vectors))

        /* Code */
        *(.text)
        *(.text*)

        /* Read-only data (constants, string literals) */
        *(.rodata)
        *(.rodata*)

        /* ARM-specific sections */
        *(.ARM.extab*)
        *(.ARM.exidx*)

        . = ALIGN(4);
        _etext = .;          /* end of flash-resident content */
    } > FLASH

    /* --------------------------------------------------------
     * .data — initialised globals
     *   VMA (runtime address) : RAM
     *   LMA (load/flash address) : FLASH, right after .text
     * ------------------------------------------------------ */
    _sidata = LOADADDR(.data);    /* flash source address for startup copy */

    .data :
    {
        . = ALIGN(4);
        _sdata = .;          /* start of .data in RAM */
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;          /* end of .data in RAM */
    } > RAM AT > FLASH

    /* --------------------------------------------------------
     * .bss — zero-initialised globals (no flash storage needed)
     * ------------------------------------------------------ */
    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    /* --------------------------------------------------------
     * Sanity-check: catch sections that would end up nowhere
     * ------------------------------------------------------ */
    /DISCARD/ :
    {
        *(.ARM.attributes)
        *(.comment)
        *(.note*)
        *(.eh_frame)
    }
}
