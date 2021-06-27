/**************************************************************
ECU Prototype Ver. 0.1.1 - Arduino
-Handles Analog INPUT from sensors (pH,EC)
-Calculates a readable form of sensor data
-Sends sensor data to Pi via serial periodically
-Handels OUTPUT to Relay Box
-Recieves commands from Pi regarding each relays data line

Serial communication is formatted as single alphanumeric command (1, P, etc.) followed by new line ('\n')
**************************************************************/
#include <SoftwareSerial.h>

SoftwareSerial linkSerial(2, 3); // RX, TX soft serial pins

//*************************************************************
//Constants

//Define Analog Sensor pins
#define EC_PIN_NUM 1
#define PH_PIN_NUM 2
//Define Relay OUTPUT pins
#define relay1_pin 7 //Define pin for closed socket
#define relay2_pin 6 //pin for humidifier
#define relay3_pin 5 //pin for heater
#define relay4_pin 4 //pin for ac unit

int ecPin = A1; //The analog input pin used by the EC sensor //idk why he put this in here
int phPin = A2; //Thw analog input pin used by the PH sensor


//*************************************************************
//Analog Sensor states

//Variables to store the current values
float finalPhValue = 0;
float finalEcValue = 0;
//Define variables used to calculate 
int rawPhReading = 0, rawEcReading = 0;
int processedEcValue = 0, processedPhValue = 0;


//*************************************************************

//Timing states - delay is the frequency of sensor polling
unsigned long debounceTime = 0;
unsigned long debounceDelay = 2000;

//place to hold relay commands from pi (i.e. 1,2,3,4,A,B,C,D)
String commandString;


//*************************************************************
void setup(){
  Serial.begin(57600); //hard serial higher baud than soft serial
  while (!Serial) continue; //continue if no hard serial
  
  pinMode(relay1_pin, OUTPUT);
  pinMode(relay2_pin, OUTPUT);
  pinMode(relay3_pin, OUTPUT);
  pinMode(relay4_pin, OUTPUT);

 // pinMode(pHsensor_pin,INPUT); //I dont think I need to specify analog pins
 // pinMode(ECsensor_pin,INPUT);

  linkSerial.begin(9600);
}

//*************************************************************
void loop(){
  unsigned long currentTime = millis(); //variable to keep time every loop
  
  if (currentTime - debounceTime > debounceDelay) { // timer to delay polling of sensors
    debounceTime = currentTime;

    ProcessPH(true); //check sensor, calculate and send data to pi
    ProcessEC(true);
  }

  if (linkSerial.available() > 0) {  //check if datas on the line
    String data = linkSerial.readStringUntil('\n'); //look for data ending in new line
    
    commandString = data; //hold the recieved string in variable
  }
}

//*************************************************************
// Mostly pasted from manufacturer example
void ProcessPH(bool printToConsole){
  rawPhReading = analogRead(PH_PIN_NUM);
  processedPhValue = map(rawPhReading, 0.00, 1023, 0, 14.00);
  finalPhValue = rawPhReading*14.00/1024;
  
  if (printToConsole){ //added 'send command to pi' to if statement 
    linkSerial.print('P'); // this will be the switch case for pi know which sensor
    linkSerial.print(finalPhValue);
    linkSerial.print('\n');

    Serial.print("PH > ");
    PrintParamsToConsole(rawPhReading, processedPhValue, finalPhValue);
  }
}

//*************************************************************
void ProcessEC(bool printToConsole){
  rawEcReading = analogRead(EC_PIN_NUM);
  processedEcValue = map(rawEcReading, 0, 1023, 0, 5000);
  finalEcValue = rawEcReading*5.00 /1024;
  //Readible EC
  finalEcValue = finalEcValue / 1.00;
  
  if (printToConsole){
    linkSerial.print('E'); // this will be the switch case for pi know which sensor
    linkSerial.print(finalEcValue);
    linkSerial.print('\n');

    Serial.print("EC > ");
    PrintParamsToConsole(rawEcReading, processedEcValue, finalEcValue);
  }
}

//*************************************************************
void PrintParamsToConsole(int rawReading, int processedValue, float finalCalculation){
  Serial.print("");
  Serial.println(finalCalculation);
  Serial.print("");
}

//*************************************************************
//Function to interperet commands from Pi
void processSerialIn() {
  int commandInt = commandString.toInt(); //Convert string to int
  
  switch (commandInt) { //switch only understands int and char
      case '1':
        digitalWrite(relay1_pin, HIGH); //case1 and caseA connect to plug1 and so on
        break;
      case '2':
        digitalWrite(relay2_pin, HIGH);
        break;
      case '3':
        digitalWrite(relay3_pin, HIGH);
        break;
      case '4':
        digitalWrite(relay4_pin, HIGH);
        break;
      case 'A':
        digitalWrite(relay1_pin, LOW); //numbers are ON - alphas are OFF
        break;
      case 'B':
        digitalWrite(relay2_pin, LOW);
        break;
      case 'C':
        digitalWrite(relay3_pin, LOW);
        break;
      case 'D':
        digitalWrite(relay4_pin, LOW);
        break;
      default:  //do nothing - should I make the default something?
        ;
        break;
  }
}
