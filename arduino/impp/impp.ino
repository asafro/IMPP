#include <PID_v1.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "stage.h"
#include "stages_manager.h"
#include "states.h"


// Data wire is plugged into pin 3 on the Arduino
#define ONE_WIRE_BUS 10

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// Thrmometer address will be resolved in setup
DeviceAddress imppThermometer;

const int peltierPin = 13;       // pin that the LED is attached to


// Define Variables for PID
double  Input, Output;
double Setpoint = 18.0;

// Define control flags
bool isRunning = false;
bool stagesLoaded = false;

//Debug control
bool DEBUG = false;
//bool DEBUG = true;

//Define the aggressive and conservative Tuning Parameters
double aggKp = 100, aggKi = 200, aggKd = 0.05;
double consKp = 50, consKi = 100, consKd = 0.05;

//Specify the links and initial tuning parameters
PID myPID(&Input, &Output, &Setpoint, 2, 5, 1, REVERSE);

ImpState state = IMP_READY;
StagesManager stagesManager =  StagesManager();


void setup(void) {
  // Initrialize the OnwWire
  setOneWireDevices();

  // initialize the LED pin as an output:
  pinMode(peltierPin, OUTPUT);

  Serial.begin(9600);
  // Set short timeout so iteratino blocking part is short
  Serial.setTimeout(100);

  sensors.begin();
  // set the resolution to 10 bit (good enough?)
  sensors.setResolution(imppThermometer, 10);

  myPID.SetMode(AUTOMATIC);
}


void loop(void) {
  delay(1000);

  // Read the current temperature to PID input so we can use later in several places
  Input = getTemperature(imppThermometer);

  // Read serial to see if need to stop
  String line = Serial.readString();

  if (DEBUG) {
    Serial.print("State: ");
    Serial.println(state);
    Serial.println(line);
  }

  // If a line was read there might be something to do
  if (line != "") {

    if (line.startsWith("debug 0")) {
      DEBUG = false;
      return;

    } else if (line.startsWith("debug 1")) {
      DEBUG = true;
      return;

    } else if (line.startsWith("stages")) {
      stagesManager.printStages();
      return;

    } else if (line.startsWith("stop")) {
      doCompletedStages();
      return;

    } else if (line.startsWith("done")) {
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
      //int timeSec = line.substring(idx + 1, line.length() - 1).toInt();
      int timeSec = line.substring(idx + 1, line.length()).toInt();


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
      doCompletedStages();
      return;
    }

    // Read target temperature to PID setpoint
    Serial.print("targetTemp: ");
    Serial.println(pStage->temperature());

    Serial.print("Current temp is (C): ");
    Serial.println(Input);

    Setpoint = pStage->temperature();

    if (Input > Setpoint) {

      setMyPidTuningParameters();

      bool worked = myPID.Compute();
      Serial.print("Compute worked: ");
      Serial.println(worked);
      Serial.print("Output is: ");
      Serial.println(Output);

      //analogWrite(13, Output);
    } else {
      Serial.println("Output is: 0000 ");
      Output = 0;
      //analogWrite(13, 0);
    }
    logTemperatures(pStage->temperature(), Input, Output);
    analogWrite(13, Output);
  }

  // Report to UI the current temprature
  float curTemp = getTemperature(imppThermometer);
  Serial.print("curTemp: ");
  Serial.println(curTemp);
}


void doCompletedStages(void) {
  state = IMP_READY;
  stagesManager.resetStages();
  // Signal the UI with "finished" that stages are complete
  Serial.println("finished");
  analogWrite(13, 0);
}

void logTemperatures(float target, float current, float Output) {
  contorlMessage(String("step::" + String(target) + "," + current + "," + Output));
  
}

void contorlMessage(String message) {
  Serial.println(String("impp_ctrl::" + String(millis()) + "::" + message));
}

/*
   This function assumes that there is only one OnwWire device.
   It will pick the first one it finds
*/
void setOneWireDevices(void) {
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];

  while (oneWire.search(addr)) {
    for ( i = 0; i < 8; i++) {
      byte addrVal;
      if (addr[i] < 16) {
        addrVal = 0;
      }
      Serial.print(addr[i], HEX);
      if (i <= 7) {
        addrVal = addr[i];
      }
      imppThermometer[i] = addrVal;
    }
    if ( OneWire::crc8( addr, 7) != addr[7]) {
      // TODO(asaf): print is not enough. Consider some led indication
      Serial.print("CRC is not valid!\n");
      return;
    }
  }
  oneWire.reset_search();
  return;
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
  if (gap < 4)
  { //we're close to setpoint, use conservative tuning parameters
    myPID.SetTunings(consKp, consKi, consKd);
  }
  else
  {
    //we're far from setpoint, use aggressive tuning parameters
    myPID.SetTunings(aggKp, aggKi, aggKd);
  }
}

