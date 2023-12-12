#ifndef _SERIALWRENCHSTREAM_H_
#define _SERIALWRENCHSTREAM_H_

#include "mbed.h"

#define PC_BAUD_RATE  115200


class SerialWrenchStream{
    private:
        BufferedSerial pc;
        uint8_t buff[25];

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

    public:

        SerialWrenchStream():
        pc(USBTX, USBRX){
            pc.set_baud(PC_BAUD_RATE);
        }

        void send_wrench(float *w){
            serialize( buff, w);
            pc.write(&buff, 25);
        }


};

#endif // _SERIALWRENCHSTREAM_H_


