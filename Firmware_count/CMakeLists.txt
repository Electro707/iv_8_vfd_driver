# CMakeLists.txt

cmake_minimum_required(VERSION 3.16)
project(IV_8_Driver)

# Set the target MCU and programmer
set(MCU attiny406) # Replace with your target MCU
set(F_CPU 16000000UL) # Replace with your clock speed if different
set(PROGRAMMER jtag2updi) # For programming with avrdude
set(PROGRAMMER_COM /dev/ttyUSB0) # For programming with avrdude
set(PROGRAMMER_OPT -p attiny406)

# Specify AVR-GCC and AVR-OBJCOPY
set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_OBJCOPY avr-objcopy)

# Add flags for the AVR architecture
set(CMAKE_C_FLAGS "-mmcu=${MCU} -DF_CPU=${F_CPU} -Os -Wall -Wextra -std=gnu99 -Wl,-Map=$(TARGET).map,--cref")

# Add the source file(s)
add_executable(${PROJECT_NAME}.elf main.c)

# Generate the .hex file
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
    COMMENT "Generating ${PROJECT_NAME}.hex"
)

# Optional: Add a flash target for programming the device
add_custom_target(flash
    COMMAND avrdude -p ${MCU} -c ${PROGRAMMER} -P ${PROGRAMMER_COM} ${PROGRAMMER_OPT}
            -U flash:w:${PROJECT_NAME}.hex 
            -U fuse2:w:0x01:m
    COMMENT "Flashing the microcontroller"
)
