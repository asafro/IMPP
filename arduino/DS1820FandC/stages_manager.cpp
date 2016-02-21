#include "stages_manager.h"


StagesManager::StagesManager() {}

void StagesManager::addStage(Stage s) {
  if (_num_stages >= MAX_STAGES) {
    // (TODO asaf): probably want to crash or something like that...
    return;
  }
  _stages[_num_stages] = s;
  _num_stages++;
}

bool StagesManager::hasMoreStages() {
  return _cur_stage < _num_stages;
}

Stage& StagesManager::getNextStage() {
  unsigned int ret_stage_idx = _cur_stage;
  _cur_stage++;
  return _stages[ret_stage_idx];
}
/*
#ifndef Stages_manager_h
#define Stages_manager_h

//#include "Arduino.h"
#include "stage.h"

const unsigned int MAX_STAGES = 1000;

class StagesManager
{
  public:
    StagesManager();
    void addStage(Stage& s);
    bool hasMoreStaegs();
    Stage& getNextStage();
    
  private:
    Stage _stages[MAX_STAGES];
    unsigned int cur_stage = 0;
    unsigned int num_stages = 0;
};

#endif
 */
