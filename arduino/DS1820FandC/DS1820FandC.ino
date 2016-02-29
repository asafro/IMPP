#include "PID_v1.h"
#include "OneWire.h"
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

// http://www.pjrc.com/teensy/td_libs_OneWire.html
// The DallasTemperature library can do all this work for you!
// http://milesburton.com/Dallas_Temperature_Control_Library

OneWire  ds(10);  // on pin 10 (need 4.7K resistor b/w data & ground)
const int ledPin = 13;       // pin that the LED is attached to
const int threshold = 21;   // arbitrary threshold level
const int BAD_READING = -1;

// Define Variables for PID
double  Input, Output;
double Setpoint = 28.0;

// Define control flags
bool isRunning = false;
bool stagesLoaded = false;

//Debug control
bool DEBUG = true;

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
  Serial.begin(19200);
  myPID.SetMode(AUTOMATIC);
}



/*
   GetCelsius: Assumes there is only one parasite device connected (addr:28 FF 9C 29 0 16 1 25)
               and tells it to get temperature reading and return the value in integer celsius
*/
double getCelsius(void) {
  int16_t res;
  byte i;
  byte data[12];
  byte addr[8];



  Serial.println("-1-");
  while ( !ds.search(addr)) {
    Serial.print("No more addresses.\n");
    ds.reset_search();
    delay(250);
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
    Serial.print("CRC is not valid!\n");
    return BAD_READING;
  }

  // The DallasTemperature library can do all this work for you!
  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  ds.reset();
  ds.select(addr);
  ds.write(0xBE);         // Read Scratchpad
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }

  int16_t raw = (data[1] << 8) | data[0];
  // Assumoing our chip is XXXb we...
  byte cfg = (data[4] & 0x60);
  // at lower res, the low bits are undefined, so let's zero them
  if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
  else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
  else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
  //// default is 12 bit resolution, 750 ms conversion time

  res = (float)raw / 16.0;
  return double(res);
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
      return;
    }

    //(TODO asaf: add work here)



  }
}



