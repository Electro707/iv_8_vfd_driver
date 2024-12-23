/**
 * IV-8 Driver Board Firmware
 * by electro707
 * 
 * Useful notes:
 *     - CPU clock: 16Mhz
 *     - Periferal Clock: 16Mhz
 *     - Real time interrupt: 1Khz
 */
#include <stdbool.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "main.h"
#include "comms.h"

/** Program Configs **/
#define N_SEGMENTS 2

/** Misc Macros **/
#define EVER ;;

/** Pin enable/disable Macros **/
#define PIN_ENA_FILAMENT PORTB_OUTSET = (1 << 5)
#define PIN_DIS_FILAMENT PORTB_OUTCLR = (1 << 5)

#define PIN_ENA_HIV PORTB_OUTSET = (1 << 1)
#define PIN_DIS_HIV PORTB_OUTCLR = (1 << 1)

#define PIN_LDR_RST PORTC_OUTCLR = (1 << 2)
#define PIN_LDR_SET PORTC_OUTSET = (1 << 2)

#define SHIFT_O_ENA     PORTC_OUTCLR = (1 << 3)
#define SHIFT_O_DIS     PORTC_OUTSET = (1 << 3)

#define SHIFT_RESET     PORTA_OUTCLR = (1 << 5)
#define SHIFT_UNRESET   PORTA_OUTSET = (1 << 5)

#define VFD_1_EN        PORTA_OUTSET = (1 << 7)
#define VFD_1_DIS        PORTA_OUTCLR = (1 << 7)

#define VFD_2_EN        PORTA_OUTSET = (1 << 6)
#define VFD_2_DIS        PORTA_OUTCLR = (1 << 6)

// constant to convert an integer into what to be displated
const uint8_t numberToSeg[10] = {0xed, 0x28, 0xce, 0x6e, 0x2b, 0x67, 0xe7, 0x2c, 0xef, 0x6f};
// macros to set or clear the dot in the display
#define SET_DOT(_SEG)   _SEG |= (1 << 5)
#define CLR_DOT(_SEG)   _SEG &= ~(1 << 5)

// a global counter of the current displayed number
uint8_t dispN = 0;

void mcuInit(void){
    CCP = 0xD8;                     // unlock io write for below
    CLKCTRL_MCLKCTRLB = 0x00;       // no divider for periferal clocks

    // init pin output
    SHIFT_O_DIS;        // OE defaults to high, disabled
    SHIFT_RESET;
    VFD_2_DIS;
    VFD_1_DIS;

    // init ports
    PORTA_DIR = 0b11101010;
    PORTB_DIR = 0b00100110;
    PORTC_DIR = 0b1100;

    // init spi for shift register
    SPI0_CTRLB = 0x04;
    SPI0_CTRLA = 0x63;
    SPI0_INTFLAGS = 0;

    // init uart
    USART0_BAUD = 555;
    USART0_CTRLC = 0x03;        // 8 bit, no parity, 1 stop bit
    USART0_CTRLB = 0xc0;        // enable transmitter and receiver
    USART0_CTRLA = 0x80;
    
    // set the shift register output to 0 (nothing is enabled)
    SHIFT_UNRESET;
    PIN_LDR_RST;
    SPI0_DATA = 0;
    while((SPI0_INTFLAGS & (1 << 7)) == 0);
    PIN_LDR_SET;

    // init real time clock, 1Khz
    TCB0_CTRLB = 0x00;
    TCB0_INTCTRL = 1;
    TCB0_CCMP = 16000UL;
    TCB0_CTRLA = 0x01;      // enable clock, no divider
}

int main(void){
    mcuInit();

    // PIN_ENA_FILAMENT;    // not needed
    PIN_ENA_HIV;

    sei();

    for(EVER){
        // everything is handled in interrupt
        processUart();
    }

    return 0;
}

ISR(TCB0_INT_vect){
    static uint8_t swapDispCnt = 0;         // counter to swap the shown display
    static uint8_t currDisp = 0;               // the current shown display
    static uint16_t incSegCnt = 0;             // counter to increment the displayed number

    uint8_t toShowN;                        // current number to be shown on the current segment

    TCB0_INTFLAGS = 1;

    if(swapDispCnt++ >= 2){
        SHIFT_O_DIS;
        VFD_1_DIS;
        VFD_2_DIS;

        swapDispCnt = 0;

        currDisp++;
        currDisp &= 1;

        if(currDisp == 0){
            toShowN = dispN % 10;
        } else {
            toShowN = dispN / 10;
        }

        PIN_LDR_RST;
        // SPI0_INTFLAGS = 0;
        SPI0_DATA = numberToSeg[toShowN];
        while((SPI0_INTFLAGS & (1 << 7)) == 0);
        PIN_LDR_SET;
        
        if(currDisp == 0){
            VFD_1_DIS;
            VFD_2_EN;
        } else {
            VFD_1_EN;
            VFD_2_DIS;
        }
        SHIFT_O_ENA;
    }

    // if(incSegCnt++ >= 250){      // increment number every 250mS
        // incSegCnt = 0;
        // dispN++;
        // if(dispN == 100){
        //     dispN = 0;
        // }
        // USART0_TXDATAL = (dispN%10) + '0';
    // }
}