#ifndef _FORCETORQUESENSOR_H_
#define _FORCETORQUESENSOR_H_


#include "mbed.h"

#define SENSOR_RATE_OFFSET     2ms //500 Hz

//force sensor
#define SENSOR_VDD 5.00
#define MOSI    PE_6    
#define MISO    PE_5     
#define CLK     PE_2      
#define CS      PE_4   



class ForceTorqueSensor {
    private:
        const char unlock[3]            = {0x06, 0x55, 0x00};  // Comando Unlock 000 0110 0101 0101
        const char wakeup[3]            = {0x00, 0X33, 0X00};  // risponde 0033
        const char reset[3]             = {0x11, 0X00, 0X00};  //risponde con ff26
        const char resetCRC[3]          = {0X1F, 0X1F, 0X00};
        const char dummyCmd[3]          = {0x00, 0X00, 0X00};
        const char readClockCmd[3]      = {0xA1, 0X80, 0X00};  //read clock register

        const char writeClockCmd[3]     = {0x61, 0X80, 0X00};  //write clock register
        const char ClockMsg[3]          = {0x3F, 0X1A, 0X00};  //msg da inviare al clock per 500SPS

        const char write_osr_500sps[6]          = {0x61, 0X80, 0X00, 0x3F, 0X1A, 0X00};
        const char write_osr_1000sps[6]         = {0x61, 0X80, 0X00, 0x3F, 0X16, 0X00};
        const char write_osr_2000sps[6]         = {0x61, 0X80, 0X00, 0x3F, 0X12, 0X00};
        const char write_osr_4000sps[6]         = {0x61, 0X80, 0X00, 0x3F, 0X0E, 0X00};

        const char readidregCmd[3]      = {0x00, 0Xa0, 0X00};
        const char readGAIN1_REGCmd[3]  = {0xA2, 0X00, 0X00};  //RESET(0000h)
        const char writeGAIN1_REGCmd[3] = {0x62, 0X00, 0X00};  //echo 0x42 0x00
        const char write4GAIN1_REGCmd[3]= {0x11, 0X11, 0X00};  //

        const char status[3]          = {0x00, 0X00, 0X00}; // Comando null -> risponde con status register

        const double codToNperVolt[6] = {0.021366862951596, 0.021957198382339, 0.019318183316805, 0.000262142472899, 0.000266067418322, 0.000386155074803};  //sensor calibration gain
        
        double codToN[6];  //transduction factor 
        char readCommand1[24] ; //reading buffer
        int32_t offset[6];

        SPI spi;
        DigitalOut cs;

    public:

        ForceTorqueSensor():
        spi(MOSI, MISO, CLK),
        cs(CS){
            for(int i = 0; i<6;i++){
                codToN[i] = codToNperVolt[i]/SENSOR_VDD;
            }

            spi.format(8, 1); // Formato: 8 bit di dati, modalità 1
            spi.frequency(1000000); // Frequenza SPI
            cs = 0; // Attiva il chip select

        }

        void print_clock_register(){
            spi.write(readClockCmd,3,readCommand1,24);
            spi.write(nullptr,0,readCommand1,24);
            printf("\n\nClock_reg: %02x %02x %02x\n", readCommand1[0], readCommand1[1], readCommand1[2]);
            ThisThread::sleep_for(2000ms);
        }

        void set_500sps(){
            spi.write(write_osr_500sps,6,readCommand1,24);
            spi.write(nullptr,0,readCommand1,24);
        }

        void set_1000sps(){
            spi.write(write_osr_1000sps,6,readCommand1,24);
            spi.write(nullptr,0,readCommand1,24);
        }

        void set_2000sps(){
            spi.write(write_osr_2000sps,6,readCommand1,24);
            spi.write(nullptr,0,readCommand1,24);
        }

        void set_4000sps(){
            spi.write(write_osr_4000sps,6,readCommand1,24);
            spi.write(nullptr,0,readCommand1,24);
        }


        void read_force_N(float* w){
            int32_t hex_val;
            spi.write(status, 3, readCommand1, 24);
               
            for(int j=0;j<6;j++){
                hex_val = ((readCommand1[3+j*3] << 16) | (readCommand1[4+j*3] << 8) | (readCommand1[5+j*3])) & 0x00FFFFFF;
                if(hex_val > 0x7FFFFF) hex_val = - ((~hex_val & 0x00FFFFFF) + 1);  // twos complement
                w[j] =(hex_val-offset[j])*codToN[j];
            }
        }

        void compute_offset(int N_sample = 500){
            int32_t hex_val;
            for (int i=0; i<N_sample; ++i) {
                spi.write(status, 3, readCommand1, 24);

                for(int j=0;j<6;j++){
                    hex_val = ((readCommand1[3+j*3] << 16) | (readCommand1[4+j*3] << 8) | (readCommand1[5+j*3])) & 0x00FFFFFF;
                    if(hex_val > 0x7FFFFF) hex_val = - ((~hex_val & 0x00FFFFFF) + 1);  // twos complement

                    offset[j] += hex_val; //TODO potrebbe essere in float
                }
                ThisThread::sleep_for(SENSOR_RATE_OFFSET);
            }
            
            for (int j =0;j<6;j++){
                offset[j] /= N_sample;
            }
        }

        void print_offset(){
            printf("\nOffset =[ ");     //debug
            for (int j =0;j<6;j++){
                printf("%d \t",offset[j]);
            }
            printf("]\n\n");
        }






};


#endif // _FORCETORQUESENSOR_H_