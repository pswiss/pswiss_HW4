
#define CS LATBbits.LATB9       // chip select pin
void initSPI1() 
{
    // Initialize the SPI connection
    // Portions of code are taken from the example
    // Set up chip select pin as output
    TRISBbits.TRISB9 = 0;
    CS = 1;
    
    // Setup SPI1
    int rData;
    SPI1CON = 0;
    rData = SPI1BUF;
    
    SPI1BRG = 0x01;
    
    SPI1STATbits.SPIROV = 0;
    SPI1CONbits.CKE = 1;
    SPI1CONbits.MSTEN = 1;
    SPI1CONbits.ON = 1;
    
    
    // Configure Pin Bits
    //Put SD01 on RPA1
    RPA1Rbits.RPA1R = 0b0011;
    // Put SDI1 on RPB8
    SDI1Rbits.SDI1R = 0b0100;
    
    
 }

unsigned char SPI1_IO(unsigned char write)
{
    SPI1BUF = write;

    while(!SPI1STATbits.SPIRBF) 
    { // wait to receive the byte
      ;
    }
    return SPI1BUF;
}

void setVoltage(unsigned char channel, unsigned char voltage)
{
    // Logic to set voltage using SPI
    short message = 0b0011000000000000;
    short mask;
    mask = 0b1000000000000000*channel;
    
    message = message | mask;
    
    mask = 0+voltage;
    mask = mask << 4;
    message = message | mask;
    
    unsigned char msg1 = (message&0xFF00)>>8;
    unsigned char msg2 = message&0x00FF;
    
    
    CS = 0;
    SPI1_IO(msg1); // MSB
    SPI1_IO(msg2); // LSB
    CS = 1;
    
}
