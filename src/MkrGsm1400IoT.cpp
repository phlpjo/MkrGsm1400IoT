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


int writeSim (uint8_t adress, int input_data_uint16_t) {
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
  uint16_t _data = 0;
  int i1, i2, i3, i4;
  if (checkSimStatus() == 1) {
    MODEM.sendf("AT+CPBR=%d", adress);
    if (adress > 250 || adress < 0){
      return -1;
    }
    if (MODEM.waitForResponse(10000, &response) == 1) {
      if (response.startsWith("+CPBR:")) {
        char data_string[50];
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
  } else {
    return -2; //sim not ready
  }
}
