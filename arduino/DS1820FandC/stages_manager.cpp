#include "stages_manager.h"
#include "OneWire.h"

StagesManager::StagesManager() {}

void StagesManager::addStage(Stage s) {
  if (_numStages >= MAX_STAGES) {
    // (TODO asaf): probably want to crash or something like that...
    return;
  }
  _stages[_numStages] = s;
  _numStages++;
}

Stage* StagesManager::getStage() {
  // If this is the first stage
  if (_curStage == -1) {
    Serial.println("first stage");
    if (_numStages > 0) {
      _curStage++;
      _stages[_curStage].startStage();
      return &_stages[_curStage];
    }
  } else if (_stages[_curStage].tryFinishAndGetIfDone()) {
    Serial.println("stage done");
    // If a stage was finished try to move to the next stage
    if (_curStage < _numStages - 1) {
      _curStage++;
      _stages[_curStage].startStage();
      return &_stages[_curStage];
    }
  } else {
    Serial.println("still working");
    // If the current stage is a real stage return it.
    if (_stages[_curStage].isSet()) {
      return &_stages[_curStage];
    }
  }
  // All stages are done or no stages.
  return NULL;
}

void StagesManager::addStage(int temperatureC, int timeMs) {
  Stage s = Stage(temperatureC, timeMs);
  addStage(s);
}

void StagesManager::resetStages() {
  //doneLoadingStages = false;
  for (int i = 0 ; i < MAX_STAGES ; i++) {
    _stages[i].resetStage();
  }

  _numStages = 0;
  _curStage = -1;
}

void StagesManager::printStages() {
  Serial.println("Stages:");
  for (int i = 0 ; i < MAX_STAGES ; i++) {
    if (_stages[i].isSet()) {
      Serial.print("Stage [");
      Serial.print(i);
      Serial.println("] :");
      _stages[i].printStage();
    }
  }
}

