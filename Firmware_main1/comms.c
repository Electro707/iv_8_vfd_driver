/**
 * Communication Handler
 * by electro707
 * 
 * This files handles communication functions over UART to/from the PC
 * 
 */

#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "comms.h"
#include "main.h"

const char helpTxt[] = \
    "Commands:\n" \
    "\tping: pong!\n" \
    "\td X: Set the display number to X\n" \
    "\0";

uint8_t rxBuff[FIFO_SIZE];
fifo_t rxFifo = {
    .buff = rxBuff,
    .in = 0,
    .out = 0,
};

uint8_t txBuff[FIFO_SIZE];
fifo_t txFifo = {
    .buff = txBuff,
    .in = 0,
    .out = 0,
};

uint8_t resv_nl = 0;        // flag when we receive new line

void uartTxString(const char *toSend);
int fifoGet(fifo_t *f, uint8_t *out);

// this function gets called when the application has time to process and UART commands
// low priority
void processUart(void){
    if(resv_nl == 0){
        return;
    }
    resv_nl = 0;

    // if we finally got a line, process it
    // while we can do a binary search tree, for now read entire buffer then process on it with strcmp
    char readTxt[32];
    uint8_t readTxtIdx = 0;
    uint8_t stat;
    char *token;

    while(1){
        stat = fifoGet(&rxFifo, (uint8_t *)(readTxt + readTxtIdx));
        if(stat != 0){
            break;
        }
        if(readTxt[readTxtIdx] == '\n'){
            readTxt[readTxtIdx] = 0;
            break;
        }
        // todo: handle '\r' characters
        if(++readTxtIdx >= 32){
            readTxt[readTxtIdx-1] = 0;
            break;
        }
    }

    token = strtok(readTxt, " ");

    if(!strcmp(token, "help")){
        uartTxString(helpTxt);
    }
    else if(!strcmp(token, "ping")){
        uartTxString("pong!\n");
    }
    else if(!strcmp(token, "d")){
        token = strtok(NULL, " ");
        stat = atoi(token);
        if(stat >= 100){
            stat = 99;
        }
        dispN = stat;
    }
    else{
        uartTxString("invalid\n");
    }
}

void uartTxString(const char *toSend){
    // for now no FIFO, just send direct
    while(*toSend){
        USART0_TXDATAL = *toSend++;
        while((USART0_STATUS & (1 << 5)) == 0);
    }
    
}

int fifoGet(fifo_t *f, uint8_t *out){
    // if we are at the same pointer as IN, that means we don't have any more data to read
    if(f->in == f->out){
        *out = 0;
        return -1;
    }

    *out = f->buff[f->out];

    f->out++;
    f->out &= FIFO_MSK;

    return 0;
}

int fifoPut(fifo_t *f, uint8_t in){
    // if the next write will wrap over to OUT (i.e we didn't read the data), don't and return error
    if(((f->out - 1) & FIFO_MSK) == f->in){
        return -1;
    }

    f->buff[f->in] = in;

    f->in++;
    f->in &= FIFO_MSK;

    return 0;
}

ISR(USART0_RXC_vect){
    // todo: read what we received
    char c = USART0_RXDATAL;

    fifoPut(&rxFifo, c);

    if(c == '\n'){
        resv_nl = 1;
    }
    
}