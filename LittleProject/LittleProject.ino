//  Connect IN /Data to GPIO2 or D4 on Nodemcu / Wemos
//

#include <NeoPixelBus.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ESP8266HTTPClient.h>



Adafruit_BME280 bme; // I2C

const char* ssid = "FMI-AIR-316";
const char* password = "";

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

const uint16_t PixelCount = 6; // this example assumes 4 pixels, making it smaller will cause a failure
//const uint8_t PixelPin = 2;  // make sure to set this to the correct pin, ignored for Esp8266


#define Cred     RgbColor(255, 0, 0)
#define Cpink    RgbColor(255, 0, 128)
#define Clila    RgbColor(255, 0, 255)
#define Cviolet  RgbColor(128, 0, 255)
#define Cblue    RgbColor(0, 0, 255)
#define Cmblue   RgbColor(0, 128, 255)
#define Ccyan    RgbColor(0, 255, 255)
#define Cgreen   RgbColor(0, 255, 0)
#define Cyellow  RgbColor(255, 255, 0)
#define Corange  RgbColor(255, 100, 0)
#define Cwhite   RgbColor(255, 255, 255)
#define Cblack   RgbColor(0) 
RgbColor allColors[] = {Cred, Clila, Cmblue, Cgreen, Cyellow, Corange}; 
const char* names[] = {"RED" , "" , "" , "GREEN" };
// Uart method is good for the Esp-01 or other pin restricted modules
// NOTE: These will ignore the PIN and use GPI02 pin
NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod> strip(PixelCount, D4);
//NeoPixelBus<NeoRgbFeature, NeoEsp8266Uart400KbpsMethod> strip(PixelCount, PixelPin);


void setup()
{
    Serial.begin(115200);
    while (!Serial); // wait for serial attach

    Serial.println();
    Serial.println("Initializing...");
    Serial.flush();
    
   // Connect to WiFi network
   Serial.println();
   Serial.println();
   Serial.print("Connecting to ");
   Serial.println(ssid);
  
   WiFi.begin(ssid, password);
  
   while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
   }
   Serial.println("");
   Serial.println("WiFi connected");
  
  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());

  bool status;

    Wire.begin(D6,D7);
    status = bme.begin(0x76);
    
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1);
    }

    
    // this resets all the neopixels to an off state
    strip.Begin();
    strip.Show();

    Serial.println();
    Serial.println("Running...");
}


bool TurnTheLEDon(const String& req,String& resp){


   int choice;
  if (req.indexOf("/red/1") != -1)
    choice = 0;
  else if (req.indexOf("/green/1") != -1)
    choice = 3;
  else 
    return false;
  
   RgbColor c = allColors[choice]; //the color
   c = RgbColor::LinearBlend(c, Cblack, 0.9F); //the brigthness 
   strip.SetPixelColor(1, c);
   strip.Show();
      
   delay(5000);   
   Serial.println("Off ...");
   strip.SetPixelColor(1, Cblack);
   strip.Show();
   delay(100);

   resp += names[choice];
   resp += "LED now is ON<br>";
   return true;

}

void sendHTTPCall(float t) {
  
  // wait for WiFi connection
  if ((WiFi.status() == WL_CONNECTED)) {

    HTTPClient http;

   Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    //http.begin("https://192.168.1.12/test.html", "7a 9c f4 db 40 d3 62 5a 6e 21 bc 5c cc 66 c8 3e a1 45 59 38"); //HTTPS
    String s = "http://api.thingspeak.com/update?key=NTXK72ZTV9L6FXCU&field1=";
    s+= t;
    http.begin(s); //HTTP

    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }

  delay(10000);
}




void loop()
{


  // Check if a client has connected
  Serial.println("waiting for client ...");
  WiFiClient client ;
  do{
     client= server.available();
  }while(!client);
  
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
  
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();

  bool isValidReq = false;
  
  String resp = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n";
  
  if ((req.indexOf("/temp") != -1) || (req.indexOf("/temp/") != -1)){

    float temp = bme.readTemperature();
    resp += "current temperatute is around : ";
    resp+= temp;
    resp+=" *C<br>";
    isValidReq = true;
    
  }

  if ((req.indexOf("/upload") != -1) || (req.indexOf("/upload/") != -1)){

    sendHTTPCall(bme.readTemperature());
    resp += "uploading successful<br>";
    isValidReq = true;
    
  }
  
  if (req.indexOf("/LED/") != -1){
    
    isValidReq = TurnTheLEDon(req,resp);
    
  }
  

  if(!isValidReq){
    Serial.println("invalid request");
    client.print("HTTP/1.1 404 NOT FOUND\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nInvalid Request!!!");
    client.stop();
  }else
      client.print(resp);
   
}
