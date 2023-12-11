/*
 * Test lettura ADC (ADS131...) per nuovo sensore di forza NDT
 * Nucleo f767zi
 */

#include "mbed.h"
#include <cstdint>
#include <cstdio>
#include <string>
using namespace std;

#define MOSI    PE_6    
#define MISO    PE_5     
#define CLK     PE_2      
#define CS      PE_4   

constexpr char unlock[3]            = {0x06, 0x55, 0x00};  // Comando Unlock 000 0110 0101 0101
constexpr char wakeup[3]            = {0x00, 0X33, 0X00};  // risponde 0033
constexpr char reset[3]             = {0x11, 0X00, 0X00};  //risponde con ff26
constexpr char resetCRC[3]          = {0X1F, 0X1F, 0X00};
constexpr char dummyCmd[3]          = {0x00, 0X00, 0X00};
constexpr char readClockCmd[3]      = {0xA1, 0X80, 0X00};  
constexpr char readidregCmd[3]      = {0x00, 0Xa0, 0X00};
constexpr char readGAIN1_REGCmd[3]  = {0xA2, 0X00, 0X00}; //RESET(0000h)
constexpr char writeGAIN1_REGCmd[3] = {0x62, 0X00, 0X00}; //echo 0x42 0x00
constexpr char write4GAIN1_REGCmd[3]= {0x11, 0X11, 0X00}; //


SPI spi(MOSI, MISO, CLK);
DigitalOut cs(CS);

