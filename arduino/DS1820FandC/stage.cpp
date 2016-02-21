#include "stage.h"

Stage::Stage() {
  _cmd = NOOP;
  _temperature_c = 0;
  _duration_sec = 0;
}

Stage::Stage(cmdType cmd, long temperature, unsigned long duration) {
  _cmd = cmd;
  _temperature_c = temperature;
  _duration_sec = duration;
}

cmdType Stage::cmd() {
  return _cmd;
}

long Stage::temprature() {
  return _temperature_c;
}

unsigned long Stage::duration() {
  return _duration_sec;
}

