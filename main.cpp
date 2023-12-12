/*
 * Test lettura ADC (ADS131...) per nuovo sensore di forza NDT
 * Nucleo f767zi
 */

#include "mbed.h"
#include "filters.h"
#include "ForceTorqueSensor.h"
#include "SerialWrenchStream.h"


#define SENSOR_RATE     2ms //500 Hz

float  wrench[6]; 
float  wrench_filtered[6]; 

double f_c = 15; //Hz cutoff
double Ts = 5e-3; 

LPF lowpass(6,f_c,Ts);

ForceTorqueSensor sensor;

SerialWrenchStream send;


int main() {
    sensor.set_500sps();
    sensor.compute_offset(2000);
    
    while (1) {

        sensor.read_force_N(wrench);
        lowpass.filter(wrench, wrench_filtered);

        send.send_wrench(wrench_filtered);

        ThisThread::sleep_for(SENSOR_RATE);
    }
    return 0;
}
