/*
 * Author: Ramesh Yerraballi
 * Date: Fall 2026
*/
#include <stdint.h>  // C99
#include "gpio.h"

static GPIO_Regs * const GPIOB = ((GPIO_Regs *) GPIOB_BASE);
static IOMUX_Regs * const IOMUX = ((IOMUX_Regs *) IOMUX_BASE);

void Delay(uint32_t cycles){
    /* There will be a 2 cycle delay here to fetch & decode instructions
     * if branch and linking to this function */

    /* Subtract 2 net cycles for constant offset: +2 cycles for entry jump,
     * +2 cycles for exit, -1 cycle for a shorter loop cycle on the last loop,
     * -1 for this instruction */

    /* Local numeric label (1: / 1b), not a named "Delay_Loop:" label --
     * GCC's -O2 IPA constant-propagation clones this function for the
     * Delay(24) call in PB0Init() below (same translation unit, constant
     * argument), copying this asm text verbatim into the clone. A plain
     * named label would then be defined twice in this one .s file (the
     * general-purpose Delay() plus the specialized clone), which is
     * exactly the "symbol `Delay_Loop' is already defined" error. GNU as
     * local numeric labels are designed to be reused any number of times
     * within a file for exactly this reason, so this is collision-proof
     * regardless of how many times the compiler clones the function.
     *
     * "%0"/"+r"(cycles), not a hardcoded "R0" -- this was BASIC asm (no
     * operand list), which only worked by accident: it assumed `cycles`
     * would already be sitting in R0 because that's where the AAPCS
     * calling convention puts a function's first argument, but nothing
     * here told the compiler that this asm actually reads the C variable
     * `cycles`. That's exactly what the same IPA constant-propagation
     * clone above breaks: the cloned, cycles=24-specialized copy has no
     * ordinary C statement that reads `cycles` (only this opaque asm
     * text, which the optimizer can't see into), so the compiler
     * concluded the parameter was unused and stopped loading 24 into R0
     * before the clone's body runs -- R0 is then whatever garbage was
     * left over from the caller, and the loop counts down (or up, after
     * wrapping) from that garbage value instead of from 24, looking like
     * it hangs. The "+l"(cycles) operand below makes the dependency
     * explicit and applies regardless of what register the compiler
     * picks or how many times it clones the function.
     *
     * "+l", not "+r" -- restricts the register choice to R0-R7, which
     * is required for several Thumb-1 16-bit encodings (though it turned
     * out not to be the actual fix below needed -- see next comment).
     *
     * .syntax unified is emitted HERE, inside this same asm block, not
     * as an earlier separate __asm(".syntax unified") statement (which
     * is what this function used to do, and is a documented GCC/GAS
     * pitfall: GCC's inline asm defaults to pre-UAL "divided" syntax on
     * Arm/Thumb-1 targets, and that default is applied per asm block --
     * an earlier, separate asm() statement setting unified mode doesn't
     * reliably carry over into a different one. SUBS (the explicit "S"
     * suffix form) requires unified syntax to be recognized on Cortex-M0
     * at all; under divided syntax it's rejected outright regardless of
     * which register it operates on ("instruction not supported in
     * Thumb16 mode"), which is why changing "+r" to "+l" alone didn't
     * help -- the register was never the actual problem. .syntax divided
     * at the end restores the default so this doesn't affect any other
     * inline asm elsewhere in the file. */
    __asm volatile(
".syntax unified\n\t"
"            SUBS  %0, %0, #2; \n"
"1:          SUBS  %0, %0, #4; \n" // C=1 if no overflow
"            NOP;              \n" // C=0 when R0 passes through 0
"            BHS   1b;         \n"
".syntax divided\n\t"
        /* Return: 2 cycles */
        : "+l" (cycles)
        :
        : "cc"
    );
}

void PB0Init(){
  // Reset Port B
  GPIOB->GPRCM.RSTCTL = (uint32_t)0xB1000003;
  // Power up Port B
  GPIOB->GPRCM.PWREN = (uint32_t)0x26000001;
  Delay(24); // The Reset and Power steps take time to
             // to complete: at least 24 bus cylces
  IOMUX->SECCFG.PINCM[PB0INDEX] = (uint32_t) 0x00000081;
  // Output Enable PB0
  GPIOB->DOE31_0 |= 1;
}
