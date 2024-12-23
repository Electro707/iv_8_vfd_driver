/**
 * Communication Handler
 * by electro707
 * 
 * This files handles communication functions over UART to/from the PC
 * 
 */
#include <stdint.h>
#include <stdlib.h>

#define FIFO_SIZE       64     // must be power of two, and less than 256 (counters are 8-bits)

#define FIFO_MSK         (FIFO_SIZE - 1)

typedef struct{
    uint8_t *buff;  // the buffer
    uint8_t in;     // counter for current in position
    uint8_t out;    // counter for out position
}fifo_t;

void processUart(void);