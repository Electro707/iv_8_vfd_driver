#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#define N_SEGMENTS 2

#define EVER ;;

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

const uint8_t numberToSeg[10] = {0xed, 0x28, 0xce, 0x6e, 0x2b, 0x67, 0xe7, 0x2c, 0xef, 0x6f};

uint8_t dispN = 0;

#define SET_DOT(_SEG)   _SEG |= (1 << 5)
#define CLR_DOT(_SEG)   _SEG &= ~(1 << 5)

int main(void){
    CCP = 0xD8;
    CLKCTRL_MCLKCTRLB = 0x00;

    SHIFT_O_DIS;        // OE defaults to high, disabled
    SHIFT_RESET;
    VFD_2_DIS;
    VFD_1_DIS;


    PORTA_DIR = 0b11101010;
    PORTB_DIR = 0b00100110;
    PORTC_DIR = 0b1100;

    SPI0_CTRLB = 0x04;
    SPI0_CTRLA = 0x63;
    SPI0_INTFLAGS = 0;

    SHIFT_UNRESET;

    // send only one byte to SPI
    PIN_LDR_RST;
    SPI0_DATA = 0;
    while((SPI0_INTFLAGS & (1 << 7)) == 0);
    PIN_LDR_SET;

    TCB0_CTRLB = 0x00;
    TCB0_INTCTRL = 1;
    TCB0_CCMP = 16000UL;
    TCB0_CTRLA = 0x01;      // enable clock, no divider

    // PIN_ENA_FILAMENT;
    PIN_ENA_HIV;

    sei();

    for(EVER){
        // everything is handled in interrupt
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

    if(incSegCnt++ >= 250){      // increment number every 250mS
        incSegCnt = 0;
        dispN++;
        if(dispN == 100){
            dispN = 0;
        }
    }
}