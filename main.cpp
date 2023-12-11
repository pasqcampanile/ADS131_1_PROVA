/*
 * Test lettura ADC (ADS131...) per nuovo sensore di forza NDT
 * Nucleo f767zi
 */

#include "mbed.h"
#include <cstdint>
#include <cstdio>
#include <string>
using namespace std;


#define SENSOR_RATE     2ms //500 Hz
#define PC_BAUD_RATE  115200


//force sensor
#define SENSOR_VDD 3.3
#define MOSI    PE_6    
#define MISO    PE_5     
#define CLK     PE_2      
#define CS      PE_4   

constexpr char unlock[3]            = {0x06, 0x55, 0x00};  // Comando Unlock 000 0110 0101 0101
constexpr char wakeup[3]            = {0x00, 0X33, 0X00};  // risponde 0033
constexpr char reset[3]             = {0x11, 0X00, 0X00};  //risponde con ff26
constexpr char resetCRC[3]          = {0X1F, 0X1F, 0X00};
constexpr char dummyCmd[3]          = {0x00, 0X00, 0X00};
constexpr char readClockCmd[3]      = {0xA1, 0X80, 0X00};  //read clock register

constexpr char writeClockCmd[3]     = {0x61, 0X80, 0X00};  //write clock register
constexpr char ClockMsg[3]          = {0x3F, 0X1A, 0X00};  //msg da inviare al clock per 500SPS

constexpr char write_osr_500sps[6]          = {0x61, 0X80, 0X00, 0x3F, 0X1A, 0X00};
constexpr char write_osr_1000sps[6]         = {0x61, 0X80, 0X00, 0x3F, 0X16, 0X00};
constexpr char write_osr_2000sps[6]         = {0x61, 0X80, 0X00, 0x3F, 0X12, 0X00};
constexpr char write_osr_4000sps[6]         = {0x61, 0X80, 0X00, 0x3F, 0X0E, 0X00};

constexpr char readidregCmd[3]      = {0x00, 0Xa0, 0X00};
constexpr char readGAIN1_REGCmd[3]  = {0xA2, 0X00, 0X00};  //RESET(0000h)
constexpr char writeGAIN1_REGCmd[3] = {0x62, 0X00, 0X00};  //echo 0x42 0x00
constexpr char write4GAIN1_REGCmd[3]= {0x11, 0X11, 0X00};  //


double codToNperVolt[6] = {0.021366862951596, 0.021957198382339, 0.019318183316805, 0.000262142472899, 0.000266067418322, 0.000386155074803};
double voltage = SENSOR_VDD;
double codToN[6];
char status[3]          = {0x00, 0X00, 0X00}; // Comando null -> risponde con status register
char readCommand1[24]   = {0};

int32_t offset[6];




float  wrench[6]; //
uint8_t buff[25];
static BufferedSerial pc(USBTX, USBRX);


SPI spi(MOSI, MISO, CLK);
DigitalOut cs(CS);


/* buff_out len 1+24 =25, meas len 6, meas dim 4 B (float) */
void serialize( uint8_t* buffer, const float* wrench){
    buffer[0]  = '$';
    uint32_t val =*((uint32_t*)&wrench[0]);
    buffer[1]  = val >> 24;
    buffer[2]  = val >> 16;
    buffer[3]  = val >> 8;
    buffer[4]  = val;
    val =*((uint32_t*)&wrench[1]);
    buffer[5]  = val >> 24;
    buffer[6]  = val >> 16;
    buffer[7]  = val >> 8;
    buffer[8]  = val;
    val =*((uint32_t*)&wrench[2]);
    buffer[9]  = val >> 24;
    buffer[10] = val >> 16;
    buffer[11] = val >> 8;
    buffer[12] = val;
    val =*((uint32_t*)&wrench[3]);
    buffer[13] = val >> 24;
    buffer[14] = val >> 16;
    buffer[15] = val >> 8;
    buffer[16] = val;
    val =*((uint32_t*)&wrench[4]);
    buffer[17] = val >> 24;
    buffer[18] = val >> 16;
    buffer[19] = val >> 8;
    buffer[20] = val;
    val =*((uint32_t*)&wrench[5]);
    buffer[21] = val >> 24;
    buffer[22] = val >> 16;
    buffer[23] = val >> 8;
    buffer[24] = val;
}



int main() {

    pc.set_baud(PC_BAUD_RATE);

    // INIT
    for(int i = 0; i<6;i++){
        codToN[i] = codToNperVolt[i]/voltage;
    }

    spi.format(8, 1); // Formato: 8 bit di dati, modalità 1
    spi.frequency(1000000); // Frequenza SPI
    cs = 0; // Attiva il chip select
    
    
    
    int32_t hex_val;
    

//******INIT******reset-unlock-wakeup-readclock(check 3F0Eh)
    
    ThisThread::sleep_for(2000ms);
//read clock reg
    spi.write(readClockCmd,3,readCommand1,24);
    spi.write(nullptr,0,readCommand1,24);
    printf("\n\nClock_reg: %02x %02x %02x\n", readCommand1[0], readCommand1[1], readCommand1[2]);
    ThisThread::sleep_for(2000ms);

//write clock reg ( ? )
    printf("writing clock register...");    
    spi.write(write_osr_500sps,6,readCommand1,24);
    spi.write(nullptr,0,readCommand1,24);
    printf("\n\nResponse: %02x %02x %02x\n", readCommand1[0], readCommand1[1], readCommand1[2]);
    ThisThread::sleep_for(500ms);



    spi.write(readClockCmd,3,readCommand1,24);
    spi.write(nullptr,0,readCommand1,24);
    printf("\n\nClock_reg: %02x %02x %02x\n", readCommand1[0], readCommand1[1], readCommand1[2]);
    ThisThread::sleep_for(2000ms);



    spi.write(status, 3, readCommand1, 24);

 

// void compute_offset()
    int N_sample = 100;
    for (int i=0; i<N_sample; ++i) {
        spi.write(status, 3, readCommand1, 24);

        for(int j=0;j<6;j++){
            hex_val = ((readCommand1[3+j*3] << 16) | (readCommand1[4+j*3] << 8) | (readCommand1[5+j*3])) & 0x00FFFFFF;
            if(hex_val > 0x7FFFFF) hex_val = - ((~hex_val & 0x00FFFFFF) + 1);  // twos complement

            offset[j] += hex_val; //TODO potrebbe essere in float
        }
        
    }
    
    for (int j =0;j<6;j++){
        offset[j] /= N_sample;
    }

    printf("\nOffset =[ ");     //debug
    for (int j =0;j<6;j++){
        printf("%d \t",offset[j]);
    }
    printf("]\n\n");


    
    while (1) {

        spi.write(status, 3, readCommand1, 24);
               
        for(int j=0;j<6;j++){
            hex_val = ((readCommand1[3+j*3] << 16) | (readCommand1[4+j*3] << 8) | (readCommand1[5+j*3])) & 0x00FFFFFF;
            if(hex_val > 0x7FFFFF) hex_val = - ((~hex_val & 0x00FFFFFF) + 1);  // twos complement

            wrench[j] =(hex_val-offset[j])*codToN[j];
        }

        // printf("\n Wrench =[ ");     //debug
        // for (int j =0;j<6;j++){
        //     printf("%f \t",wrench[j]);
        // }
        // printf("]\n\n");
        
        serialize( buff, wrench);
        pc.write(&buff, 25);

        ThisThread::sleep_for(SENSOR_RATE);
    }
    return 0;
}