int main() {
    spi.format(8, 1); // Formato: 8 bit di dati, modalità 1
    spi.frequency(1000000); // Frequenza SPI

    cs = 0; // Attiva il chip select
    
    char status[3]          = {0x00, 0X00, 0X00}; // Comando null -> risponde con status register
    char readCommand1[24]   = {0};
    char clock_reg[3]       = {0x03, 0x00, 0x00};
    
    int32_t hex_val;
    int32_t ch[6] {0,0,0,0,0,0}; 

//******INIT******reset-unlock-wakeup-readclock(check 3F0Eh)

    spi.write(readClockCmd,3,readCommand1,24);
    spi.write(nullptr,0,readCommand1,24);
    printf("\n\nClock_reg: %02x %02x %02x\n", readCommand1[0], readCommand1[1], readCommand1[2]);
    ThisThread::sleep_for(2000ms);

//Scrittura sul registro GAIN1 -> 4 
    spi.write(readGAIN1_REGCmd,3,readCommand1,24);
    spi.write(nullptr,0,readCommand1,24);  
    printf("GAIN1 Register: "); 
    for (int i=0; i<24; ++i) 
        printf("%02x ", readCommand1[i]);
    printf("\n");
    //printf("GAIN1 Register: %02x %02x %02x\n", readCommand1[0], readCommand1[1], readCommand1[2]);
    ThisThread::sleep_for(2000ms);
/*
    spi.write(writeGAIN1_REGCmd,3,readCommand1,24);
    spi.write(nullptr,0,readCommand1,24);   
    printf("Write GAIN1 Register: %02x %02x %02x\n", readCommand1[0], readCommand1[1], readCommand1[2]);

    spi.write(write4GAIN1_REGCmd,3,readCommand1,24);
    spi.write(nullptr,0,readCommand1,24);   
    printf("Write 4 GAIN1 Register: %02x %02x %02x\n", readCommand1[0], readCommand1[1], readCommand1[2]);
*/
    printf("\n*************\n**START**\n\n\n");

    spi.write(status, 3, readCommand1, 24);
    float fx      = 0.0;
    float fy      = 0.0;
    float fz      = 0.0;
    float meanx   = 0.0;
    float meany   = 0.0;
    float meanz   = 0.0;

    float mx      = 0.0;
    float my      = 0.0;
    float mz      = 0.0;
    float meanmx  = 0.0;
    float meanmy  = 0.0;
    float meanmz  = 0.0;

    float FXsens = (0.00427/5)*3.3;
    float FYsens = (0.00439/5)*3.3;
    float FZsens = (0.00386/5)*3.3;

    for (int i=0; i<100; ++i) {
        spi.write(status, 3, readCommand1, 24);
        hex_val = ((readCommand1[3] << 16) | (readCommand1[4] << 8) | (readCommand1[5])) & 0x00FFFFFF;
        if(hex_val > 0x7FFFFF)    // if(ch[k] & (1 << 23))
            hex_val = - ((~hex_val & 0x00FFFFFF) + 1);  // twos complement
        fx += hex_val;

        hex_val = ((readCommand1[6] << 16) | (readCommand1[7] << 8) | (readCommand1[8])) & 0x00FFFFFF;
        if(hex_val > 0x7FFFFF)    // if(ch[k] & (1 << 23))
            hex_val = - ((~hex_val & 0x00FFFFFF) + 1);  // twos complement
        fy += hex_val;

        hex_val = ((readCommand1[9] << 16) | (readCommand1[10] << 8) | (readCommand1[11])) & 0x00FFFFFF;
        if(hex_val > 0x7FFFFF)    // if(ch[k] & (1 << 23))
            hex_val = - ((~hex_val & 0x00FFFFFF) + 1);  // twos complement
        fz += hex_val;
//****MX MY MZ

        hex_val = ((readCommand1[12] << 16) | (readCommand1[13] << 8) | (readCommand1[14])) & 0x00FFFFFF;
        if(hex_val > 0x7FFFFF)    // if(ch[k] & (1 << 23))
            hex_val = - ((~hex_val & 0x00FFFFFF) + 1);  // twos complement
        mx += hex_val;

        hex_val = ((readCommand1[15] << 16) | (readCommand1[16] << 8) | (readCommand1[17])) & 0x00FFFFFF;
        if(hex_val > 0x7FFFFF)    // if(ch[k] & (1 << 23))
            hex_val = - ((~hex_val & 0x00FFFFFF) + 1);  // twos complement
        my += hex_val;

        hex_val = ((readCommand1[18] << 16) | (readCommand1[19] << 8) | (readCommand1[20])) & 0x00FFFFFF;
        if(hex_val > 0x7FFFFF)    // if(ch[k] & (1 << 23))
            hex_val = - ((~hex_val & 0x00FFFFFF) + 1);  // twos complement
        mz += hex_val;
    }
    meanx=fx/100;
    meany=fy/100;
    meanz=fz/100;
    meanmx=mx/100;
    meanmy=my/100;
    meanmz=mz/100;
    printf("\nMean Fx: \t%4f\n",meanx);
    printf("\nMean Fy: \t%4f\n",meany);
    printf("\nMean Fz: \t%4f\n",meanz);
    printf("\nMean Mx: \t%4f\n",meanmx);
    printf("\nMean My: \t%4f\n",meanmy);
    printf("\nMean Mz: \t%4f\n",meanmz);

    
    while (1) {

        spi.write(status, 3, readCommand1, 24);
        fx=0.0;
        fy=0.0;
        fz=0.0;
        //for (int i=0; i<20; ++i) {
            hex_val = ((readCommand1[3] << 16) | (readCommand1[4] << 8) | (readCommand1[5])) & 0x00FFFFFF;
            if(hex_val > 0x7FFFFF)    // if(ch[k] & (1 << 23))
                hex_val = - ((~hex_val & 0x00FFFFFF) + 1);  // twos complement
            fx+=(hex_val-meanx)*FXsens;
            
            hex_val = ((readCommand1[6] << 16) | (readCommand1[7] << 8) | (readCommand1[8])) & 0x00FFFFFF;
            if(hex_val > 0x7FFFFF)    // if(ch[k] & (1 << 23))
                hex_val = - ((~hex_val & 0x00FFFFFF) + 1);  // twos complement
            fy+=(hex_val-meany)*FYsens;

            hex_val = ((readCommand1[9] << 16) | (readCommand1[10] << 8) | (readCommand1[11])) & 0x00FFFFFF;
            if(hex_val > 0x7FFFFF)    // if(ch[k] & (1 << 23))
                hex_val = - ((~hex_val & 0x00FFFFFF) + 1);  // twos complement
            fz+=(hex_val-meanz)*FZsens;
            wait_us(6);  
        //}
       // fx=fx/20.0;
       // fy=fy/20.0;
       // fz=fz/20.0;
        printf("\tFx: \t%f N",fx);   
        printf("\tFy: \t%f N",fy);
        printf("\tFz: \t%f N\n",fz); 
        


/*
        hex_val = ((readCommand1[3] << 16) | (readCommand1[4] << 8) | (readCommand1[5])) & 0x00FFFFFF;
        if(hex_val > 0x7FFFFF)    // if(ch[k] & (1 << 23))
            hex_val = - ((~hex_val & 0x00FFFFFF) + 1);  // twos complement
        printf("\nFx: %4d",hex_val-meanx);

        hex_val = ((readCommand1[6] << 16) | (readCommand1[7] << 8) | (readCommand1[8])) & 0x00FFFFFF;
        if(hex_val > 0x7FFFFF)    // if(ch[k] & (1 << 23))
            hex_val = - ((~hex_val & 0x00FFFFFF) + 1);  // twos complement
        printf("\t\tFy: %4d",hex_val-meany);

        hex_val = ((readCommand1[9] << 16) | (readCommand1[10] << 8) | (readCommand1[11])) & 0x00FFFFFF;
        if(hex_val > 0x7FFFFF)    // if(ch[k] & (1 << 23))
            hex_val = - ((~hex_val & 0x00FFFFFF) + 1);  // twos complement
        fz=(hex_val-meanz)*FZsens;
        printf("\t\tFz: %f N",fz);

        hex_val = ((readCommand1[12] << 16) | (readCommand1[13] << 8) | (readCommand1[14])) & 0x00FFFFFF;
        if(hex_val > 0x7FFFFF)    // if(ch[k] & (1 << 23))
            hex_val = - ((~hex_val & 0x00FFFFFF) + 1);  // twos complement
        printf("\t\tMx: %4d",hex_val-meanmx);

        hex_val = ((readCommand1[15] << 16) | (readCommand1[16] << 8) | (readCommand1[17])) & 0x00FFFFFF;
        if(hex_val > 0x7FFFFF)    // if(ch[k] & (1 << 23))
            hex_val = - ((~hex_val & 0x00FFFFFF) + 1);  // twos complement
        printf("\t\tMy: %4d",hex_val-meanmy);

        hex_val = ((readCommand1[18] << 16) | (readCommand1[19] << 8) | (readCommand1[20])) & 0x00FFFFFF;
        if(hex_val > 0x7FFFFF)    // if(ch[k] & (1 << 23))
            hex_val = - ((~hex_val & 0x00FFFFFF) + 1);  // twos complement
        printf("\t\tMz: %4d",hex_val-meanmz);
*/
        ThisThread::sleep_for(30ms);
    }
    return 0;
}