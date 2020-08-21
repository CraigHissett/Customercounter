#include <SPI.h>
#include <WiFiNINA.h>
#include <NewPing.h>

/*  Customer Counter Conecpt
 * Use four ping sensors to monitor customers entering or exiting premises.
 * Using two on each door can allow for determining the direction of travel based on the order they are triggered;
 * which will allow for the capture of those leaving via the wrong door.
 * The triggering of the sensors will result in a customer counter being increased or decreased.
 * At the end of a loop thisvalue will be checkedand used to toggle a pin (or a number of pins) high/low.
 * This pin can then be used to switch a relay on/off to use lights or other measures to indicate the premises is at capacity.
 * Future update will implement an RFID reader. This will allow staff members to scan a card before passing the sensor and not affect the customer total
 */
 
#include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                 // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
WiFiServer server(80);

#define SONAR_NUM 4             // Number of sensors in use
#define MAX_DISTANCE 25        // Max Distance. Use this to set the width of monitored doorways; create more if doors different
#define OVER_CAPACITY 4        // Pin for relay- to be turned high if premises at capacity
#define UNDER_CAPACITY 5       // Pin for relay- to be turned high if premises under capacity

int custCount;
int custMax;
boolean custEntering;
boolean custLeaving;

NewPing sonar[SONAR_NUM] = {    // Sensor object array.
  NewPing(10, 11, MAX_DISTANCE),
  NewPing(6, 7, MAX_DISTANCE),   
  NewPing(12,13, MAX_DISTANCE),
  NewPing(8, 9, MAX_DISTANCE)
};

void setup() {
  Serial.begin(115200);
  
  pinMode(OVER_CAPACITY, OUTPUT);
  pinMode(UNDER_CAPACITY, OUTPUT);
  digitalWrite(OVER_CAPACITY, LOW);
  digitalWrite(UNDER_CAPACITY, LOW);
  
  custMax = 50;
  custCount = 0;
  custEntering = false;
  custLeaving = false;

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
  server.begin();                           // start the web server on port 80
  printWifiStatus();                        // you're connected now, so print out the status
}


void loop() {
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("Max capacity is currently: ");
            client.print(custMax + "<br>");
            client.print("Click <a href=\"/maxup\">here</a> to increase max capacity<br>");
            client.print("Click <a href=\"/maxdown\">here</a> to decrease max capacity<br><br>");
            client.print("current count is ");
            client.print(custCount + "<br>");
            client.print("Click <a href=\"/add\">here</a> to increase count<br>");
            client.print("Click <a href=\"/subtract\">here</a> to decrease decrease count<br><br>");



            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /add")) {
          custCount++;
        }
        if (currentLine.endsWith("GET /subtract")) {
         custCount--;
        }
        if (currentLine.endsWith("GET /maxup")) {
         custMax++;
        }
        if (currentLine.endsWith("GET /maxdown")) {
         custMax--;
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
  for (uint8_t i = 0; i < SONAR_NUM; i++) { // Loop through each sensor and display results.
    delay(50); // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
    Serial.print(i);
    Serial.print("=");
    Serial.print(sonar[i].ping_cm());
    Serial.print("cm ");
    if(sonar[i].ping_cm()<MAX_DISTANCE && sonar[i].ping_cm()>0)
    {
      //Triggered - sonar[0] and sonar[1] for entrance, sonar[2] and sonar[3] for exit.
      //[1] and [3] will act as the main sensors for each door and run the logic required to change the cuntomer counts.
      //[0] and [2] are placed after the main sensors, and can be used to determine if a patron is heading the wrong way.
      //If these secondary ones are triggered before the main sensor then it signals the customer has gone the wrong way, and will adjust the
      //counter accordingly.
      
      if (i == 0)
      {
        custEntering = true;
      }
      if (i == 2)
      {
        custLeaving = true;
      }
      if (i == 1)
      {
        custEntering = true;
        if (custEntering == true) 
        {
          //customer entering
          Serial.println("Customer in");
          custCount++;
        }
        else
        {
          custCount--;
        }
        custEntering = false;      
      }
      if (i ==3)
      {
        custLeaving = true;
        if (custLeaving == true) 
        {
          //customer leaving
          custCount--;
          Serial.println("Customer out");
        }
        else
        {
          custCount++;
        }
        custLeaving = false;
      }      
    }
  }
  Serial.println();
  CapacityCheck();
}

void CapacityCheck(){
  Serial.println(custCount);
 if(custCount < custMax)
 {
  //All good
  digitalWrite(OVER_CAPACITY, HIGH);
  digitalWrite(UNDER_CAPACITY, LOW);
 }
 else
 {
  //At capacity
  Serial.println("MAX CAPACITY REACHED");
  digitalWrite(OVER_CAPACITY, LOW);
  digitalWrite(UNDER_CAPACITY, HIGH); 
 }
 delay(1000);
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}
