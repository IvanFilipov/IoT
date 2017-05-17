#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

#define USE_SERIAL Serial

ESP8266WiFiMulti WiFiMulti;


#define SSID "FMI-AIR-316"
#define PASSWORD ""

void setup() {

    USE_SERIAL.begin(115200);
    // USE_SERIAL.setDebugOutput(true);
    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

    for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }

    pinMode(LED_BUILTIN, OUTPUT);
    
    WiFiMulti.addAP(SSID, PASSWORD);

}


void UPdate(){


    Serial.println("UPDATING");
    // wait for WiFi connection
    if((WiFiMulti.run() == WL_CONNECTED)) {

        t_httpUpdate_return ret = ESPhttpUpdate.update("10.10.52.132",80,"/ESP-gr1/arduino1.bin");
        //t_httpUpdate_return  ret = ESPhttpUpdate.update("https://server/file.bin");

        switch(ret) {
            case HTTP_UPDATE_FAILED:
                USE_SERIAL.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
                break;

            case HTTP_UPDATE_NO_UPDATES:
                USE_SERIAL.println("HTTP_UPDATE_NO_UPDATES");
                break;

            case HTTP_UPDATE_OK:
                USE_SERIAL.println("HTTP_UPDATE_OK");
                break;
        }
    }

  
}


char line[200];

int readLine(int timeout) {
  
  unsigned long deadline = millis() + timeout;
  int i = 0;
  while (millis() < deadline) {
    if (Serial.available()) {
      line[i++] = (char) Serial.read();
      if      (line[i-1] == '\r')  i--;
      else if (i == sizeof(line) - 1)  break;
      else if (line[i-1] == '\n')  {i--; break;}
    }
  }
  line[i] = 0;
  return i;
}


void LedOn(){

  Serial.println("LEDON");
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
                        // wait for a second

    delay(5000);
}

void loop() {


  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(2000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(2000);                       // wait for a second

  Serial.println("3 sec to write");
  if (readLine(3000) > 0)
      UPdate();
  

}

