
/*
 * FastestSPI
 * (c)2015 Josh Levine [http://josh.com]
 * 
 * Demo showing a technique to improve the maximum SPI thoughput 
 * by cycle counting and blindly sending new data into the SPI hardware
 * without checking status bit. 
 * 
 * Full article:
 * http://wp.josh.com/2015/09/29/bare-metal-fast-spi-on-avr/
 */


#include <SPI.h>

inline static void fastSpiTransmit( const void *buf , unsigned int len ) {

  if (len == 0) return;     // Do nothin if len zero
    
  asm volatile (
  
        "LOOP_LEN_%=:           \n\t"
                                                // Cycles
                                                // ======
          "ld __tmp_reg__,%a[buf]+   \n\t"      //     2    - load next byte 
          "out %[spdr],__tmp_reg__   \n\t"      //     1    - transmit byte

          "rjmp   .+0               \n\t"       //     2
          "rjmp   .+0               \n\t"       //     2
          "rjmp   .+0               \n\t"       //     2
          "rjmp   .+0               \n\t"       //     2
          "rjmp   .+0               \n\t"       //     2 
          "rjmp   .+0               \n\t"       //     2 
          "nop                      \n\t"       //     1

          
          "sbiw %[len], 1           \n\t"       //     2 
          "brne LOOP_LEN_%=         \n\t"       //     2

          "nop                      \n\t"       //     1 - use up the cycle we saved from the above branch not taken
                                                //         this makes sure that if we have two transmits in a row, the second one will not
                                                //         step on the first. 
                                                
        
              :         // Outputs: (these are actually inputs, but we mark as read/write output since they get changed during execution)
                        // "there is no way to specify that input operands get modified without also specifying them as output operands."
                
          [buf]   "+e" (buf),         // pointer to buffer 
          [len]   "+w" (len)          // length of buffer
        
                
              :         // Inputs:
          "[buf]" (buf),              // pointer to buffer
          "[len]" (len),              // length of buffer
        
        
        
        [spdr]  "I" (_SFR_IO_ADDR(SPDR))   // SPI data register
                
        :         // Clobbers
        "cc"                  // special name that indicates that flags may have been clobbered 
        
        
  );

}

void setup() {
  SPI.setClockDivider(SPI_CLOCK_DIV2);     // Fastest possible speed
  SPI.begin();
}


void loop() {
      
     char *data = "SPI is fun";
     
     fastSpiTransmit( data , strlen(data) );
     
}
