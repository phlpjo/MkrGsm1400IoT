#ifndef MkrGsm1400IoT_H
#define MkrGsm1400IoT_H
#include <Arduino.h>


void    resetUblox();

int     unlockSim(String pin),
        checkSimStatus(), 
        writeSim(uint8_t adress, int input_data),
        readSim(uint8_t adress);


#endif
