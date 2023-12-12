/*
 * Test lettura ADC (ADS131...) per nuovo sensore di forza NDT
 * Nucleo f767zi
 */

#include "mbed.h"
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <string>
// using namespace std;
#include "filters.h"
#include "ForceTorqueSensor.h"


#define SENSOR_RATE     2ms //500 Hz
#define PC_BAUD_RATE  115200

float  wrench[6]; //
float  wrench_filtered[6]; //
uint8_t buff[25];
static BufferedSerial pc(USBTX, USBRX);

double f_c = 15; //Hz cutoff
double Ts = 5e-3; 

LPF lowpass(6,f_c,Ts);

ForceTorqueSensor sensor;

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
    sensor.set_500sps();
    sensor.compute_offset(2000);
    
    while (1) {

        sensor.read_force_N(wrench);
        lowpass.filter(wrench, wrench_filtered);


        serialize( buff, wrench_filtered);
        pc.write(&buff, 25);

        ThisThread::sleep_for(SENSOR_RATE);
    }
    return 0;
}
