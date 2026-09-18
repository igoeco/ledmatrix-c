This is a bare bones project for the 
BBC Microbit v1 demonstrating the NeoPixel LED strip WS2815


Note:
The Cortex_M_Startup.s file in the libs folder does the "sta
rtup" where it
declares the Vector Table and the Reset Handler. The Reset H
andler simply
transfers unconditional control to the main.

To build:
  $ ./compile

To run:
Open the openocd utility in Powershell:
  > openocd -f board/nordic_nrf51_mkit.cfg

Open the debugger in a wsl bash shell:
  $ gdb-multiarch -tui -iex "set auto-load safe-path /" ledmatrix.elf

