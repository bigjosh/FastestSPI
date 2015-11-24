/*
  SPI speed tester

  This code tests different strategies for driving the AVR SPI transmitter. 
  It is based on the Arduino DigitalPotControl library example.
  
  * CS - to digital pin 10  (SS pin)
  * SDI - to digital pin 11 (MOSI pin)
  * CLK - to digital pin 13 (SCK pin)

  This first version uses the standard transfer() function, which runs in 14.82us

*/


// inslude the SPI library:
#include <SPI.h>

// set Digital Pin 10 as the slave select and goes low durring transfers
const int slaveSelectPin = 10;
#define SS_DDR  DDRB
#define SS_PORT PORTB
#define SS_BIT  2

void setup() {
  // set the slaveSelectPin as an output:
  SS_DDR |= _BV(SS_BIT);

  // Start with SS pin high to de-select the chip:
  SS_PORT |= _BV(SS_BIT);
  
  // initialize SPI:
  SPI.begin();
  SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE0));
}

// A nice message
static uint8_t message[] = "SPI is fun";

void loop() {

  // Interrupt would be distracting
  cli();  

  // take the SS pin low to select the chip:
  SS_PORT &= ~_BV(SS_BIT);
  
  //  send a nice message
  SPI.transfer( message , sizeof(message)-1 );
  
  // take the SS pin high to de-select the chip:
  SS_PORT |= _BV(SS_BIT);

  // Interrupts back on
  sei();

  delay(100);
}
