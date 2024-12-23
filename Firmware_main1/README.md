# Firmware

## Cmake Targets
- `<No Arguments>`: Builds the firmware
- `flash`: Flashes the firmware using jtag2updi, which can be uploaded unto an Arduino board and connected to this board with a 4.7k series resistor
    - See the [jtag2udpi](https://github.com/ElTangas/jtag2updi) project for more details on programming and connections
    - You may have to change the jtag2updi port in CmakeLists.txt