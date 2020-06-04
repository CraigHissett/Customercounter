
#include "AsyncSonarLib.h"
//Library allows ping sensors to require only arduino 1 pin for use;
//It also allows for Asynchronous function, allowing all sensors to continue to be checked while Counter calculations take place

int custCount;
int custMax;
void PingRecieved(AsyncSonar&);
void TimeOut0(AsyncSonar&);
void TimeOut1(AsyncSonar&);
void TimeOut3(AsyncSonar&);
void TimeOut4(AsyncSonar&);

AsyncSonar InA(A0, PingRecieved, TimeOut);
AsyncSonar InB(A1, PingRecieved, TimeOut);
AsyncSonar OutA(A2, PingRecieved, TimeOut);
AsyncSonar OutB(A3, PingRecieved, TimeOut);

// ping complete callback

void PingRecieved(AsyncSonar& sonar)
{
  //Add in values to track which sensor triggered.
  // If 0 triggered then 1, then cutomer has entered/exited
  //At the moment it just returns the measurement
  Serial.print("Ping");
  Serial.print(&sonar);
  Serial.print(": ");
  Serial.println(sonar.GetMeasureMM());
}

// timeout callback
void TimeOut(AsyncSonar& sonar)
{
  Serial.println("TimeOut");
}

void setup()
{
  Serial.begin(115200);

  custMax = 10;
  custCount = 0;

  InA.Start(500); 
  InB.Start(500); 
  OutA.Start(500);
  OutB.Start(500);
}

void loop()
{
  InA.Update(&InB);
  InB.Update(&OutA);
  OutA.Update(&OutB);
  OutB.Update(&InA);
}
