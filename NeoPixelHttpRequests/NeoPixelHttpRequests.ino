//  Connect IN /Data to GPIO2 or D4 on Nodemcu / Wemos
//

#include <NeoPixelBus.h>
#include <ESP8266WiFi.h>

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

    // this resets all the neopixels to an off state
    strip.Begin();
    strip.Show();

    Serial.println();
    Serial.println("Running...");
}


void loop()
{

  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
  
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();
  
  // Match the request
  int choice;
  if (req.indexOf("/red/1") != -1)
    choice = 0;
  else if (req.indexOf("/green/1") != -1)
    choice = 3;
  else {
    Serial.println("invalid request");
    client.stop();
    return;
  }

   RgbColor c = allColors[choice]; //the color
   c = RgbColor::LinearBlend(c, Cblack, 0.9F); //the brigthness 
   strip.SetPixelColor(1, c);
   strip.Show();
      
   delay(5000);

   Serial.println("Off ...");
   strip.SetPixelColor(1, Cblack);
   strip.Show();
   delay(100);
   

  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n";
  s += names[choice];
  s += "LED now is ON";
  client.print(s);
  delay(1);
  Serial.println("Client disonnected");
  
  //a short delay then turns the light off
   delay(5000);

   Serial.println("Off ...");
   strip.SetPixelColor(1, Cblack);
   strip.Show();
   delay(100);
   
}
