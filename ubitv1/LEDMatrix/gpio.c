#include "gpio.h"
void gpio_cfg_output(unsigned int pin_num){
  CLOCK->HFCLKSTARTED = 0;
  CLOCK->HFCLKSTART    = 1;
  while (CLOCK->HFCLKSTARTED == 0);
  CLOCK->HFCLKSTARTED = 0;
  GPIO->DIRSET = (1uL << pin_num);
  GPIO->OUTCLR = (1uL << pin_num);
}

/* Delay(count) runs a loop that
 *   takes roughly 4 cycles per iteration (volatile decrement + branch),
 *   so Delay(count) ≈ 4*count cycles ≈ 75 µs  for count=300 
*/
void Delay(int count) {
  volatile int i;

  i = count;
  while(--i > 0);
}
