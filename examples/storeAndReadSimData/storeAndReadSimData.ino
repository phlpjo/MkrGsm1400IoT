/*
  storeAndReadSimData

  This example sketch depneds on the "arduino-libraries/MKRGSM".
  Since the Arduino MKR GSM 1400 has no non-volatile memory,
  the only way to cache data is to store it into SIM card.
  This example sketch writes data to the phonebook, like writing data to an EEPROM
  of the sim card and reads it out and displays it on the serial monitor.
  So, for example, in your own code, control commands can be read from the phone book
  after a restart and used in the program code.

  but attention: the sim card memory has a limited read-write cycle.
  no warranty for defective sim cards.
  use these functions with care.

  define your sim pin in the arduino_secrets.h

  created 23. Sep 2020
  by Philip Decker
*/


#include "Modem.h"
#include "MkrGsm1400IoT.h"


String response;


void resetUblox() {
  pinMode(GSM_RESETN, OUTPUT);
  digitalWrite(GSM_RESETN, HIGH);
  delay(100);
  digitalWrite(GSM_RESETN, LOW);
}

int unlockSim(String pin) {
  MODEM.sendf("AT+CPIN=\"%s\"", pin.c_str());
  if (MODEM.waitForResponse(10000) == 1) {
    return 1; //sim unlocked
  }
  int checkUp = checkSimStatus();
  if (checkUp == -1) {
    return -1; //wrong pin
  }
  if (checkUp == -2) {
    return -2; //need puk
  }
  return -3;
}



int checkSimStatus () {
  for (unsigned long start = millis(); (millis() - start) < 1000;) {
    MODEM.send("AT+CPIN?");
    if (MODEM.waitForResponse(10000, &response) == 1) {
      if (response.startsWith("+CPIN: ")) {
        if (response.endsWith("READY")) {
          return 1; //SIM ready
        } else if (response.endsWith("SIM PIN")) {
          return -1; // sim pin required
        } else if (response.endsWith("SIM PUK")) {
          return -2; // sim puk required
        } else {
          return -3; // other fail.
        }
      }
    }
  }
}


int writeSim (uint8_t adress, uint16_t input_data_uint16_t) {
  if (checkSimStatus() == 1) {
    MODEM.sendf("AT+CPBW=%d,\"%d\",,\"%d\"", adress, input_data_uint16_t, input_data_uint16_t );
    if (MODEM.waitForResponse(10000, &response) == 1) {
      if (response.lastIndexOf("OK\r\n")) {
        return 1; //successfull write data to phonebook
      } else {
        return -1;  //error to write data
      }
    }
  } else {
    return -2; //sim not ready
  }
}


int readSim (uint8_t adress) {
  char data_string[50];
  int i1, i2, i3, i4;
  if (checkSimStatus() == 1) {
    MODEM.sendf("AT+CPBR=%d", adress);
    if (adress > 250 || adress < 0) {
      if (Serial.available()) {
        Serial.println("insert a valid address 0 ... 250 ");
      }
    }
    if (MODEM.waitForResponse(10000, &response) == 1) {
      if (response.startsWith("+CPBR:")) {
        response.toCharArray(data_string, 50);
        if (4 == sscanf(data_string,
                        "%*[^0123456789]%d%*[^0123456789]%d%*[^0123456789]%d%*[^0123456789]%d",
                        &i1,
                        &i2,
                        &i3,
                        &i4))
        {
          return i2;
        }
      }
    }
  } else if (Serial.available()) {
    Serial.println("sim not ready");
  }
}
