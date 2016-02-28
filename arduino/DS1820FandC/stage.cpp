#include "stage.h"
#include "OneWire.h"


Stage::Stage() {
  resetStage();
}

Stage::Stage(long temperature, unsigned long durationSec) {
  _temperature_c = temperature;
  _durationMsec = durationSec * 1000;
  _set = true;
  _done = false;
}

void Stage::resetStage() {
  _temperature_c = 0;
  _durationMsec = 0;
  _startTimeMsec = 0;
  _done = false;
  _set = false;
}

long Stage::temperature() {
  return _temperature_c;
}

unsigned long Stage::durationMsec() {
  return _durationMsec;
}

void Stage::startStage() {
  _startTimeMsec = millis();

}

bool Stage::isSet() {
  return _set;
}

void Stage::finishStage() {
  _done = true;
}

bool Stage::isDone() {
  unsigned long delta = millis() - _startTimeMsec;
  if ( delta > _durationMsec) {
    _done = true;
  }
  Serial.print("delta: ");
  Serial.println(delta);
  Serial.print("done: ");
  Serial.println(_done);
  return _done;
}

void Stage::printStage() {
  Serial.print("Temprature (c): ");
  Serial.println(_temperature_c);
  Serial.print("Duration (sec): ");
  Serial.println(_durationMsec / 1000);
  Serial.print("Done: ");
  Serial.println(_done);
  Serial.print("Set: ");
  Serial.println(_set);

}


