
#define DDR_SPI DDRB
#define PORT_SPI PORTB
#define DD_MOSI 3     // PB3 = Pin 17 = Digital 11
#define DD_MISO 4     // PB4 = Pin 18 = Digital 12
#define DD_SCK  5     // PB5 = Pin 19 = Digital 13
#define DD_SS   2     // PB2 = Pin 16 = Digital 10

// "19.3.2: If SS is configured as an output, the pin is a general output pin which does not affect the SPI system." 
// So we must keep SS output, but can be generally used.

void SPI_MasterInit(void)
{
    /* Set MOSI,SCK,SS output */
    DDR_SPI |= (1<<DD_MOSI)|(1<<DD_SCK) | (1<<DD_SS);    

    // 8mhz
    /* Enable SPI, Master, set clock rate 8Mhz */
    SPCR = (1<<SPE)|(1<<MSTR);  
    SPSR =  _BV(SPI2X);

    //4mhz
    /* Enable SPI, Master, set clock rate 4Mhz */
    //SPCR = (1<<SPE)|(1<<MSTR);
    //SPSR =  0;
    

    //2Mhz
    /* Enable SPI, Master, set clock rate 2Mhz */
    //SPCR |= (1<<SPE)|(1<<MSTR) | (1<<SPR0);
    //SPSR =  _BV(SPI2X);

    //1Mhz
    /* Enable SPI, Master, set clock rate 2Mhz */
    //SPCR |= (1<<SPE)|(1<<MSTR) | (1<<SPR0);
    //SPSR =  0;

    //500Khz
    /* Enable SPI, Master, set clock rate 2Mhz */
    //SPCR |= (1<<SPE)|(1<<MSTR) | (1<<SPR1);
    //SPSR =  _BV(SPI2X);
    

    //250Khz
    /* Enable SPI, Master, set clock rate 2Mhz */
    //SPCR |= (1<<SPE)|(1<<MSTR) | (1<<SPR1);
    //SPSR =  0;

   //SPDR = 0;    //prime

            
}


// This function moved inline so we can check for seial bytes while waiting for SPI to flush....
 
inline void SPI_MasterTransmit(char cData)
{
  
  // Wait for any previous transmission to complete 
  // Start transmission 
  // TODO: Why don't these work? SPI hardware bug?
  //while(!(SPSR & (1<<SPIF)));  
 
 /*
  do {
    SPDR = cData;
  } while (SPSR & _BV(WCOL));

*/


  
  // TODO: Blind send on the SPI to maximize output speed. Just need to count clock cycles to make sure we don't overrrun.
  
}




  inline static void test0(void *buf, size_t count) {
    if (count == 0) return;
    uint8_t *p = (uint8_t *)buf;
    SPDR = *p;
    while (--count > 0) {
      uint8_t out = *(p + 1);
      while (!(SPSR & _BV(SPIF))) ;
      uint8_t in = SPDR;
      SPDR = out;
      *p++ = in;
    }
    while (!(SPSR & _BV(SPIF))) ;
    *p = SPDR;
  }  



inline static void test1( const void *buf , unsigned int len ) {
  
  asm volatile (
  
        "LOOP_LEN_%=:           \n\t"

        "WAIT_TRANS_%=:           \n\t"   // Wait for transmission to complete
          "in __tmp_reg__, %[spsr]  \n\t"
          "sbrs __tmp_reg__, %[spif]  \n\t"
          "rjmp WAIT_TRANS_%=       \n\r"

          "ld __tmp_reg__,%a[buf]+    \n\t"                 
          "out %[spdr],__tmp_reg__    \n\t" 
        
          "sbiw %[len], 1         \n\t"
          "brne LOOP_LEN_%=       \n\t"       
        
              :         // Outputs: (these are actually inputs, but we mark as read/write output since they get changed during execution)
          [buf]   "+e" (buf),         // pointer to buffer - 
          [len]   "+w" (len)          // length of buffer
        
                
              :         // Inputs:
        [spdr]  "I" (_SFR_IO_ADDR(SPDR)),   // SPI data register
        [spsr]  "I" (_SFR_IO_ADDR(SPSR)),   // SPI status register (we only need SPIF bit)
        [spif]  "M" (SPIF)            // SPI SPI Interrupt Flag
        
        
        :         // Clobbers
                // (no clobbers)
        
        
  );
        
}

