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

#define SONAR_NUM 4             // Number of sensors in use
#define MAX_DISTANCE 200        // Max Distance. Use this to set the width of monitored doorways; create more if doors different
#define OVER_CAPACITY 0        // Pin for relay- to be turned high if premises at capacity
#define UNDER_CAPACITY 1       // Pin for relay- to be turned high if premises under capacity

int custCount;
int custMax;
boolean custEntering;
boolean custLeaving;

NewPing sonar[SONAR_NUM] = {    // Sensor object array.
  NewPing(4, 5, MAX_DISTANCE),   
  NewPing(6, 7, MAX_DISTANCE), 
  NewPing(8, 9, MAX_DISTANCE),
  NewPing(10,11, MAX_DISTANCE)
};

void setup() {
  Serial.begin(115200);
  
  pinMode(OVER_CAPACITY, OUTPUT);
  pinMode(UNDER_CAPACITY, OUTPUT);
  digitalWrite(OVER_CAPACITY, LOW);
  digitalWrite(UNDER_CAPACITY, LOW);
  
  custMax = 10;
  custCount = 0;
  custEntering = false;
  custLeaving = false;
}

void loop() { 
  for (uint8_t i = 0; i < SONAR_NUM; i++) { // Loop through each sensor and display results.
    delay(50); // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
    Serial.print(i);
    Serial.print("=");
    Serial.print(sonar[i].ping_cm());
    Serial.print("cm ");
    if(sonar[i].ping_cm()<MAX_DISTANCE)
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
        if (custEntering == true) 
        {
          //customer entering
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
        if (custLeaving == true) 
        {
          //customer leaving
          custCount--;
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
}
