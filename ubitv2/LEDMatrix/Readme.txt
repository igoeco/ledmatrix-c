This is a bare bones project for the 
BBC Microbit v2 demonstrating the NeoPixel LED strip WS2815


To build:
  $ ./compile

To run:
Open the openocd utility in Powershell:
  > openocd -f board/nordic_nrf52833_mkit.cfg

Open the debugger in a wsl bash shell:
  $ gdb-multiarch -tui -iex "set auto-load safe-path /" ledmatrix.elf

