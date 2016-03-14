#include <PID_v1.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "stage.h"
#include "stages_manager.h"
#include "states.h"

/*
   TODO:
   1. Make sure we read temperature freom the right device
   2. change return type for temperature to float so we can deal with fractions of deg.
   3. Why is the 0x10 check on device type fails ????
   4. Create debug controllable printouts
*/


DeviceAddress imppThermometer = { 0x28, 0xB1, 0x61, 0x5D, 0x07, 0x00, 0x00, 0xCD };

// Data wire is plugged into pin 3 on the Arduino
#define ONE_WIRE_BUS 3

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);


const int ledPin = 13;       // pin that the LED is attached to


// Define Variables for PID
double  Input, Output;
double Setpoint = 28.0;

// Define control flags
bool isRunning = false;
bool stagesLoaded = false;

//Debug control
bool DEBUG = false;
//bool DEBUG = true;

//Define the aggressive and conservative Tuning Parameters
double aggKp = 4, aggKi = 0.2, aggKd = 1;
double consKp = 1, consKi = 0.05, consKd = 0.25;

//Specify the links and initial tuning parameters
PID myPID(&Input, &Output, &Setpoint, 2, 5, 1, REVERSE);

ImpState state = IMP_READY;
StagesManager stagesManager =  StagesManager();


void setup(void) {
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  
  Serial.begin(9600);
  // Set short timeout so iteratino blocking part is short
  Serial.setTimeout(100);
  
  sensors.begin();
  // set the resolution to 10 bit (good enough?)
  sensors.setResolution(imppThermometer, 10);

  myPID.SetMode(AUTOMATIC);
}



float getTemperature(DeviceAddress deviceAddress)
{
  sensors.requestTemperatures();
  
  float tempC = sensors.getTempC(deviceAddress);
  
  // Check if error read
  if (tempC == -127.00) {
    //TODO (asaf): report error on the serial and make sure the UI can reflect
  }

  return tempC;
}


void setMyPidTuningParameters() {
  double gap = abs(Setpoint - Input); //distance away from setpoint
  if (gap < 10)
  { //we're close to setpoint, use conservative tuning parameters
    myPID.SetTunings(consKp, consKi, consKd);
  }
  else
  {
    //we're far from setpoint, use aggressive tuning parameters
    myPID.SetTunings(aggKp, aggKi, aggKd);
  }
}


void loop(void) {

  delay(1000);

  // Read serial to see if need to stop
  String line = Serial.readString();

  if (DEBUG) {
    Serial.print("State: ");
    Serial.println(state);
    //Serial.println(line);
  }

  // If a line was read there might be something to do
  if (line != "") {

    if (line == "Debug 0\n") {
      DEBUG = false;
      return;

    } else if (line == "Debug 1\n") {
      DEBUG = true;
      return;

    } else if (line == "Stages\n") {
      stagesManager.printStages();
      return;

    } else if (line == "Stop\n") {
      // If got "Stop" means the experiment is over and we reset the states.
      // Clear the stages buffer and reset flags
      stagesManager.resetStages();
      state = IMP_READY;
      return;

    } else if (line == "Done\n") {
      // If got "Done" we shold move to execution mode and start the experiment
      state = IMP_RUNNING;
      //stagesManager.doneLoadingStages = true;

      Stage* pStage = stagesManager.getStage();
      if (pStage != NULL) {
        pStage->startStage();
      }

      return;

    } else if (state != IMP_RUNNING) {
      // If we are here we parse the line as "Temperature#, Time#"
      int idx = line.indexOf(',');
      int tempratureC = line.substring(0, idx).toInt();
      int timeSec = line.substring(idx + 1, line.length() - 1).toInt();

      if ((idx != -1) && (timeSec != 0)) {
        state = IMP_LOADING;
        stagesManager.addStage(tempratureC, timeSec);
      }
    }
  }

  if (state == IMP_RUNNING) {
    // Get stage
    Stage* pStage = stagesManager.getStage();

    // If no available stage we are done.

    if (pStage == NULL) {

      state = IMP_READY;
      stagesManager.resetStages();
      Serial.println("Stages complete");
      return;
    }

    //(TODO asaf: add work here)



  }

  // Report to UI the current temprature
  float curTemp = getTemperature(imppThermometer);
  Serial.print("curTemp: ");
  Serial.println(curTemp);
  //Input = getCelsius();
  /*
  setMyPidTuningParameters();
  bool worked = myPID.Compute();
  Serial.print("Compute worked: ");
  Serial.println(worked);
  Serial.print("Tempratue is: ");
  Serial.println(Input);
  Serial.print("Output is: ");
  Serial.println(Output);
  Serial.print("Setpoint is: ");
  Serial.println(Setpoint);

  analogWrite(13, Output);*/
}



