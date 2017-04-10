#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
# include<math.h>       // I like to do math

// DEVCFG0
#pragma config DEBUG = ON // no debugging
#pragma config JTAGEN = OFF // no jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // no write protect
#pragma config BWP = OFF // no boot write protect
#pragma config CP = OFF // no code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = OFF // turn off secondary oscillator
#pragma config IESO = OFF // no switching clocks
#pragma config POSCMOD = HS // high speed crystal mode
#pragma config OSCIOFNC = OFF // free up secondary osc pins
#pragma config FPBDIV = DIV_1 // divide CPU freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // do not enable clock switch
#pragma config WDTPS = PS1048576 // slowest wdt
#pragma config WINDIS = OFF // no wdt window
#pragma config FWDTEN = OFF // wdt off by default
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the CPU clock to 48MHz
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz
#pragma config UPLLIDIV = DIV_2 // divider for the 8MHz input clock, then multiply by 12 to get 48MHz for USB
#pragma config UPLLEN = ON // USB clock on

// DEVCFG3
#pragma config USERID = 0 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations
#pragma config FUSBIDIO = ON // USB pins controlled by USB module
#pragma config FVBUSONIO = ON // USB BUSON controlled by USB module


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

int main() {

    __builtin_disable_interrupts();

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    // do your TRIS and LAT commands here

    __builtin_enable_interrupts();
        
    // Initialize the Timer
    _CP0_SET_COUNT(0);
    int timetoWait = 48000000*0.001/2;
    // Initialize counters for the triangle and sin wave
    int triangleCycleLimit = 200;
    int sinCycleLimit = 100;
    
    int triangleCounter = 0;
    int sinCounter = 0;
    
    // Voltage Level Variables
    unsigned char triangleLevel = 0;
    unsigned char sinLevel = 125;
    
    // Configure Bits
    ANSELA = 0;
    ANSELB = 0;
    
    // Initialize the SPI communication
    initSPI1();

    while(1) {
        
        // Wait for 0.001 s
        if(_CP0_GET_COUNT()>timetoWait)
        {
            _CP0_SET_COUNT(0);
            
            // If counters have exceeded limits, reset them
            if(triangleCounter >=triangleCycleLimit)
            {
                triangleCounter = 0;
            }
            if(sinCounter >= sinCycleLimit)
            {
                sinCounter = 0;
            }
            // Calculate the desired wave levels
            sinLevel = 125+125.0*sin(2*3.1415*(float)sinCounter/(float)sinCycleLimit);
            triangleLevel = 255*(float)triangleCounter/(float)triangleCycleLimit;
            //sinLevel = 125;
            //triangleLevel = 255;
            
            // Call the DAC function in order to write the levels
            setVoltage(0, sinLevel);
            setVoltage(1, triangleLevel);
            
            // Increment Counters
            triangleCounter++;
            sinCounter++;
        }

        
    }
}