inline static void test2( const void *buf , unsigned int len ) {
  
  unsigned char scratch;
  
  asm volatile (
  

          "ld %[scratch],%a[buf]+   \n\t"                 
          "out %[spdr],%[scratch]   \n\t"       // Send first byte to prime the flag
        
        "LOOP_LEN_%=:           \n\t"          
          "ld %[scratch],%a[buf]+   \n\t"       // Get next byte ready
        "WAIT_TRANS_%=:           \n\t"         // Wait for transmission to complete
          "in __tmp_reg__, %[spsr]  \n\t"
          "sbrs __tmp_reg__, %[spif]  \n\t"
          "rjmp WAIT_TRANS_%=       \n\r"
          "out %[spdr],%[scratch]   \n\t"       // I think this is the fastest sequence to test a bit and stote a byte
          "sbiw %[len], 1         \n\t"
          "brne LOOP_LEN_%=       \n\t"       

        "WAIT_POST_%=:           \n\t"   // Wait for final byte to complete
          "in __tmp_reg__, %[spsr]  \n\t"
          "sbrs __tmp_reg__, %[spif]  \n\t"
          "rjmp WAIT_POST_%=       \n\r"
        
              :         // Outputs: (these are actually inputs, but we mark as read/write output since they get changed during execution)
                        // "there is no way to specify that input operands get modified without also specifying them as output operands."                
          [buf]   "+e" (buf),         // pointer to buffer 
          [len]   "+w" (len),         // length of buffer
        [scratch] "+r" (scratch)        // A scratch register to preload the byte we are ready to send
                        
              :         // Inputs:
          "[buf]" (buf),              // pointer to buffer
          "[len]" (len),              // length of buffer
        
        
        
        [spdr]  "I" (_SFR_IO_ADDR(SPDR)),   // SPI data register
        [spsr]  "I" (_SFR_IO_ADDR(SPSR)),   // SPI status register (we only need SPIF bit)
        [spif]  "M" (SPIF)            // SPI SPI Interrupt Flag
        
        
        :         // Clobbers
        "cc"                  // special name that indicates that flags may have been clobbered 
        
        
  );

}

inline static void test3( const void *buf , unsigned int len ) {

  if (len == 0) return;     // Do nothin if len zero
  
  unsigned char scratch;
  
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

          "nop                      \n\t"       //     1
        
              :         // Outputs: (these are actually inputs, but we mark as read/write output since they get changed during execution)
                        // "there is no way to specify that input operands get modified without also specifying them as output operands."
                
          [buf]   "+e" (buf),         // pointer to buffer 
          [len]   "+w" (len)          // length of buffer
        
                
              :         // Inputs:
          "[buf]" (buf),              // pointer to buffer
          "[len]" (len),              // length of buffer
        
        
        
        [spdr]  "I" (_SFR_IO_ADDR(SPDR)),   // SPI data register
        [spsr]  "I" (_SFR_IO_ADDR(SPSR)),   // SPI status register (we only need SPIF bit)
        [spif]  "M" (SPIF)                  // SPI SPI Interrupt Flag
        
        
        :         // Clobbers
        "cc"                  // special name that indicates that flags may have been clobbered 
        
        
  );

}

void setup() {
  // put your setup code here, to run once:


  SPI_MasterInit();

  DDRD = _BV(6);
}




void loop() {
    
      char *data = "SPI is fun";
     unsigned len = strlen( data );

     uint64_t josh=0;


     cli();     // Don't let an interrupt happen in the middle and slow us down

     PORTD |= _BV(6);   // Select slave


/*
    // Normal = 15.12us
   
     while (--len) {
      while(!(SPSR & (1<<SPIF)));      // Wait until previous byte complete
      SPDR = *(data++);
     }
*/

/*
      // Non optimized asm = 16.24
      
      test1( data , --len );
*/


/*      
      // optimized loop asm = 15.76
      
      test2( data , --len );
*/

      // optimized straight asm = 12.24
      
      test3( data , len );

     
   //  while(!(SPSR & (1<<SPIF)));      // Wait until final byte complete

     PORTD &= ~_BV(6);

     sei();

    delay(1000);
     
}
